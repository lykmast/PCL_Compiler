/* ------------------------------------------
ast.hpp
Contains class declarations for AST and all
  subclasses.
------------------------------------------ */
#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include "pcl_lexer.hpp"


class LValue;
struct FunctionEntry;

class AST {
public:
	void add_parse_info(int no, char* buf,char* txt){
		lineno=no;
		strncpy(linebuf,buf,500);
		strncpy(text,txt,100);
	}
	virtual ~AST() {}
	virtual void printOn(std::ostream &out) const {out<<"";}
	virtual void sem(){}
	void report_error(char*msg){
		// lyyerror(lineno,linebuf,text,msg);
	}
protected:
	int lineno;
	char linebuf[500];
	char text[100];
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

	virtual Type* clone();

	virtual bool should_delete() const;

	virtual bool doCompare(Type* t);

protected:
	std::string name;
};

class LABEL: public Type{
private:
	LABEL():Type("label"){}  //private constructor to prevent instancing
public:
	static LABEL* getInstance(){
		static LABEL instance;
		return &instance;
	}
};

class INTEGER: public Type{
private:
	INTEGER():Type("integer"){}  //private constructor to prevent instancing
public:
	static INTEGER* getInstance(){
		static INTEGER instance;
		return &instance;
	}

};

class REAL: public Type{
private:
	REAL():Type("real"){}  //private constructor to prevent instancing
public:
	static REAL* getInstance(){
		static REAL instance;
		return &instance;
	}
};

class BOOLEAN: public Type{
private:
	BOOLEAN():Type("boolean"){}  //private constructor to prevent instancing
public:
	static BOOLEAN* getInstance(){
		static BOOLEAN instance;
		return &instance;
	}
};

class CHARACTER: public Type{
private:
	CHARACTER():Type("character"){}  //private constructor to prevent instancing
public:
	static CHARACTER* getInstance(){
		static CHARACTER instance;
		return &instance;
	}
};

class ANY: public Type{
private:
	ANY():Type("any"){}  //private constructor to prevent instancing
public:
	static ANY* getInstance(){
		static ANY instance;
		return &instance;
	}
};


class PtrType: public Type{
public:
	PtrType(Type* t);
	PtrType(std::string name,Type* t);
	~PtrType();

	virtual Type* clone();

	Type* get_type();

	virtual void printOn(std::ostream &out) const override;

	virtual bool should_delete() const override;

	virtual bool doCompare(Type* t) override;

protected:
	Type* type;
};


class ArrType: public PtrType{
public:
	ArrType(int s,Type* t);
	ArrType(Type* t);

	virtual Type* clone();


	int get_size();

	virtual bool doCompare(Type* t) override;

	virtual void printOn(std::ostream &out) const override;

protected:
	int size;
};

class CallableType: public Type{
public:
	CallableType(std::string func_type, std::vector<Type*> formalTs, std::vector<bool> ref);
	virtual bool should_delete() const override;
	void typecheck_args(std::vector<Type*> arg_types);

	void check_passing(std::vector<bool> ref);

	std::vector<bool> get_by_ref();


	std::vector<Type*> get_types();

	std::vector<std::string> get_outer_vars();

	void add_outer(std::string name);

protected:
	std::vector<Type*> formal_types;
	std::vector<bool> by_ref;
	std::vector<std::string> outer_vars;
};

class FunctionType: public CallableType{
public:
	FunctionType( Type* ret_ty ,std::vector<Type*> formalTs, std::vector<bool> ref);
	Type* get_ret_type();
private:
	Type* ret_type;
};

class ProcedureType: public CallableType{
public:
	ProcedureType(std::vector<Type*> formalTs, std::vector<bool> ref);
};

class Const;
class Expr: public AST {
public:
	virtual Type* get_type()=0;
	virtual bool isLValue() const{return false;}
};

class Stmt: virtual public AST {
public:
	virtual void printOn(std::ostream &out) const override {
		out << "Stmt()";
	}
	virtual bool isReturn(){return false;}
};

class Return: public Stmt{
public:
	virtual void printOn(std::ostream &out) const override {
		out << "Return";
	}
	virtual bool isReturn() override{return true;}
};

class Const: public Expr{
public:
	Const(Type* ty);
	~Const();
	virtual Type* get_type() override;
protected:
	Type* type;
};

class LValue: public Expr{
public:
	LValue(bool dyn=false):dynamic(dyn){}
	bool isDynamic(){return dynamic;}
	virtual bool isLValue() const override{return true;}
protected:
	bool dynamic;
};


class Rconst: public Const {
public:
	Rconst(double n);
	virtual void printOn(std::ostream &out) const override;
private:
	double num;
};


class Iconst: public Const {
public:
	Iconst(int n);
	virtual void printOn(std::ostream &out) const override;
private:
	int num;
};

