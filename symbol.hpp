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
#include <set>
#include <string>
#include "ast.hpp"



struct SymbolEntry {
	TSPtr type;
	SymbolEntry() {}
	SymbolEntry(TSPtr t) : type(t) {}
};

struct FunctionEntry {
	SPtr<CallableType> type;
	Body* body;
	FunctionEntry() {}
	FunctionEntry(SPtr<CallableType> t, Body* bod) : type(t), body(bod) {}
};

class Scope {
public:
	Scope() : locals(), thisFunction(nullptr){}
	Scope(FunctionEntry *e) :
		locals(), thisFunction(e) {}
	FunctionEntry* getParent() const{return thisFunction;}
	SymbolEntry *lookup(std::string name) {
		if (locals.find(name) == locals.end()) return nullptr;
		return &(locals[name]);
	}
	FunctionEntry *function_lookup(std::string name) {
		if (functions.find(name) == functions.end()) return nullptr;
		return &(functions[name]);
	}
	void insert(std::string name, TSPtr t) {
		if (locals.find(name) != locals.end()) {
			std::cerr << "Duplicate variable " << name << std::endl;
			exit(1);
		}
		locals[name] = SymbolEntry(t);
	}
	void insert_function(std::string name, SPtr<CallableType> t, Body* bod) {
		if (functions.find(name) != functions.end()) {
			if(functions[name].body){
				std::cerr << "Duplicate function " << name << std::endl;
				exit(1);
			}
		}
		functions[name] = FunctionEntry(t, bod);
	}

	void add_outer(TSPtr t, std::string name){
		thisFunction->type->add_outer(t,name);
		insert(name, t);
	}

	void insert_label(std::string lbl){
		if(labels.find(lbl)!=labels.end()){
			std::cerr << "Label " << lbl << " already declared in this scope."
				<< std::endl;
			exit(1);
		}
		labels.insert(lbl);
	}

	void label_lookup(std::string lbl){
		if(labels.find(lbl)==labels.end()){
			std::cerr << "Label " << lbl << " already declared in this scope."
				<< std::endl;
			exit(1);
		}
	}
private:
	std::map<std::string, SymbolEntry> locals;
	std::map<std::string, FunctionEntry> functions;
	std::set<std::string> labels;
	FunctionEntry* thisFunction;
};


class SymbolTable {
public:
	void openScope(std::string name) {
		if (scopes.size()>1){
			FunctionEntry *e = function_lookup(name);
			scopes.push_back(Scope(e));
		}
		else{
			scopes.push_back(Scope());
		}
	}
	void closeScope() { scopes.pop_back(); };

	SymbolEntry *lookup(std::string name) {
		std::vector<Scope>::iterator it;
		SymbolEntry *e;
		e=scopes.back().lookup(name);
		if(e) return e;

		// it's on an outer scope (or not existent)
		for (auto i = scopes.rbegin()+1; i != scopes.rend(); ++i) {
			it=i.base();// forward 'it' points to next scope from i
			e= i->lookup(name);
			if (e != nullptr) break;
		}
		if(e){
			// add e as implicit parameter
			//   to all scopes from 'it' to end.
			for(auto i= it; i!=scopes.end();++i){
				i->add_outer(e->type, name);
			}
			// return newly added entry on current scope
			return scopes.back().lookup(name);
		}
		return nullptr;
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

	void insert_label(std::string lbl){
		scopes.back().insert_label(lbl);
	}

	void label_lookup(std::string lbl){
		scopes.back().label_lookup(lbl);
	}

	void insert(std::string name, TSPtr t) { scopes.back().insert(name, t); }
	void insert_function(std::string name, SPtr<CallableType> t, Body* bod) { scopes.back().insert_function(name, t, bod); }
	FunctionEntry *getParentOfCurrentScope() const {return scopes.back().getParent();}
private:
	std::vector<Scope> scopes;
};

SymbolTable st;
