#include "../ai_engine_api.h"

extern "C"{	
	cJSON* cJSON_Loade(const char *filename);
}

//编码表
const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

class chik_engine:public aiEngine_api
{
public:
    chik_engine();
    virtual ~chik_engine();
protected:
    char m_engurl[255]= { 0 };    //识别引擎
    char m_admin[64]= { 0 };
    char m_pass[64]= { 0 };
    char *m_JsonMessge;
    //
    int  m_taskID  ;  //当前识别图片

public:
    //引擎初始化
    RknnRet RknnInit(RknnDatas *pRknn);
    //引擎反初始化
    RknnRet RknnDeinit();
    //引擎推理
    RknnRet Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID);
    RknnRet Inferenct(ImageSpec &bgr_data,detect_result_group_t *detect_result_group,char* taskID);
};