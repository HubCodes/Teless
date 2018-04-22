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

	// ����� ����
	NodeKind kind;

	// �ܺ� ȯ�� �����
	Env* env;

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
		std::list<Node*> then;
		Node* elif;
		Node* els;
	} ifStatement;

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

	// �ĺ� ��� ����
	struct {
		std::string ident;
		Node* expr;			// �Լ� ��ü ����. + ���� lazy evaluation�� ����.
	} valRef;

	// ������(�μ���) ����Ʈ
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