#include "ast.hpp"
Const* Const::eval(){return this->clone();}
value Const::get_value() const{value v; v.lval=nullptr; return v;}

UnnamedLValue* UnnamedLValue::getBox(){
	return this;
}
Const* UnnamedLValue::eval(){
	if(value)
		return value->eval();
	else
		return nullptr;
}
void UnnamedLValue::let(Const* c){
	if(value)delete value;
	value=c;
	if(type)
		if(type->should_delete())
			delete type;
	type=c->get_type();
}

Const* Rconst::clone() {return new Rconst(*this);}
value Rconst::get_value() const{
	value v; v.r=num;
	return v;
}
UnnamedLValue* REAL::create() const{
	return new UnnamedLValue(REAL::getInstance(),true);
}

Const* Iconst::clone(){return new Iconst(*this);}
value Iconst::get_value() const {
	value v; v.i=num;
	return v;
}
UnnamedLValue* INTEGER::create() const{
	return new UnnamedLValue(INTEGER::getInstance(),true);
}

Const* Cconst::clone() {return new Cconst(*this);}
value Cconst::get_value() const {
	value v; v.c=ch;
	return v;
}
UnnamedLValue* CHARACTER::create() const{
	return new UnnamedLValue(CHARACTER::getInstance(),true);
}

Const* Pconst::clone() {return new Pconst(*this);}
value Pconst::get_value() const {
	value v; v.lval=ptr;
	return v;
}
UnnamedLValue* PtrType::create() const{
	return new UnnamedLValue(new PtrType(type),true);
}

UnnamedLValue* ArrType::create() const{
	return new UnnamedLValue(new DynamicArray(size,type), new ArrType(size,type),true);
}

Const* Bconst::clone(){return new Bconst(*this);}
value Bconst::get_value() const {
	value v; v.b=boo;
	return v;
}
UnnamedLValue* BOOLEAN::create() const{
	return new UnnamedLValue(INTEGER::getInstance(),true);
}

Const* Id::eval(){
	int abs_ofs=find_absolute_offset();
	Const *c = rt_stack[abs_ofs]->eval();
	if(!c and !type->get_name().compare("array")){
		ArrType* arrT=static_cast<ArrType*>(type);
		create_static_array(abs_ofs, arrT);
		return rt_stack[abs_ofs]->eval();
	}
	else return c;
}
void Id::create_static_array(int abs_ofs, ArrType* t){
	Type* inside_t=t->get_type();
	int s = t->get_size();
	Const *c = new StaticArray(s,inside_t,abs_ofs+1);
	rt_stack[abs_ofs]->let(c);
}
void Id::let(Const* c){
	rt_stack[find_absolute_offset()]->let(c);
}
UnnamedLValue* Id::getBox(){
	return rt_stack[find_absolute_offset()]->getBox();
}

int Id::find_absolute_offset(){
	int prev_fp=fp;
	for(int diff=current_nesting-decl_nesting; diff>0; diff--){
		prev_fp=rt_stack[prev_fp]->eval()->get_value().i;
	}
	return prev_fp+offset;
}

