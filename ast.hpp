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
  std::string s;
}
union expr_type{
  int i;
  double d;
  bool b;
}

extern std::map<std::string, value> globals;

class AST {
public:
  virtual ~AST() {}
  virtual void printOn(std::ostream &out) const = 0;
};

inline std::ostream& operator<< (std::ostream &out, const AST &t) {
  t.printOn(out);
  return out;
}

class Expr: public AST {
public:
  virtual expr_type eval() const = 0;
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
  virtual value eval() const override {
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
  std::string get_type(){return type;}
protected:
  Type* type;
};
class SConst: public Const {
public:
  SConst(std::string s):type(new ArrType(s.size()+1,new Type("character"))){
    str=malloc(sizeof(char)*(s.size()+1));
    s.copy(str,s.size());
    str[s.size()]='\0';
  }
  virtual void printOn(std::ostream &out) const override {
    out << "SConst(" << str << ")";
  }
  virtual char* eval() const override {
    return str;
  }
private:
  char *str;
};

class IConst: public Const {
public:
  IConst(int n):num(n){}
  virtual void printOn(std::ostream &out) const override {
    out << "IConst(" << num << ")";
  }
  virtual int eval() const override {
    return num;
  }
private:
  int num;
};

class DConst: public Const {
public:
  IConst(double n):num(n){}
  virtual void printOn(std::ostream &out) const override {
    out << "DConst(" << num << ")";
  }
  virtual double eval() const override {
    return num;
  }
private:
  int num;
};

class CConst: public Const {
public:
  CConst(char c):ch(c){}
  virtual void printOn(std::ostream &out) const override {
    out << "CConst(" << ch << ")";
  }
  virtual char eval() const override {
    return ch;
  }
private:
  char ch;
};

class CConst: public Const {
public:
  CConst(std::string p):ptr(0){}
  virtual void printOn(std::ostream &out) const override {
    out << "PConst(" << ptr << ")";
  }
  virtual char eval() const override {
    return ptr;
  }
private:
  int ptr;
};

class BConst: public Const {
public:
  BConst(str::string b){
    if(!(c.compare("true"))) boo=true;
    else boo=false;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "BConst(" << boo << ")";
  }
  virtual char eval() const override {
    return boo;
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
  virtual expr_type eval() const override {
    if(!(op.compare("+")) and right)return left->eval() + right->eval();
    if(!(op.compare("+")))return left->eval(); //UnOp
    if(!(op.compare("-")) and right) return left->eval() - right->eval();
    if(!(op.compare("-"))) return (-left->eval()); //UnOp
    if(!(op.compare("*"))) return left->eval() * right->eval();
    if(!(op.compare("/")) or !(op.compare("div"))) return left->eval() / right->eval();
    if(!(op.compare("mod"))) return left->eval() % right->eval();
    if(!(op.compare("<>"))) return left->eval() != right->eval();
    if(!(op.compare("<="))) return left->eval() <= right->eval();
    if(!(op.compare(">="))) return left->eval() >= right->eval();
    if(!(op.compare("="))) return left->eval() == right->eval();
    if(!(op.compare(">"))) return left->eval() > right->eval();
    if(!(op.compare("<"))) return left->eval() < right->eval();
    if(!(op.compare("and"))) return left->eval() and right->eval();
    if(!(op.compare("or"))) return left->eval() or right->eval();
    if(!(op.compare("not"))) return not left->eval();
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
    globals[id->name()]=expr;
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
    if(expr->eval()) stmt1->run();
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
    while(expr->eval()) stmt->run();
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

class Type: public AST{
public:
  Type(std::string t):name(t){}
  std::string get_name(){
    return name;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Type(" << name << ")";
  }
protected:
  std::string name;
};

class PtrType: public Type{
public:
  PtrType(Type* t):name("pointer"),type(t){}
  ~PtrType(){delete type;}
  Type* get_type(){ return type;}
  virtual void printOn(std::ostream &out) const override {
    out << "PtrType(" << name <<"of type "<< *type << ")";
  }
protected:
  Type* type;
}

class ArrType: public PtrType{
public:
  ArrType(int s,Type* t):size(s),name("array"),type(t){}
  ArrType(Type* t):size(-1),name("array"),type(t){}
  ~ArrType(){delete type;}
  int get_size(){return size;}
  virtual void printOn(std::ostream &out) const override {
    out << "ArrType(" << name <<"["<<size<<"]"<<"of type "<< *type << ")";
  }
protected:
  int size;
}

class Local: public AST{

};

class Decl: public AST{
  Decl(std::string i,std::string ty)id(i),decl_type("unknown"){}
  virtual void printOn(std::ostream &out) const override {
    out << "Decl(" << decl_type <<":"<<id<<")";
  }
  std::string get_id(){return id;}
protected:
  std::string id;
  std::string decl_type;
};

class LabelDecl:public Decl{
  LabelDecl(Decl* d):id(d->get_id),decl_type("label"){delete d;}
  virtual void printOn(std::ostream &out) const override {
    out << "LabelDecl("<<id<<")";
  }
}
class VarDecl: public Decl{
  VarDecl(Decl* d):id(d->get_id),decl_type("var"),type(nullptr){delete d;}
  VarDecl(Decl* d,Type* t):id(d->get_id),decl_type("var"),type(t){delete d;}
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
