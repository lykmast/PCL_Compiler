#include "stdint.h"
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"

void writeInteger_pcl(int32_t i){
	printf("%d",i);
}

void writeBoolean_pcl(uint8_t b){
	printf("%s", b ? "true" : "false");
}

void writeChar_pcl(uint8_t c){
	printf("%c",c);
}

void writeReal_pcl(double r){
	printf("%lf",r);
}

void writeString_pcl(uint8_t s[]){
	printf("%s",s );
}

int32_t readInteger_pcl(){
	char buf[256];
	if(fgets(buf,sizeof(buf),stdin)){
		return atoi(buf);
	}
	else{
		fprintf(stderr,"Too long input\n");
		exit(1);
	}

}

uint8_t readBoolean_pcl(){
	char word[7];
	uint8_t b;
	if(!fgets(word, 7, stdin)){
		fprintf(stderr,"Too long input\n");
		exit(1);
	}
	word[strlen(word)-1]='\0';
	if(!strcmp(word,"true")){
		b=1;
	}
	else if(!strcmp(word,"false")){
		b=0;
	}
	else{
		fprintf(stderr,"Invalid input; expected"
			" \"true\" or \"false\" but received"
			" unknown \"%s\"\n",word);
		exit(1);
	}
	return b;
}



uint8_t readChar_pcl(){
	return getc(stdin);
}




double readReal_pcl(){
	char buf[256];
	if(!fgets(buf,sizeof(buf),stdin)){
		fprintf(stderr,"Too long input\n");
		exit(1);
	}
	return atof(buf);
}



void readString_pcl(int32_t size, uint8_t s[]){
	int i=0;
	char c=getc(stdin);
	while(c!='\n' && c!= EOF && i<size-1){
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
