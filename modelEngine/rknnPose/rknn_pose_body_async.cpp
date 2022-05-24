#include "rknn_pose_body_async.h"

// 模型推理相关辅助函数--------------------------------------------------------------------------------------------------------------------
double Angle(rockx_point_t o, rockx_point_t s, rockx_point_t e)
{
	double dsx = s.x - o.x;
	double dsy = s.y - o.y;
	double dex = e.x - o.x;
	double dey = e.y - o.y;
	
	double angle1 = atan2(dsy, dsx);
	angle1 = int(angle1 * 180 / PI);
	double angle2 = atan2(dey, dex);
	angle2 = int(angle2 * 180 / PI);

	if (angle1 < 0) angle1 += 360;
	if (angle2 < 0) angle2 += 360;

	double included_angle = 0;
	if (angle1 > angle2)
		included_angle = 360 - (abs(angle2 - angle1));
	else
		included_angle = abs(angle2 - angle1);

	return included_angle;
}

//first的值升序排序
bool key_sore(pair<int,int>a,pair<int,int>b){
    return a.first < b.first;
}
 
//second的值升序排序
bool value_sore(pair<int, int>a, pair<int, int>b){
    return a.second < b.second;
}

float average(float *x, int len)
{
    float sum = 0;
    for (int i = 0; i < len; i++) 
        sum += x[i];
    return sum/len; 
}

int get_pose_number(PoseModelData poseModelData, rockx_keypoints_t keypoints,Coordinate_t &in){
	//1、get pose type:return type index
	int beOk=1;
	for(int index =0 ;index<poseModelData.poseNum; index++)
	{
		int pointIndex = poseModelData.poseList[index];
		if(keypoints.score[pointIndex]<=0 && pointIndex!=0)
		{
			beOk =-1;
		}
		////dzlog_debug("===> PoseIdex[%d]: %d(%f) (%d, %d)", index, pointIndex, keypoints.score[pointIndex],
		//		keypoints.points[pointIndex].x,keypoints.points[pointIndex].y);
	}
	if(beOk<0)
	{// every point's score must >0
		////dzlog_debug("===> PoseIndex get fail \n");
		return -1;
	}
	//when the neck(1) point's score <0, use the nose(0) point's score
	if(keypoints.score[1]>0)
	{
		keypoints.score[0] = keypoints.score[1];
		keypoints.points[0].x = keypoints.points[1].x ;
		keypoints.points[0].y = keypoints.points[1].y+10 ;
	}

	//2、get the angle of the points
	pose_name poseAgl = {0};
	// 5
	int pose_arr[maxAngle] = {0};
	// get 5 angles from 8 points
	for(int index =0 ;index < poseModelData.angleNum ;index++)
	{
		if( (0 < keypoints.score[poseModelData.PoseIdex[index].indexo]) && 
		 	(0 < keypoints.score[poseModelData.PoseIdex[index].indexs]) && 
		 	(0 < keypoints.score[poseModelData.PoseIdex[index].indexe]) )
		{//get the poseAgl :now get 5 angles from 8 points
			// get angle
			pose_arr[index] = Angle(keypoints.points[poseModelData.PoseIdex[index].indexo],
				keypoints.points[poseModelData.PoseIdex[index].indexs],keypoints.points[poseModelData.PoseIdex[index].indexe]);
			////dzlog_debug("===> PoseIdex[%d](angle:%d): %d(%f),%d(%f),%d(%f)", index, pose_arr[index],PoseIdex[index].indexo, keypoints.score[PoseIdex[index].indexo],
			//		PoseIdex[index].indexs, keypoints.score[PoseIdex[index].indexs], PoseIdex[index].indexe, keypoints.score[PoseIdex[index].indexe]);
		}else
		{
			////dzlog_debug("!!!@@@@ index:%d, %f,%f,%f", index,
			//keypoints.score[PoseIdex[index].indexo],
			//keypoints.score[PoseIdex[index].indexs],
			//keypoints.score[PoseIdex[index].indexe]);
			continue;
		}
	}
	// 获取5个夹角
	int index_buf[poseModelData.classNum] = {0};
	int maxPosition = -1;
	int maxPositionIndex = -1;
	memcpy(&poseAgl, pose_arr, sizeof(pose_name));
	// every class have 5 angles ,we will get the difference of the absolute value
	for(int index =0;index < poseModelData.classNum; index++)
	{// classNum(type):
		////dzlog_debug("!!!@@@@ classNum  index:%d",  index);
		for(int i =0;i < poseModelData.angleNum; i++)
		{// the difference of the absolute value
			int* pNorm_int = (int*)&(poseModelData.norm_angle[index]);
			int* pAgl_int = (int*)&poseAgl;
			//  abs  < angle_diff, angle is ok
			////dzlog_debug("!!!@@@@pNorm_int[i]:%d    pAgl_int[i]:%d  angle_diff:%d", pNorm_int[i], pAgl_int[i],angle_diff);
			if(abs(pNorm_int[i]- pAgl_int[i])<=poseModelData.angle_diff)
			{
				index_buf[index]++;
				pNorm_int++;
				pAgl_int++;
				////dzlog_debug("!!!!11111111111111111");
			}
		}
		////dzlog_debug("!!!@@@@index_buf[%d]: %d", index, index_buf[index]);
		if(index_buf[index]>maxPosition)
		{
			maxPosition = index_buf[index];
			maxPositionIndex = index;
		}
	}

	//最符合  动作的 位置 序号
	////dzlog_debug("now_angleNumOK:%d ", maxPosition);
	////dzlog_debug("maxPositionIndex:%d,",maxPositionIndex);

	if(poseModelData.angleNum == maxPosition)
	{//5个夹角都满足条件，返回 动作类型
		////dzlog_info("maxPositionIndex:%d,now_angleNumOK:%d, angleNumOK:%d ",maxPositionIndex,index_buf[maxPositionIndex],angleNum);
		return maxPositionIndex;
	}else{
		////dzlog_info("maxPositionIndex:%d,now_angleNumOK:%d, angleNumOK:%d ",maxPositionIndex,index_buf[maxPositionIndex],angleNum);
		return -1;
	}
}

