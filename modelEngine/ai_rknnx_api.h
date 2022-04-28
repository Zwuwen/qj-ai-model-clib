#ifndef  __AI_RKNNX_API_H__
#define  __AI_RKNNX_API_H__

#include "rknn_ai.h"

extern "C"{
     /****************************
          * func: creat_rknn_model_engine
          * param: moduleUrl[input]: the path of model file 
          * return: rknn_ai*
          * des: create rknn model engine
     *****************************/
//     rknn_ai* creat_rknn_model_engine(char* modelUrl,float threshold);
void* creat_rknn_model_engine(char* modelUrl,float threshold);

/****************************
     * func: init_rknn_model_engine
     * param:rknnEngine[input]: the pointer of rknn recognition  engine
     * return: the result of detection result
     * des: init rknn model engine
*****************************/
     RknnRet init_rknn_model_engine(rknn_ai* rknnEngine);

     /****************************
          * func: rknn_model_engine_inference
          * param:rknnEngine[input]: the pointer of rknn recognition  engine
               imageBuf[input]: the buf of image 
               imageBufSize[input]: the size of image buf
               imageBufType[input]: the type of image buf
               taskID[input]: the ID of image
          * return: the result of detection result 
          * des: use the rknn model engine to inference the image
     *****************************/
     char* rknn_model_engine_inference(rknn_ai* rknnEngine , uint8_t* imageBuf, uint32_t imageBufSize, char* imageBufType,char* taskID);

     /****************************
          * func: delete_rknn_model_engine
          * param: rknnEngine[input]: the only ID of rknn model engine 
          * return: RknnRet
          * des: delete the rknn model engine
     *****************************/
     RknnRet delete_rknn_model_engine(void* rknnEngine);
}

#endif

