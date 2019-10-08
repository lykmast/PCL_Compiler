#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>

class Const;
union value{
  char c;
  int i;
  double r;
  bool b;
  char* s;
  Const* con;
};

template <class T>
class Triplet{
public:
  T* first,*second,*third;
};

class AST {
public:
  virtual ~AST() {}
  virtual void printOn(std::ostream &out) const = 0;
};

inline std::ostream& operator<< (std::ostream &out, const AST &t) {
	t.printOn(out);
	return out;
}

class Type: public AST{
public:
  Type(std::string t):name(t){}
  std::string get_name(){
    return name;
  }
  virtual Type* get_type() const{ return nullptr;}//TODO erase
  virtual Const* create() const{return nullptr;}
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
  virtual Const* create() const;

};

class REAL: public Type{
private:
  REAL():Type("real"){}  //private constructor to prevent instancing
public:
  virtual Const* create() const;
	static REAL* getInstance(){
		static REAL instance;
		return &instance;
	}
};

class BOOLEAN: public Type{
private:
	BOOLEAN():Type("boolean"){}  //private constructor to prevent instancing
public:
  virtual Const* create() const;
	static BOOLEAN* getInstance(){
		static BOOLEAN instance;
		return &instance;
	}
};

class CHARACTER: public Type{
private:
  CHARACTER():Type("character"){}  //private constructor to prevent instancing
public:
  virtual Const* create() const;
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
  virtual Const* create() const;
  Type* get_type(){ return type;}
  virtual void printOn(std::ostream &out) const override {
    out << "PtrType(" << name <<"of type "<< *type << ")";
  }
  virtual bool should_delete() const override{
    return true;
  }
  virtual bool doCompare(Type* t) override{
    if (!(name.compare(t->get_name())))
      return type->doCompare(t->get_type());
    return false;
  }
protected:
  Type* type;
};

class ArrType: public PtrType{
public:
  ArrType(int s,Type* t):size(s),PtrType("array",t){}
  ArrType(Type* t):size(-1),PtrType("array",t){}
  ~ArrType(){if(type->should_delete()) delete type;}
  virtual Const* create() const;
  int get_size(){return size;}
  virtual void printOn(std::ostream &out) const override {
    out << "ArrType(" << name <<"["<<size<<"]"<<"of type "<< *type << ")";
  }
protected:
  int size;
};

// const Type * INTEGER=new Type("integer"),* REAL=new Type("real"),
// *BOOLEAN=new Type("boolean"), CHARACTER=new Type("char");

class Const; //forward declaration of Const to use as eval() return Type

extern std::map<std::string, Const*> globals; // map variable names to values
extern std::map<std::string, Type*> declared; // map variable names to values


class Expr: public AST {
public:
  virtual Const* eval() = 0;
  virtual void typecheck() {};
};

class Stmt: public AST {
public:
  virtual void run() const{}; //empty stmt -> do nothing
};


class Id: public Expr {
public:
  Id(std::string v): var(v) {}
  virtual void printOn(std::ostream &out) const override {
    out << "Id(" << var << ")";
  }
  virtual Const* eval() override {
    std::map<std::string,Const*>::iterator it;
    it=globals.find(var);
    if(it==globals.end()){//TODO error or uninitialized value
      return nullptr;/*TODO throw error not found*/
    }
    return it->second;
  }
  Type* get_type() const{
    return declared[var];
  }
  std::string name(){
    return var;
  }
private:
  std::string var;
};

class Const: public Expr{
public:
  Const(Type* ty,bool dyn = false):type(ty),dynamic(dyn){}
  Type* get_type(){return type;}
  virtual void printOn(std::ostream &out) const =0;
	virtual Const* eval() override{return this;}
  virtual value get_value() const=0;
  bool isDynamic(){return dynamic;}
protected:
  bool dynamic;
  Type* type;
};

class Sconst: public Const {
public:
  Sconst(std::string s):Const(new ArrType(s.size()-1,CHARACTER::getInstance())) {
    str=(char*)(malloc(sizeof(char)*(s.size()-1)));
    s.substr(1,s.size()-2).copy(str,s.size()-2); //to char[] without quotes
    str[s.size()-2]='\0';
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Sconst(" << str << ")";
  }
  virtual value get_value() const override {
    value v; v.s=str;
    return v;
  }
private:
  char *str;
};

