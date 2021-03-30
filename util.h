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
#include "hashmap.h"
#include "strprocessing.h"
#include "parsing.h"

#define WRITE_END 1
#define READ_END 0

#define GREEN "\001\e[1;32m\002"
#define BLUE "\001\e[1;34m\002"
#define RESET "\001\e[0m\002"

unsigned int round2(unsigned int n);
void readConfig(struct hashmap_s *aliases);
int freeHashmapElements(void* const ctx, struct hashmap_element_s* const e);
void aliasCleanup(struct hashmap_s *map);
void init(struct hashmap_s *aliases);
char* getPrompt(char** prompt);
void changeDir(int argc,char** argv);
int execProg(char** argv,char* in, char* out, bool append, int stream, int* fd, bool last);
void exec(char* args,struct hashmap_s *aliases);
#endif
