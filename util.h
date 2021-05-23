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
#ifndef _UTIL_H
#define _UTIL_H
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <getopt.h>
#include "hashmap.h"
#include "strprocessing.h"
#include "parsing.h"

#define WRITE_END 1
#define READ_END 0

#define GREEN "\001\e[1;32m\002"
#define BLUE "\001\e[1;34m\002"
#define RESET "\001\e[0m\002"

int exitCode;
struct hashmap_s aliases;

typedef struct{
	char** args;
	int len;
	bool saveOutput;
	char* output;
	bool bg;
	char* in;
	char* out;
	bool append;
	int stream;
	int fd;
	bool last;
}command;

unsigned int round2(unsigned int n);
void readConfig();
int freeHashmapElements(void* const ctx, struct hashmap_element_s* const e);
void aliasCleanup(struct hashmap_s *map);
void init();
char* getPrompt(char** prompt);
int execProg(char** argv,char* in, char* out, bool append, int stream, int* fd, bool last, bool saveOutput, char** output,bool bg);
void exec(char* args,bool saveOutput, char** output, bool bg);
#endif
