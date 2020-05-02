#include "ast.hpp"

Const::Const(Type* ty):type(ty){}
Const::~Const(){
	if(type)
		if(type->should_delete())
			delete type;
}


UnnamedLValue::UnnamedLValue(Type *ty,bool dyn):LValue(dyn),type(ty){}
UnnamedLValue::UnnamedLValue(value val, Type* ty,bool dyn):LValue(dyn),val(val),type(ty){}
UnnamedLValue::~UnnamedLValue(){
	if(type)
		if(type->should_delete())
			delete type;
}
void UnnamedLValue::printOn(std::ostream &out) const{
	if(type)
		out << "UnnamedLValue(" << val.lval<<","<<*type << ")";
	else
		out << "UnnamedLValue(EMPTY)";
}


Rconst::Rconst(double n):Const(REAL::getInstance() ),num(n){}
void Rconst::printOn(std::ostream &out) const{
	out << "Rconst(" << num << ")";
}

Iconst::Iconst(int n):Const(INTEGER::getInstance()),num(n){}
void Iconst::printOn(std::ostream &out) const{
	out << "Iconst(" << num << ")";
}

Cconst::Cconst(char c):Const(CHARACTER::getInstance() ),ch(c){}
void Cconst::printOn(std::ostream &out) const {
	out << "Cconst(" << ch << ")";
}

Pconst::Pconst():Const(new PtrType(ANY::getInstance())),ptr(nullptr){}//TODO implement for pointers different than nil
Pconst::Pconst(LValue* pval,Type *t):Const(new PtrType(t) ),ptr(pval){}
void Pconst::printOn(std::ostream &out) const {
	out << "Pconst(" << ptr << "of type "<<*type<< ")";
	if(ptr)
		out<<"->"<<*ptr;
	else
		out<<"->nil";
}


StaticArray::StaticArray(int s, Type* t, int o):Arrconst(s,t), offset(o){
	child_size=find_child_size(t);

	if(!t->get_name().compare("array")){
		ArrType* arrT=static_cast<ArrType*>(t);
		if(arrT->get_size()>0){
			for(int i=0; i<s; i++){
				rt_stack[offset+i*child_size]= new StaticArray(
					arrT->get_size(),
					arrT->get_type(),
					offset+i*child_size+1
				);
			}
		}
	}
}
void StaticArray::printOn(std::ostream &out) const{
	out << "StaticArray( ["<<size<<"]"<<"of type "<< *type << ")";
}
int StaticArray::find_child_size(Type* t){
	if(!t->get_name().compare("array")){
		ArrType* arrT=static_cast<ArrType*>(t);
		int children=arrT->get_size();
		if(children>0){
			return children*find_child_size(arrT->get_type())+1;
		}
	}
	return 1;
}


DynamicArray::DynamicArray(int s, Type* t):Arrconst(s,t){
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
DynamicArray::~DynamicArray(){
	for (auto p:arr)
		delete p;
}
DynamicArray::DynamicArray(std::string s):
		Arrconst(s.size()+1,CHARACTER::getInstance()) {
	char* str=(char*)(malloc(sizeof(char)*(s.size()+1)));
	s.copy(str,s.size()); //to char[]
	str[s.size()]='\0';
	arr.resize(s.size()+1);
	for(uint i=0;i<s.size()+1;i++){
		arr[i]=CHARACTER::getInstance()->create();
	}
	for(int i=0; i<size;i++){
		value v; v.c=str[i];
		arr[i]->let(v);
	}
}
void DynamicArray::printOn(std::ostream &out) const{
	out << "DynamicArray([" << arr <<"] ["<<size<<"]"<<"of type "<< *type << ")";
}


Id::Id(std::string v): name(v), offset(-1), decl_nesting(0), current_nesting(0), type(nullptr) {}
void Id::printOn(std::ostream &out) const {
	out << "Id(" << name <<"@"<<offset<<":"<<decl_nesting<<")";
}



Bconst::Bconst(bool b):Const(BOOLEAN::getInstance() ),boo(b){}

void Bconst::printOn(std::ostream &out) const {
	out << "Bconst(" << boo <<")";
}





Op::Op(Expr *l, std::string o, Expr *r): left(l), op(o), right(r),
	leftType(nullptr), rightType(nullptr), resType(nullptr) {}
Op::Op(std::string o, Expr *l): left(l), op(o), right(nullptr) {}
Op::~Op() { delete left; delete right; }
void Op::printOn(std::ostream &out) const {
	if(right) out << op << "(" << *left << ", " << *right << ")";
	else  out << op << "(" << *left << ")";
}

Reference::Reference(LValue* lval):lvalue(lval){}
void Reference::printOn(std::ostream &out) const {
	out << "Reference" << "(" << *lvalue << ")";
}





Dereference::Dereference(Expr *e):expr(e){}
void Dereference::printOn(std::ostream &out) const {
	out << "Dereference" << "(" << *expr << ")";
}


Brackets::Brackets(LValue *lval, Expr* e):lvalue(lval),expr(e){}
void Brackets::printOn(std::ostream &out) const{
	out << "Brackets" << "(" << *lvalue<< ", " << *expr << ")";
}

Let::Let(LValue* lval,Expr* e):lvalue(lval),expr(e),different_types(false),
	is_right_int(false){}
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
VarDecl::VarDecl(Decl* d,Type* t):Decl(d->get_id(),"var"),type(t){delete d;}
void VarDecl::printOn(std::ostream &out) const{
	if(type)
	out << "VarDecl(" <<id<<" of type "<< *type << ")";
	else
	out << "VarDecl(" <<id<<" of type NOTSET)";
}

FormalDecl::FormalDecl(Decl *d, bool ref):VarDecl(d), byRef(ref){}
FormalDecl::FormalDecl(Decl* d,Type* t,bool ref):VarDecl(d,t), byRef(ref){}
bool FormalDecl::isByRef(){return byRef;}

DeclList::DeclList(Decl* d):List<Decl>(d){}
DeclList::DeclList():List<Decl>(){}
void DeclList::toVar(Type* t){
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
void DeclList::toFormal(Type* t, bool ref){
	for(auto p=list.begin();p!=list.end();p++){
		Decl *d=new FormalDecl(*p,t,ref);
		*p=d;
	}
}


Body::Body():declarations(nullptr),statements(nullptr),size(0),defined(false){}
Body::Body(DeclList* d, StmtList* s):declarations(d),statements(s),size(0),defined(true){}
Body::~Body(){delete declarations; delete statements;}
void Body::printOn(std::ostream &out) const {
	out << "Body("<<*declarations<<","<<*statements<<")";
}

Procedure::Procedure(std::string name, DeclList *decl_list, Body* bod, std::string decl_type):
	Decl(name,decl_type), body(bod),formals(static_cast<FormalDeclList*>(decl_list)){}
void Procedure::printOn(std::ostream &out) const {
	if(body)
		out << decl_type<<"("<<*formals<<",,,"<<*body<<")";
	else
		out<<decl_type<<"(<forward>,"<<*formals<<")";
}

Function::Function(std::string name, DeclList *decl_list, Type* return_type, Body* bod)
	:Procedure(name,decl_list,bod,"function"), ret_type(return_type){}

Program::Program(std::string nam, Body* bod):name(nam),body(bod),size(0){}
void Program::printOn(std::ostream &out) const {
	out << "Program(" << name <<" ::: "<<*body<< ")";
}


Call::Call(std::string nam, ExprList* exp): name(nam), exprs(exp),
	by_ref(exp->size()), nesting_diff(0), next_fp_offset(0), body(nullptr){}

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