class Cconst: public Const {
public:
	Cconst(char c);
	virtual void printOn(std::ostream &out) const override ;
private:
	char ch;
};

class NilConst: public Const {
public:
	NilConst();

	virtual void printOn(std::ostream &out) const override;
};

class Sconst: public LValue{
public:
	Sconst(std::string s);
	virtual void printOn(std::ostream &out) const override;
	virtual Type* get_type() override;
private:
	std::string str;
};

class Id: public LValue {
public:
	Id(std::string v);
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	std::string name;
	Type* type;

};


class Bconst: public Const {
public:
	Bconst(bool b);
	virtual void printOn(std::ostream &out) const override;
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
	virtual Type* get_type() override;

private:
	Expr *left;
	std::string op;
	Expr *right;
	Type *leftType,*rightType,*resType;
};

class Reference: public Expr{
public:
	Reference(LValue* lval);

	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;

	virtual Type* get_type() override;

	LValue* get_lvalue(){ return lvalue;}

	Expr* simplify(int &count);
protected:
	LValue* lvalue;
	int count;
};

class Dereference: public LValue{
public:
	Dereference(Expr *e);

	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;

	virtual Type* get_type() override;

	Expr* get_expr(){ return expr;}

	Expr* simplify(int &count);

protected:
	Expr *expr;
	int count;
};


class Brackets: public LValue{
public:
	Brackets(LValue *lval, Expr* e);

	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;

	virtual Type* get_type() override;
protected:
	LValue* element();
	LValue* lvalue;
	Expr* expr;
};

class Let: public Stmt {
public:
	Let(LValue* lval,Expr* e);
	~Let();
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
	static bool typecheck(Type* lType, Type* rType);
	static char parse_as(Type *t);
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
private:
	Expr *expr;
	Stmt *stmt1,*stmt2;
};


class While: public Stmt {
public:
	While(Expr* e,Stmt* s);
	virtual void printOn(std::ostream &out) const override;
	virtual void sem() override;
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
protected:
	LValue *lvalue;
};


class DisposeArr: public Stmt{
public:
	DisposeArr(LValue* lval);
	~DisposeArr();
	virtual void sem() override;
	virtual void printOn(std::ostream &out) const override;
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
};

class ExprList: public List<Expr> {
public:
	ExprList();
	ExprList(Expr *e);
	virtual void printOn(std::ostream &out) const override;

	std::vector<Type*> get_type();

};

class Decl: public AST{
public:
	Decl(std::string i):id(i),decl_type("unknown"){}
	Decl(std::string i,std::string ty):id(i),decl_type(ty){}
	virtual void printOn(std::ostream &out) const override {
		out << "Decl(" << decl_type <<":"<<id<<")";
	}
	virtual Type* get_type(){return nullptr;}
	std::string get_id(){return id;}
protected:
	std::string id;
	std::string decl_type;
};

class LabelDecl:public Decl{
public:
	LabelDecl(Decl* d);
	virtual void printOn(std::ostream &out) const override;
	virtual Type* get_type() override;
	virtual void sem() override;
};

class VarDecl: public Decl{
public:
	VarDecl(Decl* d);
	VarDecl(Decl* d,Type* t);
	virtual void printOn(std::ostream &out) const override;
	virtual Type* get_type() override;
	virtual void sem() override;

protected:
	Type* type;
};


class FormalDecl: public VarDecl{
public:
	FormalDecl(Decl *d, bool ref);
	FormalDecl(Decl* d,Type* t,bool ref);
	bool isByRef();
protected:
	bool byRef;
};

class DeclList: public List<Decl>{
public:
	DeclList(Decl* d);
	DeclList();
	void toVar(Type* t);
	virtual void sem() override;

	void toLabel();
	void toFormal(Type* t, bool ref);
	std::vector<Type*> get_type();
};

class FormalDeclList: public DeclList{
public:
	std::vector<bool> get_by_ref();
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
protected:
	void sem_helper(bool isFunction=false, Type* ret_type=nullptr);
	Body* body;
	FormalDeclList* formals;
};

class Function:public Procedure{
public:
	Function(std::string name, DeclList *decl_list,
		Type* return_type, Body* bod);
	virtual void sem() override;
protected:
	Type* ret_type;
};

class Program: public AST{
public:
	Program(std::string nam, Body* bod);
	virtual void printOn(std::ostream &out) const override;

	virtual void sem() override;
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

};

class ProcCall: public Call, public Stmt{
public:
	ProcCall(std::string nam, ExprList* exp);
	virtual void sem() override;
	virtual void printOn(std::ostream &out) const override;
};

class FunctionCall: public Call, public Expr{
public:
	FunctionCall(std::string nam, ExprList* exp);
	virtual void sem() override;
	virtual Type* get_type() override;

	virtual void printOn(std::ostream &out) const override ;
private:
	Type* type;
};
