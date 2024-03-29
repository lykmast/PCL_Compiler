/* ------------------------------------------
ast.cpp
Contains constructors and printOn for AST and
  all subclasses.
------------------------------------------ */
#include "ast.hpp"

Const::Const(TSPtr ty):type(ty){}
// Const::~Const(){
// 	if(type)
// 		if(type->should_delete())
// 			delete type;
// }

Rconst::Rconst(double n):Const(REAL::getInstance()),num(n){}
void Rconst::printOn(std::ostream &out) const{
	out << num;
}

Iconst::Iconst(int n):Const(INTEGER::getInstance()),num(n){}
void Iconst::printOn(std::ostream &out) const{
	out << num;
}

Cconst::Cconst(char c):Const(CHARACTER::getInstance() ),ch(c){}
void Cconst::printOn(std::ostream &out) const {
	out << "'" << ch << "'";
}

NilConst::NilConst():Const(std::make_shared<PtrType>(PtrType(ANY::getInstance()))){}
void NilConst::printOn(std::ostream &out) const {
	out << "nil";
}

// LValue(false,true): Sconst is not dynamic but constant.
Sconst::Sconst(std::string s):LValue(false,true),str(s){}
void Sconst::printOn(std::ostream &out) const {
	out<< "\""<<str<<"\"";
}

Id::Id(std::string v): name(v), type(nullptr) {}
void Id::printOn(std::ostream &out) const {
	out << name;
}



Bconst::Bconst(bool b):Const(BOOLEAN::getInstance()),boo(b){}

void Bconst::printOn(std::ostream &out) const {
	out <<  boo;
}





Op::Op(Expr *l, std::string o, Expr *r): left(l), op(o), right(r),
	leftType(nullptr), rightType(nullptr), resType(nullptr) {}
Op::Op(std::string o, Expr *l): left(l), op(o), right(nullptr) {}
Op::~Op() {
	delete left; delete right;
	// delete_type(leftType); delete_type(rightType); delete_type(resType);
}
void Op::printOn(std::ostream &out) const {
	if(right) out <<"("<< *left<<" "<< op << " "<< *right<<")";
	else  out<<"(" << op  << *left <<")";
}

Reference::Reference(LValue* lval):lvalue(lval), count(0){}
void Reference::printOn(std::ostream &out) const {
	out << "(@" << *lvalue << ")";
}





Dereference::Dereference(Expr *e):expr(e), count(0){}
void Dereference::printOn(std::ostream &out) const {
	out << "(" << *expr << "^)";
}


Brackets::Brackets(LValue *lval, Expr* e):lvalue(lval),expr(e){
	if (lval->isConst()) this->setConst();
}
void Brackets::printOn(std::ostream &out) const{
	out << *lvalue<< "[" << *expr << "]";
}

LabelStmt::LabelStmt(std::string id, Stmt* s):label_id(id), stmt(s) {}

LabelStmt::~LabelStmt(){delete stmt;}

void LabelStmt::printOn(std::ostream &out) const{
	out<<"Label("<<label_id<<": "<<*stmt<<")";
}

Goto::Goto(std::string id): label_id(id) {}

void Goto::printOn(std::ostream &out) const{
	out<<"Goto("<<label_id<<")";
}

Let::Let(LValue* lval,Expr* e):
		lvalue(lval),expr(e),different_types(false),is_right_int(false){
	if(lvalue->isConst()){
		// assignment to constant ("foo":= or "foo"[2]:=)
		std::ostringstream stream;
		stream<<"Cannot assign to constant "<<*lvalue;
		// this->report_error(stream.str().c_str());
		yyerror(stream.str().c_str());
	}
}
Let::~Let(){delete lvalue; delete expr;}
void Let::printOn(std::ostream &out) const {
	out << "Let(" << *lvalue << ":=" << *expr << ")";
}




If::If(Expr* e,Stmt* s1, Stmt* s2):expr(e),stmt1(s1),stmt2(s2){}
void If::printOn(std::ostream &out) const {
	if (stmt2)
	out << "If(" << *expr << "then" << *stmt1 << "else" << *stmt2 << ")";
	else
	out << "If(" << *expr << "then" << *stmt1 << ")";
}

While::While(Expr* e,Stmt* s):expr(e),stmt(s){}
void While::printOn(std::ostream &out) const {
	out << "While(" << *expr << "do" << *stmt << ")";
}


New::New(LValue* lval, Expr* e):expr(e),lvalue(lval){}
New::~New(){delete expr; delete lvalue;}
void New::printOn(std::ostream &out) const{
	if(expr)
		out << "New( [" << *expr << "] " << *lvalue << ")";
	else
		out << "New( [] of " << *lvalue << ")";
}


Dispose::Dispose(LValue* lval):lvalue(lval){}
Dispose::~Dispose(){delete lvalue;}
void Dispose::printOn(std::ostream &out) const{
		out << "Dispose( " << *lvalue << ")";
}


