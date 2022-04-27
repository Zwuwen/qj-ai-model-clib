#include "ai_rknnx_api.h"
extern "C"{
	//==================================================================================
	rknn_ai* creat_rknn_model_engine(char* modelUrl,float threshold)
	{
		return new rknn_ai(modelUrl,threshold);
	}

	RknnRet init_rknn_model_engine(rknn_ai* rknnEngine)
	{
		if(rknnEngine==NULL)
		{
			return RKNN_ERR;
		}else
		{  
	  		return rknnEngine->init_model_engine();
		}
		return RKNN_ERR;
	}

	char* rknn_model_engine_inference(rknn_ai* rknnEngine , uint8_t* imageBuf, uint32_t imageBufSize, char* imageBufType,char* taskID)
	{
		if(rknnEngine==NULL)
		{
			return NULL;
		}else
		{
			return  rknnEngine->model_engine_inference(imageBuf, imageBufSize,imageBufType,taskID);	
		}
		return NULL;
	}


	RknnRet delete_rknn_model_engine(rknn_ai* rknnEngine)
	{
		if(rknnEngine==nullptr)
		{
			return RKNN_ERR;
		}else
		{
			RknnRet ret = rknnEngine->deInint_model_engine();
			delete rknnEngine;
			return ret;
		}
		return RKNN_ERR;
	}

}
