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
		std::ostringstream stream;
		stream<<"Id '"<<name<<"' not declared";
		this->report_error(stream.str().c_str());
		exit(1);
	}
	type = e->type;
}

void Op::sem(){
	// sets leftType, rightType and resType fields
	// it should be run only once even when we have repeated evals
	// e.g in while
	left->sem();
	leftType=left->get_type();

	if(right!=nullptr){//BinOps
		right->sem();
		rightType=right->get_type();
		if(!(op.compare("+")) or !(op.compare("-")) or !(op.compare("*"))){
			//real or int operands-> real or int result
			if( (leftType->doCompare(REAL::getInstance()) or leftType->doCompare(INTEGER::getInstance()))
			and (rightType->doCompare(REAL::getInstance()) or rightType->doCompare(INTEGER::getInstance())) ){
				//is a number (real or int)
				if(leftType->doCompare(REAL::getInstance()) or rightType->doCompare(REAL::getInstance()))
					// one of them is real
					resType = REAL::getInstance();
				else
					resType = INTEGER::getInstance();
			}
		}

		if(!(op.compare("/"))){
			//real or int operands-> real result
			if( (leftType->doCompare(REAL::getInstance()) or leftType->doCompare(INTEGER::getInstance()))
			and (rightType->doCompare(REAL::getInstance()) or rightType->doCompare(INTEGER::getInstance())))
				//is a number (real or int)
				resType = REAL::getInstance();
		}

		else if(!(op.compare("div")) or !(op.compare("mod")) ){
			//int operands-> int result
			if(  leftType->doCompare(INTEGER::getInstance()) and rightType->doCompare(INTEGER::getInstance()))
				//is int
				resType = INTEGER::getInstance();
		}

		else if(!(op.compare("=")) or !(op.compare("<>"))){
			// either int/real operands or any non-array type operands
			// -> bool result
			if(( leftType->doCompare(REAL::getInstance()) or leftType->doCompare(INTEGER::getInstance()))
			and (rightType->doCompare(REAL::getInstance()) or rightType->doCompare(INTEGER::getInstance())))
				// int/real operands
				resType = BOOLEAN::getInstance();

			else if(leftType->doCompare(rightType)
			and leftType->get_name().compare("array") )
				// same type and not an array operands
				resType = BOOLEAN::getInstance();
		}

		else if(!(op.compare("<=")) or !(op.compare(">=")) or !(op.compare("<"))
				or !(op.compare(">")) ){
			//real or int operands-> bool result
			if( (leftType->doCompare(REAL::getInstance()) or leftType->doCompare(INTEGER::getInstance()))
			and (rightType->doCompare(REAL::getInstance()) or rightType->doCompare(INTEGER::getInstance())))
				//is a number (real or int)
				resType = BOOLEAN::getInstance();
		}

		else if(!(op.compare("and")) or !(op.compare("or")) ){
			//bool operands-> bool result
			if(  leftType->doCompare(BOOLEAN::getInstance()) and rightType->doCompare(BOOLEAN::getInstance()))
				//is bool
				resType = BOOLEAN::getInstance();
		}
		else if(!(op.compare("[]"))){
			if(!(leftType->get_name().compare("array"))){
				if(rightType->doCompare(INTEGER::getInstance())){
					SPtr<ArrType> p=std::static_pointer_cast<ArrType>(leftType);
					resType=p->get_type();
				}
			}
		}
		if(!resType){
			std::ostringstream stream;
			stream<<"Type mismatch: Cannot apply operator '"<<op<<
				"' to operands of type '"<<*leftType<<"' and '"<<*rightType<<"'.";
			this->report_error(stream.str().c_str());
			exit(1);
		}
		return;
	}
	//UNOP
	if(!(op.compare("+")) or !(op.compare("-"))){
		//real or int operand-> real or int result
		if( leftType->doCompare(REAL::getInstance()) or leftType->doCompare(INTEGER::getInstance()) )
			resType = leftType;
	}
	else if(!(op.compare("not"))){
		//bool operand-> bool result
		if(leftType->doCompare(BOOLEAN::getInstance()) )
			resType = BOOLEAN::getInstance();
	}
	if(!resType){
		std::ostringstream stream;
		stream<<"Type mismatch: Cannot apply operator '"<<op<<
			"' to operand of type '"<<*leftType<<"'.";
		this->report_error(stream.str().c_str());
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
	TSPtr ty(expr->get_type());
	if(ty->get_name().compare("pointer")){
		std::ostringstream stream;
		stream << "Can only dereference pointer; not '"<<
			*expr<<"' of type '"<<*ty<<"'." ;
		this->report_error(stream.str().c_str());
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
	TSPtr l_ty (lvalue->get_type());
	if(l_ty->get_name().compare("array")){
		std::ostringstream stream;
		stream << "Can only apply brakets to array; not '"<<
			*lvalue<<"' of type '"<<*l_ty<<"'." ;
		this->report_error(stream.str().c_str());
		exit(1);
	}
	if(!expr->get_type()->doCompare(INTEGER::getInstance())){
		std::ostringstream stream;
		stream << "Array index should be integer; not"<<
			*expr<<"' of type '"<<*expr->get_type()<<"'." ;
		this->report_error(stream.str().c_str());
		exit(1);
	}
}

void Let::sem(){
	expr->sem();
	lvalue->sem();
	TSPtr lType (lvalue->get_type());
	TSPtr rType(expr->get_type());
	if((rType->get_name().compare("any")) and (lType->doCompare(rType))){
	// if same types (not any) return.
		if(!lType->is_incomplete() and !rType->is_incomplete()){
			return;
		}
	}
	different_types=true;
	if(!typecheck(lType,rType)){
		std::ostringstream stream;
		stream<<"Could not assign '"<<*expr<<"' of type '"<<
			*rType<<"' to '"<<*lvalue<<"' of type '"<<*lType<<"'";
		if(lType->is_incomplete()){
			stream<<" (incomplete types are not assignable)";
		}
		stream<<".";
		this->report_error(stream.str().c_str());
		exit(1);
	}
	if(rType->doCompare(INTEGER::getInstance())){
		is_right_int=true;
		// needed flag to convert int to real before assignment
	}

}

void LabelStmt::sem(){
	st.label_lookup(label_id);
	stmt->sem();
}

void Goto::sem(){
	st.label_lookup(label_id);
}

bool Let::typecheck(TSPtr lType, TSPtr rType){
 /* is rType compatible for assignment with lType? */
	// any(untyped) is incompatible with other types.
	if(!rType->get_name().compare("any")){
		// error; probably from nil^.
		return false;
	}
	// no type is compatible with incomplete type.
	if(lType->is_incomplete() or rType->is_incomplete()){
		return false;
	}
	// same types are compatible
	if(lType->doCompare(rType)) return true;

	if(lType->doCompare(REAL::getInstance()) and rType->doCompare(INTEGER::getInstance()))
	// int is compatible with real
		return true;

	if(!(lType->get_name().compare("pointer")) and !(rType->get_name().compare("pointer"))){
	 // ^array-type WITH size of t is compatible with ^array-type WITHOUT size of t
		// extract inner types from pointers
		SPtr<PtrType> lpType = std::static_pointer_cast<PtrType>(lType);
		SPtr<PtrType> rpType = std::static_pointer_cast<PtrType>(rType);
		TSPtr linType(lpType->get_type());
		TSPtr rinType(rpType->get_type());
		// try to cast inner types as arrays
		if(!(linType->get_name().compare("array")) and !(rinType->get_name().compare("array"))){
			SPtr<ArrType> larrType = std::static_pointer_cast<ArrType>(linType);
			SPtr<ArrType> rarrType = std::static_pointer_cast<ArrType>(rinType);
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
	TSPtr expr_t(expr->get_type());
	if(!(expr_t == BOOLEAN::getInstance())){
		std::ostringstream stream;
		stream<<"Expression '"<<*expr<<
			"' in 'if' statement should be 'boolean' not '"<<*expr_t<<"'.";
		this->report_error(stream.str().c_str());
		exit(1);
	}
	stmt1->sem();
	if(stmt2)
		stmt2->sem();
}

void While::sem(){
	expr->sem();
	TSPtr expr_t(expr->get_type());
	if(!(expr_t == BOOLEAN::getInstance())){
		std::ostringstream stream;
		stream<<"Expression '"<<*expr<<
			"' in 'while' statement should be 'boolean' not '"<<*expr_t<<"'.";
		this->report_error(stream.str().c_str());
		exit(1);
	}
	stmt->sem();
}

void New::sem(){
	lvalue->sem();
	if(expr){ // new array object
		expr->sem();
		TSPtr expr_t(expr->get_type());
		if(!(expr_t == INTEGER::getInstance())){
			std::ostringstream stream;
			stream<<"Expression '"<<*expr<<
				"' in 'new' statement should be 'integer' not '"<<*expr_t<<"'.";
			this->report_error(stream.str().c_str());
			exit(1);
		}
		// lvalue must have type : ^array
		TSPtr idType(lvalue->get_type());
		// try to cast as pointer-type
		if(idType->get_name().compare("pointer") ){
			std::ostringstream stream;
			stream<<"Lvalue '"<<*lvalue<<
				"' in 'new' statement should be '^array of ..' not '"<<*idType<<"'.";
			this->report_error(stream.str().c_str());
			exit(1);
		}
		// get inner type of pointer
		SPtr<PtrType> p = std::static_pointer_cast<PtrType>(idType);
		TSPtr t(p->get_type());
		// check if inner type is array-type
		if(t->get_name().compare("array") ){
			std::ostringstream stream;
			stream<<"Lvalue '"<<*lvalue<<
				"' in 'new []' statement should be pointer to array not '"
				<<*idType<<"'.";
			this->report_error(stream.str().c_str());
			exit(1);
		}
	}
	else{ // new non-array object
		// lvalue must have type: ^t
		TSPtr idType(lvalue->get_type());
		if(idType->get_name().compare("pointer") ){
			std::ostringstream stream;
			stream<<"Lvalue '"<<*lvalue<<
				"' in 'new' statement should be pointer not '"<<*idType<<"'.";
			this->report_error(stream.str().c_str());
			exit(1);
		}
	}
}

void Dispose::sem(){
	lvalue->sem();
	// lvalue must be of type pointer
	TSPtr idType(lvalue->get_type());
	if(idType->get_name().compare("pointer") ){
		std::ostringstream stream;
		stream<<"Lvalue '"<<*lvalue<<
			"' in 'dispose' statement should be pointer not '"<<*idType<<"'.";
		this->report_error(stream.str().c_str());
		exit(1);
	}
}

void DisposeArr::sem(){
	lvalue->sem();
	// lvalue must be of type: ^array
	TSPtr t(lvalue->get_type());
	// try to cast as pointer
	if(t->get_name().compare("pointer") ){
		std::ostringstream stream;
		stream<<"Lvalue '"<<*lvalue<<
			"' in 'dispose []' statement should be pointer to array not '"<<*t<<"'.";
		this->report_error(stream.str().c_str());
		exit(1);
	}
	SPtr<PtrType > pt  = std::static_pointer_cast<PtrType>(t);
	TSPtr inType(pt->get_type());
	// check if inner type is array-type
	if(inType->get_name().compare("array")){
		std::ostringstream stream;
		stream<<"Lvalue '"<<*lvalue<<
			"' in 'dispose []' statement should be pointer to array not '"<<*t<<"'.";
		this->report_error(stream.str().c_str());
		exit(1);
	}
}

void StmtList::sem(){
	for(auto p:list){
		p->sem();
	}
}

void LabelDecl::sem(){
	st.insert_label(id);
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

void Procedure::sem_helper(bool isFunction, TSPtr ret_type){
	FunctionEntry* e = st.function_decl_lookup(id);
	if(e and e->body->isDefined()){
		if(isFunction){
			std::ostringstream stream;
			stream<<"Function '"<<id<<"' already fully declared in this scope.";
			this->report_error(stream.str().c_str());
		}
		else{
			std::ostringstream stream;
			stream<<"Procedure '"<<id<<"' already fully declared in this scope.";
			this->report_error(stream.str().c_str());
		}
		exit(1);
	}

	std::vector<TSPtr> formal_types=formals->get_type();
	std::vector<bool> by_ref=formals->get_by_ref();
	if(e){ // existent previous declaration without body.
		// must be same type of callable (function / procedure) with
		//   with previous declaration.
		if(e->type->get_name().compare(decl_type)){
			std::ostringstream stream;
			stream<<"Previous declaration of '"<<id<<"' is not a "<<decl_type<<".";
			this->report_error(stream.str().c_str());
			exit(1);
		}
		// types and passing modes must match with previous declaration
		//   (will fail if not).
		if(isFunction){
			SPtr<FunctionType> func_type = std::static_pointer_cast<FunctionType>(e->type);
			TSPtr ret_t(func_type->get_ret_type());
			if(!ret_t->doCompare(ret_type)){
				std::ostringstream stream;
				stream<<"Can't declare function '"<<id<<"' with different return type.";
				this->report_error(stream.str().c_str());
				exit(1);
			}
			func_type->typecheck_args(formal_types);
			func_type->check_passing(by_ref);
		}
		else{
			SPtr<ProcedureType> proc_type = std::static_pointer_cast<ProcedureType>(e->type);
			proc_type->typecheck_args(formal_types);
			proc_type->check_passing(by_ref);
		}
	}

	if(!body->isDefined()){
		// this is only subprogram header
		SPtr<CallableType> subp_type;
		if(isFunction){
			subp_type = std::static_pointer_cast<CallableType>(
				std::make_shared<FunctionType>(ret_type, formals)
			);
		}
		else{
			subp_type = std::static_pointer_cast<CallableType>(
				std::make_shared<ProcedureType>(formals)
			);
		}
		st.insert_function(id,subp_type,body);
		type=subp_type;
		// if(body->isLibrary()){
		// 	// library subprogram; setup type
		// 	type = subp_type;
		// }
		// no body to sem; return.
		return;
	}

	// this is full declaration of subprogram
	if(e){ // existent previous declaration.
		// attach body to previous declaration.
		e->body->add_body(body);
		type=e->type;
	}
	else{ // first declaration of this subprogram
		SPtr<CallableType> subp_type;
		if(isFunction){
			subp_type = std::static_pointer_cast<CallableType>(
				std::make_shared<FunctionType>(ret_type, formals)
			);
		}
		else{
			subp_type = std::static_pointer_cast<CallableType>(
				std::make_shared<ProcedureType>(formals)
			);
		}
		st.insert_function(id,subp_type,body);
		type = subp_type;
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
	st.openScope("library");
	// outer scope contains only library functions
	// load library subprograms
	for(auto p:library_subprograms){
		p->sem();
	}
	st.openScope(name);
	body->sem();
	st.closeScope();
	st.closeScope();
}

void ProcCall::sem(){
	FunctionEntry* e =check_passing();
	if(e->type->get_name().compare("procedure")){
		std::ostringstream stream;
		stream<<"Can't call function '"<<name<<"' as a procedure.";
		this->report_error(stream.str().c_str());
		exit(1);
	}
}

void FunctionCall::sem(){
	FunctionEntry* e = check_passing();
	if(e->type->get_name().compare("function")){
		std::ostringstream stream;
		stream<<"Can't call procedure '"<<name<<"' as a function.";
		this->report_error(stream.str().c_str());
		exit(1);
	}
	type = std::static_pointer_cast<FunctionType>(e->type)->get_ret_type();
}

FunctionEntry* Call::check_passing(){
 /* validate call against declaration (
    check that respective arguments have correct types);
    return FunctionEntry. */
	FunctionEntry* e = st.function_lookup(name);
	body=e->body;
	by_ref=e->type->get_by_ref();
	std::vector<TSPtr> types=e->type->get_types();
	for(uint i=0; i<types.size();i++){
		Expr* expr=(*exprs)[i];
		expr->sem();
		if (by_ref[i] and not expr->isLValue()){
			std::ostringstream stream;
			stream<<"Argument '"<<*expr<<"' of '"<<name<<
				"' should be an lvalue expression.";
			this->report_error_from_child(stream.str().c_str());
			exit(1);
		}

		TSPtr lType(types[i]);
		TSPtr rType(expr->get_type());

		if(by_ref[i]){ // pass by-reference
			// ^type of parameter must be compatible for assignment with
			//    ^type of argument
			SPtr<PtrType> lp (new PtrType(lType));
			SPtr<PtrType> rp (new PtrType(rType));
			if(Let::typecheck( lp, rp)){
				continue;
			}
		}
		else{ // pass by-value
			// type of parameter must be compatible for assignment with
			//    type of argument
			if(Let::typecheck(lType, rType))
			continue;
		}

		/*TODO error type mismatch*/
		std::ostringstream stream;

		stream<<"Type mismatch in call of '"<<name<<"' ('"<<*expr<<
			"' is of type '"<<*rType<<"'; '"<<*lType<<"' was expected).";
		this->report_error_from_child(stream.str().c_str());
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
		std::ostringstream stream;
		stream<<"Subprogram '"<<id<<"' already has body.";
		this->report_error(stream.str().c_str());
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
