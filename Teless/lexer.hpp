#pragma once

#include "Teless.hpp"

#include <string>
#include <vector>

class TokenStream {
public:
	explicit TokenStream();
	TokenStream(const TokenStream& other);
	TokenStream& operator=(const TokenStream& other);
	virtual ~TokenStream();

	void push(Token* tok);
	Token* get();
	Token* peek();
	void unget();
	bool eof();
private:
	std::vector<Token*> stream;
	int nowPtr;
};

class Lexer {
public:
	explicit Lexer();
	Lexer(const Lexer& other) = delete;
	Lexer& operator=(const Lexer& other) = delete;
	virtual ~Lexer();
	TokenStream lexicalAnalyze(const std::string source, const std::string fileName, Location location);
};