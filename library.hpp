#include "ast.hpp"
extern std::vector<Procedure*> library_subprograms;
extern FILE* inp;
//------write procedures-----------
class _writeInteger: public Stmt{
	friend class writeInteger;
public:
	virtual void run() const override;
	virtual void sem() override;
private:
	_writeInteger();
	Id* i;
	static _writeInteger* getInstance(){
		static _writeInteger instance;
		return &instance;
	}
};

class writeInteger: public Procedure{
private:
	writeInteger();
public:
	static writeInteger* getInstance(){
		static writeInteger instance;
		return &instance;
	}
};

class _writeBoolean: public Stmt{
	friend class writeBoolean;
public:
	virtual void run() const override;
	virtual void sem() override;
private:
	_writeBoolean();
	Id* i;
	static _writeBoolean* getInstance(){
		static _writeBoolean instance;
		return &instance;
	}
};

class writeBoolean: public Procedure{
private:
	writeBoolean();
public:
	static writeBoolean* getInstance(){
		static writeBoolean instance;
		return &instance;
	}
};

class _writeChar: public Stmt{
	friend class writeChar;
public:
	virtual void run() const override;
	virtual void sem() override;
private:
	_writeChar();
	Id* i;
	static _writeChar* getInstance(){
		static _writeChar instance;
		return &instance;
	}
};

class writeChar: public Procedure{
private:
	writeChar();
public:
	static writeChar* getInstance(){
		static writeChar instance;
		return &instance;
	}
};

class _writeReal: public Stmt{
	friend class writeReal;
public:
	virtual void run() const override;
	virtual void sem() override;
private:
	_writeReal();
	Id* i;
	static _writeReal* getInstance(){
		static _writeReal instance;
		return &instance;
	}
};

class writeReal: public Procedure{
private:
	writeReal();
public:
	static writeReal* getInstance(){
		static writeReal instance;
		return &instance;
	}
};

class _writeString: public Stmt{
	friend class writeString;
public:
	virtual void run() const override;
	virtual void sem() override;
private:
	_writeString();
	Id* i;
	static _writeString* getInstance(){
		static _writeString instance;
		return &instance;
	}
};

class writeString: public Procedure{
private:
	writeString();
public:
	static writeString* getInstance(){
		static writeString instance;
		return &instance;
	}
};

//----read subprograms------------

class _readInteger: public Expr{
	friend class readInteger;
public:
	virtual value eval() override;
	virtual Type* get_type() override;
private:
	_readInteger(){}
	static _readInteger* getInstance(){
		static _readInteger instance;
		return &instance;
	}
};

class readInteger: public Function{
private:
	readInteger();
public:
	static readInteger* getInstance(){
		static readInteger instance;
		return &instance;
	}
};

class _readBoolean: public Expr{
	friend class readBoolean;
public:
	virtual value eval() override;
	virtual Type* get_type() override;
private:
	_readBoolean(){}
	static _readBoolean* getInstance(){
		static _readBoolean instance;
		return &instance;
	}
};

class readBoolean: public Function{
private:
	readBoolean();
public:
	static readBoolean* getInstance(){
		static readBoolean instance;
		return &instance;
	}
};

class _readChar: public Expr{
	friend class readChar;
public:
	virtual value eval() override;
	virtual Type* get_type() override;
private:
	_readChar(){}
	static _readChar* getInstance(){
		static _readChar instance;
		return &instance;
	}
};

class readChar: public Function{
private:
	readChar();
public:
	static readChar* getInstance(){
		static readChar instance;
		return &instance;
	}
};

class _readReal: public Expr{
	friend class readReal;
public:
	virtual value eval() override;
	virtual Type* get_type() override;
private:
	_readReal(){}
	static _readReal* getInstance(){
		static _readReal instance;
		return &instance;
	}
};

class readReal: public Function{
private:
	readReal();
public:
	static readReal* getInstance(){
		static readReal instance;
		return &instance;
	}
};

class _readString: public Stmt{
	friend class readString;
public:
	virtual void run() const override;
	virtual void sem() override;
private:
	_readString();
	Id* sz;
	Id* str;
	static _readString* getInstance(){
		static _readString instance;
		return &instance;
	}
};

class readString: public Procedure{
private:
	readString();
public:
	static readString* getInstance(){
		static readString instance;
		return &instance;
	}
};

//-------math functions--------
class _abs_pcl: public Expr{
	friend class abs_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_abs_pcl();
	Id* i;
	static _abs_pcl* getInstance(){
		static _abs_pcl instance;
		return &instance;
	}
};

class abs_pcl: public Function{
private:
	abs_pcl();
public:
	static abs_pcl* getInstance(){
		static abs_pcl instance;
		return &instance;
	}
};

class _fabs_pcl: public Expr{
	friend class fabs_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_fabs_pcl();
	Id* i;
	static _fabs_pcl* getInstance(){
		static _fabs_pcl instance;
		return &instance;
	}
};

