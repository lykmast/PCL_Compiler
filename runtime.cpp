/* ------------------------------------------
runtime.cpp
Contains member functions that are used in
  runtime such as eval, let, run
------------------------------------------ */
#include "ast.hpp"
// common helper functions used in multiple basic members
inline void print_stack(){
	std::cout<<"STACK:"<<std::endl;
	for(auto p : rt_stack ){
		if(p)
			std::cout<<*p;
		else
			std::cout<<"EMPTY";
	std::cout<<std::endl;
	}
}


LValue* Brackets::element(){
	// i= index
	value v = expr->eval();
	int i = v.i;
	// find arrconst by eval
	v = lvalue->eval();
	Arrconst *arr = static_cast<Arrconst*>(v.lval);
	// retrieve i-th element
	return arr->get_element(i);
}

LValue* StaticArray::get_element(int i){
	return rt_stack[offset+i*child_size]; // offset is absolute
}
LValue* DynamicArray::get_element(int i){
	return arr[i];
}


int Id::find_absolute_offset(){
/* finds absolute offset of id in stack (
    in contrast with relative to fp)*/
	unsigned long prev_fp=fp;
	for(int diff=current_nesting-decl_nesting; diff>0; diff--){
		// if id is declared in outer scope
		//    follows access links until correct frame is found
		prev_fp=rt_stack[prev_fp]->eval().uli;
	}
	return prev_fp+offset;
}


// Dynamic object creation
// UnnamedLValue* Type::create()
UnnamedLValue* REAL::create() const{
	return new UnnamedLValue(REAL::getInstance(),true);
}

UnnamedLValue* INTEGER::create() const{
	return new UnnamedLValue(INTEGER::getInstance(),true);
}

UnnamedLValue* CHARACTER::create() const{
	return new UnnamedLValue(CHARACTER::getInstance(),true);
}
UnnamedLValue* PtrType::create() const{
	return new UnnamedLValue(new PtrType(type),true);
}

UnnamedLValue* ArrType::create() const{
	return new DynamicArray(size,type);
}

UnnamedLValue* BOOLEAN::create() const{
	return new UnnamedLValue(BOOLEAN::getInstance(),true);
}
// StaticArray creation
//   (used in Id::eval() and Id::getBox())
void Id::create_static_array(int abs_ofs, ArrType* t){
	Type* inside_t=t->get_type();
	int s = t->get_size();
		rt_stack[abs_ofs] = new StaticArray(s,inside_t,abs_ofs+1);
}

// value Expr::eval()
//   evaluates expression; returns result (of type value)
value UnnamedLValue::eval(){
	return val;
}

value Rconst::eval(){
	value v; v.r=num;
	return v;
}

value Iconst::eval() {
	value v; v.i=num;
	return v;
}

value Cconst::eval() {
	value v; v.c=ch;
	return v;
}

value Pconst::eval() {
	value v; v.lval=ptr;
	return v;
}

value Bconst::eval() {
	value v; v.b=boo;
	return v;
}

value Id::eval(){
	int abs_ofs=find_absolute_offset();
	value v = rt_stack[abs_ofs]->eval();
	if(!v.lval and !type->get_name().compare("array")){ // empty array
		// create static array before returning
		ArrType* arrT=static_cast<ArrType*>(type);
		create_static_array(abs_ofs, arrT);
		return rt_stack[abs_ofs]->eval();
	}
	else return v;
}

