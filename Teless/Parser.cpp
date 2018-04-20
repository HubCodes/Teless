#include "Parser.hpp"
#include "error.hpp"
#include "Env.hpp"

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

static Node* errNode;
static Node* nil;

static Node* getIdent(TokenStream& stream);
static Node* getKeyword(TokenStream& stream, KeywordKind kind);
static Node* getNumber(TokenStream& stream);
static Node* getString(TokenStream& stream);
static Node* getFuncdef(TokenStream& stream);
static Node* getIf(TokenStream& stream);
static Node* getFor(TokenStream& stream);
static Node* getValdef(TokenStream& stream);
static Node* parse(TokenStream& stream);

Node* Parser::parse(TokenStream& stream) {
	return parse(stream);
}

// 전제조건 : 괄호로 시작한다.
static Node* parse(TokenStream& stream) {
	// 파싱하는 동안에는 식이 반드시 () 형태임을 확신 가능하다. 
	// 실제 코드는 ()의 집합이므로, 파싱 뒤 런타임 환경의 형태로 저장된다
	Node* result;
	Token* tok = stream.peek();

	// 아래 switch는 peek된 토큰에 대해서 동작한다.
	switch (tok->tokenKind)
	{
	case TokenKind::OPEN_PAREN:
		tok = stream.get();
		result = parse(stream);
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

	// 현재의 네임스페이스에 식별자가 존재한다면 
	if (nowScope->existKey(head->tokenStr))
	{
		// 함수 정의이거나 식별상수 정의
		auto reference = nowScope->getFromKey(head->tokenStr);
		result->kind = reference->kind;

		// 함수 호출일 경우
		if (result->kind == NodeKind::FUNCALL)
		{
			result->funcCall.ident = head->tokenStr;
			result->funcCall.where = reference;
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
		}

		// 변수 지정일 경우
		else
		{
			result->valRef.ident = head->tokenStr;
			result->valRef.expr = reference;	// lazy eval을 위해..
		}
	}
	else
	{
		goto error;
	}

	return result;

	// 식별자가 없다면 에러
error:
	errorOccur({ "Undefined or unreachable identifier " + head->tokenStr, head->location });
		return errNode;
}

static Node* getKeyword(TokenStream& stream, KeywordKind kind) {
	switch (kind)
	{
	case KeywordKind::FUNC:
		return getFuncdef(stream);
	case KeywordKind::IF:
		return getIf(stream);
	case KeywordKind::FOR:
		return getFor(stream);
	case KeywordKind::VAL:
		return getValdef(stream);
	}
}

// getFuncdef : 함수 정의일 때 호출됨
Node* getFuncdef(TokenStream& stream) {
	// 아마도 func또는 lambda
	Token* hasName = stream.get();
	Node* result = new Node;
	result->kind = NodeKind::FUNCDEF;

	if (hasName->tokenStr == "func")
	{
		auto ident = stream.get();	// 함수의 이름.
		result->func.isLambda = false;
		result->func.ident = ident->tokenStr;
	}
	else
	{
		result->func.isLambda = true;
	}
	
	auto open = stream.get();
	if (open->tokenKind != TokenKind::OPEN_PAREN)
	{
		errorOccur({ "Expected ( but cannot found.", open->location });
	}

	Token* close = stream.get();
	std::list<std::string> arguments;
	while (close->tokenKind != TokenKind::CLOSE_PAREN)
	{
		arguments.push_back(close->tokenStr);
		close = stream.get();
	}

	Token* lookahead = stream.peek();
	while (lookahead->tokenKind == TokenKind::OPEN_PAREN)
	{
		result->func.body.push_back(parse(stream));
		lookahead = stream.peek();
	}
	return result;
}

Node* getIf(TokenStream& stream) {
	return nullptr;
}

Node* getFor(TokenStream& stream) {
	return nullptr;
}

Node* getValdef(TokenStream& stream) {
	return nullptr;
}

void initParser() {
	nowScope = new Env;
	nowScope->setParent(nullptr);
	errNode = new Node;
	errNode->kind = NodeKind::ERROR;
	nil = new Node;
	nil->kind = NodeKind::NIL;
}
