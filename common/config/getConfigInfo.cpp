#include "getConfigInfo.h"

char *GetIniKeyString(char *title,char *key,const char *filename)
{
	FILE *fp;
	char szLine[1024];
	static char tmpstr[1024];
	int rtnval;
	int i = 0;
	int flag = 0;
	char *tmp;

	if((fp = fopen(filename, "r")) == NULL)
	{
		printf("have   no   such   file \n");
		return NULL;
	}
	while(!feof(fp))
	{
		rtnval = fgetc(fp);
		if(rtnval == EOF)
		{
			break;
		}
		else
		{
			szLine[i++] = rtnval;
		}
		if(rtnval == '\n')
		{
#ifndef WIN32
			i--;
#endif
			szLine[--i] = '\0';
			i = 0;
			tmp = strchr(szLine, '=');

			if(( tmp != NULL )&&(flag == 1))
			{
				if(strstr(szLine,key)!=NULL)
				{
					//
					if ('#' == szLine[0])
					{
					}
					else if ( '\/' == szLine[0] && '\/' == szLine[1] )
					{

					}
					else
					{
						//
						strcpy(tmpstr,tmp+1);
						fclose(fp);
						return tmpstr;
					}
				}
			}
			else
			{
				strcpy(tmpstr,"[");
				strcat(tmpstr,title);
				strcat(tmpstr,"]");
				if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 )
				{
					//
					flag = 1;
				}
			}
		}
	}
	fclose(fp);

	return NULL;
}


int GetIniKeyInt(char *title,char *key,const char *filename)
{
	return atoi(GetIniKeyString(title,key,filename));
}

float GetIniKeyFloat(char *title,char *key,const char *filename)
{
	return atof(GetIniKeyString(title,key,filename));
}

vector<int> GetStrInfo(char *str)
{
    vector<int> resultInfo;
    char delims[] = ",";
    char *result = NULL;
    result = strtok( str, delims );
    while( result != NULL )
    {
        //printf( "result is \"%s\"\n", result );
        resultInfo.push_back(atoi(result));
        result = strtok( NULL, delims );
    }
    return  resultInfo;
}


char * readLine(FILE *fp, char *buffer, int *len)
{
    int ch;
    int i = 0;
    size_t buff_len = 0;

    buffer = (char *)malloc(buff_len + 1);
    if (!buffer) return NULL;  // Out of memory

    while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
        buff_len++;
        void *tmp = realloc(buffer, buff_len + 1);
        if (tmp == NULL) {
            free(buffer);
            return NULL; // Out of memory
        }
        buffer = (char *)tmp;

        buffer[i] = (char) ch;
        i++;
    }
    buffer[i] = '\0';

	*len = buff_len;

    // Detect end
    if (ch == EOF && (i == 0 || ferror(fp))) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

int readLines(const char *fileName, char *lines[], int max_line,bool saveFirstLine)
{
	FILE* file = fopen(fileName, "r");
	char *s;
	int i = 0;
	int n = 0;

	bool beFirst = true;

	while ((s = readLine(file, s, &n)) != NULL) 
	{
		if(beFirst==true && saveFirstLine == false)
		{
			beFirst=false;
			continue;
		}
		lines[i++] = s;
        if (i >= max_line) break;
	}
	fclose(file);
	return i;
}