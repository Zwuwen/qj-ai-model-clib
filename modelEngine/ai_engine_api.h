#ifndef __AI_ENGINE_API_H__
#define __AI_ENGINE_API_H__

#include "common.h"
#include "pix_formatter.h"

//编解码方法抽象类
class aiEngine_api
{
public:
    aiEngine_api()= default;;
   virtual ~aiEngine_api()= default;;
protected:
    RknnDatas m_rknnData;
public:
    //引擎初始化
    virtual RknnRet RknnInit(RknnDatas *pRknn)= 0;
    //引擎反初始化
    virtual RknnRet RknnDeinit()= 0;
    //引擎推理
    virtual RknnRet Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID)= 0;
    virtual RknnRet Inferenct(ImageSpec &bgr_data,detect_result_group_t *detect_result_group,char* taskID)= 0;
};


#endif // __AI_ENGINE_API_H__