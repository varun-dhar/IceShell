/*
Copyright 2021 Varun Dhar

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
except in compliance with the License. You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the 
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
either express or implied. See the License for the specific language governing permissions 
and limitations under the License.
*/
#include "strprocessing.h"

char* deleteSpaces(char* str){
	if(!*str){
		free(str);
		return NULL;
	}
	char *start,*end;
	start=str;
	for(;isspace(*(start));start++);
	if(!*start){
		free(str);
		return NULL;
	}
	end = start+strlen(start)-1;
	for(;isspace(*(end)) && end>start;end--);
	end[1] = 0;
	char *ret = strdup(start);
	free(str);
	return ret;
}

int count(char* str,char c){
	int ct = 0;
	for(;(str=strchr(str,c));ct++,str++);
	return ct;
}

int countStr(char* str, char* s){
	int ct = 0;
	for(;(str=strstr(str,s));ct++,str++);
	return ct;
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

/*
char** _tokenize(char* str, ...){
	va_list args;
	va_start(args,str);
	char *arg,*s=strdup(str);
	char** tokens = NULL;
	int len = strlen(str),size=0;
	for(int i = 0;(arg=va_arg(args,char*))!=NULL;i++){
		int p;
		while((p=strcspn(str,arg))!=len){
			for(int j = 0;i<strlen(arg);i++){
				s[p+j] = 0;
			}
		}
	}
	va_end(args);
	char* p = s;
	for(int i = 0;i<len;i++){
		tokens = realloc(tokens,(size+1)*sizeof(*tokens));
		tokens[size++] = strdup(p);
		p+=strlen(p);
		for(;i<len && !*p;i++,p++);
	}
	free(s);
	return tokens;
}
*/
char* tokenize(char** str,const char* format){
	char* fmt = strdup(format);
	size_t len = strlen(*str), min = len,tokenLen = 1;
	for(size_t i = 0;i<strlen(format);){
		size_t pos = i;
		for(;fmt[pos] == fmt[pos+1];pos++);
		char tmp = fmt[++pos];
		fmt[pos] = 0;
		char* substr = strstr(*str,fmt+i);
		//printf("%s %s\n",substr,fmt+i);
		min = (substr && (substr-*str)<min)?tokenLen=strlen(fmt+i),substr-*str:min;
		//tokenLen = (strlen(fmt+i)>tokenLen)?strlen(fmt+i):tokenLen;
		i+=strlen(fmt+i);
		fmt[pos] = tmp;
	}
	free(fmt);
	if(min!=len){
		char* ret = strndup(*str,min);
		*str+=min+tokenLen;
		return ret;
	}
	return NULL;
}
