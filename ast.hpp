/* ------------------------------------------
ast.hpp
Contains class declarations for AST and all
  subclasses.
------------------------------------------ */
#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include "pcl_lexer.hpp"
// --------LLVM includes---------
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <llvm/IR/Value.h>
struct symbol_loc {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
};

class Type;

template<class T>
using SPtr = std::shared_ptr<T>;
using TSPtr = SPtr<Type>;


class LValue;
struct FunctionEntry;

class AST {
public:
	void add_parse_info(struct symbol_loc loc, char* buf){
		location = loc;
		strncpy(linebuf,buf,500);
	}
	virtual ~AST() {}
	virtual void printOn(std::ostream &out) const {out<<"";}
	virtual void sem(){}
	void report_error( const char*msg){
		std::cerr<<
			"Line "<<location.last_line<<
			": " <<msg << "\n"<<linebuf<<std::endl;
		exit(1);
	}
protected:
	struct symbol_loc location{0,0,0,0};
	char linebuf[500]="";
};

template<class T>
std::ostream& operator <<(std::ostream &out,const std::vector<T*> v) {
	for(auto p :v)
	out<<*p<<",";
	return out;
}

inline std::ostream& operator<< (std::ostream &out, const AST &t) {
	t.printOn(out);
	return out;
}

class Type: public AST{
public:
	Type(std::string t);
	std::string get_name();

	virtual void printOn(std::ostream &out) const override;

	// virtual TSPtr clone();

	virtual bool should_delete() const;

	virtual bool doCompare(TSPtr t);

	virtual llvm::Type* cgen(){return nullptr;}

protected:
	std::string name;
};

// static void delete_type(TSPtr t){
// 	if(t->should_delete())
// 		delete t;
// }


class LABEL: public Type{
private:
	LABEL():Type("label"){}  //private constructor to prevent instancing
public:
	static TSPtr* getPtrInstance(){
		static LABEL instance;
		static TSPtr ptr_inst(&instance);
		return &ptr_inst;
	}

	static TSPtr getInstance(){
		return *getPtrInstance();
	}

};

class INTEGER: public Type{
private:
	INTEGER():Type("integer"){}  //private constructor to prevent instancing
public:
	static TSPtr* getPtrInstance(){
		static INTEGER instance;
		static TSPtr ptr_inst(&instance);
		return &ptr_inst;
	}

	static TSPtr getInstance(){
		return *getPtrInstance();
	}


	virtual llvm::Type* cgen() override;

};

class REAL: public Type{
private:
	REAL():Type("real"){}  //private constructor to prevent instancing
public:
	static TSPtr* getPtrInstance(){
		static REAL instance;
		static TSPtr ptr_inst(&instance);
		return &ptr_inst;
	}

	static TSPtr getInstance(){
		return *getPtrInstance();
	}

	virtual llvm::Type* cgen() override;
};

class BOOLEAN: public Type{
private:
	BOOLEAN():Type("boolean"){}  //private constructor to prevent instancing
public:
	static TSPtr* getPtrInstance(){
		static BOOLEAN instance;
		static TSPtr ptr_inst(&instance);
		return &ptr_inst;
	}

	static TSPtr getInstance(){
		return *getPtrInstance();
	}

	virtual llvm::Type* cgen() override;
};

class CHARACTER: public Type{
private:
	CHARACTER():Type("character"){}  //private constructor to prevent instancing
public:
	static TSPtr* getPtrInstance(){
		static CHARACTER instance;
		static TSPtr ptr_inst(&instance);
		return &ptr_inst;
	}

	static TSPtr getInstance(){
		return *getPtrInstance();
	}

	virtual llvm::Type* cgen() override;
};

class ANY: public Type{
private:
	ANY():Type("any"){}  //private constructor to prevent instancing
public:
	static TSPtr* getPtrInstance(){
		static ANY instance;
		static TSPtr ptr_inst(&instance);
		return &ptr_inst;
	}

	static TSPtr getInstance(){
		return *getPtrInstance();
	}

	virtual llvm::Type* cgen() override;
};

// singleton global types
// extern TSPtr intType;
// extern TSPtr realType;
// extern TSPtr boolType;
// extern TSPtr charType;
// extern TSPtr anyType;
// extern TSPtr labelType;


class PtrType: public Type{
public:
	PtrType(TSPtr t);
	PtrType(std::string name,TSPtr t);
	// ~PtrType();

