#ifndef _YOLO_H_
#define _YOLO_H_

#include "../ai_engine_api.h"

typedef struct{
	float x,y,w,h;
}box;

typedef struct detection{
    box bbox;
    int classes;
    float *prob;          //
    float objectness;     //
    int sort_class;       //class index
} detection;


typedef struct{
	//1)[GRIDInfo]-----------------------------------------
    int nyolo=3; //n yolo layers;
    int GRID0=52;
    int GRID1=26;
    int GRID2=13;

    //2)[anchorInfo]-------------------------------------
    int nanchor=3;             //n anchor per yolo layer
    int masks_0[3] = {0, 1, 2};
    int masks_1[3] = {3, 4, 5};
    int masks_2[3] = {6, 7, 8};
    float anchorsList[18] = {4,7, 7,15, 13,25,   25,42, 41,67, 75,94,   91,162, 158,205, 250,332};

    int nboxes_0=GRID0*GRID0*nanchor;
    int nboxes_1=GRID1*GRID1*nanchor;
    int nboxes_2=GRID2*GRID2*nanchor;
    int nboxes_total=nboxes_0+nboxes_1+nboxes_2;

    //3)[detectInfo]----------------------------------------
    float OBJ_THRESH=0.6;          // obj thresh
    float DRAW_CLASS_THRESH=0.6;   //
    float NMS_THRESH=0.45;          //darknet demo nms=0.4
}YoloModelData;

typedef struct
{
	// model labels info
	char *labels[91];
    //
    YoloModelData modelData;
	// model engine-------------------------------------------
    rknn_context ctx;
}RknnYoloEng;

class cyolo_engine:public aiEngine_api
{
public:
    cyolo_engine();
    virtual ~cyolo_engine();
protected:
    RknnYoloEng m_rknnYoloEng;
public:
    //引擎初始化
    RknnRet RknnInit(RknnDatas *pRknn);
    //引擎反初始化
    RknnRet RknnDeinit();
    //引擎推理
    RknnRet Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID);

};
#endif