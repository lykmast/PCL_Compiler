#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>

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
	~PtrType(){if(type->should_delete()) delete type;}
	virtual UnnamedLValue* create() const;
	Type* get_type(){ return type;}
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
	~ArrType(){if(type->should_delete()) delete type;}
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

// const Type * INTEGER=new Type("integer"),* REAL=new Type("real"),
// *BOOLEAN=new Type("boolean"), CHARACTER=new Type("char");


extern std::map<std::string, Const*> globals; // map variable names to values
extern std::map<std::string, Type*> declared; // map variable names to values


class Expr: public AST {
public:
	virtual Const* eval() = 0;
	virtual Type* get_type()=0;
};

class Stmt: virtual public AST {
public:
	virtual void run() const{}; //empty stmt -> do nothing
	virtual void printOn(std::ostream &out) const override {
		out << "Stmt()";
	}
};

class Const: public Expr{
public:
	Const(Type* ty):type(ty){}
	~Const(){if(type->should_delete()) delete type;}
	virtual Type* get_type() override{return type;}
	virtual void printOn(std::ostream &out) const =0;
	virtual Const* eval() override{return this;}
	virtual value get_value() const=0;
	virtual Const* copyToType() const {return nullptr;}
protected:
	Type* type;
};



class LValue: public Expr{
public:
	LValue(bool dyn=false):dynamic(dyn){}
	virtual void let(Const* c)=0;
	bool isDynamic(){return dynamic;}
protected:
	bool dynamic;
};


class Id: public LValue {
public:
	Id(std::string v): name(v) {}
	virtual void printOn(std::ostream &out) const override {
		out << "Id(" << name << ")";
	}
	virtual Const* eval() override {
		if(!globals_lookup()){//TODO error or uninitialized value
			std::cout<<"ERROR: Id '"<<name<<"' not initialized!"<<std::endl;
			return nullptr;/*TODO throw error not found*/
		}
		return globals[name];
	}

	virtual void sem() override{
		if(!declared_lookup()){
			//TODO throw error variable not declared
			std::cout<<"Id '"<<name<<"' not declared"<<std::endl;
		}
	}

	virtual void let(Const* c){
		bool hasValue=globals_lookup();
		if(hasValue) delete globals[name];
		globals[name]=c;
	}
	virtual Type* get_type(){
		return declared[name];
	}
	std::string get_name(){
		return name;
	}
private:
	std::string name;

	bool declared_lookup(){//returns true if the id is declared
		std::map<std::string,Type*>::iterator it;
		it=declared.find(name);
		if(it==declared.end()){
			return false;
		}
		return true;
	}
	bool globals_lookup(){//returns true if the id already exists in globals
		std::map<std::string,Const*>::iterator it;
		it=globals.find(name);
		if(it==globals.end()){
			return false;
		}
		return true;
	}
};

class UnnamedLValue: public LValue{
public:
	UnnamedLValue(Type *ty,bool dyn=false):LValue(dyn),value(nullptr),type(ty){}
	UnnamedLValue(Const* val, Type* ty,bool dyn=false):LValue(dyn),value(val),type(ty){}
	~UnnamedLValue(){delete value;}
	virtual void printOn(std::ostream &out) const override {
		if(value)
			out << "UnnamedLValue(" << *value<<","<<*type << ")";
		else
			out << "UnnamedLValue(EMPTY, "<<*type << ")";
	}
	virtual void let(Const* c) override{
		if(value) delete value;
		value=c;
	}
	virtual Const* eval() override{
		return value;
	}
	virtual Type* get_type(){
		return type;
	}
protected:
	Const* value;
	Type* type;
};



