#include "strprocessing.h"

char* deleteSpaces(char* str){
	char *start,*end;
	start=str;
	while(isspace(*(start++)));
	start--;
	if(!*start){
		free(str);
		return NULL;
	}
	end = start+strlen(start)-1;
	while(isspace(*(end--)) && end>start);
	end[2] = 0;
	char *ret = strdup(start);
	free(str);
	return ret;
}

int count(char* str,char c){
	int ct = 0;
	for(int i = 0;str[i];ct+=(str[i++]==c));
	return ct;
}

int countStr(char* str, char* s){
	int i = 0;
	char* p = str;
	for(;(p=strstr(p,s));i++,p++);
	return i;
}

bool strlncmp(char** l1, char** l2, int n1,int n2){
	if(n1<n2){
		return false;
	}
	for(int i = 0;i<n2;i++){
		if(strcmp(l1[i],l2[i])){
			return false;
		}
	}
	return true;
}
