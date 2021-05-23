#include "builtins.h"

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

