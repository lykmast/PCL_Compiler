#include "ast.hpp"

UnnamedLValue* UnnamedLValue::getBox(){
	return this;
}
value UnnamedLValue::eval(){
	return val;
}
void UnnamedLValue::let(value v){
	val=v;
}

value Rconst::eval(){
	value v; v.r=num;
	return v;
}
UnnamedLValue* REAL::create() const{
	return new UnnamedLValue(REAL::getInstance(),true);
}

value Iconst::eval() {
	value v; v.i=num;
	return v;
}
UnnamedLValue* INTEGER::create() const{
	return new UnnamedLValue(INTEGER::getInstance(),true);
}

value Cconst::eval() {
	value v; v.c=ch;
	return v;
}
UnnamedLValue* CHARACTER::create() const{
	return new UnnamedLValue(CHARACTER::getInstance(),true);
}

value Pconst::eval() {
	value v; v.lval=ptr;
	return v;
}
UnnamedLValue* PtrType::create() const{
	return new UnnamedLValue(new PtrType(type),true);
}

UnnamedLValue* ArrType::create() const{
	return new DynamicArray(size,type);
}

value Bconst::eval() {
	value v; v.b=boo;
	return v;
}
UnnamedLValue* BOOLEAN::create() const{
	return new UnnamedLValue(BOOLEAN::getInstance(),true);
}

value Id::eval(){
	int abs_ofs=find_absolute_offset();
	value v = rt_stack[abs_ofs]->eval();
	if(!v.lval and !type->get_name().compare("array")){
		ArrType* arrT=static_cast<ArrType*>(type);
		create_static_array(abs_ofs, arrT);
		return rt_stack[abs_ofs]->eval();
	}
	else return v;
}
void Id::create_static_array(int abs_ofs, ArrType* t){
	Type* inside_t=t->get_type();
	int s = t->get_size();
		rt_stack[abs_ofs] = new StaticArray(s,inside_t,abs_ofs+1);
}
void Id::let(value v){
	rt_stack[find_absolute_offset()]->let(v);
}
UnnamedLValue* Id::getBox(){
	int abs_ofs=find_absolute_offset();
	value v = rt_stack[abs_ofs]->eval();
	if(!v.lval and !type->get_name().compare("array")){
		ArrType* arrT=static_cast<ArrType*>(type);
		create_static_array(abs_ofs, arrT);
	}
	return rt_stack[abs_ofs]->getBox();
}

int Id::find_absolute_offset(){
	unsigned long prev_fp=fp;
	for(int diff=current_nesting-decl_nesting; diff>0; diff--){
		prev_fp=rt_stack[prev_fp]->eval().uli;
	}
	return prev_fp+offset;
}

value Op::eval() {
	// before the first eval of any op, there must have been one typecheck to
	// fill resType, leftType, rightType
	Type* realType=REAL::getInstance();
	Type* intType=INTEGER::getInstance();
	value leftValue=left->eval();
	value rightValue;
	value ret;
	if(right){
		rightValue=right->eval();
	}
	if(!(op.compare("+")) and right){
		//BinOp
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
		int li=leftValue.i;
		int ri=rightValue.i;
		ret.i = li/ri;
	}
	else if(!(op.compare("mod"))) {
		int li=leftValue.i;
		int ri=rightValue.i;
		ret.i = li%ri;
	}
	else if(!(op.compare("<>"))) {
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
		bool rb=rightValue.b;
		ret.b = lb and rb;
	}
	else if(!(op.compare("or"))) {
		bool lb=leftValue.b;
		bool rb=rightValue.b;
		ret.b = lb or rb;
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
void Dereference::let(value v){
	value vlval = expr->eval();
	(vlval.lval)->let(v);
}
UnnamedLValue* Dereference::getBox(){
	value v = expr->eval();
	return (v.lval)->getBox();
}

value Brackets::eval(){
	return element()->eval();
}
void Brackets::let(value v){
	element()->let(v);
}
UnnamedLValue* Brackets::getBox(){
	return element()->getBox();
}
LValue* Brackets::element(){
	value v = expr->eval();
	int i = v.i;
	v = lvalue->eval();
	Arrconst *arr = static_cast<Arrconst*>(v.lval);
	return arr->get_element(i);
}

LValue* StaticArray::get_element(int i){
	return rt_stack[i*child_size+offset];
}
LValue* DynamicArray::get_element(int i){
	return arr[i];
}


void Let::run() const{
	value v = expr->eval();
	if(different_types and is_right_int){
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
	if(expr){
		value v = expr->eval();
		int i = v.i;
		if(i<=0) {
			/*TODO ERROR wrong value*/
			std::cerr<<
				"ERROR: Wrong value for array size in New statement!"
				<<std::endl;
			exit(1);
		}
		Type* idType=lvalue->get_type();
		PtrType* p=static_cast<PtrType*>(idType);
		Type* t=p->get_type();
		ArrType* arrT = static_cast<ArrType*>(t);
		ArrType* arrT_size = new ArrType(i,arrT->get_type());
		value vlval; vlval.lval = arrT_size->create();
		lvalue->let(vlval);
	}
	else{
		Type* idType=lvalue->get_type();
		PtrType* p=static_cast<PtrType*>(idType);
		Type* t=p->get_type();
		value vlval; vlval.lval = t->create();
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


std::vector<value> ExprList::eval(std::vector<bool> by_ref){
	std::vector<value> ret(list.size());
	for(uint i=0; i<list.size(); i++){
		if(by_ref[i]){
			ret[i].lval=static_cast<LValue*>(list[i]);
		}
		else{
			ret[i]=list[i]->eval();
		}
	}
	return ret;
}


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

void Body::run() const{
	statements->run();
	print_stack();
}

void Program::run(){
	value v; v.uli=0;
	rt_stack.push_back(new UnnamedLValue(v,INTEGER::getInstance()));
	fp=0;
	for(int i=0; i<body->get_size()-1; i++){
		value v; v.lval=nullptr;
		rt_stack.push_back(new UnnamedLValue(v,ANY::getInstance()));
	}
	body->run();
	for(int i=0; i<body->get_size(); i++){
		delete rt_stack.back();
		rt_stack.pop_back();
	}
}

int Body::get_size(){
	return size;
}

void Call::before_run(bool isFunction) const{
	// first evaluate arguments
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
		if(by_ref[i]){
			rt_stack.push_back(args[i].lval->getBox());
		}
		else{
			rt_stack.push_back(new UnnamedLValue(args[i],ANY::getInstance()));
		}
	}
	// push empty spaces for callee locals (body size-arguments that are already pushed)
	int size=body->get_size()-exprs->size()-1;
	if(isFunction)
		size-=1;
	for (int i = 0; i < size; ++i){
		value v; v.lval=nullptr;
		rt_stack.push_back(new UnnamedLValue(v,ANY::getInstance()));
	}
	// set fp to next
	fp=next_fp;
}

void Call::after_run(bool isFunction) const{
	// restore old fp (stack[next_fp-1])
	unsigned long next_fp=fp;
	fp=rt_stack[next_fp-1]->eval().uli;
	// pop locals and arguments from stack
	int size=body->get_size()-exprs->size()-1;
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

value FunctionCall::eval(){
	before_run(true); // flag for function
	body->run();
	// take function result from stack[fp+1]
	value res = rt_stack[fp+1]->eval();
	after_run(true);
	return res;
}
