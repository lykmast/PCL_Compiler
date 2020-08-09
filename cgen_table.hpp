#include <string>
#include <vector>
#include <map>
#include "ast.hpp"

class CgenScope{
public:
	CgenScope(llvm::Function* func):
		vars(), addrs(), functions(), labels(),
		TheFunction(func), CurrentBB(nullptr){}

	llvm::Function* getFunction(){ return TheFunction; }

	llvm::AllocaInst* lookup(std::string name, bool& ref){
		if (addrs.find(name) != addrs.end()){
			ref=true;
			return addrs[name];
		}
		else if (vars.find(name) != vars.end()){
			ref=false;
			return vars[name];
		}
		else{
			return nullptr;
		}
	}

	void insert(std::string name, llvm::AllocaInst* alloca, bool ref){
		if(ref){
			addrs[name]= alloca;
		}
		else{
			vars[name] = alloca;
		}
	}

	void insert_function(std::string name, llvm::Function* func){
		functions[name]=func;
	}

	void insert_label(std::string label, llvm::BasicBlock* BB){
		labels[label]=BB;
	}

	llvm::BasicBlock* label_lookup(std::string label){
		return labels[label];
	}

	llvm::Function* function_lookup(std::string name){
		if (functions.find(name) == functions.end()) return nullptr;
		return functions[name];
	}

	void setCurrentBB(llvm::BasicBlock* BB){
		CurrentBB = BB;
	}

	llvm::BasicBlock* getCurrentBB(){
		return CurrentBB;
	}
private:
	std::map<std::string, llvm::AllocaInst*> vars;
	std::map<std::string, llvm::AllocaInst*> addrs;
	std::map<std::string, llvm::Function*> functions;
	std::map<std::string, llvm::BasicBlock*> labels;
	llvm::Function *TheFunction;
	llvm::BasicBlock* CurrentBB;

};


class CgenTable{
public:
	CgenTable():scopes(){}
	void openScope(llvm::Function* func=nullptr){
		scopes.push_back(CgenScope(func));
	}
	void closeScope(){
		scopes.pop_back();
	}
	void insert(std::string name, llvm::AllocaInst* alloca, bool ref=false){
		scopes.back().insert(name, alloca, ref);
	}
	llvm::AllocaInst* lookup(std::string name, bool& ref){
		return scopes.back().lookup(name, ref);
	}
	llvm::Function* getFunction(){
		return scopes.back().getFunction();
	}
	void insert_function(std::string name, llvm::Function* func){
		scopes.back().insert_function(name, func);
	}

	void insert_label(std::string label, llvm::BasicBlock* BB){
		scopes.back().insert_label(label, BB);
	}

	llvm::BasicBlock* label_lookup(std::string label){
		return scopes.back().label_lookup(label);
	}


	void setCurrentBB(llvm::BasicBlock* BB){
		scopes.back().setCurrentBB(BB);
	}

	llvm::BasicBlock* getCurrentBB(){
		return scopes.back().getCurrentBB();
	}

	llvm::Function* function_lookup(std::string name){
		for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
			llvm::Function *f = i->function_lookup(name);
			if (f != nullptr) return f;
		}
		std::cerr << "Cgen:: Unknown function " << name << std::endl;
		exit(1);
	}
private:
	std::vector<CgenScope> scopes;
};

CgenTable ct;
