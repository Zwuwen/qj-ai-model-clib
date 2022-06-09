#include "gdd.h"

// gdd 模型初始化
RknnRet cgdd_engine::RknnInit(RknnDatas *pRknn) {
	if(pRknn==nullptr) {return RKNN_ERR ;}
	//1、辅助模型数据
//	memcpy(&rknn_data, pRknn,sizeof(RknnDatas));
    m_rknnData = *pRknn;
	// 模型输入尺寸
//	int MODEL_INPUT_SIZE =rknn_data.inputSize;
//	// 模型识别类个数
//	int NUM_CLASS = rknn_data.numClass;

	GddiRet ret;

	//1 模型对应的 config 文件，共达地为 ./alg_config.json  文件-------------------------------------------------------------------
//	printf("Loading CONF_FILE_PATH %s ...\n",rknn_data.priboxPath.c_str());
//	printf("Loading MODLE_PATH %s ...\n",rknn_data.modelPath.c_str());

	//2 共达地模型初始化识别引擎
//	m_alg_impl = new AlgImpl();
//    cout<<"m_alg_impl_new:"<<m_alg_impl<<endl;
    //初始化实例
	ret = m_alg_impl.Init(0, std::string(m_rknnData.priboxPath),std::string(m_rknnData.modelPath),1);
	if(ret != GDDI_SUCCESS)
	{
	     printf("alg impl Init errno:  ret=%d\n", ret);
	     return RKNN_ERR;
	}

	// 输出
	printf("alg impl Init success:  ret=%d\n", ret);
	return RKNN_SUCCESS;
}

// 反初始化======================================================================================================================
RknnRet cgdd_engine::RknnDeinit()
{
	printf("RknnDeinit gdd ------------------------------------\n");
//	if(m_alg_impl!=NULL)
//	{
//        cout<<"m_alg_impl_delete:"<<m_alg_impl<<endl;
//		delete m_alg_impl;
//	}
	return RKNN_SUCCESS;
}


// 模型推理======================================================================================================================
RknnRet cgdd_engine::Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID)
{
	try
	{
		if(srcimg.empty() || inputImg.empty() || detect_result_group == nullptr ) {
			printf("param error\n");
			return RKNN_ERR;
        }
		cv::imwrite("testgdd.jpg",srcimg);
		GddiRet ret;
		// srcimg 为原始图片
		// 1）根据每一帧创建  TRANSFER_BUFFER实例，并进行图片前处理
		auto* tp=new TransferBuffer(std::string("0"),srcimg.rows,srcimg.cols,srcimg.data,0);
		
		std::shared_ptr<TransferBuffer> transfer_buffer(tp);
		// 图片 图片前处理
		ret = m_alg_impl.PreProcess(transfer_buffer);
		if(ret != GDDI_SUCCESS)
		{
			printf(" PreProcess errno: %d",ret);
			return RKNN_ERR;
		}

		// 图片推理
//		printf(" PreProcess srcimg.rows: %d   srcimg.cols:%d  \n",srcimg.rows,srcimg.cols);
		ret =  m_alg_impl.Process(transfer_buffer);
		if(ret != GDDI_SUCCESS)
		{
			printf(" PreProcess errno: %d",ret);
			return RKNN_ERR;
		}

		// 图片后处理
//		printf(" PreProcess transfer_buffer\n");
		AlgOutput alg_res;
		ret = m_alg_impl.PostProcess(transfer_buffer, alg_res);
		if(ret != GDDI_SUCCESS)
		{
			printf(" PreProcess errno: %d \n",ret);
			return RKNN_ERR;
		}

		// 结果分析
//		printf(" PreProcess result\n");
		if(alg_res.task_ == CLASSIFY_TASK)
		{// 分类模型
//			auto* classify_res =  (ClassifyRes*)alg_res.data_.get();
//			printf("classid: %d   name:%s   prob:%f  \n",classify_res->class_id_  ,classify_res->name_vector_[classify_res->class_id_],classify_res->prob_ );
		}else if(alg_res.task_ == DETECT_TASK)
		{// 目标检测模型
			std::vector<DetectRes> detect_res = *(std::vector<DetectRes>*)alg_res.data_.get();
			// 结果存储
			int last_count = 0;
			detect_result_group->count = 0;

			///* box valid detect target */
			float score_threshold = m_rknnData.threshold;
//			printf("score_threshold:%f   objsize:%d \n",score_threshold  ,detect_res.size());

			for(auto it = detect_res.begin(); it < detect_res.end(); it++ )
			{
				if(it->class_id_ != -1)
				{
					if (it->prob_ < score_threshold )
					{
						printf("prob_:%f    score_threshold:%f\r\n",it->prob_ , score_threshold);
						continue;
					}

					if(it->bbox_[0] <0)  it->bbox_[0] =0;
					detect_result_group->results[last_count].box.left   = int(it->bbox_[0]);
				
					if(it->bbox_[1] <0)  it->bbox_[1] =0;
					detect_result_group->results[last_count].box.top	 = int(it->bbox_[1]);

					detect_result_group->results[last_count].box.right  = int(it->bbox_[0]) + int(it->bbox_[2]);
					detect_result_group->results[last_count].box.bottom = int(it->bbox_[1]) + int(it->bbox_[3]);
					detect_result_group->results[last_count].prop = it->prob_;
					
					strcpy(detect_result_group->results[last_count].name, it->name_vector_[it->class_id_].c_str());
//					printf("######## %d %d %d %d %f %s  \n",detect_result_group->results[last_count].box.left,detect_result_group->results[last_count].box.top,
//							detect_result_group->results[last_count].box.right,detect_result_group->results[last_count].box.bottom,
//							detect_result_group->results[last_count].prop = it->prob_ , detect_result_group->results[last_count].name);

					last_count++;
				}else
				{
					printf("######## not object  \n");
				}
			}
			detect_result_group->count = last_count;
			printf("########object count:%d \n",detect_result_group->count);
		}
		else
		{
			std::cerr << "not support task!"<<std::endl;
		}
		//释放资源
	}catch(const std::exception& e){
		std::cout<<"an exception!!!:"<<e.what()<<std::endl;
	}
	
	return RKNN_SUCCESS;
}

