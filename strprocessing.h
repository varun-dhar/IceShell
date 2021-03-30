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
#ifndef _STRPROCESSOR_H
#define _STRPROCESSOR_H
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

char* deleteSpaces(char* str);
int count(char* str,char c);
int countStr(char* str, char* s);
bool strlncmp(char** l1, char** l2, int n1,int n2);
#endif
