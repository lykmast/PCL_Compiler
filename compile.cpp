#include "ast.hpp"
#include "cgen_table.hpp"
#include "library.hpp"

const char *filename="llvm_output.out";

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
// TODO delete; probably not needed.
static std::map<std::string, llvm::Value *> NamedValues;


// useful llvm types
llvm::Type *i8=llvm::Type::getInt8Ty(TheContext);
llvm::Type *i32=llvm::Type::getInt32Ty(TheContext);
llvm::Type *i64=llvm::Type::getInt64Ty(TheContext);
llvm::Type *doubleTy=llvm::Type::getDoubleTy(TheContext);
llvm::Type *voidTy=llvm::Type::getVoidTy(TheContext);
// Useful LLVM helper functions.
static llvm::ConstantInt* c8_b(bool b) {
	return llvm::ConstantInt::get(TheContext, llvm::APInt(8, b, true));
}
static llvm::ConstantInt* c8(char c) {
	return llvm::ConstantInt::get(TheContext, llvm::APInt(8, c, true));
}
static llvm::ConstantInt* c32(int n) {
	return llvm::ConstantInt::get(TheContext, llvm::APInt(32, n, true));
}
static llvm::ConstantInt* c64(int n) {
	return llvm::ConstantInt::get(TheContext, llvm::APInt(64, n, true));
}
static llvm::ConstantFP* cf(double d){
	return llvm::ConstantFP::get(TheContext, llvm::APFloat(d));
}

// Code genaration for Type
//     return llvm::Type*
llvm::Type* INTEGER::cgen(){
	return i32;
}

llvm::Type* REAL::cgen(){
	return doubleTy;
}

llvm::Type* BOOLEAN::cgen(){
	return i8;
}

llvm::Type* CHARACTER::cgen(){
	return i8;
}

llvm::Type* ANY::cgen(){
	// placeholder; will probably be changed by cast
	return i32;
}

std::vector<llvm::Type*> CallableType::cgen_argTypes(){
	std::vector<llvm::Type*> argTypes(formal_types.size());
	for(uint i=0; i<formal_types.size();i++){
		if(by_ref[i]){
			argTypes[i] = llvm::PointerType::get(formal_types[i]->cgen(), 0);
		}
		else{
			argTypes[i] = formal_types[i]->cgen();
		}
	}
	for(auto p:outer_types){
		argTypes.push_back(llvm::PointerType::get(p->cgen(), 0));
	}
	return argTypes;
}


llvm::Type* FunctionType::cgen(){
	// last argument is false for fixed number of arguments
	return llvm::FunctionType::get(ret_type->cgen(), cgen_argTypes(), false);
}

llvm::Type* ProcedureType::cgen(){
	// last argument is false for fixed number of arguments
	return llvm::FunctionType::get(voidTy, cgen_argTypes(), false);
}

llvm::Type* PtrType::cgen(){
	return llvm::PointerType::get(type->cgen(), 0);
}

llvm::Type* ArrType::cgen(){
	if(size>0)
		return llvm::ArrayType::get(type->cgen(),size);
	else
		return llvm::ArrayType::get(type->cgen(),0);
}


// Code genaration for Const
//    return llvm::Value*
llvm::Value* Rconst::cgen(){
	return cf(num);
}

llvm::Value* Iconst::cgen(){
	return c32(num);
}

llvm::Value* Cconst::cgen(){
	return c8(ch);
}

llvm::Value* Bconst::cgen(){
	return c8_b(boo);
}

llvm::Value* Sconst::cgen(){
	return Builder.CreateGlobalStringPtr(str);
}
llvm::Value* NilConst::cgen(){
	// i32 by default; will probably be changed by cast
	return llvm::Constant::getNullValue(i32);
}

