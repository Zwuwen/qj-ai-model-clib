#ifndef __AI_ENGINE_API_H__
#define __AI_ENGINE_API_H__

#include "common.h"

//编解码方法抽象类
class aiEngine_api
{
protected:
    RknnDatas m_rknnData;
public:
    //引擎初始化
    virtual RknnRet RknnInit(RknnDatas *pRknn)= 0;
    //引擎反初始化
    virtual RknnRet RknnDeinit()= 0;
    //引擎推理
    virtual RknnRet Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID)= 0;
};

#endif // __AI_ENGINE_API_H__