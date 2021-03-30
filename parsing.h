#ifndef _PARSING_H
#define PARSING_H
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <stdlib.h>
#include "hashmap.h"
#include "strprocessing.h"

void getRedir(char* args,char** in, char** out, bool* append, int* stream);
char** replaceEV(char* args,int* n);
char** aliasReplace(char** argv, int* argc,struct hashmap_s *map);
char** argSplit(char* args, int *n, const char* fmt);
#endif
