#ifndef _SSD_H_
#define _SSD_H_

#include "../ai_engine_api.h"

#define IMG_CHANNEL         3
#define Y_SCALE  10.0f
#define X_SCALE  10.0f
#define H_SCALE  5.0f
#define W_SCALE  5.0f


typedef struct
{//存储不会变的值
	// model labels info
	char *labels[91];
	// ssd 
	float box_priors[4][10470];

	// model engine-------------------------------------------
	// rknn model engine information
    rknn_context ctx;
}RknnSSDEng;


class cssd_engine:public aiEngine_api
{
public:
    cssd_engine();
    ~cssd_engine() override;

protected:
    // model engine-------------------------------------------
    RknnSSDEng m_rknnSSDEng;
public:
    //引擎初始化
    RknnRet RknnInit(RknnDatas *pRknn) override;
    //引擎反初始化
    RknnRet RknnDeinit() override;
    //引擎推理
    RknnRet Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID) override;
    RknnRet Inferenct(ImageSpec &bgr_data,detect_result_group_t *detect_result_group,char* taskID) override;
};

#endif