Const* Op::eval() {
	// before the first eval of any op, there must have been one typecheck to
	// fill resType, leftType, rightType
	Type* realType=REAL::getInstance();
	Type* intType=INTEGER::getInstance();
	Const *leftConst=left->eval();
	Const *rightConst=nullptr;
	Const* ret=nullptr;
	if(right){
		rightConst=right->eval();
	}
	if(!(op.compare("+")) and right){
		//BinOp
		int li=0,ri=0;
		double lr=0,rr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		if(resType->doCompare(realType)){
			ret = new Rconst(li+ri+lr+rr);
		}
		else{
			ret = new Iconst(li+ri);
		}
	}
	else if(!(op.compare("+"))){
		//UnOp
		int li=0;
		double lr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(resType->doCompare(realType)){
			ret = new Rconst(li+lr);
		}
		else{
			ret = new Iconst(li);
		}
	}
	else if(!(op.compare("-")) and right) {
	//BinOp
		int li=0,ri=0;
		double lr=0,rr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		if(resType->doCompare(realType)){
			ret = new Rconst(li-ri+lr-rr);
		}
		else{
			ret = new Iconst(li-ri);
		}
	}
	else if(!(op.compare("-"))){
		//UnOp
		int li=0;
		double lr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(resType->doCompare(realType)){
			ret = new Rconst(-li-lr);
		}
		else{
			ret = new Iconst(-li);
		}
	}
	else if(!(op.compare("*"))) {
	//BinOp
		int li=1,ri=1;
		double lr=1,rr=1;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		if(resType->doCompare(realType)){
			ret = new Rconst(li*ri*lr*rr);
		}
		else{
			ret = new Iconst(li*ri);
		}
	}
	else if(!(op.compare("/"))){
		int li=1,ri=1;
		double lr=1,rr=1;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		if(resType->doCompare(realType)){
			ret = new Rconst((li/rr)*(lr/ri));
		}
	}

	if( !(op.compare("div"))){
		value v=leftConst->get_value();
		int li=v.i;
		v=rightConst->get_value();
		int ri=v.i;
		ret = new Iconst(li/ri);
	}
	else if(!(op.compare("mod"))) {
		value v=leftConst->get_value();
		int li=v.i;
		v=rightConst->get_value();
		int ri=v.i;
		ret = new Iconst(li%ri);
	}
	else if(!(op.compare("<>"))) {
		int li=0,ri=0;
		double lr=0,rr=0;
		LValue* lptr, *rptr;
		bool isNumber=true;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else if(leftType->doCompare(intType)){
			v=leftConst->get_value();
			li=v.i;
		}
		else{
			v=leftConst->get_value();
			lptr=v.lval;
			isNumber=false;
		}

		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else if(rightType->doCompare(intType)) {
			v=rightConst->get_value();
			ri=v.i;
		}
		else{
			v=rightConst->get_value();
			rptr=v.lval;
		}
		if(isNumber)
			ret = new Bconst(li+lr!=ri+rr);
		else{
			ret = new Bconst(rptr!=lptr);
		}

	}
	else if(!(op.compare("="))) {
		int li=0,ri=0;
		double lr=0,rr=0;
		LValue* lptr, *rptr;
		bool isNumber=true;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else if(leftType->doCompare(intType)){
			v=leftConst->get_value();
			li=v.i;
		}
		else{
			v=leftConst->get_value();
			lptr=v.lval;
			isNumber=false;
		}

		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else if(rightType->doCompare(intType)) {
			v=rightConst->get_value();
			ri=v.i;
		}
		else{
			v=rightConst->get_value();
			rptr=v.lval;
		}
		if(isNumber)
			ret = new Bconst(li+lr==ri+rr);
		else{
			ret = new Bconst(rptr==lptr);
		}
	}

	else if(!(op.compare("<="))) {
		int li=0,ri=0;
		double lr=0,rr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		ret = new Bconst(li+lr<=ri+rr);
	}
	else if(!(op.compare(">="))) {
		int li=0,ri=0;
		double lr=0,rr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		ret = new Bconst(li+lr>=ri+rr);
	}
	else if(!(op.compare(">"))) {
		int li=0,ri=0;
		double lr=0,rr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		ret = new Bconst(li+lr > ri+rr);
	}
	else if(!(op.compare("<"))) {
		int li=0,ri=0;
		double lr=0,rr=0;
		value v;
		if(leftType->doCompare(realType)){
			v=leftConst->get_value();
			lr=v.r;
		}
		else{
			v=leftConst->get_value();
			li=v.i;
		}
		if(rightType->doCompare(realType)){
			v=rightConst->get_value();
			rr=v.r;
		}
		else{
			v=rightConst->get_value();
			ri=v.i;
		}
		ret = new Bconst(li+lr < ri+rr);
	}
	else if(!(op.compare("and"))) {
		value v=leftConst->get_value();
		bool lb=v.b;
		v=rightConst->get_value();
		bool rb=v.b;
		ret = new Bconst(lb and rb);
	}
	else if(!(op.compare("or"))) {
		value v=leftConst->get_value();
		bool lb=v.b;
		v=rightConst->get_value();
		bool rb=v.b;
		ret = new Bconst(lb or rb);
	}
	else if(!(op.compare("not"))) {
		//UnOp

		value v;
		v=leftConst->get_value();
		bool lb=v.b;
		ret = new Bconst(not lb);
	}
	delete leftConst;
	if(right) delete rightConst;
	return ret;
}

Const* Reference::eval(){
	return new Pconst(lvalue,lvalue->get_type());
}

Const* Dereference::eval(){
	Const* c = expr->eval();
	value v = c->get_value();
	return (v.lval)->eval();
}
void Dereference::let(Const* c){
	Const* con = expr->eval();
	value v = con->get_value();
	(v.lval)->let(c);
}
UnnamedLValue* Dereference::getBox(){
	Const* con = expr->eval();
	value v = con->get_value();
	return (v.lval)->getBox();
}

Const* Brackets::eval(){
	return element()->eval();
}
void Brackets::let(Const* c){
	element()->let(c);
}
UnnamedLValue* Brackets::getBox(){
	return element()->getBox();
}
LValue* Brackets::element(){
	value v = expr->eval()->get_value();
	int i = v.i;
	Const* c = lvalue->eval();
	Arrconst *arr = static_cast<Arrconst*>(c);
	return arr->get_element(i);
}

LValue* StaticArray::get_element(int i){
	return rt_stack[i*child_size+offset];
}
LValue* DynamicArray::get_element(int i){
	return arr[i];
}


