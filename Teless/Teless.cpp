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
	// �ɼ� ��ü �������� ���� �μ��� ����
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
	// ���� �μ��� �ϳ��� ���ٸ� REPL ���� ���� (ù ��° �μ��� ���α׷������� ����)
	if (argc == 1)
	{
		opt->set(Option::REPL);
	}

	// �μ��� �����Ѵٸ�
	else
	{
		// ����
		if (0 == std::strncmp(argv[1], "-h", 2))
		{
			std::cout << "Usage: teless (option|filepath)"
				"\n"
				"	-h:	Show this help page.\n"
				"	-v: Show version.";
			std::exit(EXIT_SUCCESS);
		}
		
		// ������ ���� �μ�
		else if (0 == std::strncmp(argv[1], "-v", 2))
		{
			std::cout << VERSION_MAJOR << '.' << VERSION_MINOR << '\n';
			std::exit(EXIT_SUCCESS);
		}

		// ��𿡵� �ش���� �ʴ´ٸ� ���� �̸��̴�.
		else
		{
			// ���� ���� ���θ� Ȯ���Ѵ�.
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

	// REPL ����
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
