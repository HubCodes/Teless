#include "Parser.hpp"
#include "error.hpp"
#include "Env.hpp"

#define SCOPE_PUSH(scope)	scopePush(scope);
#define SCOPE_POP			scopePop();

// Node class
Node::Node() {

}

Node::Node(const Node& other) : kind(other.kind) {

}

Node& Node::operator=(const Node& other) {
	return *this;
}

Node::~Node() {

}


// Parser class
Parser::Parser() {

}

Parser::~Parser() {

}

// scope
static Env* nowScope;
static Env* prevScope;

static Node* errNode;
static Node* nil;

static Node* getIdent(TokenStream& stream);
static Node* getKeyword(TokenStream& stream, KeywordKind kind);
static Node* getNumber(TokenStream& stream);
static Node* getString(TokenStream& stream);
static Node* getFuncdef(TokenStream& stream);
static Node* getIf(TokenStream& stream, KeywordKind kind);
static Node* getValdef(TokenStream& stream);
static Node* getDataList(TokenStream& stream);
static Node* parse(TokenStream& stream);

static void scopePush(Env* newScope);
static void scopePop();
static std::string lambdaNameGen();

Node* Parser::parse(TokenStream& stream) {
	return parse(stream);
}

static void scopePush(Env* newScope) {
	prevScope = nowScope;
	nowScope = newScope;
}

static void scopePop() {
	if (prevScope == nullptr) return;
	nowScope = prevScope;
}

static std::string lambdaNameGen() {
	static int count = 0;
	count++;
	return "<lambda" + std::to_string(count) + '>';
}

static Node* parse(TokenStream& stream) {
	// 파싱하는 동안에는 식이 반드시 () 형태임을 확신 가능하다. 
	// 실제 코드는 ()의 집합이므로, 파싱 뒤 런타임 환경의 형태로 저장된다
	Node* result;
	Token* tok = stream.peek();

	// 아래 switch는 peek된 토큰에 대해서 동작한다.
	switch (tok->tokenKind)
	{
	case TokenKind::OPEN_PAREN:
		stream.get();
		result = parse(stream);
		break;
	case TokenKind::OPEN_DATA:
		stream.get();
		result = getDataList(stream);
		break;
	case TokenKind::IDENT:
		result = getIdent(stream);
		break;
	case TokenKind::KEYWORD:
		result = getKeyword(stream, tok->keywordKind);
		break;
	case TokenKind::NUMBER:
		result = getNumber(stream);
		break;
	case TokenKind::STRING:
		result = getString(stream);
		break;
	case TokenKind::CLOSE_PAREN:
	case TokenKind::CLOSE_DATA:
		result = nil;
		break;
	}
	return result;
}

// getIdent : 식별자일때 호출됨
// 전제조건 : stream.get 최초 호출 시 반환되는 결과 토큰이 식별자여야 함
static Node* getIdent(TokenStream& stream) {
	// 아마도 식별자
	Token* head = stream.get();
	Node* result = new Node;
	result->env = nowScope;

	// 현재의 스코프에 식별자가 존재한다면 
	// 함수 호출이거나 식별 상수 참조
	// 단, 함수 호출이려면 받은 토큰이 리스트의 첫 원소여야만 한다.
	stream.unget();
	stream.unget();
	Token* maybeOpenParen = stream.get();
	stream.get();

	if (nowScope->existKey(head->tokenStr))
	{
		auto ref = nowScope->getFromKey(head->tokenStr);
		result->kind = 
			(ref->kind == NodeKind::FUNCDEF &&
			maybeOpenParen->tokenKind == TokenKind::OPEN_PAREN) ?
				NodeKind::FUNCALL : 
				NodeKind::VALREF;

		// 함수 호출일 경우
		if (result->kind == NodeKind::FUNCALL)
		{
			result->funcCall.ident = head->tokenStr;
			result->funcCall.where = ref;
			std::list<Node*>& args = result->funcCall.argus;
			Node* arg;
			do
			{
				arg = parse(stream);
				if (arg->kind == NodeKind::ERROR)
					goto error;
				args.push_back(arg);
			}
			while (arg->kind != NodeKind::NIL);
			args.pop_back(); // NIL
		}

		// 변수 지정이거나 객체로서의 함수일 때
		else
		{
			result->valRef.ident = head->tokenStr;
			result->valRef.expr = ref;	// 객체로서의 함수를 위함. 혹은 추후 lazy eval을 위해..
		}
	}
	else
	{
		goto error;
	}

	return result;

	// 식별자가 없다면 에러
error:
	errorOccur({ 
		"Undefined or unreachable identifier " + head->tokenStr, 
		head->location 
	});
		return errNode;
}