void Let::run() const{
	Const* c = expr->eval();
	if(!different_types){
		lvalue->let(c);
	}
	else{
		Const* n=c->copyToType();
		lvalue->let(n);
		delete c;
	}
}

void If::run() const{
	Const * c= expr->eval();
	value v=c->get_value();
	bool e = v.b;
	if(e) stmt1->run();
	else if (stmt2) stmt2->run();
}

void While::run() const{
	Const * c= expr->eval();
	value v=c->get_value();
	bool e = v.b;
	while(e) {
		stmt->run();
		c= expr->eval();
		value v=c->get_value();
		e = v.b;
	}
}

void New::run() const{
	if(expr){
		Const* c= expr->eval();
		value v = c->get_value();
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
		Pconst *ret = new Pconst(arrT_size->create(), arrT_size );
		lvalue->let(ret);
	}
	else{
		Type* idType=lvalue->get_type();
		PtrType* p=static_cast<PtrType*>(idType);
		Type* t=p->get_type();
		Pconst *ret = new Pconst(t->create(), t);
		lvalue->let(ret);
	}
}

void Dispose::run() const{
	Const *c = lvalue->eval();
	value v = c->get_value();
	LValue* ptr = v.lval;
	if(!(ptr->isDynamic())){
		/*TODO ERROR non dynamic pointer (RUNTIME)*/
		std::cerr<<
			"ERROR: non dynamic pointer in Dispose (RUNTIME)"
			<<std::endl;
		exit(1);
	}
	delete ptr;
	lvalue->let(new Pconst());
}

void DisposeArr::run() const{
	Const *c = lvalue->eval();
	value v = c->get_value();
	LValue* ptr = v.lval;
	if(!(ptr->isDynamic())){
		/*TODO ERROR non dynamic pointer (RUNTIME)*/
		std::cerr<<
			"ERROR: non dynamic pointer in DisposeArr (RUNTIME)"
			<<std::endl;
		exit(1);
	}
	delete ptr;
	lvalue->let(new Pconst());
}

void StmtList::run() const{
	for(auto p:list){
		if(p->isReturn())
			return;
		p->run();
	}
}


std::vector<Expr*> ExprList::eval(std::vector<bool> by_ref){
	std::vector<Expr*> ret(list.size());
	for(uint i=0; i<list.size(); i++){
		if(by_ref[i]){
			ret[i]=list[i];
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
	rt_stack.push_back(new UnnamedLValue(new Iconst(0),INTEGER::getInstance()));
	fp=0;
	for(int i=0; i<body->get_size()-1; i++){
		rt_stack.push_back(new UnnamedLValue(nullptr));
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
	int next_fp=fp+next_fp_offset;
	// push current fp at next_fp-1 offset in stack
	rt_stack.push_back(new UnnamedLValue(new Iconst(fp),INTEGER::getInstance())); //next_fp-1
	// push access link at next_fp offset in stack
	if(nesting_diff<0){
		rt_stack.push_back(new UnnamedLValue(new Iconst(fp),INTEGER::getInstance())); //next_fp
	}
	else{
		int prev_fp=fp;
		for(int diff=nesting_diff; diff>0; diff--){
			prev_fp=rt_stack[prev_fp]->eval()->get_value().i;
		}
		Const* c=rt_stack[prev_fp]->eval();
		rt_stack.push_back(new UnnamedLValue(c,INTEGER::getInstance())); //next_fp

	}
	if(isFunction){
		// push one more value for function result at next_fp+1 offset
		rt_stack.push_back(new UnnamedLValue(nullptr));
	}
	// push arguments in stack (lvalues for references)
	std::vector<Expr*> args(exprs->eval(by_ref));
	for(uint i=0; i<args.size(); i++){
		if(by_ref[i]){
			rt_stack.push_back(static_cast<LValue*>(args[i])->getBox());
		}
		else{
			rt_stack.push_back(new UnnamedLValue(static_cast<Const*>(args[i]),args[i]->get_type()));
		}
	}
	// push empty spaces for callee locals (body size-arguments that are already pushed)
	int size=body->get_size()-exprs->size()-1;
	if(isFunction)
		size-=1;
	for (int i = 0; i < size; ++i) rt_stack.push_back(new UnnamedLValue(nullptr));
	// set fp to next
	fp=next_fp;
}

void Call::after_run(bool isFunction) const{
	// restore old fp (stack[next_fp-1])
	int next_fp=fp;
	fp=rt_stack[next_fp-1]->eval()->get_value().i;
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

Const* FunctionCall::eval(){
	before_run(true); // flag for function
	body->run();
	// take function result from stack[fp+1]
	Const* res = rt_stack[fp+1]->eval();
	after_run(true);
	return res;
}
