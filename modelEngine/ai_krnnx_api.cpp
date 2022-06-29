#include "ai_rknnx_api.h"
#include "Loggers.h"
extern "C" {
    /**
     * 创建模型引擎
     * @param modelUrl
     * @param threshold
     * @return
     */
    void *creat_rknn_model_engine(char *modelUrl, float threshold) {
        Loggers::init_multi_sink();
        SPDLOG_TRACE("creat_rknn_model_engine({},{})",modelUrl,threshold);
        auto rknnEngine = new rknn_ai(modelUrl, threshold);
        SPDLOG_TRACE("creat_rknn_model_engine return {}",(void*)rknnEngine);
        return rknnEngine;
    }
    /**
     * 初始化模型引擎
     * @param rknnEngine
     * @return
     */
    RknnRet init_rknn_model_engine(rknn_ai *rknnEngine) {
        SPDLOG_TRACE("init_rknn_model_engine({})",(void*)rknnEngine);
        if (rknnEngine == nullptr) {
            SPDLOG_TRACE("init_rknn_model_engine return {}",RKNN_ERR);
            return RKNN_ERR;
        } else {
            auto result = rknnEngine->init_model_engine();
            SPDLOG_TRACE("init_rknn_model_engine return {}",result);
            return result;
        }
    }
    /**
     * 引擎识别
     * @param rknnEngine
     * @param imageBuf
     * @param imageBufSize
     * @param imageBufType
     * @param taskID
     * @param width
     * @param height
     * @return
     */
    char *rknn_model_engine_inference(
            rknn_ai *rknnEngine, uint8_t *imageBuf, uint32_t imageBufSize, char *imageBufType, char *taskID,int width,
            int height
            ) {

        SPDLOG_TRACE( "rknn_model_engine_inference({},{},{},{},{},{},{})",(void *)rknnEngine,(void *)imageBuf,imageBufSize,imageBufType,taskID, width,height );
        if (rknnEngine == nullptr) {
            SPDLOG_ERROR("rknn_model_engine_inference() return null");
            return nullptr;
        } else {
            auto result = rknnEngine->model_engine_inference(imageBuf, imageBufSize, imageBufType, taskID,width,height);
            if(!result){
                SPDLOG_ERROR("rknn_model_engine_inference() return null");
            }
            else{
                SPDLOG_TRACE("rknn_model_engine_inference() return");
            }
            return result;

        }
    }


    /**
     * 删除引擎
     * @param rknnEngine
     * @return
     */
    RknnRet delete_rknn_model_engine(rknn_ai *rknnEngine) {
        SPDLOG_TRACE("delete_rknn_model_engine({})",(void*)rknnEngine);
        if (rknnEngine == nullptr) {
            SPDLOG_ERROR("delete_rknn_model_engine return {}",RKNN_ERR);
            return RKNN_ERR;
        } else {
            delete rknnEngine;
            rknnEngine= nullptr;
            SPDLOG_TRACE("delete_rknn_model_engine return {}",RKNN_SUCCESS);
            return RKNN_SUCCESS;
        }
    }

    /**
     * 获取设备注册信息
     * @param deviceType
     * @param filePath
     * @return
     */
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
                SPDLOG_TRACE("rknnx_get_device_info() return {}",RKNN_SUCCESS);
			    return RKNN_SUCCESS;
		    }
	    }

        SPDLOG_ERROR("rknnx_get_device_info() return {}",RKNN_ERR);
        return RKNN_ERR;
    }
}
