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
		// ���� ������ ������ �ȿ� ���������� ���� �������� ���� ��
		if (table.find(ident) == table.end())
		{
			return parent->getFromKey(ident);
		}

		// ���� �������� ���� ��
		else
		{
			return table[ident];
		}
	}
	
	// ������ �ȿ� ���� ��
	return nullptr;
}
