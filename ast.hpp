#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "symbol.hpp"

class LValue;
union value{
	char c;
	int i;
	double r;
	bool b;
	char* s;
	LValue* lval;
};


template<class T>
std::ostream& operator <<(std::ostream &out,const std::vector<T*> v) {
	for(auto p :v)
		out<<*p<<",";
	return out;
}

class AST {
public:
	virtual ~AST() {}
	virtual void printOn(std::ostream &out) const = 0;
	virtual void sem(){}
};

inline std::ostream& operator<< (std::ostream &out, const AST &t) {
	t.printOn(out);
	return out;
}
class Const;
class UnnamedLValue;
class Type: public AST{
public:
	Type(std::string t):name(t){}
	std::string get_name(){
		return name;
	}
	virtual Type* get_type() const{ return nullptr;}//TODO erase
	virtual UnnamedLValue* create() const{return nullptr;}
	virtual void printOn(std::ostream &out) const override {
		out << "Type(" << name << ")";
	}
	virtual Type* clone(){return this;}
	virtual bool should_delete() const{//should not delete Singleton
		return false;
	}
	virtual bool doCompare(Type* t){
		return !(name.compare(t->get_name()));
	}
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
	virtual UnnamedLValue* create() const;

};

class REAL: public Type{
private:
	REAL():Type("real"){}  //private constructor to prevent instancing
public:
	virtual UnnamedLValue* create() const;
	static REAL* getInstance(){
		static REAL instance;
		return &instance;
	}
};

class BOOLEAN: public Type{
private:
	BOOLEAN():Type("boolean"){}  //private constructor to prevent instancing
public:
	virtual UnnamedLValue* create() const;
	static BOOLEAN* getInstance(){
		static BOOLEAN instance;
		return &instance;
	}
};

class CHARACTER: public Type{
private:
	CHARACTER():Type("character"){}  //private constructor to prevent instancing
public:
	virtual UnnamedLValue* create() const;
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
	PtrType(Type* t):Type("pointer"),type(t){}
	PtrType(std::string name,Type* t):Type(name),type(t){}//TODO erase
	~PtrType(){
		if(type)
			if(type->should_delete())
				delete type;
	}
	virtual UnnamedLValue* create() const;
	virtual Type* clone(){
		return new PtrType(type->clone());
	}
	Type* get_type(){ return type->clone();}
	virtual void printOn(std::ostream &out) const override {
		out << "PtrType(" << name <<"of type "<< *type << ")";
	}
	virtual bool should_delete() const override{
		return true;
	}
	virtual bool doCompare(Type* t) override{
		if (!(name.compare(t->get_name()))){
			PtrType* pTy=static_cast<PtrType*>(t);
			return type->doCompare(pTy->get_type());
		}
		return false;
	}
protected:
	Type* type;
};

class ArrType: public PtrType{
public:
	ArrType(int s,Type* t):PtrType("array",t),size(s){}
	ArrType(Type* t):PtrType("array",t),size(-1){}
	virtual Type* clone(){
		return new ArrType(size,type->clone());
	}
	virtual UnnamedLValue* create() const;
	int get_size(){return size;}
	virtual bool doCompare(Type* t) override{
		if (!(name.compare(t->get_name()))){
			ArrType* arrTy=static_cast<ArrType*>(t);
			return size==arrTy->get_size() and type->doCompare(arrTy->get_type());
		}
		return false;
	}
	virtual void printOn(std::ostream &out) const override {
		out << "ArrType(" << name <<"["<<size<<"]"<<"of type "<< *type << ")";
	}
protected:
	int size;
};

class CallableType: public Type{
public:
	CallableType(std::string func_type, std::vector<Type*> formalTs, std::vector<bool> ref):
		Type(func_type), formal_types(formalTs), by_ref(ref){}
		virtual bool should_delete() const override{
			return true;
		}
		void typecheck_args(std::vector<Type*> arg_types){
			if (arg_types.size()!=formal_types.size()){
				std::cerr<<"Expected "<<formal_types.size()<<" arguments, "
					<<arg_types.size()<<" were given."<<std::endl;
					exit(1);
			}
			for(uint i=0; i<arg_types.size(); i++){
				if(!formal_types[i]->doCompare(arg_types[i])){
					std::cerr<<"Expected argument "<<i<<" to be "<<*formal_types[i]
						<<" but it was "<<*arg_types[i]<<std::endl;
						exit(1);
				}
			}
		}

		void check_passing(std::vector<bool> ref){
			for(uint i=0; i<by_ref.size(); i++){
				if(by_ref[i]!=ref[i]){
					std::string whatis, whatisnt;
					if(ref[i]){
						whatis="by reference";
						whatisnt="by value";
					}
					else{
						whatisnt="by reference";
						whatis="by value";
					}
					std::cerr<<"Expected argument "<<i<<"of "<<name<<" to be passed "<<whatisnt
						<<", but it was passed "<<whatis<<"."<<std::endl;
						exit(1);
				}
			}
		}
		std::vector<bool> get_by_ref(){
			return by_ref;
		}
protected:
	std::vector<Type*> formal_types;
	std::vector<bool> by_ref;
};

class FunctionType: public CallableType{
public:
	FunctionType( Type* ret_ty ,std::vector<Type*> formalTs, std::vector<bool> ref):
		CallableType("function", formalTs, ref),ret_type(ret_ty){}
		Type* get_ret_type(){
			return ret_type->clone();
		}

private:
	Type* ret_type;
};

class ProcedureType: public CallableType{
public:
	ProcedureType(std::vector<Type*> formalTs, std::vector<bool> ref):
		CallableType("procedure", formalTs, ref){}
};

extern std::vector<LValue*> rt_stack;
extern unsigned long fp;

class Expr: public AST {
public:
	virtual Const* eval() = 0;
	virtual Type* get_type()=0;
	virtual bool isLValue() const{return false;}
};

class Stmt: virtual public AST {
public:
	virtual void run() const{}; //empty stmt -> do nothing
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
	Const(Type* ty):type(ty){}
	~Const(){
		if(type)
			if(type->should_delete())
				delete type;
	}
	virtual Type* get_type() override{return type->clone();}
	virtual void printOn(std::ostream &out) const =0;
	virtual Const* clone()=0;
	virtual Const* eval() override{return this->clone();}
	virtual value get_value() const{value v; v.lval=nullptr; return v;}
	virtual Const* copyToType() const {return nullptr;}
protected:
	Type* type;
};



