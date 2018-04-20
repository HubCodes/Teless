#pragma once

#include <list>

#include "lexer.hpp"

class Node;

enum class NodeKind {
	ENV,
	FUNCDEF,
	FUNCALL,
	IF,
	FOR,
	VAL,
	LITERAL,
	STRING,
	VALREF,
	NIL,
	ERROR
};

class Node {
public:
	explicit Node();
	Node(const Node& other);
	Node& operator=(const Node& other);
	virtual ~Node();

	// 노드의 종류
	NodeKind kind;

	// 외부 환경 저장용
	struct {
		Node* environment;
	} env;

	// 함수/람다 정의
	struct {
		bool isLambda;
		std::string ident;	// 만약 isLambda가 참이라면, 이 필드는 무의미함
		std::list<std::string> arguments;
		std::list<Node*> body;
	} func;

	// 함수 호출
	struct {
		std::string ident;
		Node* where;		// 함수 정의가 들어 있는 노드
		std::list<Node*> argus;
	} funcCall;

	// 조건절
	struct {
		Node* cond;
		Node* then;
		Node* elif;
		Node* els;
	} ifStatement;

	// 반복절
	struct {
		Node* iter;
		Node* body;
	} forStatement;

	// 식별자 정의
	struct {
		std::string ident;
		Node* expr;
	} valDef;

	// 리터럴(숫자, 불)
	struct {
		bool isNumber;
		double number;
		bool truth;			// true or false 리터럴
	} literal;

	// 문자열
	struct {
		std::string str;
	} string;

	// 변수 참조
	struct {
		std::string ident;
		Node* expr;			// 향후 lazy evaluation을 위해..
	} valRef;
};

class Parser {
public:
	explicit Parser();
	Parser(const Parser& other) = delete;
	Parser& operator=(const Parser& other) = delete;
	virtual ~Parser();

	Node* parse(TokenStream& stream);
};

void initParser();