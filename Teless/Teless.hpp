#pragma once

struct Location {
	int line;
};

enum class KeywordKind {
	NO_KEYWORD,
	IF,
	ELIF,
	ELSE,
	RETURN,
	VAL,
	FUNC,
	TRUE,
	FALSE
};

enum class TokenKind {
	OPEN_PAREN,
	CLOSE_PAREN,
	OPEN_DATA,
	CLOSE_DATA,
	KEYWORD,
	IDENT,
	NUMBER,
	STRING
};

struct Token {
	Location location;
	TokenKind tokenKind;
	KeywordKind keywordKind;
	std::string tokenStr;
	double number;
};
