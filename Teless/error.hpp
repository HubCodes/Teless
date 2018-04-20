#pragma once

#include <string>
#include <stack>
#include <iostream>
#include <cstdlib>

struct ErrorInfo {
	std::string msg;
	Location location;
};

using ErrorStack = std::stack<ErrorInfo>;

class ErrorManager final {
public:
	static ErrorManager& get() {
		ErrorManager manager;
		return manager;
	}

	void pushError(const ErrorInfo& errorMessage) {
		stack.push(errorMessage);
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

inline void errorOccur(const ErrorInfo& errorMsg) {
	ErrorManager::get().pushError(errorMsg);
}