int Mat2RockxImag(cv::Mat &frame,rockx_image_t &input_image){
	input_image.pixel_format = ROCKX_PIXEL_FORMAT_BGR888;
	input_image.width = frame.cols;
	input_image.height = frame.rows;
	input_image.data = frame.data;
}


//模型初始化相关辅助函数-----------------------------------------------------------------------------------------------------------
int GetConfigsInfo(PoseModelData* poseModelData, const char *CONF_FILE_PATH)
{
    std::cout <<"Config---------------------------------------------------" << "\n";
    std::cout <<"CONF_FILE_PATH:"<< CONF_FILE_PATH << "\n";
    //1)poseInfo
   poseModelData->poseNum=GetIniKeyInt("poseInfo","poseNum",CONF_FILE_PATH);

    char poseListC[256];
    vector<int> resultList;
    strcpy(poseListC,GetIniKeyString("poseInfo","poseList",CONF_FILE_PATH));
    resultList=GetStrInfo(poseListC);
    if(resultList.size() !=  poseModelData->poseNum)
    	return -1;

    for(int index=0;index< poseModelData->poseNum;index++)
    {
    	poseModelData->poseList[index]=resultList[index];
    	std::cout <<"poseList["<<index<<"]:"<< poseModelData->poseList[index] << "\n";
    }

    //2)angelInfo
     poseModelData->angleNum=GetIniKeyInt("angelInfo","angleNum",CONF_FILE_PATH);

    strcpy(poseListC,GetIniKeyString("angelInfo","poseIndex_List",CONF_FILE_PATH));
    resultList=GetStrInfo(poseListC);
    if(resultList.size() !=  poseModelData->angleNum*3)
        return -1;

    for(int index=0;index< poseModelData->angleNum;index++)
    {
    	poseModelData->PoseIdex[index].indexo = resultList[index*3+0];
    	poseModelData->PoseIdex[index].indexs = resultList[index*3+1];
    	poseModelData->PoseIdex[index].indexe = resultList[index*3+2];
    	std::cout <<"PoseIdex["<<index<<"]:" << poseModelData->PoseIdex[index].indexo <<","<<poseModelData->PoseIdex[index].indexs<<","<<poseModelData->PoseIdex[index].indexe<< "\n";
    }

    //3)classInfo
    poseModelData->classNum=GetIniKeyInt("classInfo","classNum",CONF_FILE_PATH);
    std::cout <<"classNum:" <<  poseModelData->classNum << "\n";

    strcpy(poseListC,GetIniKeyString("classInfo","class_List",CONF_FILE_PATH));
    resultList=GetStrInfo(poseListC);
    if(resultList.size() != poseModelData->classNum* poseModelData->angleNum)
        return -1;

    for(int index=0;index<poseModelData->classNum;index++)
    {
    	std::cout <<"class_List["<< index<<"]:";
    	for(int index2=0;index2< poseModelData->angleNum;index2++)
    	{
    		int temp =  resultList[index* poseModelData->angleNum+index2];
    		switch(index2)
    		{
    		case 0: poseModelData->norm_angle[index].Ang1 = temp;break;
    		case 1: poseModelData->norm_angle[index].Ang2 = temp;break;
    		case 2: poseModelData->norm_angle[index].Ang3 = temp;break;
    		case 3: poseModelData->norm_angle[index].Ang4 = temp;break;
    		case 4: poseModelData->norm_angle[index].Ang5 = temp;break;
    		case 5: poseModelData->norm_angle[index].Ang6 = temp;break;
    		case 6: poseModelData->norm_angle[index].Ang7 = temp;break;
    		case 7: poseModelData->norm_angle[index].Ang8 = temp;break;
    		case 8: poseModelData->norm_angle[index].Ang9 = temp;break;
    		case 9: poseModelData->norm_angle[index].Ang10 = temp;break;
    		}
    		std::cout <<temp << " ";
    	}
    	std::cout <<"\n";
    }


    //4)detectInfo
    poseModelData->angle_diff=GetIniKeyInt("detectInfo","angle_diff",CONF_FILE_PATH);
    std::cout <<"angle_diff:"<< poseModelData->angle_diff << "\n";

    std::cout << "\n"<<"---------------------------------------------------------" << "\n";
    return 0;
}

