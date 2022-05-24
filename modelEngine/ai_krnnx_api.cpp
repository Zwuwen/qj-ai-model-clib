#include "ai_rknnx_api.h"
#include "Loggers.h"
extern "C" {
    //==================================================================================
    void *creat_rknn_model_engine(char *modelUrl, float threshold) {
        Loggers::init_multi_sink();
        SPDLOG_TRACE("creat_rknn_model_engine({},{})",modelUrl,threshold);
        auto rknnEngine = new rknn_ai(modelUrl, threshold);
        return rknnEngine;
    }
    //==================================================================================
    RknnRet init_rknn_model_engine(rknn_ai *rknnEngine) {
        SPDLOG_TRACE("init_rknn_model_engine({})",(void*)rknnEngine);
        if (rknnEngine == nullptr) {
            return RKNN_ERR;
        } else {
            return rknnEngine->init_model_engine();
        }
    }

    char *rknn_model_engine_inference(
            rknn_ai *rknnEngine, uint8_t *imageBuf, uint32_t imageBufSize, char *imageBufType, char *taskID,int width,
            int height
            ) {

        SPDLOG_TRACE( "rknn_model_engine_inference({},{},{},{},{},{},{})",(void *)rknnEngine,(void *)imageBuf,imageBufSize,imageBufType,taskID, width,height );
        if (rknnEngine == nullptr) {
            return nullptr;
        } else {
            return rknnEngine->model_engine_inference(imageBuf, imageBufSize, imageBufType, taskID,width,height);
        }
    }


    RknnRet delete_rknn_model_engine(rknn_ai *rknnEngine) {
        SPDLOG_TRACE("delete_rknn_model_engine({})",(void*)rknnEngine);
        if (rknnEngine == nullptr) {
            return RKNN_ERR;
        } else {
            delete rknnEngine;
            return RKNN_SUCCESS;
        }
    }

    //设备注册信息
    RknnRet rknnx_get_device_info(char* deviceType ,char* filePath){
        SPDLOG_TRACE("rknnx_get_device_info({},{})",deviceType,filePath);
        if (nullptr == deviceType || filePath == nullptr){
			return RKNN_ERR;
	    }
	    // 根据设备类型进行判断，获取设备信息
	    if(0 == strcmp(deviceType, "Gdd"))
	    {
		    GddiRet temp =AlgImpl::getGxtFile(string(filePath));
		    if(temp==GDDI_SUCCESS)
		    {
			    return RKNN_SUCCESS;
		    }
	    }

	    return RKNN_ERR;
    }
}