class LValue: public Expr{
public:
	LValue(bool dyn=false):dynamic(dyn){}
	virtual void let(Const* c)=0;
	bool isDynamic(){return dynamic;}
	virtual bool isLValue() const override{return true;}
protected:
	bool dynamic;
};

class UnnamedLValue: public LValue{
public:
	UnnamedLValue(Type *ty,bool dyn=false):LValue(dyn),value(nullptr),type(ty){}
	UnnamedLValue(Const* val, Type* ty,bool dyn=false):LValue(dyn),value(val),type(ty){}
	~UnnamedLValue(){
		delete value;
		if(type)
			if(type->should_delete())
				delete type;
	}
	virtual void printOn(std::ostream &out) const override {
		if(value)
			out << "UnnamedLValue(" << *value<<","<<*type << ")";
		else if(type)
			out << "UnnamedLValue(EMPTY, "<<*type << ")";
		else
			out << "UnnamedLValue(EMPTY)";
	}
	virtual void let(Const* c) override{
		if(value)delete value;
		value=c;
		if(type)
			if(type->should_delete())
				delete type;
		type=c->get_type();
	}
	virtual Const* eval() override{
		if(value)
			return value->eval();
		else
			return nullptr;
	}
	virtual Type* get_type(){
		return type->clone();
	}
protected:
	Const* value;
	Type* type;
};





class Rconst: public Const {
public:
	Rconst(double n):Const(REAL::getInstance() ),num(n){}
	virtual Const* clone() override{return new Rconst(*this);}
	virtual void printOn(std::ostream &out) const override {
		out << "Rconst(" << num << ")";
	}
	virtual value get_value() const override {
		value v; v.r=num;
		return v;
	}
private:
	double num;
};

inline UnnamedLValue* REAL::create() const{
	return new UnnamedLValue(REAL::getInstance(),true);
}

class Iconst: public Const {
public:
	Iconst(int n):Const(INTEGER::getInstance()),num(n){}
	virtual Const* clone() override{return new Iconst(*this);}
	virtual void printOn(std::ostream &out) const override {
		out << "Iconst(" << num << ")";
	}
	virtual value get_value() const override {
		value v; v.i=num;
		return v;
	}
	virtual Const* copyToType() const{
		return new Rconst(num*1.0);
	}
private:
	int num;
};

inline UnnamedLValue* INTEGER::create() const{
	return new UnnamedLValue(INTEGER::getInstance(),true);
}

class Cconst: public Const {
public:
	Cconst(char c):Const(CHARACTER::getInstance() ),ch(c){}
	virtual Const* clone() override{return new Cconst(*this);}
	virtual void printOn(std::ostream &out) const override {
		out << "Cconst(" << ch << ")";
	}
	virtual value get_value() const override {
		value v; v.c=ch;
		return v;
	}
private:
	char ch;
};

inline UnnamedLValue* CHARACTER::create() const{
	return new UnnamedLValue(CHARACTER::getInstance(),true);
}

class Pconst: public Const {
public:
	Pconst():Const(new PtrType(ANY::getInstance())),ptr(nullptr){}//TODO implement for pointers different than nil
	virtual Const* clone() override{return new Pconst(*this);}

	Pconst(LValue* pval,Type *t):Const(new PtrType(t) ),ptr(pval){}
	virtual void printOn(std::ostream &out) const override {
		out << "Pconst(" << ptr << "of type "<<*type<< ")";
		if(ptr)
			out<<"->"<<*ptr;
		else
			out<<"->nil";
	}
	virtual value get_value() const override {
		value v; v.lval=ptr;
		return v;
	}

	virtual Const* copyToType() const{
		Type* temp=static_cast<PtrType*>(type)->get_type();
		ArrType* arrType=static_cast<ArrType*>(temp);
		Type* inType=arrType->get_type();

		return new Pconst(ptr,new PtrType(ArrType(inType)));
	}

protected:
	LValue* ptr;
};

inline UnnamedLValue* PtrType::create() const{
	return new UnnamedLValue(new PtrType(type),true);
}
class Arrconst: public Const{
public:
	Arrconst(int s, Type* t):Const(new ArrType(s,t)),size(s){}
	virtual Const* clone() override{return this;}
	virtual LValue* get_element(int i)=0;

protected:
	int size;
};

class StaticArray: public Arrconst{
public:
	StaticArray(int s, Type* t, int o):Arrconst(s,t), offset(o){
		child_size=find_child_size(t);

		if(!t->get_name().compare("array")){
			ArrType* arrT=static_cast<ArrType*>(t);
			if(arrT->get_size()>0){
				for(int i=0; i<s; i++){
					rt_stack[offset+i*child_size]->let(
						new StaticArray(
							arrT->get_size(),
							arrT->get_type(),
							offset+i*child_size+1
						)
					);
				}
			}
		}
	}

	virtual void printOn(std::ostream &out) const override {
		out << "StaticArray( ["<<size<<"]"<<"of type "<< *type << ")";
	}

	virtual LValue* get_element(int i){
		return rt_stack[i*child_size+offset];
	}

private:
	int find_child_size(Type* t){
		if(!t->get_name().compare("array")){
			ArrType* arrT=static_cast<ArrType*>(t);
			int children=arrT->get_size();
			if(children>0){
				return children*find_child_size(arrT->get_type())+1;
			}
		}
		return 1;
	}
	int offset;
	int child_size;
};

class DynamicArray: public Arrconst{
public:
	DynamicArray(int s, Type* t):Arrconst(s,t){
		if(s<0){
			/*TODO error wrong value*/
			std::cerr<<"ERROR: "<< s <<" is bad size for array!"<<std::endl;
			exit(1);
		}
		arr.resize(s);
		for(int i=0;i<s;i++){
			arr[i]=t->create();
		}
	}
	~DynamicArray(){
		for (auto p:arr)
			delete p;
	}
	LValue* get_element(int i){
		return arr[i];
	}
	void fromString(char* str){
		for(int i=0; i<size;i++){
			arr[i]->let(new Cconst(str[i]));
		}
	}
	virtual void printOn(std::ostream &out) const override {
		out << "DynamicArray([" << arr <<"] ["<<size<<"]"<<"of type "<< *type << ")";
	}
protected:
	std::vector<UnnamedLValue*> arr;
};