	// virtual Type* clone() override;

	TSPtr get_type();

	virtual void printOn(std::ostream &out) const override;

	virtual bool should_delete() const override;

	virtual bool doCompare(TSPtr t) override;

	virtual llvm::Type* cgen() override;
protected:
	TSPtr type;
};


class ArrType: public PtrType{
public:
	ArrType(int s,TSPtr t);
	ArrType(TSPtr t);

	// virtual Type* clone() override;

	int get_size();

	virtual bool doCompare(TSPtr t) override;

	virtual void printOn(std::ostream &out) const override;
	bool is_1D();
	virtual llvm::Type* cgen() override;
protected:
	int size;
};

class FormalDeclList;
class CallableType: public Type{
public:
	CallableType(std::string func_type, FormalDeclList* formals);
	virtual bool should_delete() const override;
	void typecheck_args(std::vector<TSPtr> arg_types);

	void check_passing(std::vector<bool> ref);

	std::vector<bool> get_by_ref();


	std::vector<TSPtr> get_types();

	std::vector<std::string> get_outer_vars();

	void add_outer(TSPtr t, std::string name);

	std::vector<std::string> get_formal_vars();

protected:
	std::vector<TSPtr> formal_types;
	std::vector<std::string> formal_vars;
	std::vector<bool> by_ref;
	std::vector<std::string> outer_vars;
	std::vector<TSPtr> outer_types;
	std::vector<llvm::Type*> cgen_argTypes();
};

class FunctionType: public CallableType{
public:
	FunctionType(TSPtr ret_ty , FormalDeclList* formals);
	TSPtr get_ret_type();
	virtual llvm::Type* cgen() override;
private:
	TSPtr ret_type;
};

class ProcedureType: public CallableType{
public:
	ProcedureType(FormalDeclList* formals);
	virtual llvm::Type* cgen() override;
};

class Const;
class Expr: public AST {
public:
	virtual TSPtr get_type()=0;
	virtual llvm::Value* cgen()=0;
	virtual bool isLValue() const{return false;}
	virtual Expr* simplify(int &count) {return nullptr;}
};

class Stmt: virtual public AST {
public:
	virtual void printOn(std::ostream &out) const override {
		out << "Stmt()";
	}
	virtual void cgen(){}
	virtual bool isReturn(){return false;}
};

class Return: public Stmt{
public:
	virtual void printOn(std::ostream &out) const override {
		out << "Return";
	}
	virtual bool isReturn() override{return true;}
	virtual void cgen() override;
};

class Const: public Expr{
public:
	Const(TSPtr ty);
	// ~Const();
	virtual TSPtr get_type() override;
protected:
	TSPtr type;
};

class LValue: public Expr{
public:
	LValue(bool dyn=false,bool con=false):dynamic(dyn),is_const(con){}
	bool isDynamic(){return dynamic;}
	virtual bool isLValue() const override{return true;}
	virtual llvm::Value* getAddr()=0;
	void setConst(){
		is_const=true;
	}
	bool isConst(){
		return is_const;
	}
protected:
	bool dynamic;
	bool is_const;
};


class Rconst: public Const {
public:
	Rconst(double n);
	virtual void printOn(std::ostream &out) const override;
	virtual llvm::Value* cgen() override;
private:
	double num;
};


class Iconst: public Const {
public:
	Iconst(int n);
	virtual void printOn(std::ostream &out) const override;
	virtual llvm::Value* cgen() override;
private:
	int num;
};

class Cconst: public Const {
public:
	Cconst(char c);
	virtual void printOn(std::ostream &out) const override ;
	virtual llvm::Value* cgen() override;
private:
	char ch;
};

class NilConst: public Const {
public:
	NilConst();

	virtual void printOn(std::ostream &out) const override;
	virtual llvm::Value* cgen() override;
};

class Sconst: public LValue{
public:
	Sconst(std::string s);
	virtual void printOn(std::ostream &out) const override;
	virtual llvm::Value* cgen() override;
	virtual llvm::Value* getAddr() override;
	virtual TSPtr get_type() override;
private:
	std::string str;
};

class Id: public LValue {
public:
	Id(std::string v);
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual TSPtr get_type() override;
	virtual llvm::Value* getAddr() override;
private:
	std::string name;
	TSPtr type;