value Op::eval() {
	// before the first eval of any op, there must have been one typecheck to
	// fill resType, leftType, rightType
	Type* realType=REAL::getInstance();
	Type* intType=INTEGER::getInstance();
	value leftValue=left->eval();
	value rightValue;
	value ret;
	if(!(op.compare("+")) and right){
		//BinOp
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		if(resType->doCompare(realType)){
			ret.r = li+ri+lr+rr;
		}
		else{
			ret.i = li+ri;
		}
	}
	else if(!(op.compare("+"))){
		//UnOp
		int li=0;
		double lr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(resType->doCompare(realType)){
			ret.r=li+lr;
		}
		else{
			ret.i=li;
		}
	}
	else if(!(op.compare("-")) and right) {
		//BinOp
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		if(resType->doCompare(realType)){
			ret.r =li-ri+lr-rr;
		}
		else{
			ret.i = li-ri;
		}
	}
	else if(!(op.compare("-"))){
		//UnOp
		int li=0;
		double lr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(resType->doCompare(realType)){
			ret.r = -li-lr;
		}
		else{
			ret.i = -li;
		}
	}
	else if(!(op.compare("*"))) {
		//BinOp
		rightValue=right->eval();

		int li=1,ri=1;
		double lr=1,rr=1;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		if(resType->doCompare(realType)){
			ret.r = li*ri*lr*rr;
		}
		else{
			ret.i = li*ri;
		}
	}
	else if(!(op.compare("/"))){
		int li=1,ri=1;
		double lr=1,rr=1;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		if(resType->doCompare(realType)){
			ret.r = (li/rr)*(lr/ri);
		}
	}

	else if( !(op.compare("div"))){
		rightValue=right->eval();

		int li=leftValue.i;
		int ri=rightValue.i;
		ret.i = li/ri;
	}
	else if(!(op.compare("mod"))) {
		rightValue=right->eval();

		int li=leftValue.i;
		int ri=rightValue.i;
		ret.i = li%ri;
	}
	else if(!(op.compare("<>"))) {
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		LValue* lptr, *rptr;
		char type='n';
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else if(leftType->doCompare(intType)){
			li=leftValue.i;
		}
		else if (leftType->doCompare(CHARACTER::getInstance())){
			li=leftValue.c;
			type='c';
		}
		else if (leftType->doCompare(BOOLEAN::getInstance())){
			li=leftValue.b;
			type='b';
		}
		else{
			lptr=leftValue.lval;
			type='l';
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else if(rightType->doCompare(intType)){
			ri=rightValue.i;
		}
		else if (rightType->doCompare(CHARACTER::getInstance())){
			ri=rightValue.c;
		}
		else if (rightType->doCompare(BOOLEAN::getInstance())){
			ri=rightValue.b;
		}
		else{
			rptr=rightValue.lval;
		}
		switch (type) {
			case 'n':
			ret.b = li+lr!=ri+rr;
			break;
			case 'c': case 'b':
			ret.b= li!=ri;
			break;
			default:
			ret.b = rptr!=lptr;
		}

	}
	else if(!(op.compare("="))) {
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		LValue* lptr, *rptr;
		char type='n';
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else if(leftType->doCompare(intType)){
			li=leftValue.i;
		}
		else if (leftType->doCompare(CHARACTER::getInstance())){
			li=leftValue.c;
			type='c';
		}
		else if (leftType->doCompare(BOOLEAN::getInstance())){
			li=leftValue.b;
			type='b';
		}
		else{
			lptr=leftValue.lval;
			type='l';
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else if (rightType->doCompare(intType)){
			ri=rightValue.i;
		}
		else if (rightType->doCompare(CHARACTER::getInstance())){
			ri=rightValue.c;
		}
		else if (rightType->doCompare(BOOLEAN::getInstance())){
			ri=rightValue.b;
		}
		else{
			rptr=rightValue.lval;
		}
		switch (type) {
			case 'n':
			ret.b = li+lr==ri+rr;
			break;
			case 'c': case 'b':
			ret.b= li==rr;
			break;
			default:
			ret.b = rptr==lptr;
		}
	}

	else if(!(op.compare("<="))) {
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		ret.b = li+lr<=ri+rr;
	}
	else if(!(op.compare(">="))) {
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		ret.b = li+lr>=ri+rr;
	}
	else if(!(op.compare(">"))) {
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		ret.b = li+lr > ri+rr;
	}
	else if(!(op.compare("<"))) {
		rightValue=right->eval();

		int li=0,ri=0;
		double lr=0,rr=0;
		if(leftType->doCompare(realType)){
			lr=leftValue.r;
		}
		else{
			li=leftValue.i;
		}
		if(rightType->doCompare(realType)){
			rr=rightValue.r;
		}
		else{
			ri=rightValue.i;
		}
		ret.b = li+lr < ri+rr;
	}
	else if(!(op.compare("and"))) {
		bool lb=leftValue.b;
		if(!lb){
			// short-circuit and
			ret.b=false;
		}
		else{
			rightValue=right->eval();
			bool rb=rightValue.b;
			ret.b = lb and rb;
		}
	}
	else if(!(op.compare("or"))) {
		bool lb=leftValue.b;
		if(lb){
			// short-circuit or
			ret.b=true;
		}
		else{
			rightValue=right->eval();
			bool rb=rightValue.b;
			ret.b = lb or rb;
		}
	}
	else if(!(op.compare("not"))) {
		//UnOp

		bool lb=leftValue.b;
		ret.b = not lb;
	}
	return ret;
}

value Reference::eval(){
	value v; v.lval=lvalue;
	return v;
}

value Dereference::eval(){
	value v = expr->eval();
	return (v.lval)->eval();
}

value Brackets::eval(){
	return element()->eval();
}

std::vector<value> ExprList::eval(std::vector<bool> by_ref){
	// eval every expression
	// considering passing mode (by-reference / by-value)
	std::vector<value> ret(list.size());
	for(uint i=0; i<list.size(); i++){
		if(by_ref[i]){ // passing mode is by-reference
			// return same container by getBox
			ret[i].lval=static_cast<LValue*>(list[i])->getBox();
		}
		else{
			ret[i]=list[i]->eval();
		}
	}
	return ret;
}
//   (for FunctionCall::eval() see in run)


// UnnamedLValue * LValue::getBox
//    returns UnnamedLValue which is
//    the fundamental container of values
UnnamedLValue* UnnamedLValue::getBox(){
	return this;
}

UnnamedLValue* Id::getBox(){
	int abs_ofs=find_absolute_offset();
	value v = rt_stack[abs_ofs]->eval();
	if(!v.lval and !type->get_name().compare("array")){ // empty array
		// create static array before returning
		ArrType* arrT=static_cast<ArrType*>(type);
		create_static_array(abs_ofs, arrT);
	}
	return rt_stack[abs_ofs]->getBox();
}

UnnamedLValue* Dereference::getBox(){
	value v = expr->eval();
	return (v.lval)->getBox();
}

UnnamedLValue* Brackets::getBox(){
	return element()->getBox();
}



// void LValue::let(value )
//    assigns value to LValue
void UnnamedLValue::let(value v){
	val=v;
}

void Id::let(value v){
	rt_stack[find_absolute_offset()]->let(v);
}

void Dereference::let(value v){
	// vlval.lval is pointer to LValue expecting assignment
	value vlval = expr->eval();
	(vlval.lval)->let(v);
}

void Brackets::let(value v){
	element()->let(v);
}



// void Stmt::run()
void Let::run() const{
	value v = expr->eval();
	if(different_types and is_right_int){ //right is integer and left is real
		// first convert integer to real
		v.r=v.i;
	}
	lvalue->let(v);
}

void If::run() const{
	value v=expr->eval();
	bool e = v.b;
	if(e) stmt1->run();
	else if (stmt2) stmt2->run();
}

void While::run() const{
	value v=expr->eval();
	bool e = v.b;
	while(e) {
		stmt->run();
		v=expr->eval();
		e = v.b;
	}
}

void New::run() const{
	if(expr){ // new array object
		// i=size of array
		value v = expr->eval();
		int i = v.i;
		if(i<=0) {
			/*TODO ERROR wrong value*/
			std::cerr<<
				"ERROR: Wrong value for array size in New statement!"
				<<std::endl;
			exit(1);
		}
		// get array-type contained in pointer
		Type* idType=lvalue->get_type();
		PtrType* p=static_cast<PtrType*>(idType);
		Type* t=p->get_type();
		ArrType* arrT = static_cast<ArrType*>(t);
		// transform array-type to array-type with size
		ArrType* arrT_size = new ArrType(i,arrT->get_type());
		// create dynamic array according to array-type with size
		value vlval; vlval.lval = arrT_size->create();
		// assign pointer of new object to lvalue
		lvalue->let(vlval);
	}
	else{ // new non-array object
		// get type contained in pointer
		Type* idType=lvalue->get_type();
		PtrType* p=static_cast<PtrType*>(idType);
		Type* t=p->get_type();
		// create dynamic object according to type
		value vlval; vlval.lval = t->create();
		// assign pointer of new object to lvalue
		lvalue->let(vlval);
	}
}

void Dispose::run() const{
	value v = lvalue->eval();
	LValue* ptr = v.lval;
	if(!(ptr->isDynamic())){
		/*TODO ERROR non dynamic pointer (RUNTIME)*/
		std::cerr<<
			"ERROR: non dynamic pointer in Dispose (RUNTIME)"
			<<std::endl;
		exit(1);
	}
	delete ptr;
	v.lval=nullptr;
	lvalue->let(v);
}

void DisposeArr::run() const{
	value v = lvalue->eval();
	LValue* ptr = v.lval;
	if(!(ptr->isDynamic())){
		/*TODO ERROR non dynamic pointer (RUNTIME)*/
		std::cerr<<
			"ERROR: non dynamic pointer in DisposeArr (RUNTIME)"
			<<std::endl;
		exit(1);
	}
	delete ptr;
	v.lval=nullptr;
	lvalue->let(v);
}

void StmtList::run() const{
	for(auto p:list){
		if(p->isReturn())
			return;
		p->run();
	}
}


void Body::run() const{
	statements->run();
	// print_stack();
}

void Program::run(){
	// access link of program points to itself (al=fp=0)
	value v; v.uli=0;
	rt_stack.push_back(new UnnamedLValue(v,INTEGER::getInstance()));
	fp=0;
	// push empty boxes to program for locals
	for(int i=0; i<body->get_size()-1; i++){
		value v; v.lval=nullptr;
		rt_stack.push_back(new UnnamedLValue(v,ANY::getInstance()));
	}
	body->run();
	// pop locals
	for(int i=0; i<body->get_size(); i++){
		delete rt_stack.back();
		rt_stack.pop_back();
	}
}

int Body::get_size(){
/* returns size of locals and arguments of this body (
   necessary for pushing correct number of empty boxes in stack) */
	return size;
}

void Call::before_run(bool isFunction) const{
/* prepare call by changing stack state (
   evaluate arguments; push fp, access link, return value and arguments
   to stack) */
	// first evaluate arguments
	std::vector<value> outer_args(
		outer_vars->eval(std::vector<bool>(outer_vars->size(),true))
	);
	std::vector<value> args(exprs->eval(by_ref));
	// then start pushing things in the stack
	unsigned long next_fp=fp+next_fp_offset;
	// push current fp at next_fp-1 offset in stack
	value v; v.uli=fp;
	rt_stack.push_back(new UnnamedLValue(v,INTEGER::getInstance())); //next_fp-1
	// push access link at next_fp offset in stack
	if(nesting_diff<0){
		rt_stack.push_back(new UnnamedLValue(v,INTEGER::getInstance())); //next_fp
	}
	else{
		unsigned long prev_fp=fp;
		for(int diff=nesting_diff; diff>0; diff--){
			prev_fp=rt_stack[prev_fp]->eval().uli;
		}
		value v=rt_stack[prev_fp]->eval();
		rt_stack.push_back(new UnnamedLValue(v,INTEGER::getInstance())); //next_fp

	}
	if(isFunction){
		// push one more value for function result at next_fp+1 offset
		value v; v.lval=nullptr;
		rt_stack.push_back(new UnnamedLValue(v,ANY::getInstance()));
	}
	// push arguments in stack (lvalues for references)
	for(uint i=0; i<args.size(); i++){
		if(by_ref[i]){ // passing by-reference
			// exprs->eval has evaled expr to box (UnnamedLValue*)
			rt_stack.push_back(static_cast<UnnamedLValue*>(args[i].lval));
		}
		else{
			rt_stack.push_back(new UnnamedLValue(args[i],ANY::getInstance()));
		}
	}
	// push empty spaces for callee locals (body size-arguments that are already pushed)
	int size=body->get_size()-exprs->size()-1-outer_args.size();
	if(isFunction)
		size-=1;
	for (int i = 0; i < size; ++i){
		value v; v.lval=nullptr;
		rt_stack.push_back(new UnnamedLValue(v,ANY::getInstance()));
	}

	//push outer values AFTER locals passing them by reference
	for(auto outer:outer_args){
		rt_stack.push_back(static_cast<UnnamedLValue*>(outer.lval));
	}

	// set fp to next
	fp=next_fp;
}

void Call::after_run(bool isFunction) const{
/* restores stack state (restore fp,
   pop access link, arguments, locals and result) */
	// restore old fp (stack[next_fp-1])
	unsigned long next_fp=fp;
	fp=rt_stack[next_fp-1]->eval().uli;
	// pop implicit parameters without deleting
	for(uint i=0;i<outer_vars->size();i++){
		rt_stack.pop_back();
	}
	// pop locals and arguments from stack
	int size=body->get_size()-exprs->size()-1-outer_vars->size();
	if(isFunction)
		size-=1;
	for (int i = 0; i < size; ++i) {delete rt_stack.back(); rt_stack.pop_back();}
	for(int i=by_ref.size()-1; i>=0; i--){
		if(!by_ref[i]){
			delete rt_stack.back();
		}
		rt_stack.pop_back();
	}

	if(isFunction){
		// pop result
		delete rt_stack.back();
		rt_stack.pop_back();
	}
	// pop access link
	delete rt_stack[next_fp];
	rt_stack.pop_back();
	// pop old fp
	delete rt_stack[next_fp-1];
	rt_stack.pop_back();
}

void ProcCall::run() const{
	before_run();
	body->run();
	after_run();
}
// belongs to eval
value FunctionCall::eval(){
	before_run(true); // flag for function
	body->run();
	// take function result from stack[fp+1]
	value res = rt_stack[fp+1]->eval();
	after_run(true);
	return res;
}
