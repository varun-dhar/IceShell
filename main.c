/*
libedit is under the BSD 3-Clause license, reproduced here:

Copyright (c) 1992, 1993
 The Regents of the University of California.  All rights reserved.

This code is derived from software contributed to Berkeley by
Christos Zoulas of Cornell University.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <editline/readline.h>
#include "hashmap.h"
#include "util.h"
#include "strprocessing.h"
#include "parsing.h"

char *prompt = NULL;
struct hashmap_s aliases;

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