static Node* getKeyword(TokenStream& stream, KeywordKind kind) {
	static auto getBoolean = []() -> Node* {
		Node* trueNode = new Node;
		trueNode->kind = NodeKind::LITERAL;
		trueNode->env = nowScope;
		trueNode->literal.isNumber = false;
		return trueNode;
	};

	switch (kind)
	{
	case KeywordKind::FUNC:
		return getFuncdef(stream);
	case KeywordKind::IF:
		return getIf(stream, KeywordKind::IF);
	case KeywordKind::VAL:
		return getValdef(stream);
	case KeywordKind::TRUE:
		{
			Node* trueNode = getBoolean();
			trueNode->literal.truth = true;
			return trueNode;
		}
		break;
	case KeywordKind::FALSE:
		{
			Node* falseNode = getBoolean();
			falseNode->literal.truth = false;
			return falseNode;
		}
		break;
	case KeywordKind::RETURN:
		{
			Node* retNode = new Node;
			retNode->kind = NodeKind::RETURN;
			retNode->env = nowScope;
		}
		break;
	default:
		{
			errorOccur({
				stream.peek()->tokenStr + " cannot appear.",
				stream.peek()->location
			});
			return errNode;
		}
	}
}

Node* getNumber(TokenStream& stream) {
	Node* num = new Node;
	num->kind = NodeKind::LITERAL;
	num->env = nowScope;
	num->literal.isNumber = true;
	num->literal.number = stream.get()->number;

	return num;
}

Node* getString(TokenStream& stream) {
	Node* str = new Node;
	str->kind = NodeKind::STRING;
	str->env = nowScope;
	str->string.str = stream.get()->tokenStr;
	return str;
}

// getFuncdef : 함수 정의일 때 호출됨
Node* getFuncdef(TokenStream& stream) {
	// 아마도 func또는 lambda
	Token* hasName = stream.get();
	Node* result = new Node;
	result->kind = NodeKind::FUNCDEF;
	result->env = nowScope;

	// 함수, 람다 구별부
	if (hasName->tokenStr == "func")
	{
		auto ident = stream.get();	// 함수의 이름.
		result->func.isLambda = false;
		result->func.ident = ident->tokenStr;
	}
	else
	{
		result->func.isLambda = true;
		result->func.ident = lambdaNameGen();
	}
	
	auto open = stream.get();
	if (open->tokenKind != TokenKind::OPEN_PAREN)
	{
		errorOccur({ 
			"Expected ( but cannot found.", 
			open->location 
		});
	}

	// 인자별 식별자 받는 부분
	Token* close = stream.get();
	std::list<std::string> arguments;
	while (close->tokenKind != TokenKind::CLOSE_PAREN)
	{
		arguments.push_back(close->tokenStr);
		close = stream.get();
	}

	// scope 관리
	auto funcScope = new Env;
	// 현재 스코프에 함수 정의 추가
	nowScope->pushIdent(result->func.ident, result);
	// 함수 스코프를 현재 스코프의 자식으로 추가
	funcScope->setParent(nowScope);
	nowScope->pushChild(funcScope);

	// 현재 스코프 = 함수 스코프 (함수 본문을 위한 새로운 스코프)
	SCOPE_PUSH(funcScope)
	{
		Token* lookahead = stream.peek();
		while (lookahead->tokenKind == TokenKind::OPEN_PAREN)
		{
			result->func.body.push_back(parse(stream));
			lookahead = stream.peek();
		}
	}
	SCOPE_POP

	// 닫는 괄호를 건너뛴다.
	stream.get();

	return result;
} 