class Id: public LValue {
public:
	Id(std::string v): name(v), offset(-1), decl_nesting(0), current_nesting(0), type(nullptr) {}
	virtual void printOn(std::ostream &out) const override {
		out << "Id(" << name <<"@"<<offset<<":"<<decl_nesting<<")";
	}
	virtual Const* eval() override {
		int abs_ofs=find_absolute_offset();
		Const *c = rt_stack[abs_ofs]->eval();
		if(!c and !type->get_name().compare("array")){
			ArrType* arrT=static_cast<ArrType*>(type);
			create_static_array(abs_ofs, arrT);
			return rt_stack[abs_ofs]->eval();
		}
		else return c;
	}

	virtual void sem() override{
		SymbolEntry *e = st.lookup(name);
		if(!e){
			//TODO throw error variable not declared
			std::cerr<<"Id '"<<name<<"' not declared"<<std::endl;
			exit(1);
		}
		type = e->type;
		offset = e->offset;
		decl_nesting = e->nesting;
		current_nesting=st.getNestingOfCurrentScope();
	}

	virtual void let(Const* c){
		rt_stack[find_absolute_offset()]->let(c);
	}
	virtual Type* get_type(){
		return type->clone();
	}
	std::string get_name(){
		return name;
	}

	int get_offset(){
		return offset;
	}
private:
	std::string name;
	int offset;
	int decl_nesting;
	int current_nesting;
	Type* type;

	void create_static_array(int abs_ofs, ArrType* t){
		Type* inside_t=t->get_type();
		int s = t->get_size();
		Const *c = new StaticArray(s,inside_t,abs_ofs+1);
		rt_stack[abs_ofs]->let(c);
	}

	int find_absolute_offset(){
		int prev_fp=fp;
		for(int diff=current_nesting-decl_nesting; diff>0; diff--){
			prev_fp=rt_stack[prev_fp]->eval()->get_value().i;
		}
		return prev_fp+offset;
	}
};
class Sconst: public UnnamedLValue {
public:
	Sconst(std::string s):UnnamedLValue(new ArrType(s.size()-1,CHARACTER::getInstance())) {
		char* str=(char*)(malloc(sizeof(char)*(s.size()-1)));
		s.substr(1,s.size()-2).copy(str,s.size()-2); //to char[] without quotes
		str[s.size()-2]='\0';
		DynamicArray* arr = new DynamicArray(s.size()-1,CHARACTER::getInstance());
		arr->fromString(str);
		let(arr);
	}
};

inline UnnamedLValue* ArrType::create() const{
	return new UnnamedLValue(new DynamicArray(size,type), new ArrType(size,type),true);
}

class Bconst: public Const {
public:
	Bconst(bool b):Const(BOOLEAN::getInstance() ),boo(b){}
	virtual Const* clone() override{return new Bconst(*this);}
	virtual void printOn(std::ostream &out) const override {
		out << "Bconst(" << boo <<")";
	}
	virtual value get_value() const override {
		value v; v.b=boo;
		return v;
	}
private:
	bool boo;
};

inline UnnamedLValue* BOOLEAN::create() const{
	return new UnnamedLValue(INTEGER::getInstance(),true);
}

class Op: public Expr {
public:
	Op(Expr *l, std::string o, Expr *r): left(l), op(o), right(r),
		leftType(nullptr), rightType(nullptr), resType(nullptr) {}
	Op(std::string o, Expr *l): left(l), op(o), right(nullptr) {}
	~Op() { delete left; delete right; }
	virtual void printOn(std::ostream &out) const override {
		if(right) out << op << "(" << *left << ", " << *right << ")";
		else  out << op << "(" << *left << ")";
	}

	virtual void sem() override{
		// sets leftType, rightType and resType fields
		// it should be run only once even when we have repeated evals
		// e.g in while
		Type* intType=INTEGER::getInstance();
		Type* realType=REAL::getInstance();
		Type* boolType=BOOLEAN::getInstance();
		left->sem();
		leftType=left->get_type();

		if(right!=nullptr){//BinOps
			right->sem();
			rightType=right->get_type();
			if(!(op.compare("+")) or !(op.compare("-")) or !(op.compare("*"))){
				//real or int operands-> real or int result
				if( (leftType->doCompare(realType) or leftType->doCompare(intType))
				and (rightType->doCompare(realType) or rightType->doCompare(intType)) ){
					//is a number (real or int)
					if(leftType->doCompare(realType) or rightType->doCompare(realType))
						// one of them is real
						resType=realType;
					else
						resType=intType;
				}
			}

			if(!(op.compare("/"))){
				//real or int operands-> real result
				if( (leftType->doCompare(realType) or leftType->doCompare(intType))
				and (rightType->doCompare(realType) or rightType->doCompare(intType)))
					//is a number (real or int)
					resType=realType;
			}

			if(!(op.compare("div")) or !(op.compare("mod")) ){
				//int operands-> int result
				if(  leftType->doCompare(intType) and rightType->doCompare(intType))
					//is int
					resType=intType;
			}

			if(!(op.compare("=")) or !(op.compare("<>"))){
				// either int/real operands or any non-array type operands
				// -> bool result
				if(( leftType->doCompare(realType) or leftType->doCompare(intType))
				and (rightType->doCompare(realType) or rightType->doCompare(intType)))
					// int/real operands
					resType=boolType;

				else if(leftType->doCompare(rightType)
				and leftType->get_name().compare("array") )
					// same type and not an array operands
					resType=boolType;
			}

			if(!(op.compare("<=")) or !(op.compare(">=")) or !(op.compare("<"))
					or !(op.compare(">")) ){
				//real or int operands-> bool result
				if( (leftType->doCompare(realType) or leftType->doCompare(intType))
				and (rightType->doCompare(realType) or rightType->doCompare(intType)))
					//is a number (real or int)
					resType=boolType;
			}

			if(!(op.compare("and")) or !(op.compare("or")) ){
				//bool operands-> bool result
				if(  leftType->doCompare(boolType) and rightType->doCompare(boolType))
					//is bool
					resType=boolType;
			}
			if(!(op.compare("[]"))){
				if(!(leftType->get_name().compare("array"))){
					if(rightType->doCompare(intType)){
						ArrType* p=static_cast<ArrType*>(leftType);
						resType=p->get_type();
					}
				}
			}
			if(!resType){
				/*TODO ERROR type mismatch*/
				std::cerr<<"ERROR: Type mismatch in Op->sem"<<std::endl;
				exit(1);
			}
			return;
		}
		//UNOP
		if(!(op.compare("+")) or !(op.compare("-")))
			//real or int operand-> real or int result
			if( leftType->doCompare(realType) or leftType->doCompare(intType) )
				resType=leftType;

		if(!(op.compare("not"))){
			//bool operand-> bool result
			if(leftType->doCompare(boolType) )
				resType=boolType;
		}
		if(!resType){
			/*TODO ERROR type mismatch*/
			std::cerr<<"ERROR: Type mismatch in Op->sem"<<std::endl;
			exit(1);
		}
		return;
	}

