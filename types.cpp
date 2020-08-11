/* ------------------------------------------
types.cpp
Contains type classes and related members such
  as get_type create, doCompare, clone
------------------------------------------ */
#include "ast.hpp"

Type::Type(std::string t):name(t){}

std::string Type::get_name(){
	return name;
}

void Type::printOn(std::ostream &out) const{
	out << "Type(" << name << ")";
}
Type* Type::clone(){return this;}
bool Type::should_delete() const{//should not delete Singleton
	return false;
}
bool Type::doCompare(Type* t){
	return !(name.compare(t->get_name()))
		or !name.compare("any")
		or !t->get_name().compare("any");
}


PtrType::PtrType(Type* t):Type("pointer"),type(t){}

PtrType::PtrType(std::string name,Type* t):Type(name),type(t){}

PtrType::~PtrType(){
	if(type)
		if(type->should_delete())
			delete type;
}
Type* PtrType::clone(){
	return new PtrType(type->clone());
}
Type* PtrType::get_type(){ return type->clone();}
void PtrType::printOn(std::ostream &out) const {
	out << "PtrType(" << name <<"of type "<< *type << ")";
}
bool PtrType::should_delete() const{
	return true;
}


bool PtrType::doCompare(Type* t){
	if (!(name.compare(t->get_name()))){
		PtrType* pTy=static_cast<PtrType*>(t);
		return type->doCompare(pTy->get_type());
	}
	return false;
}



ArrType::ArrType(int s,Type* t):PtrType("array",t),size(s){}
ArrType::ArrType(Type* t):PtrType("array",t),size(-1){}

Type* ArrType::clone(){
	return new ArrType(size,type->clone());
}

int ArrType::get_size(){return size;}

bool ArrType::doCompare(Type* t){
	if (!(name.compare(t->get_name()))){
		ArrType* arrTy=static_cast<ArrType*>(t);
		return size==arrTy->get_size() and type->doCompare(arrTy->get_type());
	}
	return false;
}
void ArrType::printOn(std::ostream &out) const {
	out << "ArrType(" << name <<"["<<size<<"]"<<"of type "<< *type << ")";
}

bool ArrType::is_1D(){
	return type->get_name().compare("array");
}


CallableType::CallableType(std::string func_type, FormalDeclList* formals):
	Type(func_type), formal_types(formals->get_type()),
	formal_vars(formals->get_names()), by_ref(formals->get_by_ref()){}

bool CallableType::should_delete() const{
	return true;
}

void CallableType::typecheck_args(std::vector<Type*> arg_types){
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

void CallableType::check_passing(std::vector<bool> ref){
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
std::vector<bool> CallableType::get_by_ref(){
	return by_ref;
}


std::vector<Type*> CallableType::get_types(){
	return formal_types;
}

std::vector<std::string> CallableType::get_outer_vars(){
	return outer_vars;
}

std::vector<std::string> CallableType::get_formal_vars(){
	return formal_vars;
}

void CallableType::add_outer(Type*t, std::string name){
	// variable that belongs to outer scope is add as implicit
	//   argument passed by reference
	outer_vars.push_back(name);
	outer_types.push_back(t);
}


FunctionType::FunctionType( Type* ret_ty , FormalDeclList* formals):
	CallableType("function", formals),ret_type(ret_ty){}

Type* FunctionType::get_ret_type(){
	return ret_type->clone();
}

ProcedureType::ProcedureType(FormalDeclList* formals):
	CallableType("procedure", formals){}

Type* Const::get_type(){return type->clone();}

Type* Sconst::get_type(){
	return new ArrType(str.size()+1, CHARACTER::getInstance());
}
Type* Id::get_type(){
	return type->clone();
}
Type* Op::get_type(){
	return resType;
}
Type* Reference::get_type(){
	return new PtrType(lvalue->get_type());
}
Type* Dereference::get_type(){
	Type* con_ty=expr->get_type();
	PtrType* p_ty=static_cast<PtrType*>(con_ty);
	return p_ty->get_type();
}
Type* Brackets::get_type(){
	Type* l_ty = lvalue->get_type();
	ArrType* larr_ty=static_cast<ArrType*>(l_ty);
	return larr_ty->get_type();
}
Type* LabelDecl::get_type(){
	return LABEL::getInstance();
}
Type* VarDecl::get_type(){
	return type->clone();
}
Type* FunctionCall::get_type(){
	return type;
}
std::vector<Type*> DeclList::get_type(){
	std::vector<Type*> types;
	for(auto p=list.begin();p!=list.end();p++){
		types.push_back((*p)->get_type());
	}
	return types;
}
std::vector<Type*> ExprList::get_type(){
	std::vector<Type*> ret;
	for(auto p:list){
		ret.push_back(p->get_type());
	}
	return ret;
}
