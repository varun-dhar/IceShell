/*
Copyright 2021 Varun Dhar

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
except in compliance with the License. You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the 
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
either express or implied. See the License for the specific language governing permissions 
and limitations under the License.

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
extern struct hashmap_s aliases;

int main(void){
	init();
	using_history();
	if(read_history("~/.ish_history"))
		perror("read_history");
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
		exec(buf,false,NULL,false);
		free(buf);
	}
	puts("");
	free(prompt);
	aliasCleanup(&aliases);
	char* hist = malloc(strlen(getenv("HOME"))+sizeof("/.ish_history"));
	sprintf(hist,"%s/.ish_history",getenv("HOME"));
	if(write_history(hist))
		perror("write_history");
	free(hist);
}
