#include "ast.hpp"
#include "library.hpp"
#include "cstdio"
#include "cstring"
#include "math.h"
FILE* inp;
//------write procedures-----------
_writeInteger::_writeInteger():i(new Id("n")){}
void _writeInteger::run() const{
	printf("%d",i->eval().i);
}
void _writeInteger::sem(){
	i->sem();
}

writeInteger::writeInteger():Procedure (
	"writeInteger",
	new DeclList(
			new Decl("n")
	),
	new Body(new DeclList(), new StmtList(_writeInteger::getInstance()) )
){
		formals->toFormal(INTEGER::getInstance(),false);
}

_writeBoolean::_writeBoolean():i(new Id("b")){}
void _writeBoolean::run() const{
	printf("%s", i->eval().b ? "true" : "false");
}
void _writeBoolean::sem(){
	i->sem();
}

writeBoolean::writeBoolean():Procedure (
	"writeBoolean",
	new DeclList(
			new Decl("b")
	),
	new Body(new DeclList(), new StmtList(_writeBoolean::getInstance()) )
){
		formals->toFormal(BOOLEAN::getInstance(),false);
}

_writeChar::_writeChar():i(new Id("c")){}
void _writeChar::run() const{
	printf("%c",i->eval().c);
}
void _writeChar::sem(){
	i->sem();
}

writeChar::writeChar():Procedure (
	"writeChar",
	new DeclList(
			new Decl("c")
	),
	new Body(new DeclList(), new StmtList(_writeChar::getInstance()) )
){
		formals->toFormal(CHARACTER::getInstance(),false);
}

_writeReal::_writeReal():i(new Id("r")){}
void _writeReal::run() const{
	printf("%lf",i->eval().r);
}
void _writeReal::sem(){
	i->sem();
}

writeReal::writeReal():Procedure (
	"writeReal",
	new DeclList(
			new Decl("r")
	),
	new Body(new DeclList(), new StmtList(_writeReal::getInstance()) )
){
		formals->toFormal(REAL::getInstance(),false);
}

_writeString::_writeString():i(new Id("s")){}
void _writeString::run() const{
	Arrconst* arr=static_cast<Arrconst*>(i->eval().lval);
	int i=0;
	char c=arr->get_element(i)->eval().c;
	while(c!='\0'){
		printf("%c", c);
		c=arr->get_element(++i)->eval().c;
	}
}
void _writeString::sem(){
	i->sem();
}

writeString::writeString():Procedure (
	"writeString",
	new DeclList(
			new Decl("s")
	),
	new Body(new DeclList(), new StmtList(_writeString::getInstance()) )
){
		formals->toFormal(new ArrType(CHARACTER::getInstance()),true);
}


//----read subprograms------------

value _readInteger::eval(){
	value v;
	char buf[256];
	fgets(buf,sizeof(buf),inp);
	v.i=atoi(buf);
	return v;
}
Type* _readInteger::get_type(){
	return INTEGER::getInstance();
}

readInteger::readInteger():Function (
	"readInteger",
	new DeclList(),
	INTEGER::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_readInteger::getInstance())
	))
){}

value _readBoolean::eval(){
	char word[7];
	value v;
	fgets(word, 7, inp);
	word[strlen(word)-1]='\0';
	if(!strcmp(word,"true")){
		v.b=true;
	}
	else if(!strcmp(word,"false")){
		v.b=false;
	}
	else{
		std::cerr<<"Invalid input; expected \"true\" or \"false\" but received unknown \""<<word<<"\""<<std::endl;
		exit(1);
	}
	return v;
}
Type* _readBoolean::get_type(){
	return BOOLEAN::getInstance();
}


readBoolean::readBoolean():Function (
	"readBoolean",
	new DeclList(),
	BOOLEAN::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_readBoolean::getInstance()) )
	)
){}

value _readChar::eval(){
	value v;
	v.c=getc(inp);
	return v;
}
Type* _readChar::get_type(){
	return CHARACTER::getInstance();
}


readChar::readChar():Function (
	"readChar",
	new DeclList(),
	CHARACTER::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_readChar::getInstance()) )
	)
){}

