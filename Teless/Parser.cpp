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
	// �Ľ��ϴ� ���ȿ��� ���� �ݵ�� () �������� Ȯ�� �����ϴ�. 
	// ���� �ڵ�� ()�� �����̹Ƿ�, �Ľ� �� ��Ÿ�� ȯ���� ���·� ����ȴ�
	Node* result;
	Token* tok = stream.peek();

	// �Ʒ� switch�� peek�� ��ū�� ���ؼ� �����Ѵ�.
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

// getIdent : �ĺ����϶� ȣ���
// �������� : stream.get ���� ȣ�� �� ��ȯ�Ǵ� ��� ��ū�� �ĺ��ڿ��� ��
static Node* getIdent(TokenStream& stream) {
	// �Ƹ��� �ĺ���
	Token* head = stream.get();
	Node* result = new Node;
	result->env = nowScope;

	// ������ �������� �ĺ��ڰ� �����Ѵٸ� 
	// �Լ� ȣ���̰ų� �ĺ� ��� ����
	// ��, �Լ� ȣ���̷��� ���� ��ū�� ����Ʈ�� ù ���ҿ��߸� �Ѵ�.
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

		// �Լ� ȣ���� ���
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

		// ���� �����̰ų� ��ü�μ��� �Լ��� ��
		else
		{
			result->valRef.ident = head->tokenStr;
			result->valRef.expr = ref;	// ��ü�μ��� �Լ��� ����. Ȥ�� ���� lazy eval�� ����..
		}
	}
	else
	{
		goto error;
	}

	return result;

	// �ĺ��ڰ� ���ٸ� ����
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

// getFuncdef : �Լ� ������ �� ȣ���
Node* getFuncdef(TokenStream& stream) {
	// �Ƹ��� func�Ǵ� lambda
	Token* hasName = stream.get();
	Node* result = new Node;
	result->kind = NodeKind::FUNCDEF;
	result->env = nowScope;

	// �Լ�, ���� ������
	if (hasName->tokenStr == "func")
	{
		auto ident = stream.get();	// �Լ��� �̸�.
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

	// ���ں� �ĺ��� �޴� �κ�
	Token* close = stream.get();
	std::list<std::string> arguments;
	while (close->tokenKind != TokenKind::CLOSE_PAREN)
	{
		arguments.push_back(close->tokenStr);
		close = stream.get();
	}

	// scope ����
	auto funcScope = new Env;
	// ���� �������� �Լ� ���� �߰�
	nowScope->pushIdent(result->func.ident, result);
	// �Լ� �������� ���� �������� �ڽ����� �߰�
	funcScope->setParent(nowScope);
	nowScope->pushChild(funcScope);

	// ���� ������ = �Լ� ������ (�Լ� ������ ���� ���ο� ������)
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

	// �ݴ� ��ȣ�� �ǳʶڴ�.
	stream.get();

	return result;
} 

// ��������: kind �μ��� ������ �� ���� ������ ���� ���´�.
Node* getIf(TokenStream& stream, KeywordKind kind) {
	Node* ifNode = new Node;
	stream.get(); // �Ƹ��� if, elif, else

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

	// then �κ� �Ľ�
	while (stream.peek()->tokenKind == TokenKind::OPEN_PAREN)
	{
		ifNode->ifStatement.then.push_back(parse(stream));
	}

	Token* lookahead = stream.peek();

	// if���� ���ӵ� �� ����. ���� ���ӽ�Ű����, �ٸ� ��ȣ �ȿ��� �� ��
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
	stream.get(); // �Ƹ��� val
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
	stream.get(); // �Ƹ��� [ 
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