DisposeArr::DisposeArr(LValue* lval):lvalue(lval){}
DisposeArr::~DisposeArr(){delete lvalue;}
void DisposeArr::printOn(std::ostream &out) const{
		out << "Dispose[]( " << *lvalue << ")";
}

template<class T>
std::ostream& operator <<(std::ostream &out,const std::map<std::string,T*> m) {
	out<<"[";
	for(auto p :m)
		out<<"<"<<p.first<<": "<<*(p.second)<<">,";
	out<<"]";
	return out;
}



StmtList::StmtList(Stmt *s):List<Stmt>(s){}
StmtList::StmtList():List<Stmt>(){}
void StmtList::printOn(std::ostream &out) const{
	out << "StmtList(" << list << ")";
}




ExprList::ExprList():List<Expr>(){}
ExprList::ExprList(Expr *e):List<Expr>(e){}
void ExprList::printOn(std::ostream &out) const {
	out << "ExprList(" << list << ")";
}



LabelDecl::LabelDecl(Decl* d):Decl(d->get_id(),"label"){delete d;}
void LabelDecl::printOn(std::ostream &out) const {
	out << "LabelDecl("<<id<<")";
}



VarDecl::VarDecl(Decl* d):Decl(d->get_id(),"var"),type(nullptr){delete d;}
VarDecl::VarDecl(Decl* d,TSPtr t):Decl(d->get_id(),"var"),type(t){delete d;}
void VarDecl::printOn(std::ostream &out) const{
	if(type)
	out << "VarDecl(" <<id<<" of type "<< *type << ")";
	else
	out << "VarDecl(" <<id<<" of type NOTSET)";
}

FormalDecl::FormalDecl(Decl *d, bool ref):VarDecl(d), byRef(ref){}
FormalDecl::FormalDecl(Decl* d,TSPtr t,bool ref):VarDecl(d,t), byRef(ref){}
bool FormalDecl::isByRef(){return byRef;}

DeclList::DeclList(Decl* d):List<Decl>(d){}
DeclList::DeclList():List<Decl>(){}
void DeclList::toVar(TSPtr t){
	for(auto p=list.begin();p!=list.end();p++){
		Decl *d=new VarDecl(*p,t);
		*p=d;
	}
}
void DeclList::toLabel(){
	for(auto p=list.begin();p!=list.end();p++){
		Decl *d=new LabelDecl(*p);
		*p=d;
	}
}
void DeclList::toFormal(TSPtr t, bool ref){
	for(auto p=list.begin();p!=list.end();p++){
		Decl *d=new FormalDecl(*p,t,ref);
		*p=d;
	}
}


Body::Body(bool library):declarations(nullptr),statements(nullptr),
	defined(false), library(library){}
Body::Body(DeclList* d, StmtList* s):declarations(d),statements(s),
	defined(true), library(false){}
Body::~Body(){delete declarations; delete statements;}
void Body::printOn(std::ostream &out) const {
	out << "Body("<<*declarations<<","<<*statements<<")";
}

Procedure::Procedure(std::string name, DeclList *decl_list, Body* bod, std::string decl_type):
	Decl(name,decl_type), body(bod),
	formals(static_cast<FormalDeclList*>(decl_list)), type(nullptr){}
void Procedure::printOn(std::ostream &out) const {
	if(body)
		out << decl_type<<"("<<*formals<<",,,"<<*body<<")";
	else
		out<<decl_type<<"(<forward>,"<<*formals<<")";
}
void Procedure::toForward(){
	is_forward=true;
}
bool Procedure::isForward(){
	return is_forward;
}

Function::Function(std::string name, DeclList *decl_list, TSPtr return_type, Body* bod)
	:Procedure(name,decl_list,bod,"function"), ret_type(return_type){}

Program::Program(std::string nam, Body* bod):name(nam),body(bod){}
void Program::printOn(std::ostream &out) const {
	out << "Program(" << name <<" ::: "<<*body<< ")";
}


Call::Call(std::string nam, ExprList* exp): name(nam), exprs(exp),
	by_ref(exp->size()), outer_vars(new ExprList()), body(nullptr){}

ProcCall::ProcCall(std::string nam, ExprList* exp):Call(nam, exp){}
void ProcCall::printOn(std::ostream &out) const {
	out << "ProcCall(" << name<<", args:"<< *exprs<< ")";
}

FunctionCall::FunctionCall(std::string nam, ExprList* exp):Call(nam,exp), type(nullptr){}
void FunctionCall::printOn(std::ostream &out) const {
	if(type)
		out << "FunctionCall(" << name<<"with return type "<<*type<<", args:"<< *exprs<< ")";
	else
		out << "FunctionCall(" << name<<"with return type <unknown>, args:"<< *exprs<< ")";
}
