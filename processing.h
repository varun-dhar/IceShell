#ifndef _PROCESSING_H
#define _PROCESSING_H
#include <string.h>
#include <ctype.h>
#include <wordexp.h>
#include "hashmap.h"

char** argSplit(char* args, int *n, const char* fmt){
	char* argCopy = strdup(args);
	char* acfree = argCopy;
	*n=0;
	char** argv = NULL;
	char* p;
	while((p=strsep(&argCopy,fmt))){
		argv = realloc(argv,(*n+1)*sizeof(*argv));
		argv[(*n)++] = strdup(p);
	}
	free(acfree);
	return argv;
}

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

void getRedir(char* args,char** in, char** out, bool* append, int* stream){
	if(sscanf(args,"%*s > %ms%*s",out)==1 || sscanf(args,"%*s %d> %ms%*s",stream,out)==2){
		*append = false;
		char* s = strchr(args,'>');
		s-=(isdigit(s-1))?1:0;
		char* e = strstr(s,*out);
		e+=strlen(*out);
		memmove(s,e,strlen(e)+1);
	}else if(sscanf(args,"%*s >> %ms%*s",out)==1 || sscanf(args,"%*s %d>>%ms%*s",stream,out)==2){
		*append = true;
		char* s = strchr(args,'>');
		s-=(isdigit(s-1))?1:0;
		char* e = strstr(s,*out);
		e+=strlen(*out);
		memmove(s,e,strlen(e)+1);
	}
	if(sscanf(args,"%*s < %ms%*s",in)==1){
		char* s = strchr(args,'<');
		char* e = strstr(s,*in);
		memmove(s,e,strlen(e)+1);
		return;
	}
}

char** replaceEV(char* args,int* n){
	wordexp_t p;
	int c;
	if(c=wordexp(args,&p,0)){
		printf("Syntax error\n");
		return NULL;
	}
	*n = p.we_wordc;
	char** argv = malloc(*n*sizeof(*argv));
	wordfree(&p);
	return argv;
}

unsigned int round2(unsigned int n){
	n--;
	n|=n>>1;
	n|=n>>2;
	n|=n>>4;
	n|=n>>8;
	n|=n>>16;
	return ++n;
}

void readConfig(struct hashmap_s *aliases){
	char* path = malloc(strlen(getenv("HOME"))+sizeof("/.ishrc"));
	sprintf(path,"%s/.ishrc",getenv("HOME"));
	struct stat file;
	stat(path,&file);
	FILE* conf = fopen(path,"r");
	free(path);
	char* buf = malloc(file.st_size+1);
	fread(buf,1,file.st_size,conf);
	fclose(conf);
	buf[file.st_size] = 0;
	hashmap_create(round2(countStr(buf,"alias")),aliases);
	char* p = buf;
	while((p=strstr(p,"alias"))){
		char *key = NULL, *value = NULL;
		if(sscanf(p++,"alias %m[^=]=\"%m[^\"]\"",&key,&value)!=2){
			if(key){free(key);}
			if(value){free(value);}
			continue;
		}
		hashmap_put(aliases,key,strlen(key),value);
	}
	free(buf);
}

int freeElements(void* const ctx, struct hashmap_element_s* const e){
	free((char*)e->key);
	free(e->data);
	return -1;
}

void aliasCleanup(struct hashmap_s *map){
	hashmap_iterate_pairs(map,freeElements,NULL);
	hashmap_destroy(map);
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

char** aliasReplace(char** argv, int* argc,struct hashmap_s *map){
	char* alias = hashmap_get(map,argv[0],strlen(argv[0]));
	if(alias){
		int aliasArgc;
		char** aliasArgv = replaceEV(alias,&aliasArgc);
		if(*argc==1 && strcmp(aliasArgv[0],argv[0])){
			free(argv[0]);
			free(argv);
			argv = aliasArgv;
			*argc = aliasArgc;
		}else if(aliasArgc==1){
			free(argv[0]);
			argv[0] = aliasArgv[0];
			free(aliasArgv);
		}else if(!strlncmp(argv,aliasArgv,*argc,aliasArgc)){
			free(argv[0]);
			aliasArgv = realloc(aliasArgv,(--(*argc)+aliasArgc)*sizeof(*aliasArgv));
			int j = 1;
			for(int i = aliasArgc;i<*argc+aliasArgc;i++,j++){
				aliasArgv[i] = argv[j];
			}
			free(argv);
			argv = aliasArgv;
			*argc+=aliasArgc;
		}else{
			for(int i = 0;i<aliasArgc;i++){
				free(aliasArgv[i]);
			}
			free(aliasArgv);
			return argv;
		}
	}else{
		return argv;
	}
	return aliasReplace(argv,argc,map);
}
#endif
