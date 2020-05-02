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
	offset = e->offset;
	decl_nesting = e->nesting;
	current_nesting=st.getNestingOfCurrentScope();
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
	lvalue->sem();
}

void Dereference::sem(){
	expr->sem();
	Type* ty=expr->get_type();
	if(ty->get_name().compare("pointer")){
		//TODO error incorrect type
		std::cerr << "ERROR: Can only dereference pointer (not "<<
		ty->get_name()<<")" <<std::endl;
	}
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
	}

}

bool Let::typecheck(Type* lType, Type* rType){
	if(lType->doCompare(rType)) return true;

	if(lType->doCompare(REAL::getInstance()) and rType->doCompare(INTEGER::getInstance()))
		return true;
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
					return true;
			}
		}
	}
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

void Dispose::sem(){
	lvalue->sem();
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

void StmtList::sem(){
	for(auto p:list){
		p->sem();
	}
}

void LabelDecl::sem(){
	st.insert(id,LABEL::getInstance());
}

void VarDecl::sem(){
	int s=get_sizeof(type);
	st.insert(id,type,s);
}

int VarDecl::get_sizeof(Type* t){
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

void DeclList::sem(){
	for(auto p:list)
		p->sem();
}

void Body::sem(){
	if(!isDefined()){
		return;
	}
	declarations->sem();
	statements->sem();
	size = st.getSizeOfCurrentScope();
}

void Procedure::sem(){
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

	st.openScope(id);
	formals->sem();
	body->sem();
	st.closeScope();
}

void Function::sem(){
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

	st.openScope(id);
	// declare result of function as first local of function (offset 1 from fp)
	VarDecl v(new Decl("result","var"),ret_type); v.sem();
	formals->sem();
	body->sem();
	st.closeScope();
}

void Program::sem(){
	st.openScope(name);
	for(auto p:library_subprograms){
		p->sem();
	}
	body->sem();
	size=st.getSizeOfCurrentScope();
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

		if(by_ref[i]){
			if(Let::typecheck(new PtrType(lType),new PtrType(rType)))
			continue;
		}
		else{
			if(Let::typecheck(lType, rType))
			continue;
		}

		/*TODO error type mismatch*/
		std::cerr<<"ERROR: Type mismatch in call! ("<<"expected "<<*lType<<" but received "<<*rType<<" instead."<<std::endl;
		exit(1);
	}


	nesting_diff=st.getNestingOfCurrentScope()-e->nesting;
	next_fp_offset=st.getSizeOfCurrentScope()+1;
	return e;

}

void Body::add_body(Body *b){
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
	std::vector<bool> by_ref;
	for(auto p=list.begin();p!=list.end();p++){
		FormalDecl* f=static_cast<FormalDecl*>(*p);
		by_ref.push_back(f->isByRef());
	}
	return by_ref;
}