	virtual Const* eval() override {
		// before the first eval of any op, there must have been one typecheck to
		// fill resType, leftType, rightType
		Type* realType=REAL::getInstance();
		Type* intType=INTEGER::getInstance();
		Const *leftConst=left->eval();
		Const *rightConst=nullptr;
		Const* ret=nullptr;
		if(right){
			rightConst=right->eval();
		}
		if(!(op.compare("+")) and right){
			//BinOp
			int li=0,ri=0;
			double lr=0,rr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			if(resType->doCompare(realType)){
				ret = new Rconst(li+ri+lr+rr);
			}
			else{
				ret = new Iconst(li+ri);
			}
		}
		else if(!(op.compare("+"))){
			//UnOp
			int li=0;
			double lr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(resType->doCompare(realType)){
				ret = new Rconst(li+lr);
			}
			else{
				ret = new Iconst(li);
			}
		}
		else if(!(op.compare("-")) and right) {
		//BinOp
			int li=0,ri=0;
			double lr=0,rr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			if(resType->doCompare(realType)){
				ret = new Rconst(li-ri+lr-rr);
			}
			else{
				ret = new Iconst(li-ri);
			}
		}
		else if(!(op.compare("-"))){
			//UnOp
			int li=0;
			double lr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(resType->doCompare(realType)){
				ret = new Rconst(-li-lr);
			}
			else{
				ret = new Iconst(-li);
			}
		}
		else if(!(op.compare("*"))) {
		//BinOp
			int li=1,ri=1;
			double lr=1,rr=1;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			if(resType->doCompare(realType)){
				ret = new Rconst(li*ri*lr*rr);
			}
			else{
				ret = new Iconst(li*ri);
			}
		}
		else if(!(op.compare("/"))){
			int li=1,ri=1;
			double lr=1,rr=1;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			if(resType->doCompare(realType)){
				ret = new Rconst((li/rr)*(lr/ri));
			}
		}

		if( !(op.compare("div"))){
			value v=leftConst->get_value();
			int li=v.i;
			v=rightConst->get_value();
			int ri=v.i;
			ret = new Iconst(li/ri);
		}
		else if(!(op.compare("mod"))) {
			value v=leftConst->get_value();
			int li=v.i;
			v=rightConst->get_value();
			int ri=v.i;
			ret = new Iconst(li%ri);
		}
		else if(!(op.compare("<>"))) {
			int li=0,ri=0;
			double lr=0,rr=0;
			LValue* lptr, *rptr;
			bool isNumber=true;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else if(leftType->doCompare(intType)){
				v=leftConst->get_value();
				li=v.i;
			}
			else{
				v=leftConst->get_value();
				lptr=v.lval;
				isNumber=false;
			}

			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else if(rightType->doCompare(intType)) {
				v=rightConst->get_value();
				ri=v.i;
			}
			else{
				v=rightConst->get_value();
				rptr=v.lval;
			}
			if(isNumber)
				ret = new Bconst(li+lr!=ri+rr);
			else{
				ret = new Bconst(rptr!=lptr);
			}

		}
		else if(!(op.compare("="))) {
			int li=0,ri=0;
			double lr=0,rr=0;
			LValue* lptr, *rptr;
			bool isNumber=true;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else if(leftType->doCompare(intType)){
				v=leftConst->get_value();
				li=v.i;
			}
			else{
				v=leftConst->get_value();
				lptr=v.lval;
				isNumber=false;
			}

			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else if(rightType->doCompare(intType)) {
				v=rightConst->get_value();
				ri=v.i;
			}
			else{
				v=rightConst->get_value();
				rptr=v.lval;
			}
			if(isNumber)
				ret = new Bconst(li+lr==ri+rr);
			else{
				ret = new Bconst(rptr==lptr);
			}
		}

		else if(!(op.compare("<="))) {
			int li=0,ri=0;
			double lr=0,rr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			ret = new Bconst(li+lr<=ri+rr);
		}
		else if(!(op.compare(">="))) {
			int li=0,ri=0;
			double lr=0,rr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			ret = new Bconst(li+lr>=ri+rr);
		}
		else if(!(op.compare(">"))) {
			int li=0,ri=0;
			double lr=0,rr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			ret = new Bconst(li+lr > ri+rr);
		}
		else if(!(op.compare("<"))) {
			int li=0,ri=0;
			double lr=0,rr=0;
			value v;
			if(leftType->doCompare(realType)){
				v=leftConst->get_value();
				lr=v.r;
			}
			else{
				v=leftConst->get_value();
				li=v.i;
			}
			if(rightType->doCompare(realType)){
				v=rightConst->get_value();
				rr=v.r;
			}
			else{
				v=rightConst->get_value();
				ri=v.i;
			}
			ret = new Bconst(li+lr < ri+rr);
		}
		else if(!(op.compare("and"))) {
			value v=leftConst->get_value();
			bool lb=v.b;
			v=rightConst->get_value();
			bool rb=v.b;
			ret = new Bconst(lb and rb);
		}
		else if(!(op.compare("or"))) {
			value v=leftConst->get_value();
			bool lb=v.b;
			v=rightConst->get_value();
			bool rb=v.b;
			ret = new Bconst(lb or rb);
		}
		else if(!(op.compare("not"))) {
			//UnOp

			value v;
			v=leftConst->get_value();
			bool lb=v.b;
			ret = new Bconst(not lb);
		}
		delete leftConst;
		if(right) delete rightConst;
		return ret;
	}
	virtual Type* get_type() override{
		return resType;
	}
private:
	Expr *left;
	std::string op;
	Expr *right;
	Type *leftType,*rightType,*resType;
};