	virtual llvm::Value* cgen() override;
};


class Bconst: public Const {
public:
	Bconst(bool b);
	virtual void printOn(std::ostream &out) const override;
	virtual llvm::Value* cgen() override;
private:
	bool boo;
};

class Op: public Expr {
public:
	Op(Expr *l, std::string o, Expr *r);
	Op(std::string o, Expr *l);
	~Op();
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual TSPtr get_type() override;
	virtual llvm::Value* cgen() override;

private:
	Expr *left;
	std::string op;
	Expr *right;
	TSPtr leftType, rightType, resType;
};

class Reference: public Expr{
public:
	Reference(LValue* lval);

	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;

	virtual TSPtr get_type() override;

	LValue* get_lvalue(){ return lvalue;}

	virtual Expr* simplify(int &count) override;

	virtual llvm::Value* cgen() override;
protected:
	LValue* lvalue;
	int count;
};

class Dereference: public LValue{
public:
	Dereference(Expr *e);

	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;

	virtual TSPtr get_type() override;

	Expr* get_expr(){ return expr;}

	virtual Expr* simplify (int &count) override;

	virtual llvm::Value* cgen() override;

	virtual llvm::Value* getAddr() override;
protected:
	Expr *expr;
	int count;
};


class Brackets: public LValue{
public:
	Brackets(LValue *lval, Expr* e);

	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;

	virtual TSPtr get_type() override;

	virtual llvm::Value* cgen() override;

	virtual llvm::Value* getAddr() override;
protected:
	LValue* element();
	LValue* lvalue;
	Expr* expr;
};

class LabelStmt: public Stmt{
public:
	LabelStmt(std::string id, Stmt* s);
	~LabelStmt();
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual void cgen() override;
private:
	std::string label_id;
	Stmt* stmt;
};

class Goto: public Stmt{
public:
	Goto(std::string id);
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual void cgen() override;
private:
	std::string label_id;
};

class Let: public Stmt {
public:
	Let(LValue* lval,Expr* e);
	~Let();
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual void cgen() override;
	static bool typecheck(TSPtr lType, TSPtr rType);
	static char parse_as(TSPtr t);
protected:
	LValue  *lvalue;
	Expr *expr;
	bool different_types;
	bool is_right_int;
};

class If: public Stmt {
public:
	If(Expr* e,Stmt* s1, Stmt* s2);
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual void cgen() override;
private:
	Expr *expr;
	Stmt *stmt1,*stmt2;
};


class While: public Stmt {
public:
	While(Expr* e,Stmt* s);
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual void cgen() override;
private:
	Expr *expr;
	Stmt *stmt;
};

class New: public Stmt{
public:
	New(LValue* lval, Expr* e);
	~New();
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual void cgen() override;
protected:
	Expr* expr;
	LValue* lvalue;
};


class Dispose: public Stmt{
public:
	Dispose(LValue* lval);
	~Dispose();
	virtual void sem() override;
	virtual void printOn(std::ostream &out) const override;
	virtual void cgen() override;
protected:
	LValue *lvalue;
};


class DisposeArr: public Stmt{
public:
	DisposeArr(LValue* lval);
	~DisposeArr();
	virtual void sem() override;
	virtual void printOn(std::ostream &out) const override;
	virtual void cgen() override;
protected:
	LValue *lvalue;
};

template <class T>
class List: virtual public AST{
public:
	List(T *t):list(1,t){}
	List():list(){}
	~List(){
		list.clear();
	}
	void destroy(){
		for(auto p:list)
			delete p;
	}
	virtual void sem() override {
		for(auto p:list)
			p->sem();
	}
	virtual void printOn(std::ostream &out) const override {
		out << "List(" << list << ")";
	}
	void append(T *t){
		list.push_back(t);
	}
	bool isEmpty(){return list.empty();}
	void merge(List* l){
		list.insert(list.end(),l->list.begin(),l->list.end());
		delete l;
	}

	T* operator [](int i){
		return list[i];
	}

	uint size(){
		return list.size();
	}
	friend class Body;
protected:
	std::vector<T*> list;
};

class StmtList: public List<Stmt>, public Stmt{
public:
	StmtList(Stmt *s);
	StmtList();
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual void cgen() override;
};