// 전제조건: kind 인수는 무조건 세 가지 종류의 값을 갖는다.
Node* getIf(TokenStream& stream, KeywordKind kind) {
	Node* ifNode = new Node;
	stream.get(); // 아마도 if, elif, else

	switch (kind)
	{
	case KeywordKind::IF:
		ifNode->kind = NodeKind::IF;
		break;
	case KeywordKind::ELIF:
		ifNode->kind = NodeKind::ELIF;
		break;
	case KeywordKind::ELSE:
		ifNode->kind = NodeKind::ELSE;
		break;
	}

	ifNode->env = nowScope;
	if (ifNode->kind != NodeKind::ELSE)
		ifNode->ifStatement.cond = parse(stream);

	// then 부분 파싱
	while (stream.peek()->tokenKind == TokenKind::OPEN_PAREN)
	{
		ifNode->ifStatement.then.push_back(parse(stream));
	}

	Token* lookahead = stream.peek();

	// if문은 연속될 수 없다. 만약 연속시키려면, 다른 괄호 안에서 할 것
	if (lookahead->keywordKind == KeywordKind::IF)
	{
		errorOccur({
			"if statement cannot appear consecutively.",
			lookahead->location
		});
	}
	else if (lookahead->keywordKind == KeywordKind::ELIF)
	{
		if (ifNode->kind == NodeKind::ELSE)
			errorOccur({
				"elif cannot place behind of else.",
				lookahead->location
			});
		else
			ifNode->ifStatement.elif = getIf(stream, KeywordKind::ELIF);
	}
	else if (lookahead->keywordKind == KeywordKind::ELSE)
	{
		if (ifNode->kind == NodeKind::ELSE)
			errorOccur({
				"else cannot place two times.",
				lookahead->location
			});
		else
			ifNode->ifStatement.els = getIf(stream, KeywordKind::ELSE);
	}
	else if (lookahead->tokenKind == TokenKind::CLOSE_PAREN)
	{
		return ifNode;
	}
	else
	{
		errorOccur({
			"Unexpected token " + lookahead->tokenStr,
			lookahead->location
		});
		return errNode;
	}
	return ifNode;
}

Node* getValdef(TokenStream& stream) {
	Node* valdef = new Node;
	stream.get(); // 아마도 val
	valdef->kind = NodeKind::VAL;
	valdef->env = nowScope;
	Token* maybeIdent = stream.get();
	if (maybeIdent->tokenKind != TokenKind::IDENT)
	{
		errorOccur({
			"Expected identifier but " + maybeIdent->tokenStr,
			maybeIdent->location
		});
		return errNode;
	}
	valdef->valDef.ident = maybeIdent->tokenStr;
	valdef->valDef.expr = parse(stream);

	Token* maybeClose = stream.get();
	if (maybeClose->tokenKind != TokenKind::CLOSE_PAREN)
	{
		errorOccur({
			"No more code needed in val definition",
			maybeClose->location
		});
		return errNode;
	}
	return valdef;
}

Node* getDataList(TokenStream& stream) {
	Node* datalist = new Node;
	datalist->kind = NodeKind::DATALIST;
	datalist->env = nowScope;
	stream.get(); // 아마도 [ 
	Node* element = parse(stream);
	while (element->kind != NodeKind::NIL)
	{
		datalist->datalist.datas.push_back(parse(stream));
	}
	return datalist;
}

void initParser() {
	nowScope = new Env;
	nowScope->setParent(nullptr);
	prevScope = nullptr;
	errNode = new Node;
	errNode->kind = NodeKind::ERROR;
	nil = new Node;
	nil->kind = NodeKind::NIL;
}
