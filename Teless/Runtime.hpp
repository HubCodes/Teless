#pragma once

#include "Parser.hpp"

class Runtime {
public:
	Runtime(const Runtime& other) = delete;
	Runtime& operator=(const Runtime& other) = delete;
	virtual ~Runtime();
	
	static Runtime& get();
	void run(Node* top);
private:
	explicit Runtime();
};