class Iconst: public Const {
public:
  Iconst(int n, bool dyn=false):Const(INTEGER::getInstance(),dyn),num(n){}
  virtual void printOn(std::ostream &out) const override {
    out << "Iconst(" << num << ")";
  }
  virtual value get_value() const override {
    value v; v.i=num;
    return v;
  }
private:
  int num;
};

Const* INTEGER::create() const{
  return new Iconst(0, true);
}

class Rconst: public Const {
public:
  Rconst(double n, bool dyn=false):Const(REAL::getInstance(), dyn),num(n){}
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

Const* REAL::create() const{
  return new Rconst(0, true);
}

class Cconst: public Const {
public:
  Cconst(char c, bool dyn=false):Const(CHARACTER::getInstance(), dyn),ch(c){}
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

Const* CHARACTER::create() const{
  return new Cconst('\0', true);
}

class Pconst: public Const {
public:
  Pconst():Const(new PtrType(ANY::getInstance())),ptr(0){}//TODO implement for pointers different than nil
	Pconst(Const* pval,Type *t, bool dyn=false):Const(new PtrType(t), dyn),ptr(pval){}
  virtual void printOn(std::ostream &out) const override {
    out << "Pconst(" << ptr << "of type "<<*type<< ")";
  }
  virtual value get_value() const override {
    value v; v.con=ptr;
    return v;
  }
protected:
  Const* ptr;
};

Const* PtrType::create() const{
  return new Pconst(type->create(),type, true);
}

class Arrconst: public Pconst{
public:
  Arrconst(int s, Type* t, bool dyn=false):Pconst( (Const* ) (malloc(sizeof(Const)*s)), t, dyn),size(s){}
  Const* get_element(int i){
    return &(ptr[i]);
  }
protected:
  int size;
};

Const* ArrType::create() const{
  return new Arrconst(size,type, true);
}

class Bconst: public Const {
public:
  Bconst(bool b, bool dyn=false):Const(BOOLEAN::getInstance(), dyn),boo(b){}
  virtual void printOn(std::ostream &out) const override {
    out << "Bconst(" << boo << ")";
  }
  virtual value get_value() const override {
    value v; v.b=boo;
    return v;
  }
private:
  bool boo;
};

Const* BOOLEAN::create() const{
  return new Bconst(false, true);
}

class Op: public Expr {
public:
  Op(Expr *l, std::string o, Expr *r): left(l), op(o), right(r),
    leftType(nullptr), rightType(nullptr), resType(nullptr) {}
  Op(Expr *l, std::string o): left(l), op(o), right(nullptr) {}
  ~Op() { delete left; delete right; }
  virtual void printOn(std::ostream &out) const override {
    if(right) out << op << "(" << *left << ", " << *right << ")";
    else  out << op << "(" << *left << ")";
  }

