#include "ast.hpp"
#include "library.hpp"
FILE* inp;
//------write procedures-----------


writeInteger::writeInteger():Procedure (
	"writeInteger",
	new DeclList(
			new Decl("n")
	),
	new Body(true)
){
		formals->toFormal(INTEGER::getInstance(),false);
}


writeBoolean::writeBoolean():Procedure (
	"writeBoolean",
	new DeclList(
			new Decl("b")
	),
	new Body(true )
){
		formals->toFormal(BOOLEAN::getInstance(),false);
}

writeChar::writeChar():Procedure (
	"writeChar",
	new DeclList(
			new Decl("c")
	),
	new Body(true )
){
		formals->toFormal(CHARACTER::getInstance(),false);
}

writeReal::writeReal():Procedure (
	"writeReal",
	new DeclList(
			new Decl("r")
	),
	new Body(true )
){
		formals->toFormal(REAL::getInstance(),false);
}

writeString::writeString():Procedure (
	"writeString",
	new DeclList(
			new Decl("s")
	),
	new Body(true )
){
		formals->toFormal(new ArrType(CHARACTER::getInstance()),true);
}


//----read subprograms------------

readInteger::readInteger():Function (
	"readInteger",
	new DeclList(),
	INTEGER::getInstance(),
	new Body(true)
){}


readBoolean::readBoolean():Function (
	"readBoolean",
	new DeclList(),
	BOOLEAN::getInstance(),
	new Body(true)
){}

readChar::readChar():Function (
	"readChar",
	new DeclList(),
	CHARACTER::getInstance(),
	new Body(true)
){}

readReal::readReal():Function (
	"readReal",
	new DeclList(),
	REAL::getInstance(),
	new Body(true)
){}

readString::readString():Procedure (
	"readString",
	new DeclList(
			new Decl("size")
	),
	new Body(true )
){
		formals->toFormal(INTEGER::getInstance(),false);
		DeclList* d=new DeclList(new Decl("s"));
		d->toFormal(new ArrType(CHARACTER::getInstance()),true);
		formals->merge(d);
}

//-------math functions--------

abs_pcl::abs_pcl():Function (
	"abs",
	new DeclList(new Decl("n")),
	INTEGER::getInstance(),
	new Body(true)
){
	formals->toFormal(INTEGER::getInstance(),false);
}


fabs_pcl::fabs_pcl():Function (
	"fabs",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}

sqrt_pcl::sqrt_pcl():Function (
	"sqrt",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}

sin_pcl::sin_pcl():Function (
	"sin",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}

cos_pcl::cos_pcl():Function (
	"cos",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}

tan_pcl::tan_pcl():Function (
	"tan",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}


arctan_pcl::arctan_pcl():Function (
	"arctan",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}


exp_pcl::exp_pcl():Function (
	"exp",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}

ln_pcl::ln_pcl():Function (
	"ln",
	new DeclList(new Decl("r")),
	REAL::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}

pi_pcl::pi_pcl():Function (
	"pi",
	new DeclList(),
	REAL::getInstance(),
	new Body(true)
){}




trunc_pcl::trunc_pcl():Function (
	"trunc",
	new DeclList(new Decl("r")),
	INTEGER::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}



round_pcl::round_pcl():Function (
	"round",
	new DeclList(new Decl("r")),
	INTEGER::getInstance(),
	new Body(true)
){
	formals->toFormal(REAL::getInstance(),false);
}



ord_pcl::ord_pcl():Function (
	"ord",
	new DeclList(new Decl("c")),
	INTEGER::getInstance(),
	new Body(true)
){
	formals->toFormal(CHARACTER::getInstance(),false);
}



chr_pcl::chr_pcl():Function (
	"chr",
	new DeclList(new Decl("n")),
	CHARACTER::getInstance(),
	new Body(true)
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
