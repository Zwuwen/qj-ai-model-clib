
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>
#include "cJSON.h"
#include "src_json.h"


/* Read a file, parse, render back, etc. */
cJSON* cJSON_Loade(const char *filename){
    FILE *f=fopen(filename,"rb");
    if ( NULL == f){
    	printf ("OPENN FILE ERRO!\r\n");
		return NULL;
    }
    fseek(f,0,SEEK_END);  //指针指到文件尾部
    int len=ftell(f);    // ftell() 用于得到文件位置指针当前位置相对于文件首的偏移字节数S
    //fseek(f,0,SEEK_SET);
	rewind(f);
    char *data= NULL;
	data = (char*)malloc(sizeof(char)*(len+1));
	if(data == NULL)
	{
		printf("Memory error \n");
	}
	memset(data, 0, len+1);
    int result = fread(data,1,len,f);
	if(result != len)
	{
		printf("Reading error:result:%d \n",result);
	}
	printf("Reading success:result:%d \n",result);
    fclose(f);
    cJSON* root = cJSON_Parse(data);
    if( NULL == root){
		printf("error cJSON_Parse(data)!\r\n");
	}
    free(data);
    return root;
}

void json_exit(cJSON* json_root){
	cJSON_Delete(json_root);
}

int ModelAlgType_get(cJSON* root,char Path[128],int *pSize){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");

	if(NULL != rknn){
		cJSON* ModelPath  = cJSON_GetObjectItem(rknn, "ModelAlgType");
		if( NULL == ModelPath){
			printf("error load mqtt_root failed!\r\n");
			return -1;
		}
		memcpy(Path, ModelPath->valuestring, strlen(ModelPath->valuestring));
		*pSize = strlen(ModelPath->valuestring);
	}
	return 0;
}

int ModelPath_get(cJSON* root,char Path[128],int *pSize){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");
	
	if(NULL != rknn){
		cJSON* ModelPath  = cJSON_GetObjectItem(rknn, "ModelPath");
		if( NULL == ModelPath){
			printf("error load mqtt_root failed!\r\n");
			return -1;
		}
		memcpy(Path, ModelPath->valuestring, strlen(ModelPath->valuestring));
		*pSize = strlen(ModelPath->valuestring);
	}
	return 0;
}

int Labelsmap_get(cJSON* root,char Path[128],int *pSize){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");
	
	if(NULL != rknn){
		cJSON* Labelsmap  = cJSON_GetObjectItem(rknn, "Labelsmap");
		if( NULL == Labelsmap){
			printf("error load mqtt_root failed!\r\n");
			return -1;
		}
		memcpy(Path, Labelsmap->valuestring, strlen(Labelsmap->valuestring));
		*pSize = strlen(Labelsmap->valuestring);
	}
	return 0;
}

int Priorsbox_get(cJSON* root,char Path[128],int *pSize){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");
	
	if(NULL != rknn){
		cJSON* Priorsbox  = cJSON_GetObjectItem(rknn, "Priorsbox");
		if( NULL == Priorsbox){
			printf("error load mqtt_root failed!\r\n");
			return -1;
		}
		memcpy(Path, Priorsbox->valuestring, strlen(Priorsbox->valuestring));
		*pSize = strlen(Priorsbox->valuestring);
	}
	return 0;
}

int InputSize_get(cJSON* root,int *pSize){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");
	
	if(NULL != rknn){
		cJSON* InputSize  = cJSON_GetObjectItem(rknn, "InputSize");
		if( NULL == InputSize){
			printf("error %s failed!\r\n",__FUNCTION__);
			return -1;
		}
		*pSize = InputSize->valueint;
	}
	return 0;
}

int NumResults_get(cJSON* root,int *Results){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");
	
	if(NULL != rknn){
		cJSON* NumResults  = cJSON_GetObjectItem(rknn, "NumResults");
		if( NULL == NumResults){
			printf("error %s failed!\r\n",__FUNCTION__);
			return -1;
		}
		*Results = NumResults->valueint;
	}
	return 0;
}

int CropType_get(cJSON* root,int *Type){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");
	
	if(NULL != rknn){
		cJSON* CropType  = cJSON_GetObjectItem(rknn, "CropType");
		if( NULL == CropType){
			printf("error %s failed!\r\n",__FUNCTION__);
			return -1;
		}
		*Type = CropType->valueint;
		//memcpy(Path, FilePathIn->valuestring, strlen(FilePathIn->valuestring));
		//*pSize = strlen(FilePath->valuestring);
	}
	return 0;
}

int NumClass_get(cJSON* root,int *Class){
	if (NULL == root){
		return -1;
	}
	cJSON* rknn  = cJSON_GetObjectItem(root, "rknn");
	
	if(NULL != rknn){
		cJSON* NumClass  = cJSON_GetObjectItem(rknn, "NumClass");
		if( NULL == NumClass){
			printf("error %s failed!\r\n",__FUNCTION__);
			return -1;
		}
		*Class = NumClass->valueint;
	}
	return 0;
}
