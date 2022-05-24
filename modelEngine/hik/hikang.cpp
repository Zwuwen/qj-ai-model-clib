#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <curl/curl.h>
#include "hikang.h"

std::string base64Encode(const unsigned char* Data, int DataByte) {
	//返回值
	std::string strEncode;
	unsigned char Tmp[4] = { 0 };
	int LineLength = 0;
	for (int i = 0; i < (int)(DataByte / 3); i++) {
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		Tmp[3] = *Data++;
		strEncode += EncodeTable[Tmp[1] >> 2];
		strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
		strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
		strEncode += EncodeTable[Tmp[3] & 0x3F];
		if (LineLength += 4, LineLength == 76) { strEncode += "\r\n"; LineLength = 0; }
	}
	//对剩余数据进行编码
	int Mod = DataByte % 3;
	if (Mod == 1) {
		Tmp[1] = *Data++;
		strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
		strEncode += "==";
	}
	else if (Mod == 2) {
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
		strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
		strEncode += "=";
	}

	return strEncode;
}


chik_engine::chik_engine()
{
	m_JsonMessge = NULL;
}

chik_engine::~chik_engine()
{
	RknnDeinit();
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// hikang 模型初始化
RknnRet chik_engine::RknnInit(RknnDatas *pRknn)
{
	if(pRknn==NULL) {return RKNN_ERR ;}
	//1、辅助模型数据
	memcpy(&m_rknnData, pRknn,sizeof(RknnDatas));
	
	// 2)创建json 
	printf("Hikang priboxPath :%s------------------------------------\n",m_rknnData.priboxPath.c_str());
	cJSON* json_cfg = cJSON_Loade(m_rknnData.priboxPath.c_str());
	if (NULL  == json_cfg)
	{
		printf("cJSON_Loade:%s error\r\n",m_rknnData.priboxPath.c_str());
		return RKNN_ERR;
	}
	// ①获取 url   admin   pass
	cJSON* url  = cJSON_GetObjectItem(json_cfg, "url");
	if( NULL == url)
	{
		printf("error load url failed!\r\n");
		return RKNN_ERR;
	}else
	{
		memcpy(m_engurl, url->valuestring, strlen(url->valuestring));
	}
	printf("Hikang m_engurl :%s------------------------------------\n",m_engurl);

	cJSON* cJSON_admin  = cJSON_GetObjectItem(json_cfg, "admin");
	if( NULL == cJSON_admin)
	{
		printf("error load admin failed!\r\n");
		return RKNN_ERR;
	}else
	{
		memcpy(m_admin, cJSON_admin->valuestring, strlen(cJSON_admin->valuestring));
	}
	printf("Hikang admin :%s------------------------------------\n",m_admin);

	cJSON* cJSON_pass  = cJSON_GetObjectItem(json_cfg, "pass");
	if( NULL == cJSON_pass)
	{
		printf("error load pass failed!\r\n");
		return RKNN_ERR;
	}else
	{
		memcpy(m_pass, cJSON_pass->valuestring, strlen(cJSON_pass->valuestring));
	}
	printf("Hikang pass :%s------------------------------------\n",m_pass);

	// ② 获取data
	// 将 json_cfg 中的  "Binary"  替换为现在图片的 base64 编码
	cJSON* data  = cJSON_GetObjectItem(json_cfg, "data");
	if( NULL == data)
	{
		printf("error load data failed!\r\n");
		return RKNN_ERR;
	}
	//cJSON *cbase64=cJSON_CreateString(img_data.c_str());
	//cJSON_AddItemToObject(data,"Binary",cbase64);
	m_JsonMessge=cJSON_PrintUnformatted(data);

	//
	cJSON_Delete(json_cfg);

	return RKNN_SUCCESS;
}

// 反初始化======================================================================================================================
RknnRet chik_engine::RknnDeinit()
{
	if(NULL != m_JsonMessge)
	{
		m_JsonMessge = NULL ;
	}

	return RKNN_SUCCESS;
}


// 模型推理======================================================================================================================
RknnRet chik_engine::Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID)
{
	if(srcimg.empty() || inputImg.empty() || detect_result_group==NULL)
	{
		return RKNN_ERR;
	}

	printf("Inferenct_Hikang Process output ------------------------------------\n");

	//1)Mat转base64
	std::string img_data;
    std::vector<uchar> vecImg;
    std::vector<int> vecCompression_params;

    vecCompression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    vecCompression_params.push_back(90);

    cv::imencode(".jpg", srcimg, vecImg, vecCompression_params);
    img_data = base64Encode(vecImg.data(), vecImg.size());

	vector<uchar>().swap(vecImg);
	vector<int>().swap(vecCompression_params);

	//2) data 
	if(m_JsonMessge == NULL)
	{
		return RKNN_ERR;
	}
	cJSON* data = cJSON_Parse(m_JsonMessge);
	if( NULL == data)
	{
		printf("error load data failed!\r\n");
		return RKNN_ERR;
	}
	cJSON *cbase64=cJSON_CreateString(img_data.c_str());
	if(cbase64 == NULL)
	{
		cJSON_Delete(data);
		return RKNN_ERR;
	}
	cJSON_AddItemToObject(data,"Binary",cbase64);
	char *json_string = cJSON_PrintUnformatted(data);
	cJSON_Delete(data);

	
	// 3)post 请求
	CURL *curl_handle;
	struct curl_slist *hs=NULL;  
    CURLcode res;

	 // init the curl session 初始化一个curl的handle
    curl_handle = curl_easy_init();
	if(curl_handle)
	{
		// set URL to get here 
    	curl_easy_setopt(curl_handle, CURLOPT_URL,m_engurl);
		printf("Hikang  engurl :%s------------------------------------\n",m_engurl);
		//
		hs = curl_slist_append(hs, "Content-Type: text/plain");
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, hs);
		//
		curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
		curl_easy_setopt(curl_handle, CURLOPT_USERNAME, m_admin);
		printf("Hikang admin :%s------------------------------------\n",m_admin);
		curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, m_pass);
		printf("Hikang pass :%s------------------------------------\n",m_pass);
		//
		curl_easy_setopt(curl_handle,CURLOPT_POST,true);
		//
		curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDS, json_string);
		//printf("Hikang json_string :%s------------------------------------\n",json_string);

		res = curl_easy_perform(curl_handle); 
		if (res != CURLE_OK) 
		{
			printf("curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
		}

		curl_easy_cleanup(curl_handle);
		// 释放表头
    	curl_slist_free_all(hs);
	}

	json_string = NULL ;

	return RKNN_SUCCESS;
}

RknnRet chik_engine::Inferenct(ImageSpec &bgr_data,detect_result_group_t *detect_result_group,char* taskID){

	return RKNN_SUCCESS;
}
