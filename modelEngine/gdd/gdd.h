#include "../ai_engine_api.h"
#include "alg.h"

class cgdd_engine:public aiEngine_api
{
public:
    cgdd_engine()=default;
    ~cgdd_engine()override =default;
protected:
    //gdd
//	AlgImpl *m_alg_impl= NULL;
    AlgImpl m_alg_impl;
public:
    //引擎初始化
    RknnRet RknnInit(RknnDatas *pRknn);
    //引擎反初始化
    RknnRet RknnDeinit();
    //引擎推理
    RknnRet Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID);
};