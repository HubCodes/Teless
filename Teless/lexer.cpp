#include "error.hpp"
#include "lexer.hpp"

#include <cctype>
#include <map>
#include <set>
#include <sstream>


// TokenStream class
TokenStream::TokenStream() : stream(), nowPtr(0) {

}

TokenStream::TokenStream(const TokenStream& other) : stream(other.stream), nowPtr(other.nowPtr) {

}

TokenStream& TokenStream::operator=(const TokenStream& other) {
	this->stream = other.stream;
	this->nowPtr = other.nowPtr;
	return *this;
}

TokenStream::~TokenStream() {

}

void TokenStream::push(Token* tok) {
	stream.push_back(tok);
	nowPtr++;
}

Token* TokenStream::get() {
	if (nowPtr >= stream.size()) return stream.back();
	auto&& result = peek();
	nowPtr++;
	return result;
}

Token* TokenStream::peek() {
	return stream[nowPtr];
}

void TokenStream::unget() {
	if (nowPtr <= 0) return;
	nowPtr--;
}

bool TokenStream::eof() {
	return nowPtr >= stream.size();
}

// Lexer class

static std::map<std::string, KeywordKind> keywordTable = {
	{ "func",	KeywordKind::FUNC },
	{ "lambda",	KeywordKind::FUNC },
	{ "if",		KeywordKind::IF },
	{ "elif",	KeywordKind::ELIF },
	{ "else",	KeywordKind::ELSE },
	{ "val",	KeywordKind::VAL },
	{ "true",	KeywordKind::TRUE },
	{ "false",	KeywordKind::FALSE },
	{ "return",	KeywordKind::RETURN }
};

static bool isIdentOrKeyword(int c);
static Token* getIdentOrKeyword(std::istringstream& code, Location location);
static Token* getNumber(std::istringstream& code, Location location);
static Token* getString(std::istringstream& code, Location location);

Lexer::Lexer() {

}

Lexer::~Lexer() {

}

TokenStream Lexer::lexicalAnalyze(const std::string source, const std::string fileName, Location location) {
	std::istringstream stream(source);
	TokenStream result;
	int c;

	while (!stream.eof())
	{
		c = stream.peek();

		// 여는 괄호일 경우
		if (c == '(')
		{
			stream.get();
			result.push(new Token{
				location,
				TokenKind::OPEN_PAREN,
				KeywordKind::NO_KEYWORD,
				"(",
				0.0
			});
		}

		// 닫는 괄호일 경우
		else if (c == ')')
		{
			stream.get();
			result.push(new Token{
				location,
				TokenKind::CLOSE_PAREN,
				KeywordKind::NO_KEYWORD,
				")",
				0.0
			});
		}

		// 데이터 여는 괄호일 경우
		else if (c == '[')
		{
			stream.get();
			result.push(new Token{
				location,
				TokenKind::OPEN_DATA,
				KeywordKind::NO_KEYWORD,
				"[",
				0.0
			});
		}

		// 데이터 닫는 괄호일 경우
		else if (c == ']')
		{
			stream.get();
			result.push(new Token{
				location,
				TokenKind::CLOSE_DATA,
				KeywordKind::NO_KEYWORD,
				"]",
				0.0
			});
		}

		// 공백은 건너뛴다.
		else if (std::isspace(c))
		{
			stream.get();
		}

		// 큰따옴표 (문자열의 시작) 인 경우
		else if (c == '\"')
		{
			result.push(getString(stream, location));
		}

		// 식별자일 경우
		else if (isIdentOrKeyword(c))
		{
			result.push(getIdentOrKeyword(stream, location));
		}

		// 숫자인 경우
		else if (std::isdigit(c))
		{
			result.push(getNumber(stream, location));
		}

		// 그 외 (에러)
		else
		{
			die("Unexpected character : " + c);
		}
	}
}

static bool isIdentOrKeyword(int c) {
	static std::set<char> chars = {
		'!', '$', '%',
		'&', '*', '+',
		'-', '/', '<',
		'=', '>', '?',
		'@'
	};
	return std::isalpha(c) || std::isdigit(c) || (chars.find(c) != chars.end());
}

static Token* getIdentOrKeyword(std::istringstream& code, Location location) {
	int c;
	std::string ident = "";
	do
	{
		c = code.get();
		ident += c;
	}
	while (!std::isspace(c) && isIdentOrKeyword(c) && !code.eof());

	if (code.eof())
	{
		die("Code is not ended but Can't reach more codes...");
	}
	Token* result = new Token;
	result->location = location;
	result->tokenStr = ident;
	result->number = 0.0;

	// 만약 키워드라면
	if (keywordTable.find(ident) != keywordTable.end())
	{
		result->tokenKind = TokenKind::KEYWORD;
		result->keywordKind = keywordTable[ident];
		return result;
	}
	result->tokenKind = TokenKind::IDENT;
	result->keywordKind = KeywordKind::NO_KEYWORD;
	return result;
}

static Token* getNumber(std::istringstream& code, Location location) {
	int c;
	std::string number = "";
	bool noMoreDot = false;
	do
	{
		c = code.get();
		if (c == '.')
		{
			if (!noMoreDot)
			{
				noMoreDot = true;
			}
			else
			{
				errorOccur({ 
					"Number has too many dots.",
					location 
				});
				break;
			}			
		}
		number += c;
	}
	while (!std::isspace(c) && (std::isdigit(c)) && !code.eof());
	double realNumber = std::stod(number);
	return new Token{
		location,
		TokenKind::NUMBER,
		KeywordKind::NO_KEYWORD,
		number,
		realNumber
	};
}

static Token* getString(std::istringstream& code, Location location) {
	static std::map<int, int> escapeSequenceSet = {
		{ '\'', '\'' },
		{ '\"', '\"' },
		{ '\?', '\?' },
		{ '\\', '\\' },
		{ '0',	'\0' },
		{ 'a',	'\a' },
		{ 'b',	'\b' },
		{ 'n',	'\n' },
		{ 'r',	'\r' },
		{ 't',	'\t' }
	};
	code.get();	// 문자열 시작
	int c;
	std::string resultString = "";
	while (true)
	{
		c = code.get();

		// 이스케이프 시퀀스
		if (c == '\\') 
		{
			int esc = code.get();
			if (escapeSequenceSet.find(esc) != escapeSequenceSet.end())
			{
				resultString += escapeSequenceSet[esc];
			}
		}

		// 그 외
		else
		{
			if (c == '\"')
				break;
			resultString += c;
		}
	}
	return new Token{
		location,
		TokenKind::STRING,
		KeywordKind::NO_KEYWORD,
		resultString,
		std::stod(resultString)
	};
}