llvm::Value* Op::cgen(){
	Type* realType=REAL::getInstance();
	Type* intType=INTEGER::getInstance();
	llvm::Value* leftValue=left->cgen();
	llvm::Value* rightValue;
	if(!(op.compare("+")) and right){
		//BinOp
		rightValue=right->cgen();
		if(resType->doCompare(realType)){
			// fadd
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFAdd(leftValue,rightValue,"addtmp");
		}
		else{
			// add
			return Builder.CreateAdd(leftValue,rightValue,"addtmp");
		}
	}
	else if(!(op.compare("+"))){
		//UnOp
		return leftValue;
	}
	else if(!(op.compare("-")) and right) {
		//BinOp
		rightValue=right->cgen();
		if(resType->doCompare(realType)){
			// fsub
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFSub(leftValue,rightValue,"subtmp");
		}
		else{
			// sub
			return Builder.CreateSub(leftValue,rightValue,"subtmp");
		}
	}
	else if(!(op.compare("-"))){
		//UnOp
		if(leftType->doCompare(realType)){
			return Builder.CreateFNeg(leftValue);
		}
		else{
			return Builder.CreateSub(c32(0),leftValue);
		}
	}
	else if(!(op.compare("*"))) {
		//BinOp
		rightValue=right->cgen();
		if(resType->doCompare(realType)){
			// fmul
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFMul(leftValue,rightValue,"multmp");
		}
		else{
			// mul
			return Builder.CreateMul(leftValue,rightValue,"multmp");
		}
	}

	else if(!(op.compare("/"))){
		rightValue=right->cgen();
		if(leftType->doCompare(intType)){
			leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
		}
		if(rightType->doCompare(intType)){
			rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
		}
		return Builder.CreateFDiv(leftValue, rightValue,"fdivtmp");
	}

	else if( !(op.compare("div"))){
		rightValue=right->cgen();
		return Builder.CreateSDiv(leftValue, rightValue, "divtmp");
	}

	else if(!(op.compare("mod"))) {
		rightValue=right->cgen();
		return Builder.CreateSRem(leftValue, rightValue, "modtmp");
	}
	else if(!(op.compare("<>"))) {
		rightValue=right->cgen();

		if(leftType->doCompare(realType)
		or rightType->doCompare(realType)){
			// fcmp
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFCmpUNE(leftValue, rightValue, "fnetmp");
		}
		else{
			// icmp; works for bool, ptr, int, char
			return Builder.CreateICmpNE(leftValue, rightValue, "inetmp");
		}
	}
	else if(!(op.compare("="))) {
		rightValue=right->cgen();

		if(leftType->doCompare(realType)
		or rightType->doCompare(realType)){
			// fcmp
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFCmpOEQ(leftValue, rightValue, "feqtmp");
		}
		else{
			// icmp; works for bool, ptr, int, char
			return Builder.CreateICmpEQ(leftValue, rightValue, "ieqtmp");
		}
	}

	else if(!(op.compare("<="))) {
		rightValue=right->cgen();

		if(leftType->doCompare(realType)
		or rightType->doCompare(realType)){
			// fcmp
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFCmpOLE(leftValue, rightValue, "fletmp");
		}
		else{
			// icmp
			return Builder.CreateICmpSLE(leftValue, rightValue, "iletmp");
		}
	}
	else if(!(op.compare(">="))) {
		rightValue=right->cgen();

		if(leftType->doCompare(realType)
		or rightType->doCompare(realType)){
			// fcmp
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFCmpOGE(leftValue, rightValue, "fgetmp");
		}
		else{
			// icmp
			return Builder.CreateICmpSGE(leftValue, rightValue, "igetmp");
		}
	}
	else if(!(op.compare(">"))) {
		rightValue=right->cgen();

		if(leftType->doCompare(realType)
		or rightType->doCompare(realType)){
			// fcmp
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFCmpOGT(leftValue, rightValue, "fgttmp");
		}
		else{
			// icmp
			return Builder.CreateICmpSGT(leftValue, rightValue, "igttmp");
		}
	}
	else if(!(op.compare("<"))) {
		rightValue=right->cgen();

		if(leftType->doCompare(realType)
		or rightType->doCompare(realType)){
			// fcmp
			if(leftType->doCompare(intType)){
				leftValue=Builder.CreateSIToFP(leftValue,doubleTy,"loptmp");
			}
			if(rightType->doCompare(intType)){
				rightValue=Builder.CreateSIToFP(rightValue,doubleTy,"roptmp");
			}
			return Builder.CreateFCmpOLT(leftValue, rightValue, "flttmp");
		}
		else{
			// icmp
			return Builder.CreateICmpSLT(leftValue, rightValue, "ilttmp");
		}
	}
	else if(!(op.compare("and"))) {
		llvm::Value* ret;
		llvm::Function *TheFunction = ct.getFunction();

		// create blocks for short-circuiting;
		//   no short circuit block is inserted at the end of the function
		llvm::BasicBlock *ShortCircuitBB =
			llvm::BasicBlock::Create(TheContext, "andsc");
		llvm::BasicBlock *NoShortCircuitBB =
			llvm::BasicBlock::Create(TheContext, "andnsc", TheFunction);
		llvm::BasicBlock *MergeBB =
			llvm::BasicBlock::Create(TheContext, "andmerge");

		// if leftValue -> no short circuit; else short circuit
		Builder.CreateCondBr(leftValue, NoShortCircuitBB, ShortCircuitBB);

		/* no-short-circuit block */
		ct.setCurrentBB(NoShortCircuitBB);
		Builder.SetInsertPoint(NoShortCircuitBB);
		// TODO ct.setCurrentBlock(NoShortCircuit);
		rightValue=right->cgen();
		ret = Builder.CreateAnd(leftValue,rightValue,"andnsctmp");
		Builder.CreateBr(MergeBB);
		NoShortCircuitBB = Builder.GetInsertBlock();

		/* short-circuit block*/
		TheFunction->getBasicBlockList().push_back(ShortCircuitBB);
		ct.setCurrentBB(ShortCircuitBB);
		Builder.SetInsertPoint(ShortCircuitBB);
		Builder.CreateBr(MergeBB);
		ShortCircuitBB = Builder.GetInsertBlock();

		/* merge block */
		TheFunction->getBasicBlockList().push_back(MergeBB);
		ct.setCurrentBB(MergeBB);
		Builder.SetInsertPoint(MergeBB);
		llvm::PHINode *PN = Builder.CreatePHI(i8, 2, "andphitmp");

		PN->addIncoming(ret, NoShortCircuitBB);
		PN->addIncoming(c8_b(false), ShortCircuitBB);
		return PN;
	}
	else if(!(op.compare("or"))) {
		llvm::Value* ret;
		llvm::Function *TheFunction = ct.getFunction();

		// create blocks for short-circuiting;
		//   no short circuit block is inserted at the end of the function
		llvm::BasicBlock *ShortCircuitBB =
			llvm::BasicBlock::Create(TheContext, "orsc", TheFunction);
		llvm::BasicBlock *NoShortCircuitBB =
			llvm::BasicBlock::Create(TheContext, "ornsc");
		llvm::BasicBlock *MergeBB =
			llvm::BasicBlock::Create(TheContext, "ormerge");

		// if leftValue -> short circuit; else no short circuit
		Builder.CreateCondBr(leftValue, ShortCircuitBB, NoShortCircuitBB);

		/* short-circuit block*/
		ct.setCurrentBB(ShortCircuitBB);
		Builder.SetInsertPoint(ShortCircuitBB);
		Builder.CreateBr(MergeBB);
		ShortCircuitBB = Builder.GetInsertBlock();

		/* no-short-circuit block */
		TheFunction->getBasicBlockList().push_back(NoShortCircuitBB);
		ct.setCurrentBB(NoShortCircuitBB);
		Builder.SetInsertPoint(NoShortCircuitBB);
		// TODO ct.setCurrentBlock(NoShortCircuit);
		rightValue=right->cgen();
		ret = Builder.CreateOr(leftValue,rightValue,"ornsctmp");
		Builder.CreateBr(MergeBB);
		NoShortCircuitBB = Builder.GetInsertBlock();

		/* merge block */
		TheFunction->getBasicBlockList().push_back(MergeBB);
		ct.setCurrentBB(MergeBB);
		Builder.SetInsertPoint(MergeBB);
		llvm::PHINode *PN = Builder.CreatePHI(i8, 2, "orphitmp");

		PN->addIncoming(ret, NoShortCircuitBB);
		PN->addIncoming(c8_b(true), ShortCircuitBB);
		return PN;
	}
	else if(!(op.compare("not"))) {
		//UnOp
		return Builder.CreateNot(leftValue, "nottmp");
	}
	else{
		std::cerr<<"Cgen::Internal Error: Invalid BinOp."<<std::endl;
		exit(1);
	}
}

