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
#include "util.h"

int aliasCompare(const void* a,const void* b,void* data){
	const alias* a1 = a;
	const alias* a2 = b;
	return strcmp(a1->name,a2->name);
}

uint64_t aliasHash(const void* item,uint64_t seed0,uint64_t seed1){
	const alias* a = item;
	return hashmap_sip(item,strlen(a->name),seed0,seed1);
}

void readConfig(){
	extern struct hashmap* aliases;
	char* path = malloc(strlen(getenv("HOME"))+sizeof("/.ishrc"));
	sprintf(path,"%s/.ishrc",getenv("HOME"));
	FILE* conf = fopen(path,"r");
	free(path);
	aliases = hashmap_new(sizeof(struct alias),0,0,0,aliasCompare,user_compare,NULL);
	char* p = NULL;
	int n = 0;
	while(getline(&p,&n,conf) != -1){
/*		char *key = NULL, *value = NULL;
		if(sscanf(p++,"alias %m[^=]=\"%m[^\"]\"",&key,&value)!=2){
			if(key){free(key);}
			if(value){free(value);}
			continue;
		}
		hashmap_put(&aliases,key,strlen(key),value);*/
		exec(p,false,NULL,false);
		free(p);
	}
	fclose(conf);
}

bool freeHashmapElements(const void* item, void* data){
	const alias* a = item;
	free(a->name);
	free(a->command);
	return -1;
}

void aliasCleanup(struct hashmap *map){
	hashmap_scan(map,freeHashmapElements,NULL);
	hashmap_free(map);
}


void init(){
	setenv("PATH","/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/snap/bin",1);
	setenv("PWD",getenv("HOME"),1);
	chdir(getenv("PWD"));
	setenv("?","0",1);
	readConfig();
}

char* getPrompt(char** prompt){
	char* host = malloc(HOST_NAME_MAX+1);
	gethostname(host,HOST_NAME_MAX);
	*prompt = realloc(*prompt,strlen(getlogin())+strlen(host)+strlen(getenv("PWD"))+5+(2*(sizeof(GREEN)-1))+(2*(sizeof(RESET)-1)));
	char *p,*wd = strdup(getenv("PWD"));
	if((p=strstr(wd,getenv("HOME")))){
		p+=strlen(getenv("HOME"))-1;
		*p='~';
	}
	p=(!p)?wd:p;
	sprintf(*prompt,GREEN"%s@%s"RESET":"BLUE"%s"RESET"$ ",getlogin(),host,p);
	free(host);
	free(wd);
	return *prompt;
}

int execProg(char** argv,char* in, char* out,bool append,int stream, int* fd, bool last,bool saveOutput, char** output, bool bg){
	int ifd, ofd;
	int pipes[2];
	if(fd){
		pipe(pipes);
	}
	pid_t pid = fork();
	if(!pid){
		signal(SIGINT,SIG_DFL);
		if(fd){
			dup2(*fd,STDIN_FILENO);
		}else if(in){
			ifd = open(in,O_RDONLY);
			if(ifd<0){
				perror(in);
				exit(1);
			}
			dup2(ifd,STDIN_FILENO);
			close(ifd);
		}

		if(out){
			int flags = O_WRONLY|O_CREAT;
			mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
			flags|=((append)?O_APPEND:O_TRUNC);
			ofd = open(out,flags,mode);
			if(ofd<0){
				perror(out);
				exit(1);
			}
			dup2(ofd,stream);
			close(ofd);
		}

		if(fd){
			if(!last || saveOutput){dup2(pipes[WRITE_END],STDOUT_FILENO);}
			close(pipes[READ_END]);
		}

		if(!strcmp(argv[0],"history")){
			history(argv);
		}else if(execvp(argv[0],argv)==-1){
			perror(argv[0]);
			exit(errno);
		}
		exit(0);
	}
	else{
		int ex = 0;
		if(fd){
			close(pipes[WRITE_END]);
			*fd = pipes[READ_END];
		}
		if(saveOutput){
			const int blockSize = 50;
			*output = malloc(blockSize);
			int total = 0;
			int tmp = 0;
			while((tmp=read(*fd,*output,blockSize)) != -1){
				total+=tmp;
				if(tmp!=blockSize){
					*output = realloc(*output,total+1);
					(*output)[total] = 0;
					break;
				}
				*output = realloc(*output,total+blockSize);
			}
			if(tmp == -1){
				perror("read");
				exit(1);
			}
		}
		if(last){
			if(!bg)waitpid(pid,&ex,0);
			extern int exitCode;
			exitCode = ex;
		}
		return ex;
	}
}

void exec(char* args, bool saveOutput,char** output, bool bg){
	puts(args);
	int n;
	char** argPipes = argSplit(args,&n,"|");
	for(int i = 0;i<n;i++)puts(argPipes[i]);
	int pipes[2];
	int fd = STDIN_FILENO;
	if(!n){
		char *in=NULL,*out=NULL;
		bool append = false;
		int argc,stream = STDOUT_FILENO;
		getRedir(args,&in,&out,&append,&stream);

		*argPipes = parseNested(*argPipes);
		if(!**argPipes){
			goto sCleanup;
		}

/*		if(!(*argPipes = deleteSpaces(*argPipes))){
			goto sCleanup;
		}*/

		if(!(*argPipes = parseMP(*argPipes))){
			goto sCleanup;
		}

		char** argv = replaceEV(*argPipes,&argc);

		if(!argv){
			goto sCleanup;
		}

		argv = aliasReplace(argv,&argc);
		argv = realloc(argv,(argc+1)*sizeof(*argv));
		argv[argc] = NULL;

		execProg(argv,in,out,append,stream,((saveOutput)?&fd:NULL),true,saveOutput,output,bg);

		if(out){free(out);}
		if(in){free(in);}
		for(int i = 0;i<argc;i++){
			free(argv[i]);
		}
		free(argv);
sCleanup:
		free(*argPipes);
		free(argPipes);
		return;
	}
	if(n!=(count(args,'|')+1)){
		puts("Syntax error");
		goto pCleanup;
	}
	for(int i = 0;i<n;i++){
/*		if(!(argPipes[i] = deleteSpaces(argPipes[i]))){
			puts("Syntax error");
			break;
		}*/

		int argc;
		char *in=NULL, *out=NULL;
		bool append = false;
		int stream = STDOUT_FILENO;

		getRedir(argPipes[i],&in,&out,&append,&stream);

		argPipes[i] = parseNested(argPipes[i]);
		if(!*argPipes[i]){
			goto pCleanup;
		}

		if(!(*argPipes = deleteSpaces(*argPipes))){
			goto pCleanup;
		}

		if(!(argPipes[i] = parseMP(argPipes[i]))){
			continue;
		}

		char** argv = replaceEV(argPipes[i],&argc);
		if(!argv){
			goto pCleanup;
		}
		argv = aliasReplace(argv,&argc);
		argv = realloc(argv,(argc+1)*sizeof(*argv));
		argv[argc] = NULL;

		int ex = execProg(argv,in,out,append,stream,&fd,(i==(n-1)),saveOutput,output,bg);

		if(out){free(out);}
		if(in){free(in);}
		for(int i = 0;i<argc;i++){
			free(argv[i]);
		}
		free(argv);
		if(ex){break;}
	}
pCleanup:
	for(int i = 0;i<n;i++){
		free(argPipes[i]);
	}
	free(argPipes);
}
