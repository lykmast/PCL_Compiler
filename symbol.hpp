#pragma once

#include <cstdlib>
#include <vector>
#include "ast.hpp"
#include <string>
#include "symbol.hpp"

struct SymbolEntry {
	Type* type;
	int offset;
	SymbolEntry() {}
	SymbolEntry(Type* t, int ofs) : type(t), offset(ofs) {}
};

class Scope {
public:
	Scope() : locals(), offset(-1), size(0) {}
	Scope(int ofs) : locals(), offset(ofs), size(0) {}
	int getOffset() const { return offset; }
	int getSize() const { return size; }
	SymbolEntry *lookup(std::string name) {
		if (locals.find(name) == locals.end()) return nullptr;
		return &(locals[name]);
	}
	void insert(std::string name, Type* t) {
		if (locals.find(name) != locals.end()) {
			std::cerr << "Duplicate variable " << name << std::endl;
			exit(1);
		}
		locals[name] = SymbolEntry(t, offset++);
		++size;
	}
private:
	std::map<std::string, SymbolEntry> locals;
	int offset;
	int size;
};

class SymbolTable {
public:
	void openScope() {
		int ofs = scopes.empty() ? 0 : scopes.back().getOffset();
		scopes.push_back(Scope(ofs));
	}
	void closeScope() { scopes.pop_back(); };
	SymbolEntry *lookup(std::string name) {
		for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
			SymbolEntry *e = i->lookup(name);
			if (e != nullptr) return e;
		}
		std::cerr << "Unknown variable " << name << std::endl;
		exit(1);
	}
	int getSizeOfCurrentScope() const { return scopes.back().getSize(); }
	void insert(std::string name, Type* t) { scopes.back().insert(name, t); }
private:
	std::vector<Scope> scopes;
};

extern SymbolTable st;
