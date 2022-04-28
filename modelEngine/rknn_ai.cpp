#include "rknn_ai.h"


RknnRet get_model_info(RknnDatas *pRknn)
{
	if(pRknn==NULL) { return RKNN_ERR;}

	//1.get model information form module url
	//1)the path of model file
	pRknn->jsonPath = pRknn->modelDir + "/" +"video.json";
	printf("jsonPath:%s\r\n",pRknn->jsonPath.c_str());

	cJSON* json_cfg = cJSON_Loade(pRknn->jsonPath.c_str());
	if (NULL  == json_cfg)
	{
		printf("cJSON_Loade:%s error\r\n",pRknn->jsonPath.c_str());
		return RKNN_ERR;
	}

	//the information of model
	int len;
	char temp[128] = {0}; 
	//2)ModelAlgType
	ModelAlgType_get(json_cfg, temp,&len);
	pRknn->modelAlgType = temp;
	printf("##############modelAlgType:%s\r\n",pRknn->modelAlgType.c_str() );

	//3)ModelPath
	memset(temp, 0, len);
	ModelPath_get(json_cfg,temp,&len);
	pRknn->modelPath = pRknn->modelDir +  "/" + temp;
	if(0 == strcmp(pRknn->modelAlgType.c_str(), BuiltIn))
	{//rknn 1080 internal algorithm
		pRknn->modelPath= temp;
	}
	printf("##############modelPath:%s\r\n",pRknn->modelPath.c_str());

	//4)Labelsmap
	memset(temp, 0, len);
	Labelsmap_get(json_cfg,temp,&len);
	pRknn->labelPath =  pRknn->modelDir +  "/"+ temp;			
	printf("##############labelPath:%s\r\n",pRknn->labelPath.c_str());

	//5)Priorsbox
	memset(temp, 0, len);
	Priorsbox_get(json_cfg,temp,&len);
	pRknn->priboxPath = pRknn->modelDir +  "/"+ temp;	
	printf("##############priboxPath:%s\r\n",pRknn->priboxPath.c_str() );
	
	//6)
	InputSize_get(json_cfg,&pRknn->inputSize);
	NumResults_get(json_cfg,&pRknn->numResults);
	NumClass_get(json_cfg,&pRknn->numClass);

	json_exit(json_cfg);
	return RKNN_SUCCESS;
}

RknnRet check_model_info(RknnDatas *pRknn)
{
	if(pRknn==NULL) { return RKNN_ERR;}

	//1)check model type
	if( 0 != strcmp(pRknn->modelAlgType.c_str(), SSD) &&
		0 != strcmp(pRknn->modelAlgType.c_str(), Yolo) &&
		0 != strcmp(pRknn->modelAlgType.c_str(), GddModel) &&
		0 != strcmp(pRknn->modelAlgType.c_str(), HikangModel) &&
		0 != strcmp(pRknn->modelAlgType.c_str(), BuiltIn) )
	{
		printf("%s model type is not support!!!!\r\n",pRknn->modelAlgType.c_str());
		return RKNN_ERR;
	}

	//2)check whether the  model file   exists
	if( 0 != (access(pRknn->modelPath.c_str(),F_OK))  && 0 != strcmp(pRknn->modelAlgType.c_str(), BuiltIn))
	{
		printf("%s not exist!!!!\r\n",pRknn->modelPath.c_str());
		return RKNN_ERR;
	}
	if( 0 != (access(pRknn->labelPath.c_str(),F_OK)))
	{
		printf("%s not exist!!!!\r\n",pRknn->labelPath.c_str());
		return RKNN_ERR;
	}
	if( 0 != (access(pRknn->priboxPath.c_str(),F_OK)))
	{
		printf("%s not exist!!!!\r\n",pRknn->priboxPath.c_str());
		return RKNN_ERR;
	}

	return RKNN_SUCCESS;
}


