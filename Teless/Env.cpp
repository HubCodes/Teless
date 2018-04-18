#include "Env.hpp"

Env::Env() {

}

Env::~Env() {

}

void Env::setParent(Env* parent) {
	this->parent = parent;
}

bool Env::existKey(const std::string& ident) {
	bool inThisNamespace = table.find(ident) != table.end();
	return (parent == nullptr) ? 
		inThisNamespace : 
		inThisNamespace || parent->existKey(ident);
}

Node* Env::getFromKey(const std::string& ident) {
	if (existKey(ident))
	{
		// 접근 가능한 스코프 안에 존재하지만 현재 스코프에 없을 때
		if (table.find(ident) == table.end())
		{
			return parent->getFromKey(ident);
		}

		// 현재 스코프에 있을 때
		else
		{
			return table[ident];
		}
	}
	
	// 스코프 안에 없을 때
	return nullptr;
}
