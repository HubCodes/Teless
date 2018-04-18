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
	STRING
};

class Node {
public:
	explicit Node();
	Node(const Node& other);
	Node& operator=(const Node& other);
	virtual ~Node();

	// ����� ����
	NodeKind kind;

	// ���� ����, ���� ����
	struct {
		Node* prev;
		Node* next;
	} list;

	// �ܺ� ȯ�� �����
	struct {
		Node* environment;
	} env;

	// �Լ�/���� ����
	struct {
		bool isLambda;
		std::string ident;	// ���� isLambda�� ���̶��, �� �ʵ�� ���ǹ���
		std::list<std::string> arguments;
		Node* body;			
	} func;

	// �Լ� ȣ��
	struct {
		std::string ident;
		Node* where;		// �Լ� ���ǰ� ��� �ִ� ���
		Node* arguments;
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
		Node* initExpr;
	} valDef;

	// ���ͷ�(����, ��)
	struct {
		bool isNumber;
		double number;
		bool truth;			// true or false literal
	} literal;

	// ���ڿ�
	struct {
		std::string str;
	} string;
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