class Reference: public Expr{
public:
	Reference(LValue* lval):lvalue(lval){}
	virtual void printOn(std::ostream &out) const override {
		out << "Reference" << "(" << *lvalue << ")";
	}
	virtual void sem() override{
		lvalue->sem();
	}
	virtual Type* get_type(){
		return new PtrType(lvalue->get_type());
	}
	virtual Const* eval(){
		return new Pconst(lvalue,lvalue->get_type());
	}
protected:
	LValue* lvalue;
};

class Dereference: public LValue{
public:
	Dereference(Expr *e):expr(e){}
	virtual void printOn(std::ostream &out) const override {
		out << "Dereference" << "(" << *expr << ")";
	}
	virtual void sem() override{
		expr->sem();
		Type* ty=expr->get_type();
		if(ty->get_name().compare("pointer")){
			//TODO error incorrect type
			std::cerr << "ERROR: Can only dereference pointer (not "<<
			ty->get_name()<<")" <<std::endl;
		}
	}
	virtual Const* eval(){
		Const* c = expr->eval();
		value v = c->get_value();
		return (v.lval)->eval();
	}

	virtual Type* get_type(){
		Type* con_ty=expr->get_type();
		PtrType* p_ty=static_cast<PtrType*>(con_ty);
		return p_ty->get_type();
	}

	virtual void let(Const* c){
		Const* con = expr->eval();
		value v = con->get_value();
		(v.lval)->let(c);
	}

protected:
	Expr *expr;
};

class Brackets: public LValue{
public:
	Brackets(LValue *lval, Expr* e):lvalue(lval),expr(e){}
	virtual void printOn(std::ostream &out) const override {
		out << "Brackets" << "(" << *lvalue<< ", " << *expr << ")";
	}
	virtual void sem() override{
		lvalue->sem();
		expr->sem();
		Type* l_ty = lvalue->get_type();
		if(l_ty->get_name().compare("array")){
			//TODO error incorrect type
			std::cerr << "ERROR: Can only apply brakets to array (not "<<
			l_ty->get_name()<<")" <<std::endl;
			exit(1);
		}
		if(!expr->get_type()->doCompare(INTEGER::getInstance())){
			//TODO error incorrect type for array index should be int
			std::cerr << "ERROR: Array index should be int!"<<std::endl;
			exit(1);
		}
	}

	virtual Const* eval(){
		return element()->eval();
	}
	virtual void let(Const* c){
		element()->let(c);
	}
	virtual Type* get_type(){
		Type* l_ty = lvalue->get_type();
		ArrType* larr_ty=static_cast<ArrType*>(l_ty);
		return larr_ty->get_type();
	}
protected:
	LValue* element(){
		value v = expr->eval()->get_value();
		int i = v.i;
		Const* c = lvalue->eval();
		Arrconst *arr = static_cast<Arrconst*>(c);
		return arr->get_element(i);
	}
	LValue* lvalue;
	Expr* expr;
};

class Let: public Stmt { //TODO semantics
public:
	Let(LValue* lval,Expr* e):lvalue(lval),expr(e),different_types(false){}
	~Let(){delete lvalue; delete expr;}
	virtual void printOn(std::ostream &out) const override {
		out << "Let(" << *lvalue << ":=" << *expr << ")";
	}
	virtual void sem() override{
		expr->sem();
		lvalue->sem();
		Type* lType = lvalue->get_type();
		Type* rType = expr->get_type();
		if(lType->doCompare(rType)) return;
		else different_types=true;
		if(lType->doCompare(REAL::getInstance()) and rType->doCompare(INTEGER::getInstance()))
			return;
		if(!(lType->get_name().compare("pointer")) and !(rType->get_name().compare("pointer"))){
			PtrType* lpType=static_cast<PtrType*>(lType);
			PtrType* rpType=static_cast<PtrType*>(rType);
			Type* linType=lpType->get_type();
			Type* rinType=rpType->get_type();
			if(!(linType->get_name().compare("array")) and !(rinType->get_name().compare("array"))){
				ArrType* larrType=static_cast<ArrType*>(linType);
				ArrType* rarrType=static_cast<ArrType*>(rinType);
				if(rarrType->get_size()!=-1 && larrType->get_size()==-1){
					if(larrType->get_type()->doCompare(rarrType->get_type()))
						return;
				}
			}
		}
		/*TODO error type mismatch*/
		std::cerr<<"ERROR: Type mismatch in let!"<<std::endl;
		exit(1);
	}

	virtual void run() const override{
		Const* c = expr->eval();
		if(!different_types){
			lvalue->let(c);
		}
		else{
			Const* n=c->copyToType();
			lvalue->let(n);
			delete c;
		}
	}
private:
	LValue  *lvalue;
	Expr *expr;
	bool different_types;
};

class If: public Stmt {
public:
	If(Expr* e,Stmt* s1, Stmt* s2):expr(e),stmt1(s1),stmt2(s2){}
	virtual void printOn(std::ostream &out) const override {
		if (stmt2)
		out << "If(" << *expr << "then" << *stmt1 << "else" << *stmt2 << ")";
		else
		out << "If(" << *expr << "then" << *stmt1 << ")";
	}
	virtual void sem() override{
		expr->sem();
		if(!(expr->get_type()==BOOLEAN::getInstance())){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in if statement!"
				<<std::endl;
				exit(1);
		}
		stmt1->sem();
		if(stmt2)
			stmt2->sem();
	}
	virtual void run() const override{
		Const * c= expr->eval();
		value v=c->get_value();
		bool e = v.b;
		if(e) stmt1->run();
		else if (stmt2) stmt2->run();
	}
private:
	Expr *expr;
	Stmt *stmt1,*stmt2;
};

class While: public Stmt {
public:
	While(Expr* e,Stmt* s):expr(e),stmt(s){}
	virtual void printOn(std::ostream &out) const override {
		out << "While(" << *expr << "do" << *stmt << ")";
	}
	virtual void sem() override{
		expr->sem();
		if(!(expr->get_type()==BOOLEAN::getInstance())){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in while statement!"
				<<std::endl;
			exit(1);
		}
		stmt->sem();
	}
	virtual void run() const override{
		Const * c= expr->eval();
		value v=c->get_value();
		bool e = v.b;
		while(e) {
			stmt->run();
			c= expr->eval();
			value v=c->get_value();
			e = v.b;
		}
	}
private:
	Expr *expr;
	Stmt *stmt;
};