class ExprList: public List<Expr> {
public:
	ExprList();
	ExprList(Expr *e);
	virtual void printOn(std::ostream &out) const override;

	std::vector<TSPtr> get_type();

	virtual std::vector<llvm::Value*> cgen(std::vector<bool> by_ref);
};

class Decl: public AST{
public:
	Decl(std::string i):id(i),decl_type("unknown"){}
	Decl(std::string i,std::string ty):id(i),decl_type(ty){}
	virtual void printOn(std::ostream &out) const override {
		out << "Decl(" << decl_type <<":"<<id<<")";
	}
	virtual TSPtr get_type(){return nullptr;}
	std::string get_id(){return id;}
	virtual void cgen(){}
protected:
	std::string id;
	std::string decl_type;
};

class LabelDecl:public Decl{
public:
	LabelDecl(Decl* d);
	virtual void printOn(std::ostream &out) const override;
	virtual TSPtr get_type() override;
	virtual void sem() override;
	virtual void cgen() override;
};

class VarDecl: public Decl{
public:
	VarDecl(Decl* d);
	VarDecl(Decl* d,TSPtr t);
	virtual void printOn(std::ostream &out) const override;
	virtual TSPtr get_type() override;
	virtual void sem() override;
	virtual void cgen() override;

protected:
	TSPtr type;
};


class FormalDecl: public VarDecl{
public:
	FormalDecl(Decl *d, bool ref);
	FormalDecl(Decl* d,TSPtr t,bool ref);
	bool isByRef();
protected:
	bool byRef;
};

class DeclList: public List<Decl>{
public:
	DeclList(Decl* d);
	DeclList();
	void toVar(TSPtr t);
	virtual void sem() override;

	void toLabel();
	void toFormal(TSPtr t, bool ref);
	std::vector<TSPtr> get_type();
	virtual void cgen();
};

class FormalDeclList: public DeclList{
public:
	std::vector<bool> get_by_ref();
	std::vector<std::string> get_names();
};

class Call;
class Body: public AST{
public:
	Body(bool library=false);
	Body(DeclList* d, StmtList* s);
	~Body();

	virtual void sem() override;
	void add_body(Body *b);

	void run() const;
	bool isDefined();
	bool isLibrary(){return library;}

	virtual void printOn(std::ostream &out) const override ;

	void cgen();
protected:
	DeclList* declarations;
	StmtList* statements;
	bool defined;
	bool library;
};

class Procedure:public Decl{
public:
	Procedure(std::string name, DeclList *decl_list,
		Body* bod, std::string decl_type="procedure");

	virtual void printOn(std::ostream &out) const override;
	void add_body(Body* bod);

	virtual void sem() override;

	virtual void cgen() override;

	void toForward();

	bool isForward();
protected:
	void sem_helper(bool isFunction=false, TSPtr ret_type=nullptr);
	Body* body;
	FormalDeclList* formals;
	SPtr<CallableType> type;
	bool is_forward=false;
};

class Function:public Procedure{
public:
	Function(std::string name, DeclList *decl_list,
		TSPtr return_type, Body* bod);
	virtual void sem() override;
protected:
	TSPtr ret_type;
};

class Program: public AST{
public:
	Program(std::string nam, Body* bod);

	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;

	void cgen();
private:
	std::string name;
	Body* body;
};

class Call{
public:
	Call(std::string nam, ExprList* exp);
protected:
	std::string name;
	ExprList* exprs;
	std::vector<bool> by_ref;
	ExprList* outer_vars;
	Body* body;

	FunctionEntry* check_passing();

	void add_outer();

	llvm::Value* cgen_common();
	virtual void report_error_from_child(const char* msg)=0;
};

class ProcCall: public Call, public Stmt{
public:
	ProcCall(std::string nam, ExprList* exp);
	virtual void sem() override;
	virtual void printOn(std::ostream &out) const override;
	virtual void cgen() override;
	virtual void report_error_from_child(const char* msg) override{
		this->report_error(msg);
	}
};

class FunctionCall: public Call, public Expr{
public:
	FunctionCall(std::string nam, ExprList* exp);
	virtual void sem() override;
	virtual TSPtr get_type() override;

	virtual void printOn(std::ostream &out) const override ;

	virtual llvm::Value* cgen() override;
	virtual void report_error_from_child(const char* msg) override{
		this->report_error(msg);
	}
private:
	TSPtr type;
};
