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

int freeHashmapElements(void* const ctx, struct hashmap_element_s* const e){
	free((char*)e->key);
	free(e->data);
	return -1;
}

void aliasCleanup(struct hashmap_s *map){
	hashmap_iterate_pairs(map,freeHashmapElements,NULL);
	hashmap_destroy(map);
}


void init(struct hashmap_s *aliases){
	setenv("PATH","/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/snap/bin",1);
	setenv("PWD",getenv("HOME"),1);
	chdir(getenv("PWD"));
	setenv("?","0",1);
	readConfig(aliases);
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

int execProg(char** argv,char* in, char* out,bool append,int stream, int* fd, bool last){
	int ifd, ofd;
	int exitCode = strtol(getenv("?"),NULL,10);
	int pipes[2];
	if(fd){
		pipe(pipes);
	}
	pid_t pid = fork();
	if(!pid){
		signal(SIGINT,SIG_DFL);
		if(in){
			ifd = open(in,O_RDONLY);
			if(ifd<0){
				perror(in);
				exit(1);
			}
			dup2(ifd,STDIN_FILENO);
			close(ifd);
		}else if(fd){
			dup2(*fd,STDIN_FILENO);
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
			if(!last){dup2(pipes[WRITE_END],STDOUT_FILENO);}
			close(pipes[READ_END]);
		}

		if(execvp(argv[0],argv)==-1){
			char ecode[5];
			sprintf(ecode,"%d",errno);
			setenv("?",ecode,1);
			perror(argv[0]);
			exit(errno);
		}
		exit(0);
	}
	else{
		wait(NULL);
		if(fd){
			close(pipes[WRITE_END]);
			*fd = pipes[READ_END];
		}
//		waitpid(-1,&exitCode,WEXITED);
	}
	if(strtol(getenv("?"),NULL,10)!=exitCode){
		return strtol(getenv("?"),NULL,10);
	}else{
		setenv("?","0",1);
		return 0;
	}
}

void exec(char* args,struct hashmap_s *aliases){
	int n;
	char** argPipes = argSplit(args,&n,"|\n");
	if(n==1){
		char *in=NULL,*out=NULL;
		bool append = false;
		int argc,stream = STDOUT_FILENO;
		getRedir(args,&in,&out,&append,&stream);
		char** argv = replaceEV(args,&argc);
		if(!argv){
			goto freePipes;
		}
		argv = aliasReplace(argv,&argc,aliases);
		argv = realloc(argv,(argc+1)*sizeof(*argv));
		argv[argc] = NULL;

		execProg(argv,in,out,append,stream,NULL,0);

		if(out){free(out);}
		if(in){free(in);}
		for(int i = 0;i<argc;i++){
			free(argv[i]);
		}
		free(argv);
		goto freePipes;
	}
	if(n!=(count(args,'|')+1)){
		puts("Syntax error");
		goto freePipes;
	}
	int pipes[2];
	int fd = STDIN_FILENO;
	for(int i = 0;i<n;i++){
		if(!(argPipes[i] = deleteSpaces(argPipes[i]))){
			puts("Syntax error");
			break;
		}

		int argc;
		char *in=NULL, *out=NULL;
		bool append = false;
		int stream = STDOUT_FILENO;

		getRedir(argPipes[i],&in,&out,&append,&stream);
		char** argv = replaceEV(argPipes[i],&argc);
		if(!argv){
			goto freePipes;
		}
		argv = aliasReplace(argv,&argc,aliases);
		argv = realloc(argv,(argc+1)*sizeof(*argv));
		argv[argc] = NULL;
		getRedir(argPipes[i],&in,&out,&append,&stream);

		int ex = execProg(argv,in,out,append,stream,&fd,(i==(n-1)));

		if(out){free(out);}
		if(in){free(in);}
		for(int i = 0;i<argc;i++){
			free(argv[i]);
		}
		free(argv);
		if(ex){break;}
	}
freePipes:
	for(int i = 0;i<n;i++){
		free(argPipes[i]);
	}
	free(argPipes);
}