llvm::Value* Id::cgen(){
	bool ref;
	llvm::Value* var = ct.lookup(name,ref);
	if(ref){
		// load once more for reference.
		var = Builder.CreateLoad(var ,(name+"_ref").c_str());
	}
	return Builder.CreateLoad(var ,name.c_str());
}

llvm::Value* Reference::cgen(){
	llvm::Value* alloca = lvalue->getAddr();
	if(count){
		return alloca;
	}
	return Builder.CreateLoad(alloca, "reftmp");
}


llvm::Value* Dereference::cgen(){
	llvm::Value* alloca = expr->cgen();
	if(count){
		Builder.CreateLoad(alloca, "dereftmp");
		return Builder.CreateLoad(alloca, "deref2tmp");
	}
	return Builder.CreateLoad(alloca, "dereftmp");
}

llvm::Value* Brackets::cgen(){
	llvm::Value* arr;
	llvm::Value* index_v = expr->cgen();
	ArrType *arrTy = static_cast<ArrType*>(lvalue->get_type());
	arr = lvalue->getAddr();
	// GEP needs first a 0 index because arr is pointer (alloca) to array
	llvm::Value* ptr = Builder.CreateGEP(
		arrTy->cgen(), arr, std::vector<llvm::Value*> {c32(0),index_v});
	if(arrTy->is_1D()){
		return Builder.CreateLoad(ptr, "bracktmp");
	}
	return ptr;
	// load lvalue address -> create load at lvalue[index]
}

