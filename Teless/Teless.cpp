#include <ctime>
#include <iostream>
#include <string>

#include "error.hpp"
#include "lexer.hpp"

static const int VERSION_MAJOR = 1;
static const int VERSION_MINOR = 0;

#ifdef __TEST_MODE
void test_sequence() {

}
#endif

class Option;

void parseOption(Option* opt, const int argc, const char** argv);
void repl_loop(Option* opt);
void source_loop(Option* opt);
int count(const std::string& code, char ch);

int main(const int argc, const char** argv) {
	// 옵션 객체 생성으로 실행 인수를 관리
	Option* opt{ new Option };
	parseOption(opt, argc, argv);

	if (opt->get() == Option::REPL)
	{
		repl_loop(opt);
	}
	else
	{
		source_loop(opt);
	}

	delete opt;
	return 0;
}

class Option final {
public:
	enum options {
		REPL,
		SRC
	};
public:
	explicit Option() noexcept;
	Option(const Option& other) = delete;
	Option& operator=(const Option& other) = delete;
	~Option();

	void set(options opt) noexcept;
	options get() const noexcept;
	void setFileName(const std::string& fileName);
private:
	options option;
	std::string fileName;
};

Option::Option() noexcept {

}

Option::~Option() {

}

void Option::set(options opt) noexcept {
	this->option = opt;
}

Option::options Option::get() const noexcept {
	return this->option;
}

void Option::setFileName(const std::string& fileName) {
	this->fileName = fileName;
}

void parseOption(Option* opt, const int argc, const char** argv) {
	// 만약 인수가 하나도 없다면 REPL 모드로 진입 (첫 번째 인수는 프로그램명임을 주의)
	if (argc == 1)
	{
		opt->set(Option::REPL);
	}

	// 인수가 존재한다면
	else
	{
		// 도움말
		if (0 == std::strncmp(argv[1], "-h", 2))
		{
			std::cout << "Usage: teless (option|filepath)"
				"\n"
				"	-h:	Show this help page.\n"
				"	-v: Show version.";
			std::exit(EXIT_SUCCESS);
		}
		
		// 버전을 묻는 인수
		else if (0 == std::strncmp(argv[1], "-v", 2))
		{
			std::cout << VERSION_MAJOR << '.' << VERSION_MINOR << '\n';
			std::exit(EXIT_SUCCESS);
		}

		// 어디에도 해당되지 않는다면 파일 이름이다.
		else
		{
			// 파일 존재 여부를 확인한다.
			std::string fileName = argv[1];
			if (FILE *file = fopen(fileName.c_str(), "r"))
			{
				fclose(file);
				opt->setFileName(fileName);
				opt->set(Option::SRC);
			}
			else
			{
				die("Cannot open file : " + fileName);
			}
		}	
	}
}

void repl_loop(Option* opt) {
	std::time_t now = std::time(nullptr);
	std::string initMessage{
		"Teless interactive shell Version " + std::to_string(VERSION_MAJOR) + '.' + 
		std::to_string(VERSION_MINOR) + '\n' + 
		"Started on " + std::ctime(&now) + '\n'
	};
	std::string shell{ "Teless> " };
	std::cout << initMessage;

	// REPL 루프
	while (true)
	{
		std::cout << shell;
		std::string line;
		std::getline(std::cin, line);
		
		int openParens = count(line, '(');
		int closeParens = count(line, ')');
		while (openParens != closeParens)
		{
			std::cout << "... ";
			std::string more;
			std::getline(std::cin, more);
			line += more;
			openParens += count(more, '(');
			closeParens += count(more, ')');
		}

	}
}

int count(const std::string& code, char ch) {
	int count = 0;
	for (auto&& a : code)
		if (a == ch) count++;
	
	return count;
}