RknnRet cgdd_engine::Inferenct(ImageSpec &bgr_data, detect_result_group_t *detect_result_group, char *taskID) {
    try
    {
        GddiRet ret;
        auto* tp=new TransferBuffer(std::string("0"),bgr_data.height,bgr_data.width,bgr_data.data,0);
        std::shared_ptr<TransferBuffer> transfer_buffer(tp);

        // 图片 图片前处理
        ret = m_alg_impl.PreProcess(transfer_buffer);
        if(ret != GDDI_SUCCESS)
        {
            printf(" PreProcess errno: %d",ret);
            return RKNN_ERR;
        }

        // 图片推理
//		printf(" PreProcess srcimg.rows: %d   srcimg.cols:%d  \n",srcimg.rows,srcimg.cols);
        ret =  m_alg_impl.Process(transfer_buffer);
        if(ret != GDDI_SUCCESS)
        {
            printf(" PreProcess errno: %d",ret);
            return RKNN_ERR;
        }

        // 图片后处理
//		printf(" PreProcess transfer_buffer\n");
        AlgOutput alg_res;
        ret = m_alg_impl.PostProcess(transfer_buffer, alg_res);
        if(ret != GDDI_SUCCESS)
        {
            printf(" PreProcess errno: %d \n",ret);
            return RKNN_ERR;
        }

        // 结果分析
//		printf(" PreProcess result\n");
        if(alg_res.task_ == CLASSIFY_TASK)
        {// 分类模型
//			auto* classify_res =  (ClassifyRes*)alg_res.data_.get();
//			printf("classid: %d   name:%s   prob:%f  \n",classify_res->class_id_  ,classify_res->name_vector_[classify_res->class_id_],classify_res->prob_ );
        }else if(alg_res.task_ == DETECT_TASK)
        {// 目标检测模型
            std::vector<DetectRes> detect_res = *(std::vector<DetectRes>*)alg_res.data_.get();
            // 结果存储
            int last_count = 0;
            detect_result_group->count = 0;

            ///* box valid detect target */
            float score_threshold = m_rknnData.threshold;
//			printf("score_threshold:%f   objsize:%d \n",score_threshold  ,detect_res.size());

            for(auto it = detect_res.begin(); it < detect_res.end(); it++ )
            {
                if(it->class_id_ != -1)
                {
                    if (it->prob_ < score_threshold )
                    {
                        printf("prob_:%f    score_threshold:%f\r\n",it->prob_ , score_threshold);
                        continue;
                    }

                    if(it->bbox_[0] <0)  it->bbox_[0] =0;
                    detect_result_group->results[last_count].box.left   = int(it->bbox_[0]);

                    if(it->bbox_[1] <0)  it->bbox_[1] =0;
                    detect_result_group->results[last_count].box.top	 = int(it->bbox_[1]);

                    detect_result_group->results[last_count].box.right  = int(it->bbox_[0]) + int(it->bbox_[2]);
                    detect_result_group->results[last_count].box.bottom = int(it->bbox_[1]) + int(it->bbox_[3]);
                    detect_result_group->results[last_count].prop = it->prob_;

                    strcpy(detect_result_group->results[last_count].name, it->name_vector_[it->class_id_].c_str());
//					printf("######## %d %d %d %d %f %s  \n",detect_result_group->results[last_count].box.left,detect_result_group->results[last_count].box.top,
//							detect_result_group->results[last_count].box.right,detect_result_group->results[last_count].box.bottom,
//							detect_result_group->results[last_count].prop = it->prob_ , detect_result_group->results[last_count].name);

                    last_count++;
                }else
                {
                    printf("######## not object  \n");
                }
            }
            detect_result_group->count = last_count;
            printf("########object count:%d \n",detect_result_group->count);
        }
        else
        {
            std::cerr << "not support task!"<<std::endl;
        }
        //释放资源
    }catch(const std::exception& e){
        std::cout<<"an exception!!!:"<<e.what()<<std::endl;
    }
    return RKNN_SUCCESS;
}