llvm::Value* Dereference::getAddr(){
	llvm::Value* alloca = expr->cgen();
	if(count){
		return Builder.CreateLoad(alloca, "dereftmp");
	}
	return alloca;
}

llvm::Value* Sconst::getAddr(){
	return Builder.CreateGlobalStringPtr(str);
}

llvm::Value* Id::getAddr(){
	bool ref;
	llvm::Value *var = ct.lookup(name, ref);
	if(ref){
		// load once more for reference.
		var = Builder.CreateLoad(var ,(name+"_ref").c_str());
	}
	return var;
}

llvm::Value* Brackets::getAddr(){
	llvm::Value* arr;
	llvm::Value* index_v = expr->cgen();
	ArrType *arrTy = static_cast<ArrType*>(lvalue->get_type());
	arr = lvalue->getAddr();
	return Builder.CreateGEP(
		arrTy->cgen(), arr, std::vector<llvm::Value*> {c32(0),index_v});
}

void LabelStmt::cgen(){
	llvm::Function* TheFunction = ct.getFunction();
	llvm::BasicBlock *LabelBB =
		llvm::BasicBlock::Create(TheContext, label_id, TheFunction);
	ct.insert_label(label_id, LabelBB);
	Builder.CreateBr(LabelBB);
	ct.setCurrentBB(LabelBB);
	Builder.SetInsertPoint(LabelBB);
	stmt->cgen();
}

void Goto::cgen(){
	Builder.CreateBr(ct.label_lookup(label_id));
}

void Let::cgen(){
	llvm::Value *e=expr->cgen();
	llvm::Value *addr=lvalue->getAddr();
	Builder.CreateStore(e, addr);
}

void If::cgen(){
	llvm::Function* TheFunction = ct.getFunction();

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.
	llvm::BasicBlock *ThenBB =
		llvm::BasicBlock::Create(TheContext, "then", TheFunction);
	llvm::BasicBlock *ElseBB =llvm::BasicBlock::Create(TheContext, "else");
	llvm::BasicBlock *MergeBB =llvm::BasicBlock::Create(TheContext, "ifcont");

	// condition branch
	llvm::Value* CondV = expr->cgen();
	Builder.CreateCondBr(CondV, ThenBB, ElseBB);

	/* then block */
	ct.setCurrentBB(ThenBB);
	Builder.SetInsertPoint(ThenBB);
	stmt1->cgen();
	Builder.CreateBr(MergeBB);

	/* else block */
	TheFunction->getBasicBlockList().push_back(ElseBB);
	ct.setCurrentBB(ElseBB);
	Builder.SetInsertPoint(ElseBB);
	if(stmt2) stmt2->cgen();
	Builder.CreateBr(MergeBB);

	/* merge block */
	TheFunction->getBasicBlockList().push_back(MergeBB);
	ct.setCurrentBB(MergeBB);
	Builder.SetInsertPoint(MergeBB);
	//TODO set current block at ct ?
}