class fabs_pcl: public Function{
private:
	fabs_pcl();
public:
	static fabs_pcl* getInstance(){
		static fabs_pcl instance;
		return &instance;
	}
};

class _sqrt_pcl: public Expr{
	friend class sqrt_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_sqrt_pcl();
	Id* i;
	static _sqrt_pcl* getInstance(){
		static _sqrt_pcl instance;
		return &instance;
	}
};

class sqrt_pcl: public Function{
private:
	sqrt_pcl();
public:
	static sqrt_pcl* getInstance(){
		static sqrt_pcl instance;
		return &instance;
	}
};

class _sin_pcl: public Expr{
	friend class sin_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_sin_pcl();
	Id* i;
	static _sin_pcl* getInstance(){
		static _sin_pcl instance;
		return &instance;
	}
};

class sin_pcl: public Function{
private:
	sin_pcl();
public:
	static sin_pcl* getInstance(){
		static sin_pcl instance;
		return &instance;
	}
};

class _cos_pcl: public Expr{
	friend class cos_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_cos_pcl();
	Id* i;
	static _cos_pcl* getInstance(){
		static _cos_pcl instance;
		return &instance;
	}
};

class cos_pcl: public Function{
private:
	cos_pcl();
public:
	static cos_pcl* getInstance(){
		static cos_pcl instance;
		return &instance;
	}
};

class _tan_pcl: public Expr{
	friend class tan_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_tan_pcl();
	Id* i;
	static _tan_pcl* getInstance(){
		static _tan_pcl instance;
		return &instance;
	}
};

class tan_pcl: public Function{
private:
	tan_pcl();
public:
	static tan_pcl* getInstance(){
		static tan_pcl instance;
		return &instance;
	}
};

class _arctan_pcl: public Expr{
	friend class arctan_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_arctan_pcl();
	Id* i;
	static _arctan_pcl* getInstance(){
		static _arctan_pcl instance;
		return &instance;
	}
};

class arctan_pcl: public Function{
private:
	arctan_pcl();
public:
	static arctan_pcl* getInstance(){
		static arctan_pcl instance;
		return &instance;
	}
};

class _exp_pcl: public Expr{
	friend class exp_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_exp_pcl();
	Id* i;
	static _exp_pcl* getInstance(){
		static _exp_pcl instance;
		return &instance;
	}
};

class exp_pcl: public Function{
private:
	exp_pcl();
public:
	static exp_pcl* getInstance(){
		static exp_pcl instance;
		return &instance;
	}
};

class _ln_pcl: public Expr{
	friend class ln_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_ln_pcl();
	Id* i;
	static _ln_pcl* getInstance(){
		static _ln_pcl instance;
		return &instance;
	}
};

class ln_pcl: public Function{
private:
	ln_pcl();
public:
	static ln_pcl* getInstance(){
		static ln_pcl instance;
		return &instance;
	}
};



class _pi_pcl: public Expr{
	friend class pi_pcl;
public:
	virtual value eval() override;
	virtual Type* get_type() override;
private:
	_pi_pcl(){};
	static _pi_pcl* getInstance(){
		static _pi_pcl instance;
		return &instance;
	}
};

class pi_pcl: public Function{
private:
	pi_pcl();
public:
	static pi_pcl* getInstance(){
		static pi_pcl instance;
		return &instance;
	}
};



class _trunc_pcl: public Expr{
	friend class trunc_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_trunc_pcl();
	Id* i;
	static _trunc_pcl* getInstance(){
		static _trunc_pcl instance;
		return &instance;
	}
};

class trunc_pcl: public Function{
private:
	trunc_pcl();
public:
	static trunc_pcl* getInstance(){
		static trunc_pcl instance;
		return &instance;
	}
};




class _round_pcl: public Expr{
	friend class round_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_round_pcl();
	Id* i;
	static _round_pcl* getInstance(){
		static _round_pcl instance;
		return &instance;
	}
};

class round_pcl: public Function{
private:
	round_pcl();
public:
	static round_pcl* getInstance(){
		static round_pcl instance;
		return &instance;
	}
};


class _ord_pcl: public Expr{
	friend class ord_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_ord_pcl();
	Id* i;
	static _ord_pcl* getInstance(){
		static _ord_pcl instance;
		return &instance;
	}
};

class ord_pcl: public Function{
private:
	ord_pcl();
public:
	static ord_pcl* getInstance(){
		static ord_pcl instance;
		return &instance;
	}
};


class _chr_pcl: public Expr{
	friend class chr_pcl;
public:
	virtual value eval() override;
	virtual void sem() override;
	virtual Type* get_type() override;
private:
	_chr_pcl();
	Id* i;
	static _chr_pcl* getInstance(){
		static _chr_pcl instance;
		return &instance;
	}
};

class chr_pcl: public Function{
private:
	chr_pcl();
public:
	static chr_pcl* getInstance(){
		static chr_pcl instance;
		return &instance;
	}
};


//---------transformation functions------
