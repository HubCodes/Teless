#pragma once

#include <string>
#include <stack>
#include <iostream>
#include <cstdlib>

using ErrorStack = std::stack<std::stack<std::string>>;

class ErrorManager final {
public:
	static ErrorManager& get() {
		ErrorManager manager;
		return manager;
	}

	void pushNewErrorContext() {
		stack.push(std::stack<std::string>());
	}

	void pushError(const std::string& errorMessage) {
		stack.top().push(errorMessage);
	}

	
private:
	explicit ErrorManager() {

	}
	ErrorStack stack;
};

#ifdef _MSC_VER
__declspec(noreturn)
#elif defined __GNUC__
__attribute__((noreturn))
#endif
inline void die(const std::string& errorMsg) {
	std::cerr << errorMsg << '\n';
	std::exit(EXIT_FAILURE);
}