//==================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------
cpose_engine::cpose_engine()
{
	
}

cpose_engine::~cpose_engine()
{
	RknnDeinit();
}

// 内部姿态检测模型初始化
RknnRet cpose_engine::RknnInit(RknnDatas *pRknn)
{
	if(pRknn==NULL) {return RKNN_ERR ;}
	//1、辅助模型数据
	m_rknnData = *pRknn;

	int MODEL_INPUT_SIZE =m_rknnData.inputSize;
	int NUM_CLASS = m_rknnData.numClass;

	if( (0 == MODEL_INPUT_SIZE) ||(0 == NUM_CLASS) )
	{
		printf("MODEL_INPUT_SIZE:%d\r\n",MODEL_INPUT_SIZE);
		printf("NUM_CLASS:%d\r\n",NUM_CLASS);
		printf("RknnInit json get error.\n");
		return RKNN_ERR;
	}

	//1、获取模型配置文件信息-------------------------------------------------------------------Config.ini锛氬叧鑺傜偣鍒ゆ柇鐨勪竴浜涗俊鎭�
	printf("Loading CONF_FILE_PATH %s ...\n",m_rknnData.priboxPath.c_str());
	int ret=GetConfigsInfo(&(m_rknnPoseEng.poseModelData),m_rknnData.priboxPath.c_str());
	if(ret<0)
	{
		return RKNN_ERR;
	}

    // create a pose_body handle
    ret = rockx_create(&m_rknnPoseEng.poseBodyHandle, ROCKX_MODULE_POSE_BODY, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS)
    {
        printf("init rockx module ROCKX_MODULE_POSE_BODY error %d\n", ret);
        return RKNN_ERR;
    }

    // 加载labes
    printf("loadLabelName %s\r\n",m_rknnData.labelPath.c_str());
	readLines(m_rknnData.labelPath.c_str(),m_rknnPoseEng.labels, m_rknnData.numClass,false);
    if(ret < 0) {
        printf("loadLabelName fail! ret=%d\n", ret);
        return RKNN_ERR;
    }

    return RKNN_SUCCESS;
}
// 模型初始化结束======================================================================================================================

