/* ------------------------------------------
symbol.hpp
Contains symbol table and symbol entities
  (SymbolEntry, FunctionEntry, Scope)
------------------------------------------ */
#pragma once

#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include "ast.hpp"



struct SymbolEntry {
	Type* type;
	int offset;
	int nesting;
	SymbolEntry() {}
	SymbolEntry(Type* t, int ofs, int nest) : type(t), offset(ofs), nesting(nest) {}
};

struct FunctionEntry {
	CallableType* type;
	int nesting;
	Body* body;
	FunctionEntry() {}
	FunctionEntry(CallableType* t, int nest, Body* bod) : type(t), nesting(nest), body(bod) {}
};

class Scope {
public:
	Scope() : locals(), thisFunction(nullptr), offset(1), size(1), nesting(1) {}
	Scope(int nest, FunctionEntry *e) :
		locals(), thisFunction(e), offset(1), size(1), nesting(nest) {}
	int getOffset() const { return offset; }
	int getNesting() const { return nesting; }
	int getSize() const { return size; }
	SymbolEntry *lookup(std::string name) {
		if (locals.find(name) == locals.end()) return nullptr;
		return &(locals[name]);
	}
	FunctionEntry *function_lookup(std::string name) {
		if (functions.find(name) == functions.end()) return nullptr;
		return &(functions[name]);
	}
	void insert(std::string name, Type* t, int s) {
		if (locals.find(name) != locals.end()) {
			std::cerr << "Duplicate variable " << name << std::endl;
			exit(1);
		}
		locals[name] = SymbolEntry(t, offset, nesting);
		offset+=s;
		size+=s;
	}
	void insert_function(std::string name, CallableType* t, Body* bod) {
		if (functions.find(name) != functions.end()) {
			if(functions[name].body){
				std::cerr << "Duplicate function " << name << std::endl;
				exit(1);
			}
		}
		functions[name] = FunctionEntry(t, nesting, bod);
	}

	void add_outer(Type* t){
		thisFunction->type->add_outer(t);
	}
private:
	std::map<std::string, SymbolEntry> locals;
	std::map<std::string, FunctionEntry> functions;
	FunctionEntry* thisFunction;
	int offset;
	int size;
	int nesting;
};


class SymbolTable {
public:
	void openScope(std::string name) {
		int nest = scopes.empty() ? 0 : scopes.back().getNesting();
		if (!scopes.empty()){
			FunctionEntry *e = function_lookup(name);
			scopes.push_back(Scope(nest+1,e));
		}
		else{
			scopes.push_back(Scope());
		}
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

	FunctionEntry *function_lookup(std::string name) {
		for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
			FunctionEntry *e = i->function_lookup(name);
			if (e != nullptr) return e;
		}
		std::cerr << "Unknown function " << name << std::endl;
		exit(1);
	}

	FunctionEntry *function_decl_lookup(std::string name) {
		FunctionEntry* e = scopes.back().function_lookup(name);
		return e;
	}

	int getSizeOfCurrentScope() const { return scopes.back().getSize(); }
	void insert(std::string name, Type* t, int size=1) { scopes.back().insert(name, t, size); }
	void insert_function(std::string name, CallableType* t, Body* bod) { scopes.back().insert_function(name, t, bod); }
	int getNestingOfCurrentScope() const{ return scopes.back().getNesting();}
private:
	std::vector<Scope> scopes;
};

SymbolTable st;