void While::cgen(){
	llvm::Function* TheFunction = ct.getFunction();

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.
	llvm::BasicBlock *BeforeBB =
		llvm::BasicBlock::Create(TheContext, "before", TheFunction);
	llvm::BasicBlock *LoopBB =
		llvm::BasicBlock::Create(TheContext, "loop");
	llvm::BasicBlock *AfterBB =
		llvm::BasicBlock::Create(TheContext, "after");

	Builder.CreateBr(BeforeBB);

	/* before block */
	ct.setCurrentBB(BeforeBB);
	Builder.SetInsertPoint(BeforeBB);
	// condition branch
	llvm::Value* CondV = expr->cgen();
	Builder.CreateCondBr(CondV, LoopBB, AfterBB);

	/* loop block */
	TheFunction->getBasicBlockList().push_back(LoopBB);
	ct.setCurrentBB(LoopBB);
	Builder.SetInsertPoint(LoopBB);
	stmt->cgen();
	Builder.CreateBr(BeforeBB);

	/* after block */
	TheFunction->getBasicBlockList().push_back(AfterBB);
	ct.setCurrentBB(AfterBB);
	Builder.SetInsertPoint(AfterBB);
	//TODO set current block at ct ?
}

void New::cgen(){
	llvm::DataLayout* DL = new llvm::DataLayout(&(*TheModule));
	// get size of type to malloc
	llvm::Type* ty = lvalue->get_type()->cgen();
	ty = ty->getPointerElementType();
	llvm::Value *AllocSize = c64(DL->getTypeAllocSize(ty));
	if(expr){
		// array
		AllocSize=Builder.CreateMul(expr->cgen(),AllocSize);
	}

	llvm::Value *ptr = Builder.CreateCall(
		TheModule->getFunction("malloc"), AllocSize);
	Builder.CreateStore(ptr,lvalue->getAddr());
}

void Dispose::cgen(){
	llvm::Value *ptr = Builder.CreateLoad(lvalue->getAddr(),"disptmp");
	Builder.CreateCall(TheModule->getFunction("free"), ptr );
}

void DisposeArr::cgen(){
	llvm::Value *ptr = Builder.CreateLoad(lvalue->getAddr(),"disptmp");
	Builder.CreateCall(TheModule->getFunction("free"), ptr );
}

void StmtList::cgen(){
	for(auto const &p: list){
		p->cgen();
	}
}

void DeclList::cgen(){
	for(auto const &p: list){
		p->cgen();
	}
}

void VarDecl::cgen(){
	llvm::AllocaInst* alloca = Builder.CreateAlloca(type->cgen(), nullptr, id);
	ct.insert(id, alloca);
}


void Body::cgen(){
	if(defined){
		declarations->cgen();
		statements->cgen();
	}
	else if(!library){
		std::cerr<<"Cgen:: Incomplete declaration."<<std::endl;
	}
}

