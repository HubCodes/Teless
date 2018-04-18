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

static Node* getIdent(TokenStream& stream);
static Node* getKeyword(TokenStream& stream, KeywordKind kind);
static Node* getNumber(TokenStream& stream);
static Node* getString(TokenStream& stream);
static Node* parse(TokenStream& stream);

Node* Parser::parse(TokenStream& stream) {
	return parse(stream);
}

// �������� : ��ȣ�� �����Ѵ�.
static Node* parse(TokenStream& stream) {
	// �Ľ��ϴ� ���ȿ��� ���� �ݵ�� () �������� Ȯ�� �����ϴ�. 
	// ���� �ڵ�� ()�� �����̹Ƿ�, �Ľ� �� ��Ÿ�� ȯ���� ���·� ����ȴ�
	Node* start = nullptr;
	Token tok = stream.get();

	while (!stream.eof())
	{
		// ���� ��ȣ���
		if (tok.tokenKind == TokenKind::OPEN_PAREN)
		{
			tok = stream.peek();
			switch (tok.tokenKind)
			{
			case TokenKind::IDENT:
				start = getIdent(stream);
				break;
			case TokenKind::KEYWORD:

				break;
			case TokenKind::NUMBER:

				break;
			case TokenKind::OPEN_PAREN:

				break;
			case TokenKind::STRING:

				break;
			}
		}
	}
	if (start == nullptr)
		die("Cannot parse code.");

	return start;
}

// getIdent : ����Ʈ�� ��尡 �ĺ����϶� ȣ��� (�̴� �ĺ��ڰ� ������ �Լ����� �ǹ�)
// �������� : stream.get ���� ȣ�� �� ��ȯ�Ǵ� ��� ��ū�� �ĺ��ڿ��� ��
static Node* getIdent(TokenStream& stream) {
	Token head = stream.get();
	Node* result = new Node;

	// ������ ���ӽ����̽��� �ĺ��ڰ� �����Ѵٸ� 
	// �� ����Ʈ�� �Լ� ȣ�� �̿��� ��찡 ����.
	if (nowScope->existKey(head.tokenStr))
	{
		result->kind = NodeKind::FUNCALL;
		result->funcCall.ident = head.tokenStr;
		result->funcCall.where = nowScope->getFromKey(head.tokenStr);
		auto argus = parse(stream);
		if (argus->kind == NodeKind::FUNCALL || 
			argus->kind == NodeKind::LITERAL || 
			argus->kind == NodeKind::STRING)
		{
			result->funcCall.arguments = argus;
		}
		else
		{
			ErrorManager::get().pushNewErrorContext();
			ErrorManager::get().pushError("Invalid arguments in line " + std::to_string(head.location.line));
		}
	}
	else
	{
		ErrorManager::get().pushNewErrorContext();
		ErrorManager::get().pushError("Undefined or unreachable identifier " + head.tokenStr);
		result = nullptr;
	}
	return result;
}

static Node* getKeyword(TokenStream& stream, KeywordKind kind) {

}

void initParser() {
	nowScope = new Env;
	nowScope->setParent(nullptr);
}
