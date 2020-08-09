#include "cstdint"
#include "cstdio"
#include "cstring"
#include "math.h"

void writeInteger(int32_t i){
	printf("%d",i);
}

void writeBoolean(uint8_t b){
	printf("%s", b ? "true" : "false");
}

void writeChar(uint8_t c){
	printf("%c",c);
}

void writeReal(double r){
	printf("%lf",r);
}

void writeString(uint8_t *s){
	printf("%s",s );
}

int32_t readInteger(){
	char buf[256];
	if(fgets(buf,sizeof(buf),stdin)){
		return atoi(buf);
	}
	else{
		fprintf(stderr,"Too long input\n");
		exit(1);
	}

}

uint8_t readBoolean(){
	char word[7];
	uint8_t b;
	if(!fgets(word, 7, stdin)){
		fprintf(stderr,"Too long input\n");
		exit(1);
	}
	word[strlen(word)-1]='\0';
	if(!strcmp(word,"true")){
		b=true;
	}
	else if(!strcmp(word,"false")){
		b=false;
	}
	else{
		fprintf(stderr,"Invalid input; expected"
			" \"true\" or \"false\" but received"
			" unknown \"%s\"\n",word);
		exit(1);
	}
	return b;
}



uint8_t readChar(){
	return getc(stdin);
}




double readReal(){
	char buf[256];
	if(!fgets(buf,sizeof(buf),stdin)){
		fprintf(stderr,"Too long input\n");
		exit(1);
	}
	return atof(buf);
}



void readString(int32_t size, uint8_t* s){
	int i=0;
	char c=getc(stdin);
	while(c!='\n' and c!= EOF and i<size-1){
		*s++=(uint8_t)c;
		c=getc(stdin);
	}
	*s++='\0';
}



int32_t abs_pcl(int32_t i){
	return abs(i);
}

double fabs_pcl(double r){
	return fabs(r);
}

double sqrt_pcl(double r){
	return sqrt(r);
}

double sin_pcl(double r){
	return sin(r);
}

double cos_pcl(double r){
	return cos(r);
}

double tan_pcl(double r){
	return tan(r);
}

double arctan_pcl(double r){
	return atan(r);
}

double exp_pcl(double r){
	return exp(r);
}

double ln_pcl(double r){
	return log(r);
}

double pi_pcl(){
	return M_PI;
}

int32_t trunc_pcl(double r){
	return trunc(r);
}

int32_t round_pcl(double r){
	return round(r);
}


int32_t ord_pcl(uint8_t c){
	return (int32_t) c;
}

uint8_t chr_pcl(int32_t i){
	return (uint8_t) i;
}