void Procedure::cgen(){

	llvm::FunctionType* FT = static_cast<llvm::FunctionType*>(type->cgen());
	llvm::Function* F = llvm::Function::Create(
		FT, llvm::Function::ExternalLinkage, id, TheModule.get()
	);
	ct.insert_function(id, F);
	if(body->isLibrary()) return;

	ct.openScope(F);
	std::vector<std::string> formal_vars = type->get_formal_vars();
	std::vector<std::string> outer_vars = type->get_outer_vars();
	std::vector<bool> by_ref = type->get_by_ref();

	llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", F);
	ct.setCurrentBB(BB);
	Builder.SetInsertPoint(BB);
	llvm::Type* ret_type = F->getReturnType();
	bool isFunction=false;
	llvm::AllocaInst* result_alloca;
	if(!ret_type->isVoidTy()){
		isFunction=true;
		// if subprogram is function, create alloca for result.
		result_alloca = Builder.CreateAlloca(ret_type, nullptr, "result");
		ct.insert("result", result_alloca);
	}

	unsigned Idx_formal=0;
	unsigned Idx_outer=0;
	unsigned fs=formal_vars.size();
	unsigned os=outer_vars.size();
	for(auto &Arg : F->args()){
		llvm::AllocaInst* alloca;
		if(Idx_formal<fs){
			Arg.setName(formal_vars[Idx_formal]);
			alloca = Builder.CreateAlloca(Arg.getType(), nullptr, Arg.getName());
			ct.insert(Arg.getName(), alloca, by_ref[Idx_formal]);
			Idx_formal++;
		}
		else if(Idx_outer<os){
			Arg.setName(formal_vars[Idx_outer++]);
			alloca = Builder.CreateAlloca(Arg.getType(), nullptr, Arg.getName());
			// all outer arguments are passed by reference.
			ct.insert(Arg.getName(), alloca, true);
		}
		else{
			std::cerr<<"Code generation error:"
			" number of arguments in function"<<std::endl;
		}
		// Store the initial value into the alloca.
		Builder.CreateStore(&Arg, alloca);
	}

	body->cgen();
	// TODO if there is nesting we must return to initial building block.

	// return
	if(isFunction){
		llvm::Value* res = Builder.CreateLoad(result_alloca, "result");
		Builder.CreateRet(res);
	}
	else{
		Builder.CreateRetVoid();
	}
	ct.closeScope();
	// return to parent building block.
	Builder.SetInsertPoint(ct.getCurrentBB());
}

void Return::cgen(){
	llvm::Type* ret_type = ct.getFunction()->getReturnType();
	if(ret_type->isVoidTy()){
		Builder.CreateRetVoid();
	}
	else{
		bool ref;
		llvm::AllocaInst* result_alloca = ct.lookup("result", ref);
		llvm::Value *res = Builder.CreateLoad(result_alloca, "result");
		Builder.CreateRet(res);
	}
}

void Program::cgen(){
	TheModule = llvm::make_unique<llvm::Module>(filename, TheContext);
	llvm::FunctionType* main_t = llvm::FunctionType::get(
		i32, std::vector<llvm::Type *>{}, false
	);
	llvm::Function* main_f = llvm::Function::Create(
		main_t, llvm::Function::ExternalLinkage, name, TheModule.get()
	);
	ct.openScope(main_f);
	// TODO libraries codegen
	// load library subprograms
	for(auto p:library_subprograms){
		p->cgen();
	}
	llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", main_f);
	ct.setCurrentBB(BB);
	Builder.SetInsertPoint(BB);
	body->cgen();
	Builder.CreateRet(c32(0));
	ct.closeScope();
	// TODO how does this vvvv work?
	TheModule->print(llvm::outs(), nullptr);
}

std::vector<llvm::Value*> ExprList::cgen(std::vector<bool> by_ref){
	// eval every expression
	// considering passing mode (by-reference / by-value)
	std::vector<llvm::Value*> ret(list.size());
	for(uint i=0; i<list.size(); i++){
		if(by_ref[i]){ // passing mode is by-reference
			// return address
			ret[i]=static_cast<LValue*>(list[i])->getAddr();
		}
		else{
			ret[i]=list[i]->cgen();
		}
	}
	return ret;
}


llvm::Value* Call::cgen_common(){
	llvm::Function* callee = ct.function_lookup(name);
	std::vector<llvm::Value*> args = exprs->cgen(by_ref);
	// all outer vars are passed by reference
	std::vector<llvm::Value*> outer =
		outer_vars->cgen(std::vector<bool>(outer_vars->size(),true));
	// merge all arguments
	args.insert(args.end(), outer.begin(), outer.end());
	return Builder.CreateCall(callee, args);
}

void ProcCall::cgen(){
	Call::cgen_common();
}

llvm::Value* FunctionCall::cgen(){
	return Call::cgen_common();
}
