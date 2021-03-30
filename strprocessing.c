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
