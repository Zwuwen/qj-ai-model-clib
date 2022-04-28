#include "ai_rknnx_api.h"
static rknn_ai * to_delete = NULL;
extern "C"{
	//==================================================================================
	void * creat_rknn_model_engine(char* modelUrl,float threshold)
	{
        auto rknnEngine = new rknn_ai(modelUrl,threshold);
        cout<<"rknnEngine_new:"<<rknnEngine<<endl;
//        return new rknn_ai(modelUrl,threshold);
        to_delete = rknnEngine;
        return rknnEngine;
	}

//    void * creat_rknn_model_engine(char* modelUrl,float threshold)
//    {
//        auto rknnEngine = new GOO();
//        cout<<"rknnEngine_new:"<<rknnEngine<<endl;
//    //        return new rknn_ai(modelUrl,threshold);
//        to_delete = rknnEngine;
//        return rknnEngine;
//    }
//==================================================================================
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


	RknnRet delete_rknn_model_engine(void* rknnEngine)
	{
		if(rknnEngine==NULL)
		{
			return RKNN_ERR;
		}else
		{
//			RknnRet ret = rknnEngine->deInint_model_engine();
//            cout<<"rknnEngine_delete:"<<rknnEngine<<endl;
//			delete rknnEngine;
//			return ret;
//            return RKNN_SUCCESS;
			RknnRet ret = ((rknn_ai*)rknnEngine)->deInint_model_engine();
            cout<<"rknnEngine_delete1:"<<rknnEngine<<endl;
            cout<<"to_delete:"<<to_delete<<endl;
			delete ((rknn_ai *)rknnEngine);
//            delete to_delete;
			return RKNN_SUCCESS;
		}
		return RKNN_ERR;
	}

}