value _readReal::eval(){
	value v;
	char buf[256];
	fgets(buf,sizeof(buf),inp);
	v.r=atof(buf);
	return v;
}
Type* _readReal::get_type(){
	return REAL::getInstance();
}


readReal::readReal():Function (
	"readReal",
	new DeclList(),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_readReal::getInstance()) )
	)
){}

_readString::_readString():sz(new Id("size")),str(new Id("s")){}
void _readString::run() const{
	int size=sz->eval().i;
	Arrconst* arr=static_cast<Arrconst*>(str->eval().lval);
	int i=0;
	char c=getc(inp);
	while(c!='\n' and i<size-1){
		value v; v.c=c;
		arr->get_element(i++)->let(v);
		c=getc(inp);
	}
	value v; v.c='\0';
	arr->get_element(i)->let(v);
}
void _readString::sem(){
	sz->sem();
	str->sem();
}

readString::readString():Procedure (
	"readString",
	new DeclList(
			new Decl("size")
	),
	new Body(new DeclList(), new StmtList(_readString::getInstance()) )
){
		formals->toFormal(INTEGER::getInstance(),false);
		DeclList* d=new DeclList(new Decl("s"));
		d->toFormal(new ArrType(CHARACTER::getInstance()),true);
		formals->merge(d);
}

//-------math functions--------

_abs_pcl::_abs_pcl():i(new Id("n")){}
value _abs_pcl::eval(){
	value v;
	v.i=abs(i->eval().i);
	return v;
}
void _abs_pcl::sem(){
	i->sem();
}
Type* _abs_pcl::get_type(){
	return INTEGER::getInstance();
}

abs_pcl::abs_pcl():Function (
	"abs",
	new DeclList(new Decl("n")),
	INTEGER::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_abs_pcl::getInstance()) )
	)
){
	formals->toFormal(INTEGER::getInstance(),false);
}


_fabs_pcl::_fabs_pcl():i(new Id("r")){}
value _fabs_pcl::eval(){
	value v;
	v.r=fabs(i->eval().r);
	return v;
}
void _fabs_pcl::sem(){
	i->sem();
}
Type* _fabs_pcl::get_type(){
	return REAL::getInstance();
}


fabs_pcl::fabs_pcl():Function (
	"fabs",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_fabs_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}

_sqrt_pcl::_sqrt_pcl():i(new Id("r")){}
value _sqrt_pcl::eval(){
	value v;
	v.r=sqrt(i->eval().r);
	return v;
}
void _sqrt_pcl::sem(){
	i->sem();
}
Type* _sqrt_pcl::get_type(){
	return REAL::getInstance();
}


sqrt_pcl::sqrt_pcl():Function (
	"sqrt",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_sqrt_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}


_sin_pcl::_sin_pcl():i(new Id("r")){}
value _sin_pcl::eval(){
	value v;
	v.r=sin(i->eval().r);
	return v;
}
void _sin_pcl::sem(){
	i->sem();
}
Type* _sin_pcl::get_type(){
	return REAL::getInstance();
}


sin_pcl::sin_pcl():Function (
	"sin",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_sin_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}


_cos_pcl::_cos_pcl():i(new Id("r")){}
value _cos_pcl::eval(){
	value v;
	v.r=cos(i->eval().r);
	return v;
}
void _cos_pcl::sem(){
	i->sem();
}
Type* _cos_pcl::get_type(){
	return REAL::getInstance();
}


cos_pcl::cos_pcl():Function (
	"cos",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_cos_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}


_tan_pcl::_tan_pcl():i(new Id("r")){}
value _tan_pcl::eval(){
	value v;
	v.r=tan(i->eval().r);
	return v;
}
void _tan_pcl::sem(){
	i->sem();
}
Type* _tan_pcl::get_type(){
	return REAL::getInstance();
}


tan_pcl::tan_pcl():Function (
	"tan",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_tan_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}


_arctan_pcl::_arctan_pcl():i(new Id("r")){}
value _arctan_pcl::eval(){
	value v;
	v.r=atan(i->eval().r);
	return v;
}
void _arctan_pcl::sem(){
	i->sem();
}
Type* _arctan_pcl::get_type(){
	return REAL::getInstance();
}


arctan_pcl::arctan_pcl():Function (
	"arctan",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_arctan_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}