  virtual void typecheck() override{
  // sets leftType, rightType and resType fields
  // it should be run only once even when we have repeated evals
  // e.g in while
    Type* intType=INTEGER::getInstance();
    Type* realType=REAL::getInstance();
    Type* boolType=BOOLEAN::getInstance();
    Const *leftConst=left->eval();
    leftType=leftConst->get_type();

    if(right!=nullptr){//BinOps
      Const *rightConst=right->eval();
      rightType=rightConst->get_type();
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

      if(!(op.compare("=")) or !(op.compare("<>")) or !(op.compare("<="))
      or !(op.compare(">=")) or !(op.compare("<")) or !(op.compare(">")) ){
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
      if(!resType){/*TODO ERROR type mismatch*/}
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
    if(!(op.compare("@"))){
      resType=new PtrType(leftConst->get_type());
    }
    if(!(op.compare("^"))){
      if(!(leftType->get_name().compare("pointer")) ){
        PtrType* p = static_cast<PtrType*>(leftType);
        resType=p->get_type();
      }
    }
    if(!resType){/*TODO ERROR type mismatch*/}
    return;

    //TODO @ and ^ operators typecheck
  }

  virtual Const* eval() override {
    // before the first eval of any op, there must have been one typecheck to
    // fill resType, leftType, rightType
    Type* realType=REAL::getInstance();
    left->typecheck();
    Const *leftConst=left->eval();
    Const *rightConst=nullptr;
    if(right){
      right->typecheck();
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
        return new Rconst(li/ri*lr/rr);
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
      return new Bconst(li+lr!=ri+rr);
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
    if(!(op.compare("="))) {
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
      return new Bconst(li+lr==ri+rr);
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
    if(!(op.compare("@"))) return leftConst;

    if(!(op.compare("^"))) {
      value v=leftConst->get_value();
      Const* c=v.con;
      PtrType *p=static_cast<PtrType*>(leftType);
      return new Pconst(c,p->get_type());
    }
    if(!(op.compare("[]"))){
      value v = rightConst->get_value();
      int i=v.i;
      Arrconst* arr = static_cast<Arrconst*> (leftConst);
      return arr->get_element(i);
    }
    return 0;  // this will never be reached.
  }
private:
  Type *leftType,*rightType,*resType;
  Expr *left;
  std::string op;
  Expr *right;
};

class Let: public Stmt { //TODO semantics
public:
  Let(Id* i,Expr* e):id(i),expr(e){}
  ~Let(){delete id; delete expr;}
  virtual void printOn(std::ostream &out) const override {
    out << "Let(" << *id << ":=" << *expr << ")";
  }
  virtual void run() const override{
    Const* c = expr->eval();
    std::map<std::string,Const*>::iterator it;
    it=globals.find(id->name());
    if(it==globals.end()){
      std::map<std::string,Type*>::iterator it2;
      if(it2==declared.end()){
        return; //TODO throw error variable not declared
      }
      else{
        if(c->get_type()->doCompare(it2->second) ){
          globals[id->name()]=c;
          return;
        }
        //TODO throw error type mismatch
        return;
      }
    }
    else{
      Const* old=globals[id->name()];
      if(c->get_type()->doCompare(old->get_type())){
        delete globals[id->name()];
        globals[id->name()]=c;
        return;
      }
      //TODO throw error type mismatch
      return;
    }
  }
private:
  Id  *id;
  Expr *expr;
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
  virtual void run() const override{
    expr->typecheck();
    Const * c= expr->eval();
    bool e=false;
    if(c->get_type()==BOOLEAN::getInstance()){
      value v=c->get_value();
      e = v.b;
    }
    else{/*TODO ERROR incorrect type*/}
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
  virtual void run() const override{
    expr->typecheck();
    Const * c= expr->eval();
    bool e=false;
    if(c->get_type()==BOOLEAN::getInstance()){
      value v=c->get_value();
      e = v.b;
    }
    else{/*TODO ERROR incorrect type*/}
    while(e) {
      stmt->run();
      Const * c= expr->eval();
      e=false;
      if(c->get_type()==BOOLEAN::getInstance()){
        value v=c->get_value();
        e = v.b;
      }
      else{/*TODO ERROR incorrect type*/}
    }
  }
private:
  Expr *expr;
  Stmt *stmt;
};

class New: public Stmt{
public:
  New(Expr* e, Id* i):expr(e),id(i){}
  ~New(){delete expr; delete id;}
  virtual void printOn(std::ostream &out) const override {
    if(expr)
      out << "New( [" << *expr << "] of " << *id << ")";
    else
      out << "New( [] of " << *id << ")";
  }

  virtual void run() const override{
    if(expr){
      expr->typecheck();
      Const* c= expr->eval();
      if(!(c->get_type()->doCompare(INTEGER::getInstance())) ){
        /*TODO ERROR incorrect type*/
      }
      value v = c->get_value();
      int i = v.i;
      if(i<=0) {/*TODO ERROR incorrect type*/}
      Type* idType=id->get_type();
      if(idType->get_name().compare("pointer") )
      {/*TODO ERROR incorrect type*/}
      PtrType* p=static_cast<PtrType*>(idType);
      Type* t=p->get_type();
      if(t->get_name().compare("array") )
      {/*TODO ERROR incorrect type*/}
      ArrType* arrT = static_cast<ArrType*>(t);
      ArrType* arrT_size = new ArrType(i,arrT->get_type());
      delete arrT;
      Pconst *ret = new Pconst(arrT_size->create(), arrT_size );
      Let(id, ret);
    }

    Type* idType=id->get_type();
    if(idType->get_name().compare("pointer") )
    {/*TODO ERROR incorrect type*/}
    PtrType* p=static_cast<PtrType*>(idType);
    Type* t=p->get_type();
    Pconst *ret = new Pconst(t->create(), t);
    Let(id, ret);
  }

protected:
  Expr* expr;
  Id* id;
};


class Dispose: public Stmt{
public:
  Dispose(Id* i):id(i){}
  ~Dispose(){delete id;}
  virtual void run() const{
    if(id->get_type()->get_name().compare("pointer"))
    {/*TODO ERROR incorrect type*/}
    Const *c = id->eval();
    value v = c->get_value();
    Const* ptr = v.con;
    if(!(ptr->isDynamic())){/*TODO ERROR incorrect type*/}
    delete ptr;
    Let(id, new Pconst());
  }
protected:
  Id *id;
};

class DisposeArr: public Stmt{
public:
  DisposeArr(Id* i):id(i){}
  ~DisposeArr(){delete id;}
  virtual void run() const{
    Type* t=id->get_type();
    if(t->get_name().compare("pointer"))
    {/*TODO ERROR incorrect type*/}
    PtrType *pt = static_cast<PtrType*>(t);
    if(pt->get_name().compare("array"))
    {/*TODO ERROR incorrect type*/}
    Const *c = id->eval();
    value v = c->get_value();
    Const* ptr = v.con;
    if(!(ptr->isDynamic())){/*TODO ERROR incorrect type*/}
    delete ptr;
    Let(id, new Pconst());
  }
protected:
  Id *id;
};
template<class T>
std::ostream& operator <<(std::ostream &out,const std::vector<T*> v) {
  for(auto p :v)
  out<<*p<<",";
  return out;
}

template <class T>
class List: public AST{
public:
  List(T *t):list(1,t){}
  List():list(){}
  ~List(){
    for(auto p:list)
    delete p;
    list.clear();
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
  virtual void run() const{
    for(auto p:list)
      p->run();
  }
};
class ExprList: public List<Expr> {
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
  Decl(std::string i,std::string ty):id(i),decl_type("unknown"){}
  virtual void printOn(std::ostream &out) const override {
    out << "Decl(" << decl_type <<":"<<id<<")";
  }
  virtual void run() const{};
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
  virtual void run() const override{
    declared[id]=LABEL::getInstance();
  }
};
class VarDecl: public Decl{
public:
  VarDecl(Decl* d):Decl(d->get_id(),"var"),type(nullptr){delete d;}
  VarDecl(Decl* d,Type* t):Decl(d->get_id(),"var"),type(t){delete d;}
  virtual void printOn(std::ostream &out) const override {
    if(type)
    out << "VarDecl(" <<id<<"of type "<< *type << ")";
    else
    out << "VarDecl(" <<id<<"of type NOTSET)";
  }
  virtual void run() const override{
    declared[id]=type;
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
    for(auto p:list){
      Decl *d=new VarDecl(p,t);
      p=d;
    }
  }
  void run() const{
    for(auto p:list)
      p->run();
  }

  void toLabel(){
    for(auto p:list){
      Decl *d=new LabelDecl(p);
      p=d;
    }
  }
  void toFormal(Type* t, bool ref){
    for(auto p:list){
      Decl *d=new FormalDecl(p,t,ref);
      p=d;
    }
  }
};

class Body: public AST{
public:
  Body(DeclList* d, StmtList* s):declarations(d),statements(s){}
  ~Body(){delete declarations; delete statements;}
  void declare() const{
    declarations->run();
  }

  void run() const{
    statements->run();
  }

protected:
  DeclList* declarations;
  StmtList* statements;
};

class Function:public Decl{

protected:
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