RknnRet struct_to_cJSON(char **json_string,const char *filePath, detect_result_group_t Det,char* taskID)
{
	if((json_string==NULL) || (Det.count==0) || filePath == NULL || taskID == NULL){
		printf("%s: input is invalid",__func__);
	}
 
	cJSON *root=cJSON_CreateObject();
 
	if (!root){
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return RKNN_ERR;
	}
	else{
		cJSON *ImagePath=cJSON_CreateString(filePath);
		cJSON_AddItemToObject(root,"imagePath",ImagePath);

		cJSON *TaskID=cJSON_CreateString(taskID);
		cJSON_AddItemToObject(root,"taskID",TaskID);
						
		cJSON *Objs = cJSON_CreateArray();
		printf ("Det.count:%d\r\n",Det.count);
		for (int i=0;i < Det.count ; i++){
			cJSON *objInf=cJSON_CreateObject();

			cJSON *jsClassName=cJSON_CreateString(Det.results[i].name);
			cJSON_AddItemToObject(objInf,"className",jsClassName);

			cJSON *similarity=cJSON_CreateNumber(Det.results[i].prop);
			cJSON_AddItemToObject(objInf,"similarity",similarity);

			cJSON *x1=cJSON_CreateNumber(Det.results[i].box.left);
			cJSON_AddItemToObject(objInf,"X1",x1);

			cJSON *y1=cJSON_CreateNumber(Det.results[i].box.top);
			cJSON_AddItemToObject(objInf,"Y1",y1);

			cJSON *x2=cJSON_CreateNumber(Det.results[i].box.right);
			cJSON_AddItemToObject(objInf,"X2",x2);
						
			cJSON *y2=cJSON_CreateNumber(Det.results[i].box.bottom);
			cJSON_AddItemToObject(objInf,"Y2",y2);
					
			cJSON_AddItemToArray(Objs, objInf);
		}

 		cJSON_AddItemToObject(root,"alarmList",Objs);
		*json_string=cJSON_PrintUnformatted(root);
		cJSON_Delete(root);
	}
	return RKNN_SUCCESS;
}


//=================================================================================================
rknn_ai::rknn_ai(char* modelUrl,float threshold)
{
	m_rknnData.modelDir = modelUrl;
	m_rknnData.threshold = threshold;
}

rknn_ai::~rknn_ai()
{

}

RknnRet rknn_ai::init_model_engine()
{
	//1.get model info
	RknnRet ret = RKNN_ERR;
	ret = get_model_info(&m_rknnData);
	if(ret != RKNN_SUCCESS)
	{
		printf("get model info is error \r\n");
		return ret;
	}
	
	//2.check model info
	ret = check_model_info(&m_rknnData);
	if(ret != RKNN_SUCCESS)
	{
		printf("model info is error \r\n");
		return ret;
	}

	//3.creat model engine 
	//根据不同模型属性进行不同处理：模型初始化引擎
	ret = RKNN_ERR;
	printf ("================modelAlgType: %s\r\n",m_rknnData.modelAlgType.c_str());
//	if(strcmp(m_rknnData.modelAlgType.c_str(), SSD) == 0)
//	{
//		m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cssd_engine());
//		ret = m_aiEngine_api->RknnInit(&m_rknnData);
//	}
//
//	if(strcmp(m_rknnData.modelAlgType.c_str(), Yolo) == 0)
//	{
//		m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cyolo_engine());
//		ret = m_aiEngine_api->RknnInit(&m_rknnData);
//	}

	if(strcmp(m_rknnData.modelAlgType.c_str(), GddModel) == 0)
	{
//		m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cgdd_engine());
        m_aiEngine_api = (new cgdd_engine());
		ret = m_aiEngine_api->RknnInit(&m_rknnData);
	}

//	if(strcmp(m_rknnData.modelAlgType.c_str(), HikangModel) == 0)
//	{
//		m_aiEngine_api = std::shared_ptr<aiEngine_api>(new chik_engine());
//		ret = m_aiEngine_api->RknnInit(&m_rknnData);
//	}
//
//	// 内部算法
//	if (strcmp(m_rknnData.modelAlgType.c_str(), BuiltIn)==0)
//	{
//		if(strcmp(m_rknnData.modelPath.c_str(), BodyPosture)==0)
//		{
//			m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cpose_engine());
//			ret = m_aiEngine_api->RknnInit(&m_rknnData);
//		}
//	}

	return ret;
}


