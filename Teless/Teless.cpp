#include <iostream>

#include "lexer.hpp"

#ifdef __TEST_MODE
void test_sequence() {

}
#endif

class Option;

void parseOption(Option* opt, const int argc, const char** argv);

int main(const int argc, const char** argv) {
	Option* opt{ new Option };
	parseOption(opt, argc, argv);

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
private:
	options option;
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

void parseOption(Option* opt, const int argc, const char** argv) {
	// 만약 인수가 하나도 없다면 (첫 번째 인수는 프로그램명임을 주의)
	if (argc == 1)
	{
		opt->set(Option::REPL);
	}

	// 인수가 존재한다면
	else
	{
		
	}
}