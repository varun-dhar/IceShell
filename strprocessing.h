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