class New: public Stmt{
public:
	New(LValue* lval, Expr* e):expr(e),lvalue(lval){}
	~New(){delete expr; delete lvalue;}
	virtual void printOn(std::ostream &out) const override {
		if(expr)
			out << "New( [" << *expr << "] " << *lvalue << ")";
		else
			out << "New( [] of " << *lvalue << ")";
	}

	virtual void sem() override{
		lvalue->sem();
		if(expr){
			expr->sem();
			if(!(expr->get_type()->doCompare(INTEGER::getInstance())) ){
				/*TODO ERROR incorrect type*/
				std::cerr<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
				exit(1);
			}
			Type* idType=lvalue->get_type();
			if(idType->get_name().compare("pointer") ){
				/*TODO ERROR incorrect type*/
				std::cerr<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
				exit(1);
			}
			PtrType* p=static_cast<PtrType*>(idType);
			Type* t=p->get_type();
			if(t->get_name().compare("array") ){
				/*TODO ERROR incorrect type*/
				std::cerr<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
				exit(1);
			}
		}
		else{
			Type* idType=lvalue->get_type();
			if(idType->get_name().compare("pointer") ){
				/*TODO ERROR incorrect type*/
				std::cerr<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
				exit(1);
			}
		}
	}
	virtual void run() const override{
		if(expr){
			Const* c= expr->eval();
			value v = c->get_value();
			int i = v.i;
			if(i<=0) {
				/*TODO ERROR wrong value*/
				std::cerr<<
					"ERROR: Wrong value for array size in New statement!"
					<<std::endl;
				exit(1);
			}
			Type* idType=lvalue->get_type();
			PtrType* p=static_cast<PtrType*>(idType);
			Type* t=p->get_type();
			ArrType* arrT = static_cast<ArrType*>(t);
			ArrType* arrT_size = new ArrType(i,arrT->get_type());
			Pconst *ret = new Pconst(arrT_size->create(), arrT_size );
			lvalue->let(ret);
		}
		else{
			Type* idType=lvalue->get_type();
			PtrType* p=static_cast<PtrType*>(idType);
			Type* t=p->get_type();
			Pconst *ret = new Pconst(t->create(), t);
			lvalue->let(ret);
		}
	}

protected:
	Expr* expr;
	LValue* lvalue;
};


class Dispose: public Stmt{
public:
	Dispose(LValue* lval):lvalue(lval){}
	~Dispose(){delete lvalue;}
	virtual void sem() override{
		lvalue->sem();
		if(lvalue->get_type()->get_name().compare("pointer")){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in Dispose statement!"
				<<std::endl;
			exit(1);
		}
	}
	virtual void run() const{
		Const *c = lvalue->eval();
		value v = c->get_value();
		LValue* ptr = v.lval;
		if(!(ptr->isDynamic())){
			/*TODO ERROR non dynamic pointer (RUNTIME)*/
			std::cerr<<
				"ERROR: non dynamic pointer in Dispose (RUNTIME)"
				<<std::endl;
			exit(1);
		}
		delete ptr;
		lvalue->let(new Pconst());
	}
	virtual void printOn(std::ostream &out) const override {
			out << "Dispose( " << *lvalue << ")";
	}
protected:
	LValue *lvalue;
};

class DisposeArr: public Stmt{
public:
	DisposeArr(LValue* lval):lvalue(lval){}
	~DisposeArr(){delete lvalue;}
	virtual void sem() override{
		lvalue->sem();
		Type* t=lvalue->get_type();
		if(t->get_name().compare("pointer")){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in DisposeArr statement!"
				<<std::endl;
			exit(1);
		}
		PtrType *pt = static_cast<PtrType*>(t);
		if(pt->get_name().compare("array")){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in DisposeArr statement!"
				<<std::endl;
			exit(1);
		}
	}
	virtual void run() const{
		Const *c = lvalue->eval();
		value v = c->get_value();
		LValue* ptr = v.lval;
		if(!(ptr->isDynamic())){
			/*TODO ERROR non dynamic pointer (RUNTIME)*/
			std::cerr<<
				"ERROR: non dynamic pointer in DisposeArr (RUNTIME)"
				<<std::endl;
			exit(1);
		}
		delete ptr;
		lvalue->let(new Pconst());
	}
	virtual void printOn(std::ostream &out) const override {
			out << "Dispose[]( " << *lvalue << ")";
	}
protected:
	LValue *lvalue;
};

template<class T>
std::ostream& operator <<(std::ostream &out,const std::map<std::string,T*> m) {
	out<<"[";
	for(auto p :m)
		out<<"<"<<p.first<<": "<<*(p.second)<<">,";
	out<<"]";
	return out;
}

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
	virtual void sem() {
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

	uint size(){
		return list.size();
	}
	friend class Body;
protected:
	std::vector<T*> list;
};

class StmtList: public List<Stmt>, public Stmt{
public:
	StmtList(Stmt *s):List<Stmt>(s){}
	StmtList():List<Stmt>(){}
	virtual void printOn(std::ostream &out) const override {
		out << "StmtList(" << list << ")";
	}
	virtual void sem() override{
		for(auto p:list){
			p->sem();
		}
	}
	virtual void run() const{
		for(auto p:list){
			if(p->isReturn())
				return;
			p->run();
		}
	}
};
class ExprList: public List<Expr> {
public:
	ExprList():List<Expr>(){}
	ExprList(Expr *e):List<Expr>(e){}
	virtual void printOn(std::ostream &out) const override {
		out << "ExprList(" << list << ")";
	}
	virtual std::vector<Expr*> eval(std::vector<bool> by_ref){
		std::vector<Expr*> ret(list.size());
		Const *c;
		for(uint i=0; i<list.size(); i++){
			if(by_ref[i]){
				ret.push_back(list[i]);
			}
			else{
				c=list[i]->eval();
				ret.push_back(c);
			}
		}
		return ret;
	}

	virtual std::vector<Type*> get_type(){
		std::vector<Type*> ret(list.size());
		for(auto p:list){
			ret.push_back(p->get_type());
		}
		return ret;
	}

	virtual std::vector<bool> get_isLValue(){
		std::vector<bool> ret(list.size());
		for(auto p:list){
			ret.push_back(p->isLValue());
		}
		return ret;
	}

private:
	std::vector<Expr*> list;
};


