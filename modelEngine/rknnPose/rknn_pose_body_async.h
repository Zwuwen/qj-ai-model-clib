#ifndef  __RKNN_POSE_BODY_ASYNC_H__
#define  __RKNN_POSE_BODY_ASYNC_H__

#include "../ai_engine_api.h"
#include "rockx.h"

#define PI 3.14159265 

typedef struct{
	int Ang1;
	int Ang2;
	int Ang3;
	int Ang4;
	int Ang5;
	int Ang6;
	int Ang7;
	int Ang8;
	int Ang9;
	int Ang10;
}pose_name;

typedef struct{
	int indexo;
	int indexs;
	int indexe;
}pose_index;

typedef struct{
	int x1;
	int y1;
	
	int x2;
	int y2;
}Coordinate_t;

// 骨骼连线
const std::vector<std::pair<int,int>> posePairs = {
        {1,2}, {1,5}, {2,3}, {3,4}, {5,6}, {6,7},
        {1,8}, {8,9}, {9,10}, {1,11}, {11,12}, {12,13},
        {1,0}, {0,14}, {14,16}, {0,15}, {15,17}
};

#define maxAngle 10

typedef struct
{
	//1)[poseInfo]-----------------------------------------
	unsigned int poseNum; // 有效关节点数
	int poseList[18];     // 有效关节点 序号 列表，最多关节点个数 18

	//2)[angelInfo]-------------------------------------
	unsigned int angleNum=0;   //关节点夹角个数
	pose_index PoseIdex[maxAngle]; // 存储构成关节点 夹角  三个关节点 序号

	//3)[classInfo]----------------------------------------
	unsigned int classNum=0;          //识别类型数量
	pose_name norm_angle[maxAngle];       //判断每个 类型的标准，即 angleNum 个夹角的 标准 角度

	//4)[detectInfo]----------------------------------------
	int angle_diff=0;          //角度偏差最大允许值
}PoseModelData;

typedef struct
{
	// model labels info
	char *labels[91];
    //
    rockx_handle_t poseBodyHandle;

	//PoseModelData
	PoseModelData poseModelData;

}RknnPoseEng;


class cpose_engine:public aiEngine_api
{
public:
    cpose_engine();
    ~cpose_engine() override;
protected:
    //pose
	RknnPoseEng m_rknnPoseEng;
	void postProcess(cv::Mat &orig_img,void *result, size_t result_size,int leftTopX, int leftTopY, int cutImageWH,detect_result_group_t *detect_result_group) ;
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


