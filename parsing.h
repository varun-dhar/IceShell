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
#ifndef _PARSING_H
#define _PARSING_H
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <stdlib.h>
#include "hashmap.h"
#include "strprocessing.h"
#include "util.h"

void getRedir(char* args,char** in, char** out, bool* append, int* stream);
char** replaceEV(char* args,int* n);
char** aliasReplace(char** argv, int* argc);
char** argSplit(char* args, int *n, const char* fmt);
char* parseNested(char* args);
char* parseMP(char* args);
#endif