class Local: public AST{

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
	LabelDecl(Decl* d):Decl(d->get_id(),"label"){delete d;}
	virtual void printOn(std::ostream &out) const override {
		out << "LabelDecl("<<id<<")";
	}
	virtual Type* get_type() override{
		return LABEL::getInstance();
	}
	virtual void sem() override{
		st.insert(id,LABEL::getInstance());
	}
};
class VarDecl: public Decl{
public:
	VarDecl(Decl* d):Decl(d->get_id(),"var"),type(nullptr){delete d;}
	VarDecl(Decl* d,Type* t):Decl(d->get_id(),"var"),type(t){delete d;}
	virtual void printOn(std::ostream &out) const override {
		if(type)
		out << "VarDecl(" <<id<<" of type "<< *type << ")";
		else
		out << "VarDecl(" <<id<<" of type NOTSET)";
	}
	virtual Type* get_type() override{
		return type->clone();
	}
	virtual void sem() override{
		int s=get_sizeof(type);
		st.insert(id,type,s);
	}
	void set_type(Type* ty){type=ty;}
protected:
	Type* type;
	int get_sizeof(Type* t){
		if(!t->get_name().compare("array")){
			// t is ArrType
			ArrType* arr_type=static_cast<ArrType*>(t);
			int s=arr_type->get_size();
			if(s>0){
				// declared arrays of FIXED size should
				//   take more space in stack
				Type* inside=arr_type->get_type();
				return s*get_sizeof(inside)+1;
			}
		}
		// non-static-array types have size 1 in stack
		return 1;
	}
};

class FormalDecl: public VarDecl{
public:
	FormalDecl(Decl *d, bool ref):VarDecl(d), byRef(ref){}
	FormalDecl(Decl* d,Type* t,bool ref):VarDecl(d,t), byRef(ref){}
	bool isByRef(){return byRef;}
protected:
	bool byRef;
};

class DeclList: public List<Decl>{
public:
	DeclList(Decl* d):List<Decl>(d){}
	DeclList():List<Decl>(){}
	void toVar(Type* t){
		for(auto p=list.begin();p!=list.end();p++){
			Decl *d=new VarDecl(*p,t);
			*p=d;
		}
	}
	void sem() const{
		for(auto p:list)
			p->sem();
	}

	void toLabel(){
		for(auto p=list.begin();p!=list.end();p++){
			Decl *d=new LabelDecl(*p);
			*p=d;
		}
	}
	void toFormal(Type* t, bool ref){
		for(auto p=list.begin();p!=list.end();p++){
			Decl *d=new FormalDecl(*p,t,ref);
			*p=d;
		}
	}
	std::vector<Type*> get_type(){
		std::vector<Type*> types;
		for(auto p=list.begin();p!=list.end();p++){
			types.push_back((*p)->get_type());
		}
		return types;
	}
};
class FormalDeclList: public DeclList{
public:
	std::vector<bool> get_by_ref(){
		std::vector<bool> by_ref;
		for(auto p=list.begin();p!=list.end();p++){
			FormalDecl* f=static_cast<FormalDecl*>(*p);
			by_ref.push_back(f->isByRef());
		}
		return by_ref;
	}
};
inline void print_stack(){
	std::cout<<"STACK:"<<std::endl;
	for(auto p : rt_stack ){
		if(p)
			std::cout<<*p;
		else
			std::cout<<"EMPTY";
	std::cout<<std::endl;
	}
}


class Body: public AST{
public:
	Body():declarations(nullptr),statements(nullptr),size(0),defined(false){}
	Body(DeclList* d, StmtList* s):declarations(d),statements(s),size(0),defined(true){}
	~Body(){delete declarations; delete statements;}

	virtual void sem() override{
		if(!isDefined()){
			return;
		}
		declarations->sem();
		statements->sem();
		size = st.getSizeOfCurrentScope();
	}
	void add_body(Body *b){
		defined=true;
		declarations=b->declarations;
		statements=b->statements;
		// b->declarations=nullptr;
		// b->statements=nullptr;
		// delete b;
	}

	void run() const{
		statements->run();
		print_stack();
	}
	bool isDefined(){
		return defined;
	}

	virtual void printOn(std::ostream &out) const override {
		out << "Body("<<*declarations<<","<<*statements<<")";
	}

	int get_size(){
		return size;
	}
protected:
	DeclList* declarations;
	StmtList* statements;
	int size;
	bool defined;
};

class Procedure:public Decl{
public:
	Procedure(std::string name, DeclList *decl_list, Body* bod, std::string decl_type="procedure"):
		Decl(name,decl_type), body(bod),formals(static_cast<FormalDeclList*>(decl_list)){}
		void add_body(Body* bod){
			if(body->isDefined()){
				std::cerr<<id<<" already has body."<<std::endl;
				exit(1);
			}
			body->add_body(bod);
		}
		virtual void sem() override{
			std::vector<Type*> formal_types=formals->get_type();
			std::vector<bool> by_ref=formals->get_by_ref();
			if(!body->isDefined()){
				CallableType *proc_type=new ProcedureType(formal_types, by_ref);
				st.insert_function(id,proc_type,body);
				return;
			}
			FunctionEntry* e = st.function_decl_lookup(id);
			if(e and e->body->isDefined()){
				std::cerr<<"Procedure "<<id<<" already fully declared."<<std::endl;
				exit(1);
			}
			if(e){
				if(e->type->get_name().compare(decl_type)){
					std::cerr<<"Previous declaration of "<<id<<" is not a "<<decl_type<<"."<<std::endl;
					exit(1);
				}
				ProcedureType* proc_type=static_cast<ProcedureType*>(e->type);
				proc_type->typecheck_args(formal_types);
				proc_type->check_passing(by_ref);
				e->body->add_body(body);
			}
			else{
				CallableType *proc_type=new ProcedureType(formal_types, by_ref);
				st.insert_function(id, proc_type, body);
			}

			st.openScope();
			formals->sem();
			body->sem();
			st.closeScope();
		}
protected:
	Body* body;
	FormalDeclList* formals;
};

class Function:public Procedure{
public:
	Function(std::string name, DeclList *decl_list, Type* return_type, Body* bod)
		:Procedure(name,decl_list,bod,"function"), ret_type(return_type){}


