#ifndef SRC_JSON__h
#define SRC_JSON__h

#include "cJSON.h"
#ifdef __cplusplus


extern "C"
{
#endif
	cJSON* cJSON_Loade(const char *filename);
	void json_exit(cJSON* json_root);
	int ModelPath_get(cJSON* root,char Path[128],int *pSize);
	int Priorsbox_get(cJSON* root,char Path[128],int *pSize);
	int Labelsmap_get(cJSON* root,char Path[128],int *pSize);
	int ModelAlgType_get(cJSON* root,char Path[128],int *pSize);
	int InputSize_get(cJSON* root,int *pSize);
	int NumResults_get(cJSON* root,int *Results);
	int NumClass_get(cJSON* root,int *Class);
#ifdef __cplusplus
}
#endif

#endif