char* rknn_ai::model_engine_inference(uint8_t* imageBuf, uint32_t imageBufSize, char* imageBufType,char* taskID)
{
			
	detect_result_group_t detect_result_group{};

	cv::Mat image;      //the src image 
	cv::Mat inputImag;  //the image put into model
	
	//1.get  image
	printf("==================imageBuf:%d  imageBufSize:%d  imageBufType:%s\r\n",imageBuf,imageBufSize,imageBufType);
	if (strcmp(imageBufType, YUV) == 0)
	{

	}
	if (strcmp(imageBufType, JPG) == 0)
	{
		std::vector<char> vec_data(imageBuf, imageBuf+imageBufSize);
		image = cv::imdecode(vec_data, CV_LOAD_IMAGE_COLOR);  
		//
		cv::imwrite("test.jpg",image);
    	// 清空  vec_data
		std::vector<char>().swap(vec_data);
		if(m_rknnData.inputSize!=0)
		{
			cv::resize(image, inputImag, cv::Size(m_rknnData.inputSize, m_rknnData.inputSize), (0, 0), (0, 0), cv::INTER_LINEAR);
		}else
		{
			inputImag = image;
		}
	}
	printf("get inferenct image over\r\n");

	//2.put image to detection
	printf ("================modelAlgType: %s\r\n",m_rknnData.modelAlgType.c_str());
	m_aiEngine_api->Inferenct(image, inputImag,&detect_result_group,taskID);

	//3.result analyse
	printf("########m_detect_result_group count:%d \n",detect_result_group.count);

	for(int i =0;i< detect_result_group.count ; i++)
	{
		int x1 = detect_result_group.results[i].box.left;
		int y1 = detect_result_group.results[i].box.top;
		int x2 = detect_result_group.results[i].box.right;
		int y2 = detect_result_group.results[i].box.bottom;

		float prop = detect_result_group.results[i].prop;
		char* label =	detect_result_group.results[i].name;

		printf("result: (%4d, %4d, %4d, %4d), %4.2f, %s\n", x1, y1, x2, y2, prop , label);

		//绘制
		rectangle(image, Point(x1, y1), Point(x2, y2), Scalar(0, 0, 255, 255), 6);
		string temp = to_string(prop)+ "_"+ label;
		printf("draw result:%s\r\n",temp.c_str());
		putText(image,temp.c_str() , Point(x1, y1 - 12), 1, 2, Scalar(0, 255, 0, 255));
	}

	char *JsonMessge = NULL;
	if(detect_result_group.count>0)
	{
		time_t nSeconds = 0;
		time((time_t *)&nSeconds); 
		string saveFileName = std::to_string(nSeconds) + "_"+ std::to_string(rand())+".jpg";  
		printf("%s\r\n",saveFileName.c_str());
		imwrite(saveFileName.c_str(), image);

		struct_to_cJSON(&JsonMessge,saveFileName.c_str(), detect_result_group,taskID);

		printf ("JsonMessge:%s\r\n",JsonMessge);
	}

	memset(&detect_result_group,0,sizeof(detect_result_group_t));
	image.release();
	inputImag.release();

	return JsonMessge;
}

RknnRet rknn_ai::deInint_model_engine()
{
	RknnRet ret = RKNN_ERR;
	
	//根据不同模型属性进行不同处理：模型初始化引擎
	printf ("================modelAlgType: %s\r\n",m_rknnData.modelAlgType.c_str());
    if(m_aiEngine_api)
        ret = m_aiEngine_api->RknnDeinit();

    delete m_aiEngine_api;

	return ret;
}