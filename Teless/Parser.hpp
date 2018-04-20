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

	// ����� ����
	NodeKind kind;

	// �ܺ� ȯ�� �����
	struct {
		Node* environment;
	} env;

	// �Լ�/���� ����
	struct {
		bool isLambda;
		std::string ident;	// ���� isLambda�� ���̶��, �� �ʵ�� ���ǹ���
		std::list<std::string> arguments;
		std::list<Node*> body;
	} func;

	// �Լ� ȣ��
	struct {
		std::string ident;
		Node* where;		// �Լ� ���ǰ� ��� �ִ� ���
		std::list<Node*> argus;
	} funcCall;

	// ������
	struct {
		Node* cond;
		Node* then;
		Node* elif;
		Node* els;
	} ifStatement;

	// �ݺ���
	struct {
		Node* iter;
		Node* body;
	} forStatement;

	// �ĺ��� ����
	struct {
		std::string ident;
		Node* expr;
	} valDef;

	// ���ͷ�(����, ��)
	struct {
		bool isNumber;
		double number;
		bool truth;			// true or false ���ͷ�
	} literal;

	// ���ڿ�
	struct {
		std::string str;
	} string;

	// ���� ����
	struct {
		std::string ident;
		Node* expr;			// ���� lazy evaluation�� ����..
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