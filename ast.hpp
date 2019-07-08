#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>

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
  virtual int eval() const = 0;
};

class Stmt: public AST {
public:
  virtual void run() const = 0;
};

extern std::map<std::string, int> globals;

class Id: public Expr {
public:
  Id(std::string v): var(v) {}
  virtual void printOn(std::ostream &out) const override {
    out << "Id(" << var << ")";
  }
  virtual int eval() const override {
    return globals[var];
  }
private:
  std::string var;
};

class Const: public Expr {
public:
  Const(int n): num(n) {}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << num << ")";
  }
  virtual int eval() const override {
    return num;
  }
private:
  int num;
};

class BinOp: public Expr {
public:
  BinOp(Expr *l, char o, Expr *r): left(l), op(o), right(r) {}
  ~BinOp() { delete left; delete right; }
  virtual void printOn(std::ostream &out) const override {
    out << op << "(" << *left << ", " << *right << ")";
  }
  virtual int eval() const override {
    switch (op) {
      case '+': return left->eval() + right->eval();
      case '-': return left->eval() - right->eval();
      case '*': return left->eval() * right->eval();
      case '/': return left->eval() / right->eval();
      case '%': return left->eval() % right->eval();
    }
    return 0;  // this will never be reached.
  }
private:
  Expr *left;
  char op;
  Expr *right;
};