// 模型反初始化======================================================================================================================
RknnRet cpose_engine::RknnDeinit()
{
    printf("RknnDeinit pose ------------------------------------\n");
	rockx_ret_t ret = ROCKX_RET_FAIL;
	if(NULL != m_rknnPoseEng.poseBodyHandle )
	{
		ret = rockx_destroy(m_rknnPoseEng.poseBodyHandle);
		m_rknnPoseEng.poseBodyHandle =NULL;
	}else{
		ret = ROCKX_RET_SUCCESS;
	}

	if(ret== ROCKX_RET_SUCCESS)
	{
		return RKNN_SUCCESS;
	}else{
		return RKNN_ERR;
	}

	return  RKNN_ERR;
}

// 模型推理======================================================================================================================
RknnRet cpose_engine::Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID)
{
	if(srcimg.empty() || inputImg.empty()|| detect_result_group== NULL) 
	{
		return RKNN_ERR;
	}
	
	printf("===============Start rknnx_pose_Process \n");
	printf("srcimg.cols:%d,srcimg.rows:%d\r\n",srcimg.cols,srcimg.rows);
	// 原始图片截取中间区域进行人体检测
	int imageW = srcimg.cols;  //宽
	int imageH = srcimg.rows;  //高
	int  leftTopX =0,leftTopY=0,cutImageWH=0;
	if(imageW>imageH)
	{
		cutImageWH =imageH;
		leftTopX = (imageW-imageH)*0.5;
		leftTopY = 0;
	}else
	{
		cutImageWH =imageW;
		leftTopX = 0;
		leftTopY = (imageH-imageW)*0.5;
	}
	cv::Rect2f curBox(leftTopX, leftTopY, cutImageWH, cutImageWH);
	cv::Mat crop_image = srcimg(curBox);
	//cv::imwrite("crop_image1.jpg", crop_image);
	//cv::Mat testImag(crop_image1);// = crop_image1.clone();
	cv::Mat testImag = crop_image.clone();
	//crop_image1.copyTo(testImag);
	printf("crop_image1.cols:%d,crop_image1.rows:%d\r\n",testImag.cols,testImag.rows);

	rockx_ret_t ret;
	//1、将图片转换为 检测数据
	rockx_image_t input_image ={0};
	Mat2RockxImag(testImag,input_image);
    // create rockx_face_array_t for store result
	//2、存储关节点检测结果
    rockx_keypoints_array_t body_array;
    memset(&body_array, 0, sizeof(rockx_keypoints_array_t));

	//3、检测
	// 关节点检测
	ret = rockx_pose_body(m_rknnPoseEng.poseBodyHandle, &input_image, &body_array, nullptr/*callback*/);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_pose_body error %d\n", ret);
        return RKNN_ERR;
    }

    //4、结果分析与发送警告
    postProcess(srcimg,(void *)&body_array, body_array.count,leftTopX, leftTopY, cutImageWH,detect_result_group);
	printf("########detect_result_group count:%d \n",detect_result_group->count);

	return RKNN_SUCCESS;
}


