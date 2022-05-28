#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>

using namespace std;

char *GetIniKeyString(const char *title,const char *key,const char *filename);
int GetIniKeyInt(const char *title,const char *key,const char *filename);
float GetIniKeyFloat(char *title,char *key,const char *filename);
vector<int> GetStrInfo(char *str);
char * readLine(FILE *fp, char *buffer, int *len);
int readLines(const char *fileName, char *lines[], int max_line,bool saveFirstLine);