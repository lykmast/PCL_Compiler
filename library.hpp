#include "ast.hpp"
extern std::vector<Procedure*> library_subprograms;
extern FILE* inp;
//------write procedures-----------

class writeInteger: public Procedure{
private:
	writeInteger();
public:
	static writeInteger* getInstance(){
		static writeInteger instance;
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

class writeChar: public Procedure{
private:
	writeChar();
public:
	static writeChar* getInstance(){
		static writeChar instance;
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

class readInteger: public Function{
private:
	readInteger();
public:
	static readInteger* getInstance(){
		static readInteger instance;
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

class readChar: public Function{
private:
	readChar();
public:
	static readChar* getInstance(){
		static readChar instance;
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


class abs_pcl: public Function{
private:
	abs_pcl();
public:
	static abs_pcl* getInstance(){
		static abs_pcl instance;
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



class sqrt_pcl: public Function{
private:
	sqrt_pcl();
public:
	static sqrt_pcl* getInstance(){
		static sqrt_pcl instance;
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

class cos_pcl: public Function{
private:
	cos_pcl();
public:
	static cos_pcl* getInstance(){
		static cos_pcl instance;
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



class arctan_pcl: public Function{
private:
	arctan_pcl();
public:
	static arctan_pcl* getInstance(){
		static arctan_pcl instance;
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



class ln_pcl: public Function{
private:
	ln_pcl();
public:
	static ln_pcl* getInstance(){
		static ln_pcl instance;
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




class trunc_pcl: public Function{
private:
	trunc_pcl();
public:
	static trunc_pcl* getInstance(){
		static trunc_pcl instance;
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



class ord_pcl: public Function{
private:
	ord_pcl();
public:
	static ord_pcl* getInstance(){
		static ord_pcl instance;
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