void cpose_engine::postProcess(cv::Mat &orig_img,void *result, size_t result_size,int leftTopX, int leftTopY, int cutImageWH,detect_result_group_t *detect_result_group) 
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    //printf("%ld on result callback\n", (tv.tv_sec * 1000000 + tv.tv_usec));

    rockx_keypoints_array_t *body_array = (rockx_keypoints_array_t*)result;

	int last_count = 0;
	int pose_id = 0;
	static int last_pose_id = 0;

	//存储目标关节点信息
	rockx_keypoints_t detect_result_group_keypoints[5];
    // process result  // 检测到人的个数  body_array->count
    for (int i = 0; i < body_array->count; i++)
    {
        printf("\r\nperson %d", i);
        Coordinate_t in = {0};
       // 获取当前人的动作姿势
		pose_id = get_pose_number(m_rknnPoseEng.poseModelData,body_array->keypoints[i],in);
		// pose_id  当前人的动作类型
		if(0 <= pose_id)
		{
			printf("\r\n!!!!!pose_number:%d\r\n", pose_id);

			char *label = m_rknnPoseEng.labels[pose_id];
			detect_result_group->results[last_count].box.left  =  leftTopX;   //in.x1;
			detect_result_group->results[last_count].box.top   =  leftTopY; //in.y1;
			detect_result_group->results[last_count].box.right  = leftTopX+cutImageWH;  //in.x2;
			detect_result_group->results[last_count].box.bottom = leftTopY+cutImageWH;   //in.y2;
			//阈值：所有 关键点可信度的平均值
			detect_result_group->results[last_count].prop = average(body_array->keypoints[i].score,body_array->keypoints[i].count);
			strcpy(detect_result_group->results[last_count].name,label);
			// 关键点存储
			memcpy(&detect_result_group_keypoints[last_count], &(body_array->keypoints[i]),sizeof(rockx_keypoints_t));

			printf("pose result: (%4d, %4d, %4d, %4d), %4.2f, %s\n", leftTopX, leftTopY, leftTopX+cutImageWH, leftTopY+cutImageWH, detect_result_group->results[last_count].prop, label);
			last_count++;
		}
		// detect_result_group 识别到的目标人数
		detect_result_group->count = last_count;
    }

	static int pose_multi_frame = 0;
	if( (0 < detect_result_group->count)  &&(0 <= pose_id)  )
	{
		//1:draw detection area
		rectangle(orig_img, Point(leftTopX, leftTopY), Point(leftTopX+cutImageWH, leftTopY+cutImageWH), Scalar(0, 0, 255, 255), 6);
		//2:draw pic
		for(int i=0;i<last_count;i++)
		{
			//2:draw point
			int pointsCount = detect_result_group_keypoints[i].count;
			for(int j=0 ; j<pointsCount;j++)
			{
				int x = detect_result_group_keypoints[i].points[j].x+leftTopX;
				int y = detect_result_group_keypoints[i].points[j].y+leftTopY;
				//
				circle(orig_img, Point(x, y), 3, Scalar(255, 0, 0), -1);
			}
			//3:draw line
			for(int j = 0; j < posePairs.size(); j ++)
			{
			     const std::pair<int,int>& posePair = posePairs[j];
			     int x0 = detect_result_group_keypoints[i].points[posePair.first].x+leftTopX;
			     int y0 = detect_result_group_keypoints[i].points[posePair.first].y+leftTopY;
			     int x1 = detect_result_group_keypoints[i].points[posePair.second].x+leftTopX;
			     int y1 = detect_result_group_keypoints[i].points[posePair.second].y+leftTopY;

			     if( x0 > 0 && y0 > 0 && x1 > 0 && y1 > 0)
			     {
			         line(orig_img, {x0, y0}, {x1, y1}, {0, 255, 0}, 1);
			     }
			}
		}

		//2绘制人体关节点信息
		//for (int i = 0; i < detect_out.count; i++)
		//{
		//	////dzlog_info("count:%d, tracker_id:%d", detect_out.count, resultBoxs[i].tracker_id);
		//	detect_result_t *det_result = &(detect_out.results[i]);
	
		//	int x1 = det_result->box.left;
		//	int y1 = det_result->box.top;
		//	int x2 = det_result->box.right;
		//	int y2 = det_result->box.bottom;
	    //
		//	rectangle(orig_img, Point(x1, y1), Point(x2, y2), Scalar(0, 0, 255, 255), 6);
		//	putText(orig_img, to_string(det_result->prop)/*det_result->name*/, Point(x1, y1 - 12), 1, 2, Scalar(0, 255, 0, 255));
		//}
		//
	
		printf ("	$info detecount:%d\r\n",detect_result_group->count);
	
	}
}

RknnRet cpose_engine::Inferenct(ImageSpec &bgr_data,detect_result_group_t *detect_result_group,char* taskID){

	return RKNN_SUCCESS;
}