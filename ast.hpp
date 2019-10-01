#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>

union value{
  char c;
  int i;
  double r;
  bool b;
  char* s;
};

template <class T>
class Triplet{
public:
  T* first,*second,*third;
}

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
  virtual Type* get_type() const{ return nullptr;}
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
  PtrType(Type* t):Type("pointer"),type(t){}
	PtrType(std::string name,Type* t):Type(name),type(t){}
  ~PtrType(){if(type->should_delete()) delete type;}
  Type* get_type(){ return type;}
  virtual void printOn(std::ostream &out) const override {
    out << "PtrType(" << name <<"of type "<< *type << ")";
  }
  virtual bool should_delete() const override{
    return true;
  }
  virtual bool doCompare(Type* t) override{
    if (!(name.compare(t->get_name)))
      return type==t->get_type();
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


class Expr: public AST {
public:
  virtual Const* eval() = 0;
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
    return globals[var];
  }
  std::string name(){
    return var;
  }
private:
  std::string var;
};

class Const: public Expr{
public:
  Const(Type* ty):type(ty){}
  Type* get_type(){return type;}
  virtual void printOn(std::ostream &out) const =0;
	virtual Const* eval() override{return this;}
  virtual value get_value() const=0;
protected:
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
  Iconst(int n):Const(INTEGER::getInstance()),num(n){}
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


class Rconst: public Const {
public:
  Rconst(double n):Const(REAL::getInstance()),num(n){}
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

class Cconst: public Const {
public:
  Cconst(char c):Const(CHARACTER::getInstance()),ch(c){}
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

class Pconst: public Const {
public:
  Pconst():Const(new PtrType(ANY::getInstance())),ptr(0){}//TODO implement for pointers different than nil
	Pconst(int pval,Type *t):Const(t),ptr(pval){}
  virtual void printOn(std::ostream &out) const override {
    out << "Pconst(" << ptr << "of type "<<*type<< ")";
  }
  virtual value get_value() const override {
    value v; v.i=ptr;
    return v;
  }
private:
  int ptr;
};

class Bconst: public Const {
public:
  Bconst(std::string b):Const(BOOLEAN::getInstance()){
    if(!(b.compare("true"))) boo=true;
    else boo=false;
  }
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

class Op: public Expr {
public:
  Op(Expr *l, std::string o, Expr *r): left(l), op(o), right(r) {}
  Op(Expr *l, std::string o): left(l), op(o), right(nullptr) {}
  ~Op() { delete left; delete right; }
  virtual void printOn(std::ostream &out) const override {
    if(right) out << op << "(" << *left << ", " << *right << ")";
    else  out << op << "(" << *left << ")";
  }

  Triplet<Type> typecheck(){
  // returns type* of result or nullptr if types are invalid and evaluates
  // left and right to Consts
    Type* intType=INTEGER::getInstance();
    Type* realType=REAL::getInstance();
    Type* boolType=BOOLEAN::getInstance();
    Const *leftConst=left->eval()
    Type *leftType=leftConst->get_type();
    Type *ret_type=nullptr;

    if(right!=nullptr){//BinOps
      Const *rightCosnt=right->eval();
      Type *rightType=rightConst->get_type();
      if(!(op.compare("+")) or !(op.compare("-")) or !(op.compare("*"))){
      //real or int operands-> real or int result
        if( (leftType->doCompare(realType) or leftType->doCompare(intType))
        and (rightType->doCompare(realType) or rightType->doCompare(intType)) ){
        //is a number (real or int)
          if(leftType->doCompare(realType) or rightType->doCompare(realType))
          // one of them is real
            ret_type=realType;
          else
            ret_type=intType;
        }
      }

      if(!(op.compare("/"))){
      //real or int operands-> real result
        if( (leftType->doCompare(realType) or leftType->doCompare(intType))
        and (rightType->doCompare(realType) or rightType->doCompare(intType)))
        //is a number (real or int)
          ret_type=realType;
      }

      if(!(op.compare("div")) or !(op.compare("mod")) ){
      //int operands-> int result
        if(  leftType->doCompare(intType) and rightType->doCompare(intType))
        //is int
          ret_type=intType;
      }

      if(!(op.compare("=")) or !(op.compare("<>")) or !(op.compare("<="))
      or !(op.compare(">=")) or !(op.compare("<")) or !(op.compare(">")) ){
      //real or int operands-> bool result

        if( (leftType->doCompare(realType) or leftType->doCompare(intType))
        and (rightType->doCompare(realType) or rightType->doCompare(intType)))
        //is a number (real or int)
          ret_type=boolType;
      }

      if(!(op.compare("and")) or !(op.compare("or")) ){
      //bool operands-> bool result
        if(  leftType->doCompare(boolType) and rightType->doCompare(boolType))
        //is bool
          ret_type=boolType;
      }

      delete left;
      left=leftConst;
      delete right;
      right=rightConst;
      Triplet<Type> t;
      t.first=ret_type; t.second=leftType; t.third=rightType;
      return t;
    }
    //UNOP
    if(!(op.compare("+")) or !(op.compare("-")))
      //real or int operand-> real or int result
      if( leftType->doCompare(realType) or leftType->doCompare(intType) )
        ret_type=leftType;

    if(!(op.compare("not"))){
      //bool operand-> bool result
      if(leftType->doCompare(boolType) )
        ret_type=boolType;
    }

    delete left;
    left=leftConst;
    Triplet<Type> t;
    t.first=ret_type; t.second=leftType; t.third=nullptr;
    return t;
    //TODO @ and ^ operators typecheck
  }

