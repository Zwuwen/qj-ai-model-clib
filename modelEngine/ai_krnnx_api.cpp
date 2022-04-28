#include "ai_rknnx_api.h"

extern "C" {
    //==================================================================================
    void *creat_rknn_model_engine(char *modelUrl, float threshold) {
        auto rknnEngine = new rknn_ai(modelUrl, threshold);
        cout << "rknnEngine_new:" << rknnEngine << endl;
        return rknnEngine;
    }
    //==================================================================================
    RknnRet init_rknn_model_engine(rknn_ai *rknnEngine) {
        if (rknnEngine == nullptr) {
            return RKNN_ERR;
        } else {
            return rknnEngine->init_model_engine();
        }
        return RKNN_ERR;
    }

    char *rknn_model_engine_inference(rknn_ai *rknnEngine, uint8_t *imageBuf, uint32_t imageBufSize, char *imageBufType,
                                      char *taskID) {
        if (rknnEngine == nullptr) {
            return nullptr;
        } else {
            return rknnEngine->model_engine_inference(imageBuf, imageBufSize, imageBufType, taskID);
        }
        return nullptr;
    }


    RknnRet delete_rknn_model_engine(rknn_ai *rknnEngine) {
        if (rknnEngine == nullptr) {
            return RKNN_ERR;
        } else {
//            RknnRet ret = ((rknn_ai *) rknnEngine)->deInint_model_engine()
            cout << "rknnEngine_delete1:" << rknnEngine << endl;
//            delete ((rknn_ai *) rknnEngine);
            delete rknnEngine;
            return RKNN_SUCCESS;
        }
        return RKNN_ERR;
    }
}
