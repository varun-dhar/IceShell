#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <editline/readline.h>
#include "hashmap.h"
#include "util.h"
#include "strprocessing.h"
#include "parsing.h"

char *prompt=NULL;
struct hashmap_s aliases;

/*
struct hashmap_s aliases;

void init(){
	setenv("PATH","/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/snap/bin",1);
	setenv("PWD",getenv("HOME"),1);
	chdir(getenv("PWD"));
	setenv("?","0",1);
	readConfig(&aliases);
}

char* getPrompt(){
	char* host = malloc(HOST_NAME_MAX+1);
	gethostname(host,HOST_NAME_MAX);
	prompt = realloc(prompt,strlen(getlogin())+strlen(host)+strlen(getenv("PWD"))+5+(2*(sizeof(GREEN)-1))+(2*(sizeof(RESET)-1)));
	char *p,*wd = strdup(getenv("PWD"));
	if((p=strstr(wd,getenv("HOME")))){
		p+=strlen(getenv("HOME"))-1;
		*p='~';
	}
	p=(!p)?wd:p;
	sprintf(prompt,GREEN"%s@%s"RESET":"BLUE"%s"RESET"$ ",getlogin(),host,p);
	free(host);
	free(wd);
	return prompt;
}


void execProg(char** argv,char* in, char* out,bool append,int stream){
	int ifd, ofd;
	int exitCode = strtol(getenv("?"),NULL,10);
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
//		waitpid(-1,&exitCode,WEXITED);
	}
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

/*
void exec(char* args){
	int n;
	char** argPipes = argSplit(args,&n,"|\n");
	if(n==1){
		char *in=NULL,*out=NULL;
		bool append = false;
		int argc,stream = STDOUT_FILENO;
		getRedir(args,&in,&out,&append,&stream);
		char** argv = replaceEV(args,&argc);
		if(!argv){
			goto cleanPipes;
		}
		argv = aliasReplace(argv,&argc,&aliases);
		argv = realloc(argv,(argc+1)*sizeof(*argv));
		argv[argc] = NULL;
		execProg(argv,in,out,append,stream);
		if(out){free(out);}
		if(in){free(in);}
		for(int i = 0;i<argc;i++){
			free(argv[i]);
		}
		free(argv);
		goto cleanPipes;
	}
	if(n!=(count(args,'|')+1)){
		printf("Syntax error\n");
		goto cleanPipes;
	}
	int pipes[2];
	int fd = STDIN_FILENO;
	for(int i = 0;i<n;i++){
		argPipes[i] = deleteSpaces(argPipes[i]);

		int argc;
		char *in=NULL, *out=NULL;
		bool append = false;
		int stream = STDOUT_FILENO;

		getRedir(argPipes[i],&in,&out,&append,&stream);
		char** argv = replaceEV(argPipes[i],&argc);
		if(!argv){
			goto cleanPipes;
		}
		argv = aliasReplace(argv,&argc,&aliases);
		argv = realloc(argv,(argc+1)*sizeof(*argv));
		argv[argc] = NULL;
		getRedir(argPipes[i],&in,&out,&append,&stream);

		pipe(pipes);
		pid_t pid;
		if((pid=fork())==-1){
			exit(1);
		}else if(!pid){
			signal(SIGINT,SIG_DFL);
			if(in){
				close(fd);
				int ifd = open(in,O_RDONLY);
				if(ifd<0){
					perror(in);
					exit(1);
				}
				dup2(ifd,STDIN_FILENO);
				close(ifd);
			}else{
				dup2(fd,STDIN_FILENO);
			}

			if(out){
				int flags = O_WRONLY|O_CREAT;
				mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
				flags|=((append)?O_APPEND:O_TRUNC);
				int ofd = open(out,flags,mode);
				if(ofd<0){
					perror(out);
					exit(1);
				}
				dup2(ofd,stream);
				close(ofd);
			}
			if(i != (n-1)){
				dup2(pipes[WRITE_END],STDOUT_FILENO);
			}
			close(pipes[READ_END]);
			if(execvp(argv[0],argv)==-1){
				char ecode[5];
				sprintf(ecode,"%d",errno);
				setenv("?",ecode,1);
				perror(argv[0]);
				exit(errno);
			}
			exit(0);
		}else{
			wait(NULL);
			close(pipes[WRITE_END]);
			fd = pipes[READ_END];
		}
		if(out){free(out);}
		if(in){free(in);}
		for(int i = 0;i<argc;i++){
			free(argv[i]);
		}
		free(argv);
	}
cleanPipes:
	for(int i = 0;i<n;i++){
		free(argPipes[i]);
	}
	free(argPipes);
}*/

int main(void){
	init(&aliases);
	using_history();
	signal(SIGINT,SIG_IGN);
	char* buf = NULL;
	while((buf=readline(getPrompt(&prompt)))){
		buf=deleteSpaces(buf);
		if(!buf){
			continue;
		}else if(!*buf){
			free(buf);
			continue;
		}
		add_history(buf);
		if(!strncmp(buf,"cd",2)){
			int argc;
			char** argv = argSplit(buf,&argc," \n");
			changeDir(argc,argv);
			for(int i = 0;i<argc;i++){
				free(argv[i]);
			}
			free(argv);
		}else{
			exec(buf,&aliases);
		}
		free(buf);
	}
	puts("");
	free(prompt);
	aliasCleanup(&aliases);
}