_exp_pcl::_exp_pcl():i(new Id("r")){}
value _exp_pcl::eval(){
	value v;
	v.r=exp(i->eval().r);
	return v;
}
void _exp_pcl::sem(){
	i->sem();
}
Type* _exp_pcl::get_type(){
	return REAL::getInstance();
}


exp_pcl::exp_pcl():Function (
	"exp",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_exp_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}


_ln_pcl::_ln_pcl():i(new Id("r")){}
value _ln_pcl::eval(){
	value v;
	v.r=log(i->eval().r);
	return v;
}
void _ln_pcl::sem(){
	i->sem();
}
Type* _ln_pcl::get_type(){
	return REAL::getInstance();
}


ln_pcl::ln_pcl():Function (
	"ln",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_ln_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}


value _pi_pcl::eval(){
	value v;
	v.r=M_PI;
	return v;
}
Type* _pi_pcl::get_type(){
	return REAL::getInstance();
}


pi_pcl::pi_pcl():Function (
	"pi",
	new DeclList(),
	REAL::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_pi_pcl::getInstance()) )
	)
){}





_trunc_pcl::_trunc_pcl():i(new Id("r")){}
value _trunc_pcl::eval(){
	value v;
	v.i=trunc(i->eval().r);
	return v;
}
void _trunc_pcl::sem(){
	i->sem();
}
Type* _trunc_pcl::get_type(){
	return INTEGER::getInstance();
}


trunc_pcl::trunc_pcl():Function (
	"trunc",
	new DeclList(new Decl("r")),
	INTEGER::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_trunc_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}




_round_pcl::_round_pcl():i(new Id("r")){}
value _round_pcl::eval(){
	value v;
	v.i=round(i->eval().r);
	return v;
}
void _round_pcl::sem(){
	i->sem();
}
Type* _round_pcl::get_type(){
	return INTEGER::getInstance();
}


round_pcl::round_pcl():Function (
	"round",
	new DeclList(new Decl("r")),
	INTEGER::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_round_pcl::getInstance()) )
	)
){
	formals->toFormal(REAL::getInstance(),false);
}





_ord_pcl::_ord_pcl():i(new Id("c")){}
value _ord_pcl::eval(){
	value v;
	v.i=i->eval().c;
	return v;
}
void _ord_pcl::sem(){
	i->sem();
}
Type* _ord_pcl::get_type(){
	return INTEGER::getInstance();
}


ord_pcl::ord_pcl():Function (
	"ord",
	new DeclList(new Decl("c")),
	INTEGER::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_ord_pcl::getInstance()) )
	)
){
	formals->toFormal(CHARACTER::getInstance(),false);
}


_chr_pcl::_chr_pcl():i(new Id("n")){}
value _chr_pcl::eval(){
	value v;
	v.c=i->eval().i;
	return v;
}
void _chr_pcl::sem(){
	i->sem();
}
Type* _chr_pcl::get_type(){
	return CHARACTER::getInstance();
}


chr_pcl::chr_pcl():Function (
	"chr",
	new DeclList(new Decl("n")),
	CHARACTER::getInstance(),
	new Body(new DeclList(), new StmtList(
		new Let(new Id("result"),_chr_pcl::getInstance()) )
	)
){
	formals->toFormal(INTEGER::getInstance(),false);
}

std::vector<Procedure*> library_subprograms{
	writeInteger::getInstance(),
	writeBoolean::getInstance(),
	writeChar::getInstance(),
	writeReal::getInstance(),
	writeString::getInstance(),
	readInteger::getInstance(),
	readBoolean::getInstance(),
	readChar::getInstance(),
	readReal::getInstance(),
	readString::getInstance(),
	abs_pcl::getInstance(),
	fabs_pcl::getInstance(),
	sqrt_pcl::getInstance(),
	sin_pcl::getInstance(),
	cos_pcl::getInstance(),
	tan_pcl::getInstance(),
	arctan_pcl::getInstance(),
	exp_pcl::getInstance(),
	ln_pcl::getInstance(),
	pi_pcl::getInstance(),
	trunc_pcl::getInstance(),
	round_pcl::getInstance(),
	ord_pcl::getInstance(),
	chr_pcl::getInstance()
};
