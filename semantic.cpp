/* ------------------------------------------
semantic.cpp
Contains member functions that are used in
  semantic analysis (mainly sem)
------------------------------------------ */
#include "ast.hpp"
#include "symbol.hpp"
#include "library.hpp"

void Id::sem(){
	SymbolEntry *e = st.lookup(name);
	if(!e){
		//TODO throw error variable not declared
		std::cerr<<"Id '"<<name<<"' not declared"<<std::endl;
		exit(1);
	}
	type = e->type;
}

void Op::sem(){
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

void Reference::sem(){
	count=-1;
	if(Expr* e=lvalue->simplify(count)){
		delete lvalue;
		lvalue=static_cast<LValue*>(e);
	}
	lvalue->sem();
}


void Dereference::sem(){
	count=1;
	if(Expr* e=expr->simplify(count)){
		delete expr;
		expr=e;
	}
	expr->sem();
	Type* ty=expr->get_type();
	if(ty->get_name().compare("pointer")){
		//TODO error incorrect type
		std::cerr << "ERROR: Can only dereference pointer (not "<<
		ty->get_name()<<")" <<std::endl;
	}
}

Expr* Reference::simplify(int &count){
	count=count-1;
	LValue* tmp=lvalue;
	lvalue=nullptr;
	if(Expr* e= tmp->simplify(count)){
		delete tmp;
		return e;
	}
	return tmp;
}

Expr* Dereference::simplify(int &count){
	count=count+1;
	Expr* tmp=expr;
	expr=nullptr;
	if(Expr* e= tmp->simplify(count)){
		delete tmp;
		return e;
	}
	return tmp;
}

void Brackets::sem(){
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

void Let::sem(){
	expr->sem();
	lvalue->sem();
	Type* lType = lvalue->get_type();
	Type* rType = expr->get_type();
	if(lType->doCompare(rType)) return;

	different_types=true;
	if(!typecheck(lType,rType)){
		/*TODO error type mismatch*/
		std::cerr<<"ERROR: Type mismatch in let!"<<std::endl;
		exit(1);
	}
	if(rType->doCompare(INTEGER::getInstance())){
		is_right_int=true;
		// needed flag to convert int to real before assignment
	}

}

bool Let::typecheck(Type* lType, Type* rType){
 /* is rType compatible for assignment with lType? */
	// same types are compatible
	if(lType->doCompare(rType)) return true;

	if(lType->doCompare(REAL::getInstance()) and rType->doCompare(INTEGER::getInstance()))
	// int is compatible with real
		return true;

	if(!(lType->get_name().compare("pointer")) and !(rType->get_name().compare("pointer"))){
	 // ^array-type WITHOUT size of t is compatible with ^array-type WITH size of t
		// extract inner types from pointers
		PtrType* lpType=static_cast<PtrType*>(lType);
		PtrType* rpType=static_cast<PtrType*>(rType);
		Type* linType=lpType->get_type();
		Type* rinType=rpType->get_type();
		// try to cast inner types as arrays
		if(!(linType->get_name().compare("array")) and !(rinType->get_name().compare("array"))){
			ArrType* larrType=static_cast<ArrType*>(linType);
			ArrType* rarrType=static_cast<ArrType*>(rinType);
			// check sizes (left must be -1, right must be >0)
			if(rarrType->get_size()!=-1 && larrType->get_size()==-1){
				// extract inner types from arrays (must be same type)
				if(larrType->get_type()->doCompare(rarrType->get_type()))
					return true;
			}
		}
	}
	// incompatible types
	return false;
}

void If::sem(){
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

void While::sem(){
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

void New::sem(){
	lvalue->sem();
	if(expr){ // new array object
		expr->sem();
		if(!(expr->get_type()->doCompare(INTEGER::getInstance())) ){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in New statement!"
				<<std::endl;
			exit(1);
		}
		// lvalue must have type : ^array
		Type* idType=lvalue->get_type();
		// try to cast as pointer-type
		if(idType->get_name().compare("pointer") ){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in New statement!"
				<<std::endl;
			exit(1);
		}
		// get inner type of pointer
		PtrType* p=static_cast<PtrType*>(idType);
		Type* t=p->get_type();
		// check if inner type is array-type
		if(t->get_name().compare("array") ){
			/*TODO ERROR incorrect type*/
			std::cerr<<
				"ERROR: Incorrect type of expression in New statement!"
				<<std::endl;
			exit(1);
		}
	}
	else{ // new non-array object
		// lvalue must have type: ^t
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

void Dispose::sem(){
	lvalue->sem();
	// lvalue must be of type pointer
	if(lvalue->get_type()->get_name().compare("pointer")){
		/*TODO ERROR incorrect type*/
		std::cerr<<
			"ERROR: Incorrect type of expression in Dispose statement!"
			<<std::endl;
		exit(1);
	}
}

void DisposeArr::sem(){
	lvalue->sem();
	// lvalue must be of type: ^array
	Type* t=lvalue->get_type();
	// try to cast as pointer
	if(t->get_name().compare("pointer")){
		/*TODO ERROR incorrect type*/
		std::cerr<<
			"ERROR: Incorrect type of expression in DisposeArr statement!"
			<<std::endl;
		exit(1);
	}
	PtrType *pt = static_cast<PtrType*>(t);
	// check if inner type is array-type
	if(pt->get_name().compare("array")){
		/*TODO ERROR incorrect type*/
		std::cerr<<
			"ERROR: Incorrect type of expression in DisposeArr statement!"
			<<std::endl;
		exit(1);
	}
}

void StmtList::sem(){
	for(auto p:list){
		p->sem();
	}
}

void LabelDecl::sem(){
	st.insert(id,LABEL::getInstance());
}

void VarDecl::sem(){
	// insert variable to symbol table
	st.insert(id,type);
}

void DeclList::sem(){
	for(auto p:list)
		p->sem();
}

void Body::sem(){
	if(!isDefined()){
		// body not defined yet
		return;
	}
	declarations->sem();
	statements->sem();
}

void Procedure::sem_helper(bool isFunction, Type* ret_type){
	FunctionEntry* e = st.function_decl_lookup(id);
	if(e and e->body->isDefined()){
		if(isFunction){
			std::cerr<<"Function "<<id<<" already fully declared in this scope."<<std::endl;
		}
		else{
			std::cerr<<"Procedure "<<id<<" already fully declared in this scope."<<std::endl;
		}
		exit(1);
	}

	std::vector<Type*> formal_types=formals->get_type();
	std::vector<bool> by_ref=formals->get_by_ref();
	if(e){ // existent previous declaration without body.
		// must be same type of callable (function / procedure) with
		//   with previous declaration.
		if(e->type->get_name().compare(decl_type)){
			std::cerr<<"Previous declaration of "<<id<<" is not a "<<decl_type<<"."<<std::endl;
			exit(1);
		}
		// types and passing modes must match with previous declaration
		//   (will fail if not).
		if(isFunction){
			FunctionType* func_type=static_cast<FunctionType*>(e->type);
			Type* ret_t=func_type->get_ret_type();
			if(!ret_t->doCompare(ret_type)){
				std::cerr<<"Can't declare function "<<id<<" with different return type."<<std::endl;
				exit(1);
			}
			func_type->typecheck_args(formal_types);
			func_type->check_passing(by_ref);
		}
		else{
			ProcedureType* proc_type=static_cast<ProcedureType*>(e->type);
			proc_type->typecheck_args(formal_types);
			proc_type->check_passing(by_ref);
		}
	}

	if(!body->isDefined()){
		// this is only subprogram header
		CallableType *subp_type;
		if(isFunction){
			subp_type=new FunctionType(ret_type, formals);
		}
		else{
			subp_type=new ProcedureType(formals);
		}
		st.insert_function(id,subp_type,body);
		if(body->isLibrary()){
			// library subprogram; setup type
			type=subp_type;
		}
		// no body to sem; return.
		return;
	}

	// this is full declaration of subprogram
	if(e){ // existent previous declaration.
		// attach body to previous declaration.
		e->body->add_body(body);
	}
	else{ // first declaration of this subprogram
		CallableType *subp_type;
		if(isFunction){
			subp_type=new FunctionType(ret_type, formals);
		}
		else{
			subp_type=new ProcedureType(formals);
		}
		st.insert_function(id,subp_type,body);
		type=subp_type;
	}

	// valid subprogram with body
	st.openScope(id);
	if(isFunction){
		// declare result of function as first local of function.
		VarDecl v(new Decl("result","var"),ret_type); v.sem();
	}
	formals->sem();
	body->sem();
	st.closeScope();
}

void Procedure::sem(){
	sem_helper(); // not a Function
}

void Function::sem(){
	sem_helper(true, ret_type); // flag and ret_type for Function
}

void Program::sem(){
	st.openScope(name);
	// load library subprograms
	for(auto p:library_subprograms){
		p->sem();
	}
	body->sem();
	st.closeScope();
	inp=fopen("pascal_input.inp", "r");
}

void ProcCall::sem(){
	FunctionEntry* e =check_passing();
	if(e->type->get_name().compare("procedure")){
		std::cerr<<"Can't call function "<<name<<" as a procedure."<<std::endl;
		exit(1);
	}
}

void FunctionCall::sem(){
	FunctionEntry* e = check_passing();
	if(e->type->get_name().compare("function")){
		std::cerr<<"Can't call procedure "<<name<<" as a function."<<std::endl;
		exit(1);
	}
	type=static_cast<FunctionType*>(e->type)->get_ret_type();
}

FunctionEntry* Call::check_passing(){
 /* validate call against declaration (
    check that respective arguments have correct types);
    return FunctionEntry. */
	FunctionEntry* e = st.function_lookup(name);
	body=e->body;
	by_ref=e->type->get_by_ref();
	std::vector<Type*> types=e->type->get_types();
	for(uint i=0; i<types.size();i++){
		Expr* expr=(*exprs)[i];
		expr->sem();
		if (by_ref[i] and not expr->isLValue()){
			std::cerr<<"Argument should be an lvalue expression."<<std::endl;
			exit(1);
		}

		Type* lType = types[i];
		Type* rType = expr->get_type();

		if(by_ref[i]){ // pass by-reference
			// ^type of parameter must be compatible for assignment with
			//    ^type of argument
			if(Let::typecheck(new PtrType(lType),new PtrType(rType)))
			continue;
		}
		else{ // pass by-value
			// type of parameter must be compatible for assignment with
			//    type of argument
			if(Let::typecheck(lType, rType))
			continue;
		}

		/*TODO error type mismatch*/
		std::cerr<<"ERROR: Type mismatch in call! ("<<"expected "<<*lType<<" but received "<<*rType<<" instead."<<std::endl;
		exit(1);
	}

	// add implicit vars from outer scope
	for(auto name: e->type->get_outer_vars()){
		Expr* i = new Id(name); i->sem();
		outer_vars->append(i);
	}
	return e;

}

void Body::add_body(Body *b){
 /* fill body object with declarations and statements*/
	defined=true;
	declarations=b->declarations;
	statements=b->statements;
}

bool Body::isDefined(){
	return defined;
}

void Procedure::add_body(Body* bod){
	if(body->isDefined()){
		std::cerr<<id<<" already has body."<<std::endl;
		exit(1);
	}
	body->add_body(bod);
}


std::vector<bool> FormalDeclList::get_by_ref(){
 /*return passing mode of each parameter in a vector*/
	std::vector<bool> by_ref;
	for(auto p=list.begin();p!=list.end();p++){
		FormalDecl* f=static_cast<FormalDecl*>(*p);
		by_ref.push_back(f->isByRef());
	}
	return by_ref;
}

std::vector<std::string> FormalDeclList::get_names(){
 /*return passing mode of each parameter in a vector*/
	std::vector<std::string> names;
	for(auto p=list.begin();p!=list.end();p++){
		FormalDecl* f=static_cast<FormalDecl*>(*p);
		names.push_back(f->get_id());
	}
	return names;
}
