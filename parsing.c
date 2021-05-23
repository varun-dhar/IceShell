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
	*n=0;
	char** argv = NULL;
	char* p;
	while((p=tokenize(&args,fmt))){
		if((p = deleteSpaces(p))){
			argv = realloc(argv,(*n+1)*sizeof(*argv));
			argv[(*n)++] = p;
		}
	}
	if((p = deleteSpaces(strdup(args)))){
		argv = realloc(argv,(*n+1)*sizeof(*argv));
		argv[((*n)?(*n)++:*n)] = p;
	}
	return argv;
}

/*char** argSplit(char* args, int *n, const char* fmt){
	char* argCopy = strdup(args);
	char* arg = argCopy;
	*n=0;
	char** argv = NULL;
	char* p = arg;
	while((p=strsep(&arg,fmt))){
		argv = realloc(argv,(*n+1)*sizeof(*argv));
		argv[(*n)++] = strdup(p);
	}
	free(argCopy);
	return argv;
}*/

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
	if(!args || !*args){return NULL;}
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

char** aliasReplace(char** argv, int* argc){
	extern struct hashmap_s aliases;
	char* alias = hashmap_get(&aliases,argv[0],strlen(argv[0]));
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
	return aliasReplace(argv,argc);
}

char* parseNested(char* args){
	char* p = args;
	while((p=strstr(p,"$("))){
		if(p==args || *(p-1)!='\\'){
			char* end = NULL;
			int depth = 0, len = 0;
			for(int i = 1;i<strlen(p);i++){
				if(!strncmp(p+i,"$(",2) && ((p+i-1)==args || p[i-1] != '\\')){
					depth++;
					len++;
					continue;
				}
				if(p[i] == ')' && !depth){
					end = p+i;
					break;
				}else if(p[i] == ')'){
					depth--;
				}
			}
//			while((end=strchr(p,')')) && *(end-1) == '\\');
			if(!end){
				puts("Syntax error");
				return NULL;
			}

			if(!(end-(p+3*(len+1))+1)){
				char* tmp = (p==args)?"":strndup(args,p-args);
				sprintf(args,"%s%s",tmp,end+1);
				if(*tmp)free(tmp);
				continue;
			}
			char* cmd = strndup(p+2,end-(p+2));
			char* ret = NULL;
			exec(cmd,true,&ret,false);
//			ret = (!ret)?"":ret;
			free(cmd);
			if(!ret){
				char* tmp = malloc(p-args+strlen(end+1)+2);
				strncpy(tmp,args,p-args);
				tmp[p-args] = 0;
				strcat(tmp,end+1);
				free(args);
				p = tmp+(p-args);
				args = tmp;
			}else{
				int retLen = strlen(ret);
				char* tmp = malloc(p-args+strlen(end+1)+retLen+1);
				strncpy(tmp,args,p-args);
				tmp[p-args] = 0;
				sprintf(tmp,"%s%s%s",tmp,ret,end+1);
				free(ret);
				p = tmp+(p-args+retLen);
				free(args);
				args = tmp;
			}
		}else{
			p++;
		}
	}
	return args;
}

char* parseMP(char* args){
	int n;
	char** tokens = argSplit(args,&n,"&&");
	printf("1: n: %d\n",n);
	for(int i = 0;i<n;i++)puts(tokens[i]);
	if(!n){
		puts("1");
		free(*tokens);
		free(tokens);
		tokens = argSplit(args,&n,"&");
		puts("at bg");
		printf("2: n: %d\n",n);
		if(!n){
			free(*tokens);
			free(tokens);
			return args;
		}
		for(int i = 0;i<n;i++){
			if(i!=(n-1)){
				//no wait
				exec(tokens[i],false,NULL,true);
			}else{
				//normal
				exec(tokens[i],false,NULL,false);
			}
			free(tokens[i]);
		}
		free(tokens);
		free(args);
		return NULL;
	}
	for(int i = 0;i<n;i++){
		if(i!=(n-1)){
			extern int exitCode;
			exec(tokens[i],false,NULL,false);
			if(exitCode){
				for(;i<n;i++){
					free(tokens[i]);
				}
				break;
			}
		}else{
			int bgn;
			char** bg = argSplit(tokens[i],&bgn,"&");
			if(!bgn){
				free(*bg);
				free(bg);
				exec(tokens[i],false,NULL,false);
			}else{
				for(int j = 0;j<bgn;j++){
					if(j!=(bgn-1)){
						//no wait
						exec(bg[j],false,NULL,true);
					}else{
						//normal
						exec(bg[j],false,NULL,false);
					}
					free(bg[j]);
				}
				free(bg);
			}
		}
		free(tokens[i]);
	}
	free(tokens);
	free(args);
	return NULL;
}
