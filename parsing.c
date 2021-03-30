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

#include "parsing.h"

char** argSplit(char* args, int *n, const char* fmt){
	char* argCopy = strdup(args);
	char* arg = argCopy;
	*n=0;
	char** argv = NULL;
	char* p;
	while((p=strsep(&arg,fmt))){
		argv = realloc(argv,(*n+1)*sizeof(*argv));
		argv[(*n)++] = strdup(p);
	}
	free(argCopy);
	return argv;
}

void getRedir(char* args,char** in, char** out, bool* append, int* stream){
	char* s;
	if((s = strchr(args,'>'))){
		*stream = ((isdigit(*(s-1)))?*(s-1)-'0':*stream);
		*append=(*(s+1) == '>');
		char* pos = s;
		while(isspace(--pos));
		s=strchr(s,' ');
		if(!s){return;}
		char* e = strchr(++s,' ');
		e=(!e)?s+strlen(s):e;
		*out = strndup(s,e-s+1);
		(*out)[e-s]=0;
		memmove(pos,e,strlen(e)+1);
	}
	if((s = strchr(args,'<'))){
		char* pos = s;
		while(isspace(--pos));
		s=strchr(s,' ');
		if(!s){return;}
		char* e = strchr(++s,' ');
		e=(!e)?s+strlen(s):e;
		*in = strndup(s,e-s+1);
		(*in)[e-s]=0;
		memmove(pos,e,strlen(e)+1);
	}
}

char** replaceEV(char* args,int* n){
	wordexp_t p;
	if(wordexp(args,&p,0)){
		printf("Syntax error\n");
		return NULL;
	}
	*n = p.we_wordc;
	char** argv = malloc(*n*sizeof(*argv));
	for(int i = 0;i<*n;i++){
		argv[i] = strdup(p.we_wordv[i]);
	}
	wordfree(&p);
	return argv;
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
