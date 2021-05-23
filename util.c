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

unsigned int round2(unsigned int n){
	n--;
	n|=n>>1;
	n|=n>>2;
	n|=n>>4;
	n|=n>>8;
	n|=n>>16;
	return ++n;
}

void readConfig(){
	extern struct hashmap_s aliases;
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
	hashmap_create(round2(countStr(buf,"alias")),&aliases);
	char* p = buf;
	while((p=strstr(p,"alias"))){
		char *key = NULL, *value = NULL;
		if(sscanf(p++,"alias %m[^=]=\"%m[^\"]\"",&key,&value)!=2){
			if(key){free(key);}
			if(value){free(value);}
			continue;
		}
		hashmap_put(&aliases,key,strlen(key),value);
	}
	free(buf);
}

int freeHashmapElements(void* const ctx, struct hashmap_element_s* const e){
	free((char*)e->key);
	free(e->data);
	return -1;
}

void aliasCleanup(struct hashmap_s *map){
	hashmap_iterate_pairs(map,freeHashmapElements,NULL);
	hashmap_destroy(map);
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

void changeDir(int argc,char** argv){
	if(argc==1 || !strcmp(argv[1],getenv("HOME")) || !strcmp(argv[1],"~")){
		setenv("PWD",getenv("HOME"),1);
		chdir(getenv("HOME"));
		return;
	}else{
		char *res = realpath(argv[1],NULL);
		if(!res){
			char* cat = malloc(strlen(getenv("PWD"))+strlen(argv[1])+2);
			sprintf(cat,"%s/%s",getenv("PWD"),argv[1]);
			res = realpath(cat,NULL);
			free(cat);
			if(res){
				struct stat isdir;
				if(stat(res,&isdir)){
					free(res);
					return;
				}
				if(S_ISDIR(isdir.st_mode)){
					setenv("PWD",res,1);
					chdir(res);
					free(res);
					return;
				}
			}
			printf("cd: %s: No such file or directory\n",argv[1]);
			return;
		}
		struct stat isdir;
		if(stat(res,&isdir)){
			free(res);
			return;
		}
		if(S_ISDIR(isdir.st_mode)){
			setenv("PWD",res,1);
			chdir(res);
			free(res);
			return;
		}
	}
	printf("cd: %s: No such file or directory\n",argv[1]);
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

void history(char** argv){
	if(!argv[1]){
		HIST_ENTRY** list = history_list();
		for(int i = 0;list[i];i++){
			printf(" %04d  %s\n",i,list[i]->line);
		}
		return;
	}

	int argc = 0;
	while(argv[argc++]);
	argc--;
	int opt;
	char ops = 0;
	extern char* optarg;
	extern int history_length;
	extern int optind;
	while((opt=getopt(argc,argv,"cd:a::n::r:w::p:s:"))!=-1){
		switch(opt){
			case 'c':
				stifle_history(0);
				unstifle_history();
				break;
			case 'r':
				read_history(optarg);
				break;
			case 'p':
			case 's':
				ops|=(1<<(opt%8));
				break;
			case '?':
				printf("%s: history: Invalid argument\n",getenv("0"));
				break;
			default:
				break;
		}
		if(opt=='d'){
			int index = strtol(optarg,NULL,10);
			if(!index){
				printf("%s: history: History position out of range.\n",getenv("0"));
				continue;
			}
			index = (index<0)?history_length+index:index;
			HIST_ENTRY* e = remove_history(index);
			free((char*)e->line);
			free((char*)e->data);
			free(e);
		}else if(opt=='w' || opt=='a'){
			if(optarg){
				write_history(optarg);
			}else{
				char* hist = malloc(strlen(getenv("HOME"))+sizeof("/.ish_history"));
				sprintf(hist,"%s/.ish_history",getenv("HOME"));
				write_history(hist);
				free(hist);
			}
		}else if(opt=='n'){
			if(optarg){
				read_history(optarg);
			}else{
				char* hist = malloc(strlen(getenv("HOME"))+sizeof("/.ish_history"));
				sprintf(hist,"%s/.ish_history",getenv("HOME"));
				read_history(hist);
				free(hist);
			}
		}
	}
	if(ops&(1<<('p'%8))){
		for(int i = optind;argv[i];i++){
			char* expanded;
			history_expand(argv[i],&expanded);
			puts(expanded);
			free(expanded);
		}
	}
	if(ops&(1<<('s'&8))){
		char* entry = malloc(1);
		*entry = 0;
		int size = 0;
		for(int i = optind;argv[i];i++){
			entry = realloc(entry,(size=size+strlen(argv[i])+1));
			sprintf(entry,"%s %s",entry,argv[i]);
		}
		add_history(entry);
		free(entry);
	}
	if(argv[optind]){
		int len;
		if(!(len=strtol(optarg,NULL,10))){
			exit(0);
		}
		for(int i = history_length-len;i<history_length;i++){
			HIST_ENTRY* entry = history_get(i);
			printf(" %04d %s\n",i,entry->line);
		}
	}
	exit(0);
}