class Rconst: public Const {
public:
	Rconst(double n):Const(REAL::getInstance() ),num(n){}
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
	Pconst(LValue* pval,Type *t):Const(new PtrType(t) ),ptr(pval){}
	virtual void printOn(std::ostream &out) const override {
		out << "Pconst(" << ptr << "of type "<<*type<< ")";
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
	Arrconst(int s, Type* t):Const(  t ),size(s){
		if(s<0){
			/*TODO error wrong value*/
			std::cout<<"ERROR: "<< s <<" is bad size for array!"<<std::endl;
		}
		arr.resize(s);
		for(int i=0;i<s;i++){
			arr[i]=new UnnamedLValue(t);
		}
	}
	~Arrconst(){
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
		out << "Arrconst([" << arr <<"] ["<<size<<"]"<<"of type "<< *type << ")";
	}
	virtual value get_value() const{
		value v;
		v.lval=arr[0];
		return v;
	}
protected:
	std::vector<UnnamedLValue*> arr;
	int size;
};

class Sconst: public UnnamedLValue {
public:
	Sconst(std::string s):UnnamedLValue(new ArrType(s.size()-1,CHARACTER::getInstance())) {
		char* str=(char*)(malloc(sizeof(char)*(s.size()-1)));
		s.substr(1,s.size()-2).copy(str,s.size()-2); //to char[] without quotes
		str[s.size()-2]='\0';
		Arrconst* arr = new Arrconst(s.size()-1,CHARACTER::getInstance());
		arr->fromString(str);
		let(arr);
	}
};

inline UnnamedLValue* ArrType::create() const{
	return new UnnamedLValue(new Arrconst(size,type), new ArrType(size,type),true);
}

class Bconst: public Const {
public:
	Bconst(bool b):Const(BOOLEAN::getInstance() ),boo(b){}
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
				std::cout<<"ERROR: Type mismatch in Op->sem"<<std::endl;
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
			std::cout<<"ERROR: Type mismatch in Op->sem"<<std::endl;
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
				return new Rconst(li+ri+lr+rr);
			}
			else{
				return new Iconst(li+ri);
			}
		}
		if(!(op.compare("+"))){
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
				return new Rconst(li+lr);
			}
			else{
				return new Iconst(li);
			}
		}
		if(!(op.compare("-")) and right) {
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
				return new Rconst(li-ri+lr-rr);
			}
			else{
				return new Iconst(li-ri);
			}
		}
		if(!(op.compare("-"))){
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
				return new Rconst(-li-lr);
			}
			else{
				return new Iconst(-li);
			}
		}
		if(!(op.compare("*"))) {
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
				return new Rconst(li*ri*lr*rr);
			}
			else{
				return new Iconst(li*ri);
			}
		}
		if(!(op.compare("/"))){
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
				return new Rconst((li/rr)*(lr/ri));
			}
		}

		if( !(op.compare("div"))){
			value v=leftConst->get_value();
			int li=v.i;
			v=rightConst->get_value();
			int ri=v.i;
			return new Iconst(li/ri);
		}
		if(!(op.compare("mod"))) {
			value v=leftConst->get_value();
			int li=v.i;
			v=rightConst->get_value();
			int ri=v.i;
			return new Iconst(li%ri);
		}
		if(!(op.compare("<>"))) {
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
				return new Bconst(li+lr!=ri+rr);

			return new Bconst(rptr!=lptr);
		}
		if(!(op.compare("="))) {
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
				return new Bconst(li+lr==ri+rr);

			return new Bconst(rptr==lptr);
		}

		if(!(op.compare("<="))) {
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
			return new Bconst(li+lr<=ri+rr);
		}
		if(!(op.compare(">="))) {
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
			return new Bconst(li+lr>=ri+rr);
		}
		if(!(op.compare(">"))) {
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
			return new Bconst(li+lr > ri+rr);
		};
		if(!(op.compare("<"))) {
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
			return new Bconst(li+lr < ri+rr);
		}
		if(!(op.compare("and"))) {
			value v=leftConst->get_value();
			bool lb=v.b;
			v=rightConst->get_value();
			bool rb=v.b;
			return new Bconst(lb and rb);
		}
		if(!(op.compare("or"))) {
			value v=leftConst->get_value();
			bool lb=v.b;
			v=rightConst->get_value();
			bool rb=v.b;
			return new Bconst(lb or rb);
		}
		if(!(op.compare("not"))) {
			//UnOp

			value v;
			v=leftConst->get_value();
			bool lb=v.b;
			return new Bconst(not lb);
		}
		return 0;  // this will never be reached.
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
			std::cout << "ERROR: Can only dereference pointer (not "<<
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
			std::cout << "ERROR: Can only apply brakets to array (not "<<
			l_ty->get_name()<<")" <<std::endl;
		}
		if(!expr->get_type()->doCompare(INTEGER::getInstance())){
			//TODO error incorrect type for array index should be int
			std::cout << "ERROR: Array index should be int!"<<std::endl;
		}
	}
	virtual Const* eval(){
		return element()->eval();
	}
	virtual void let(Const* c){
		element()->let(c);
	}
	virtual Type* get_type(){
		return element()->get_type();
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
		std::cout<<"ERROR: Type mismatch in let!"<<std::endl;
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
			std::cout<<
				"ERROR: Incorrect type of expression in if statement!"
				<<std::endl;
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
			std::cout<<
				"ERROR: Incorrect type of expression in while statement!"
				<<std::endl;
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
				std::cout<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
			}
			Type* idType=lvalue->get_type();
			if(idType->get_name().compare("pointer") ){
				/*TODO ERROR incorrect type*/
				std::cout<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
			}
			PtrType* p=static_cast<PtrType*>(idType);
			Type* t=p->get_type();
			if(t->get_name().compare("array") ){
				/*TODO ERROR incorrect type*/
				std::cout<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
			}
		}
		else{
			Type* idType=lvalue->get_type();
			if(idType->get_name().compare("pointer") ){
				/*TODO ERROR incorrect type*/
				std::cout<<
					"ERROR: Incorrect type of expression in New statement!"
					<<std::endl;
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
				std::cout<<
					"ERROR: Wrong value for array size in New statement!"
					<<std::endl;
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
			std::cout<<
				"ERROR: Incorrect type of expression in Dispose statement!"
				<<std::endl;
		}
	}
	virtual void run() const{
		Const *c = lvalue->eval();
		value v = c->get_value();
		LValue* ptr = v.lval;
		if(!(ptr->isDynamic())){
			/*TODO ERROR non dynamic pointer (RUNTIME)*/
			std::cout<<
				"ERROR: non dynamic pointer in Dispose (RUNTIME)"
				<<std::endl;
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
			std::cout<<
				"ERROR: Incorrect type of expression in DisposeArr statement!"
				<<std::endl;
		}
		PtrType *pt = static_cast<PtrType*>(t);
		if(pt->get_name().compare("array")){
			/*TODO ERROR incorrect type*/
			std::cout<<
				"ERROR: Incorrect type of expression in DisposeArr statement!"
				<<std::endl;
		}
	}
	virtual void run() const{
		Const *c = lvalue->eval();
		value v = c->get_value();
		LValue* ptr = v.lval;
		if(!(ptr->isDynamic())){
			/*TODO ERROR non dynamic pointer (RUNTIME)*/
			std::cout<<
				"ERROR: non dynamic pointer in DisposeArr (RUNTIME)"
				<<std::endl;
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
		for(auto p:list)
			p->run();
	}
};
class ExprList: public List<Expr> {
public:
	ExprList():List<Expr>(){}
	ExprList(Expr *e):List<Expr>(e){}
	virtual void printOn(std::ostream &out) const override {
		out << "ExprList(" << list << ")";
	}
	virtual std::vector<Const*> eval(){
		std::vector<Const*> ret(list.size());
		Const *c;
		for(auto p:list){
			c=p->eval();
			ret.push_back(c);
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
	virtual void sem() override{
		declared[id]=LABEL::getInstance();
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
	virtual void sem() override{
		declared[id]=type;
		if(!type->get_name().compare("array")){
			// declared arrays of FIXED size should be initialized
			//   with empty cells (new Arrconst)
			ArrType* arrT = static_cast<ArrType*>(type);
			if(arrT->get_size()>0){
				Const* arr = new Arrconst(arrT->get_size(),arrT->get_type());
				globals[id]=arr;
			}
		}
	}
	void set_type(Type* ty){type=ty;}
protected:
	Type* type;
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
};

class Body: public AST{
public:
	Body(DeclList* d, StmtList* s):declarations(d),statements(s){}
	~Body(){delete declarations; delete statements;}

	virtual void sem() const{
		declarations->sem();
		statements->sem();
	}

	void run() const{
		statements->run();
	}

	virtual void printOn(std::ostream &out) const override {
		out << "Body("<<*declarations<<","<<*statements<<")";
	}
protected:
	DeclList* declarations;
	StmtList* statements;
};

class Function:public Decl{
	Function(std::string name,DeclList *decl_list, Type* return_type)
		:Decl(name,"function"), ret_type(return_type),
		 body(nullptr), formals(decl_list){}

	void add_body(Body* bod){
		body=bod;
	}

	virtual void sem() override{
		declared[id]=ret_type;
		formals->sem();
	}
protected:
	Type* ret_type;
	Body* body;
	DeclList* formals;
};


/*class Body: public AST{
	Body(List<Local* > *l,List<Stmt*> *s):locals(l),stmts(s){}
	virtual void run() const override{
		locals->run();
		stmts->run();
	}
protected:
	List<Local* > *locals;
	List<Stmt*> *stmts;
};
*/
