#pragma once

#include <list>

#include "lexer.hpp"
#include "Env.hpp"

class Node;

enum class NodeKind {
	FUNCDEF,
	FUNCALL,
	IF,
	ELIF,
	ELSE,
	VAL,
	LITERAL,
	STRING,
	VALREF,
	DATALIST,
	RETURN,
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
	Env* env;

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
		std::list<Node*> then;
		Node* elif;
		Node* els;
	} ifStatement;

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

	// 식별 상수 참조
	struct {
		std::string ident;
		Node* expr;			// 함수 객체 참조. + 향후 lazy evaluation을 위해.
	} valRef;

	// 데이터(로서의) 리스트
	struct {
		std::list<Node*> datas;
	} datalist;
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