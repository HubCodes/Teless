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

// 전제조건 : 괄호로 시작한다.
static Node* parse(TokenStream& stream) {
	// 파싱하는 동안에는 식이 반드시 () 형태임을 확신 가능하다. 
	// 실제 코드는 ()의 집합이므로, 파싱 뒤 런타임 환경의 형태로 저장된다
	Node* start = nullptr;
	Token tok = stream.get();

	while (!stream.eof())
	{
		// 여는 괄호라면
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

// getIdent : 리스트의 헤드가 식별자일때 호출됨 (이는 식별자가 무조건 함수임을 의미)
// 전제조건 : stream.get 최초 호출 시 반환되는 결과 토큰이 식별자여야 함
static Node* getIdent(TokenStream& stream) {
	Token head = stream.get();
	Node* result = new Node;

	// 현재의 네임스페이스에 식별자가 존재한다면 
	// 이 리스트는 함수 호출 이외의 경우가 없다.
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