	virtual void sem() override{
		std::vector<Type*> formal_types=formals->get_type();
		std::vector<bool> by_ref=formals->get_by_ref();
		if(!body->isDefined()){
			CallableType *func_type=new FunctionType(ret_type, formal_types, by_ref);
			st.insert_function(id,func_type,body);
			return;
		}
		FunctionEntry* e = st.function_decl_lookup(id);
		if(e and e->body->isDefined()){
			std::cerr<<"Function "<<id<<" already fully declared."<<std::endl;
			exit(1);
		}
		if(e){
			if(e->type->get_name().compare(decl_type)){
				std::cerr<<"Previous declaration of "<<id<<" is not a "<<decl_type<<"."<<std::endl;
				exit(1);
			}
			FunctionType* func_type=static_cast<FunctionType*>(e->type);
			Type* ret_t=func_type->get_ret_type();
			if(!ret_t->doCompare(ret_type)){
				std::cerr<<"Can't declare function "<<id<<" with different return type."<<std::endl;
				exit(1);
			}
			func_type->typecheck_args(formal_types);
			func_type->check_passing(by_ref);
			e->body->add_body(body);
		}
		else{
			CallableType *func_type=new FunctionType(ret_type, formal_types, by_ref);
			st.insert_function(id, func_type, body);
		}

		st.openScope();
		VarDecl v(new Decl("result","var"),ret_type); v.sem();
		formals->sem();
		body->sem();
		st.closeScope();
	}

protected:
	Type* ret_type;

};

class Program: public AST{
public:
	Program(std::string nam, Body* bod):name(nam),body(bod),size(0){}
	virtual void printOn(std::ostream &out) const override {
		out << "Program(" << name <<" ::: "<<*body<< ")";
	}
		virtual void sem() override{
			st.openScope();
			body->sem();
			size=st.getSizeOfCurrentScope();
			st.closeScope();
		}
		void run(){
			fp=0;
			for(int i=0; i<body->get_size()+1; i++){
				rt_stack.push_back(new UnnamedLValue(nullptr));
			}
			body->run();
			for(int i=0; i<body->get_size()+1; i++){
				delete rt_stack.back();
				rt_stack.pop_back();
			}
		}
private:
	std::string name;
	Body* body;
	int size;
};

class Call{
public:
	Call(std::string nam, ExprList* exp): name(nam), exprs(exp),
		by_ref(exp->size()), nesting_diff(0), next_fp_offset(0), body(nullptr){}
protected:
	std::string name;
	ExprList* exprs;
	std::vector<bool> by_ref;
	int nesting_diff;
	int next_fp_offset;
	Body* body;

	FunctionEntry* check_passing(){
		std::vector<Type*> types=exprs->get_type();
		FunctionEntry* e = st.function_lookup(name);
		e->type->typecheck_args(types);
		by_ref=e->type->get_by_ref();
		std::vector<bool> isLValue (exprs->get_isLValue());
		for(uint i=0; i<by_ref.size(); i++){
			if(by_ref[i] and not isLValue[i]){
				std::cerr<<"Argument "<<i<<" is wrong side value."<<std::endl;
				exit(1);
			}
		}
		nesting_diff=st.getNestingOfCurrentScope()-e->nesting;
		next_fp_offset=st.getSizeOfCurrentScope()+1;
		return e;

	}

	void before_run(bool isFunction=false) const{
		int next_fp=fp+next_fp_offset;
		if(nesting_diff<0){
			rt_stack[next_fp]=new UnnamedLValue(new Iconst(fp),INTEGER::getInstance());
		}
		else{
			int prev_fp=fp;
			for(int diff=nesting_diff; diff>0; diff--){
				prev_fp=rt_stack[prev_fp]->eval()->get_value().i;
			}
			Const* c=rt_stack[prev_fp]->eval();
			rt_stack[next_fp]=new UnnamedLValue(c,INTEGER::getInstance());

		}
		if(isFunction){
			// push one more value for function result
			rt_stack.push_back(new UnnamedLValue(nullptr));
		}
		std::vector<Expr*> args(exprs->eval(by_ref));
		for(uint i=0; i<args.size(); i++){
			if(by_ref[i]){
				rt_stack.push_back(static_cast<LValue*>(args[i]));
			}
			else{
				rt_stack.push_back(new UnnamedLValue(static_cast<Const*>(args[i]),args[i]->get_type()));
			}
		}
		int size=body->get_size()-exprs->size()-1;
		for (int i = 0; i < size; ++i) rt_stack.push_back(new UnnamedLValue(nullptr));
		rt_stack[next_fp-1]=new UnnamedLValue(new Iconst(fp),INTEGER::getInstance());
		fp=next_fp;
	}

	void after_run() const{
		int next_fp=fp;
		fp=rt_stack[next_fp-1]->eval()->get_value().i;
		int size=next_fp_offset-exprs->size()-1;
		for (int i = 0; i < size; ++i) {delete rt_stack.back(); rt_stack.pop_back();}
		for(uint i=by_ref.size()-1; i>=0; i--){
			if(!by_ref[i]){
				delete rt_stack.back();
			}
			rt_stack.pop_back();
		}
		delete rt_stack[next_fp];
		rt_stack.pop_back();
		delete rt_stack[next_fp-1];
		rt_stack.pop_back();
	}


};

class ProcCall: public Call, public Stmt{
public:
	ProcCall(std::string nam, ExprList* exp):Call(nam, exp){}
	virtual void sem() override{
		FunctionEntry* e =check_passing();
		if(e->type->get_name().compare("procedure")){
			std::cerr<<"Can't call function "<<name<<" as a procedure."<<std::endl;
			exit(1);
		}
	}
	virtual void run() const override{
		before_run();
		body->run();
		after_run();
	}
	virtual void printOn(std::ostream &out) const override {
		out << "ProcCall(" << name<<", args:"<< *exprs<< ")";
	}
};

class FunctionCall: public Call, public Expr{
public:
	FunctionCall(std::string nam, ExprList* exp):Call(nam,exp), type(nullptr){}
	virtual void sem() override{
		FunctionEntry* e = check_passing();
		if(e->type->get_name().compare("function")){
			std::cerr<<"Can't call procedure "<<name<<" as a function."<<std::endl;
			exit(1);
		}
		type=static_cast<FunctionType*>(e->type)->get_ret_type();
	}
	virtual Type* get_type(){
		return type;
	}

	virtual Const* eval() override{
		before_run(true); // flag for function
		body->run();
		Const* res = rt_stack[fp+1]->eval();
		delete rt_stack[fp+1];
		after_run();
		rt_stack.pop_back();
		return res;
	}
	virtual void printOn(std::ostream &out) const override {
		out << "FunctionCall(" << name<<"with return type "<<*type<<", args:"<< *exprs<< ")";
	}
private:
	Type* type;
};