  virtual Const* eval() override {

    Type* intType=INTEGER::getInstance();
    Type* realType=REAL::getInstance();
    Type* boolType=BOOLEAN::getInstance();
    Triplet<Type> t=typecheck();
    Type* res_type=t.first;
    Type* leftType=t.second;
    Type* rightType=t.third;
    if(!res_type){;/*TODO ERROR*/}

    if(!(op.compare("+")) and right){
      //BinOp
      int li=0,ri=0;
      double lr=0,rr=0;
      value v;
      if(leftType->doCompare(realType)){
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
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
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
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
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
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
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
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
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
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
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
        ri=v.i;
      }
      if(resType->doCompare(realType)){
        return new Rconst(li/ri*lr/rr);
      }
    }

    if( !(op.compare("div"))){
      value v=left->get_value();
      int li=v.i;
      v=right->get_value();
      int ri=v.i;
      return new Iconst(li/ri);
    }
    if(!(op.compare("mod"))) {
      value v=left->get_value();
      int li=v.i;
      v=right->get_value();
      int ri=v.i;
      return new Iconst(li%ri);
    }
    if(!(op.compare("<>"))) {
      int li=0,ri=0;
      double lr=0,rr=0;
      value v;
      if(leftType->doCompare(realType)){
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
        ri=v.i;
      }
      return new Bconst(li+lr!=ri+rr);
    }
    if(!(op.compare("<="))) {
      int li=0,ri=0;
      double lr=0,rr=0;
      value v;
      if(leftType->doCompare(realType)){
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
        ri=v.i;
      }
      return new Bconst(li+lr<=ri+rr);
    }
    if(!(op.compare(">="))) {
      int li=0,ri=0;
      double lr=0,rr=0;
      value v;
      if(leftType->doCompare(realType)){
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
        ri=v.i;
      }
      return new Bconst(li+lr>=ri+rr);
    }
    if(!(op.compare("="))) {
      int li=0,ri=0;
      double lr=0,rr=0;
      value v;
      if(leftType->doCompare(realType)){
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
        ri=v.i;
      }
      return new Bconst(li+lr==ri+rr);
    }
    if(!(op.compare(">"))) {
      int li=0,ri=0;
      double lr=0,rr=0;
      value v;
      if(leftType->doCompare(realType)){
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
        ri=v.i;
      }
      return new Bconst(li+lr > ri+rr);
    };
    if(!(op.compare("<"))) {
      int li=0,ri=0;
      double lr=0,rr=0;
      value v;
      if(leftType->doCompare(realType)){
        v=left->get_value()
        lr=v.r;
      }
      else{
        v=left->get_value()
        li=v.i;
      }
      if(rightType->doCompare(realType)){
        v=right->get_value()
        rr=v.r;
      }
      else{
        v=right->get_value()
        ri=v.i;
      }
      return new Bconst(li+lr < ri+rr);
    }
    if(!(op.compare("and"))) {
      value v=left->get_value();
      bool lb=v.b;
      v=right->get_value();
      bool rb=v.b;
      return new Bconst(lb and rb);
    }
    if(!(op.compare("or"))) {
      value v=left->get_value();
      bool lb=v.b;
      v=right->get_value();
      bool rb=v.b;
      return new Bconst(lb or rb);
    }
    if(!(op.compare("not"))) {
      //UnOp

      value v;
      v=left->get_value()
      bool lb=v.b;
      return new Bconst(not lb);
    }
    if(!(op.compare("@"))) return left->eval(); //TODO reference
    if(!(op.compare("^"))) return left->eval(); //TODO dereference
    if(!(op.compare("[]"))) return left->eval(); //TODO array index
    return 0;  // this will never be reached.
  }
private:
  Expr *left;
  std::string op;
  Expr *right;
};

class Let: public Stmt { //TODO semantics
public:
  Let(Id* i,Expr* e):id(i),expr(e){}
  ~Let(){delete id; delete e;}
  virtual void printOn(std::ostream &out) const override {
    out << "Let(" << *id << ":=" << *expr << ")";
  }
  virtual void run() const override{
    delete globals[id->name()];
    globals[id->name()]=expr->eval();
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
    Const * c= expr->eval();
    bool e=false;
    if(c->get_type()==BOOLEAN::getInstance()){
      value v=c.get_value();
      e = v.b;
    }
    else{/*TODO ERROR incorrect type*/}
    if(b) stmt1->run();
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
    Const * c= expr->eval();
    bool e=false;
    if(c->get_type()==BOOLEAN::getInstance()){
      value v=c.get_value();
      e = v.b;
    }
    else{/*TODO ERROR incorrect type*/}
    while(b) {
      stmt->run();
      Const * c= expr->eval();
      bool e=false;
      if(c->get_type()==BOOLEAN::getInstance()){
        value v=c.get_value();
        e = v.b;
      }
      else{/*TODO ERROR incorrect type*/}
    }
  }
private:
  Expr *expr;
  Stmt *stmt;
};
template<class T>
std::ostream& operator <<(std::ostream &out,const std::vector<T*> v) const{
  for(auto p :v)
  out<<*p<<",";
  return out;
}

template <class T>
class List: public AST{
public:
  List(T *t):list(1,t){}
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
  friend class Body;
protected:
  std::vector<T*> list;
};

class ExprList: public List<Expr* > {
  ExprList(Expr *e):list(1,e){}
  ~ExprList(){
    for(auto p:list)
    delete p;
    list.clear();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "ExprList(" << list << ")";
  }
  void append(Expr *e){
    list.push_back(e);
  }
  virtual std::vector<ValueType*> eval(){
    std::vector<ValueType*> ret;
    ValueType *v;
    for(auto p:list)
    v=p->eval();
    ret.push_back(v);
  }
private:
  std::vector<Expr*> list;
};


class Local: public AST{

};

class Decl: public AST{
  Decl(std::string i,std::string ty):id(i),decl_type("unknown"){}
  virtual void printOn(std::ostream &out) const override {
    out << "Decl(" << decl_type <<":"<<id<<")";
  }
  std::string get_id(){return id;}
protected:
  std::string id;
  std::string decl_type;
};

class LabelDecl:public Decl{
  LabelDecl(Decl* d):id(d->get_id()),decl_type("label"){delete d;}
  virtual void printOn(std::ostream &out) const override {
    out << "LabelDecl("<<id<<")";
  }
}
class VarDecl: public Decl{
  VarDecl(Decl* d):id(d->get_id()),decl_type("var"),type(nullptr){delete d;}
  VarDecl(Decl* d,Type* t):id(d->get_id()),decl_type("var"),type(t){delete d;}
  virtual void printOn(std::ostream &out) const override {
    if(type)
    out << "VarDecl(" <<id<<"of type "<< *type << ")";
    else
    out << "VarDecl(" <<id<<"of type NOTSET)";
  }
  void set_type(Type* ty){type=ty;}
protected:
  Type* type;
};



class Body: public AST{
  Body(List<Local* > *l,List<Stmt*> *s):locals(l),stmts(s){}
  virtual void run() const override{
    locals->run();
    stmts->run();
  }
protected:
  List<Local* > *locals;
  List<Stmt*> *stmts;
};
