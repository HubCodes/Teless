#pragma once

#include "Teless.hpp"
#include "Parser.hpp"

#include <map>
#include <list>
#include <string>

class Env {
public:
	explicit Env();
	Env(const Env& other) = delete;
	Env& operator=(const Env& other) = delete;
	virtual ~Env();
	
	void setParent(Env* parent);
	bool existKey(const std::string& ident);
	Node* getFromKey(const std::string& ident);
private:
	Env* parent;
	std::list<Env*> childs;
	std::map<std::string, Node*> table;
};
