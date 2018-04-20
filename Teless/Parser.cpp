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

// �������� : ��ȣ�� �����Ѵ�.
static Node* parse(TokenStream& stream) {
	// �Ľ��ϴ� ���ȿ��� ���� �ݵ�� () �������� Ȯ�� �����ϴ�. 
	// ���� �ڵ�� ()�� �����̹Ƿ�, �Ľ� �� ��Ÿ�� ȯ���� ���·� ����ȴ�
	Node* result;
	Token* tok = stream.peek();

	// �Ʒ� switch�� peek�� ��ū�� ���ؼ� �����Ѵ�.
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

// getIdent : �ĺ����϶� ȣ���
// �������� : stream.get ���� ȣ�� �� ��ȯ�Ǵ� ��� ��ū�� �ĺ��ڿ��� ��
static Node* getIdent(TokenStream& stream) {
	// �Ƹ��� �ĺ���
	Token* head = stream.get();
	Node* result = new Node;

	// ������ ���ӽ����̽��� �ĺ��ڰ� �����Ѵٸ� 
	if (nowScope->existKey(head->tokenStr))
	{
		// �Լ� �����̰ų� �ĺ���� ����
		auto reference = nowScope->getFromKey(head->tokenStr);
		result->kind = reference->kind;

		// �Լ� ȣ���� ���
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

		// ���� ������ ���
		else
		{
			result->valRef.ident = head->tokenStr;
			result->valRef.expr = reference;	// lazy eval�� ����..
		}
	}
	else
	{
		goto error;
	}

	return result;

	// �ĺ��ڰ� ���ٸ� ����
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

// getFuncdef : �Լ� ������ �� ȣ���
Node* getFuncdef(TokenStream& stream) {
	// �Ƹ��� func�Ǵ� lambda
	Token* hasName = stream.get();
	Node* result = new Node;
	result->kind = NodeKind::FUNCDEF;

	if (hasName->tokenStr == "func")
	{
		auto ident = stream.get();	// �Լ��� �̸�.
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
