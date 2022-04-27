#include "yolo.h"

int GetMasksInfo(YoloModelData* modelData,int nanchor,const char *CONF_FILE_PATH)
{
    char masksStr[64];
    vector<int> resultList;

	strcpy(masksStr,GetIniKeyString("anchorInfo","masks_0",CONF_FILE_PATH));
	resultList=GetStrInfo(masksStr);
	if(resultList.size() != nanchor)
		return -1;
	for(int index=0;index<resultList.size();index++)
	   modelData->masks_0[index]=resultList[index];
	std::cout <<"masks_0:"<<  modelData->masks_0[0] <<","<<  modelData->masks_0[1]<<","<<  modelData->masks_0[2]<< "\n";

	strcpy(masksStr,GetIniKeyString("anchorInfo","masks_1",CONF_FILE_PATH));
	resultList=GetStrInfo(masksStr);
	if(resultList.size() != nanchor)
		return -1;
	for(int index=0;index<resultList.size();index++)
	     modelData->masks_1[index]=resultList[index];
	std::cout <<"masks_1:"<<  modelData->masks_1[0] <<","<<  modelData->masks_1[1]<<","<<  modelData->masks_1[2]<< "\n";

    if( modelData->nyolo==3)
    {
        strcpy(masksStr,GetIniKeyString("anchorInfo","masks_2",CONF_FILE_PATH));
	    resultList=GetStrInfo(masksStr);
		if(resultList.size() != nanchor)
			return -1;
	    for(int index=0;index<resultList.size();index++)
	         modelData->masks_2[index]=resultList[index];
	    std::cout <<"masks_2:"<<  modelData->masks_2[0] <<","<<  modelData->masks_2[1]<<","<<  modelData->masks_2[2]<< "\n";
    }

	 return 0;
}

int GetAnchorsList(YoloModelData* modelData,const char *CONF_FILE_PATH)
{
    char anchorsListStr[256];
	strcpy(anchorsListStr,GetIniKeyString("anchorInfo","anchorsList",CONF_FILE_PATH));

    std::cout <<"anchorsList:";
	vector<int> resultList;
	resultList=GetStrInfo(anchorsListStr);
	for(int index=0;index<resultList.size();index++)
	{
	     modelData->anchorsList[index]=resultList[index];
	    std::cout <<  modelData->anchorsList[index]<< " ";
	 }
    std::cout <<"\n";
    return  resultList.size();
}

int GetConfigs(YoloModelData* modelData , const char *CONF_FILE_PATH)
{
    std::cout <<"Config---------------------------------------------------" << "\n";
    std::cout <<"CONF_FILE_PATH:"<< CONF_FILE_PATH << "\n";
    //1)GRIDInfo
    modelData->nyolo=GetIniKeyInt("GRIDInfo","nyolo",CONF_FILE_PATH);
    modelData->GRID0=GetIniKeyInt("GRIDInfo","GRID0",CONF_FILE_PATH);
    modelData->GRID1=GetIniKeyInt("GRIDInfo","GRID1",CONF_FILE_PATH);
    if( modelData->nyolo==3)
    {
        modelData->GRID2=GetIniKeyInt("GRIDInfo","GRID2",CONF_FILE_PATH);
        std::cout << modelData->nyolo << " (" << modelData->GRID0 << ", " << modelData->GRID1 << "," <<  modelData->GRID2<<  ")" << "\n";
    }else{
        std::cout << modelData->nyolo << " (" << modelData->GRID0 << ", " << modelData->GRID1 <<  ")" << "\n";
    }
    if( modelData->nyolo == 0 ||  modelData->GRID0==0 || modelData->GRID1==0 )
    {
    	//printf("rknn nyolo GRID0 GRID1 GRID2 fail!\n");
    	return -1;
    }

    //2)anchorInfo
    modelData->nanchor=GetIniKeyInt("anchorInfo","nanchor",CONF_FILE_PATH);
    std::cout << "nanchor:" <<  modelData->nanchor << "\n";
    int ret=GetMasksInfo(modelData, modelData->nanchor,CONF_FILE_PATH);
    if(ret<0)
    {
    	//printf("rknn nanchor fail!\n");
    	return -1;
    }

    int anchorsListcount=GetAnchorsList(modelData,CONF_FILE_PATH);
    if(anchorsListcount!=  (modelData->nyolo)*(modelData->nanchor)*2)
    {
    	//printf("rknn anchorsListcount fail!\n");
    	return -1;
    }

    // anchor 
    modelData->nboxes_0=(modelData->GRID0)*(modelData->GRID0)*(modelData->nanchor);
    modelData->nboxes_1=(modelData->GRID1)*(modelData->GRID1)*(modelData->nanchor);
    if(modelData->nyolo==3)
    {
    	modelData->nboxes_2=(modelData->GRID2)*(modelData->GRID2)*(modelData->nanchor);
        modelData->nboxes_total=modelData->nboxes_0+modelData->nboxes_1+modelData->nboxes_2;
    }else{
        modelData->nboxes_2=0;
       modelData->nboxes_total=modelData->nboxes_0+modelData->nboxes_1;
    }

    std::cout << "\n"<<"---------------------------------------------------------" << "\n";
    return 0;
}
//==================================================================================================================

//------------------------------------------------------------------------------
void free_detections(detection *dets, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        free(dets[i].prob);
    }
    free(dets);
}

box get_yolo_box(float *x, float *biases, int n, int index, int i, int j, int lw, int lh, int netw, int neth, int stride)
{
    box b;
    b.x = (i + x[index + 0*stride]) / lw;
    b.y = (j + x[index + 1*stride]) / lh;
    b.w = exp(x[index + 2*stride]) * biases[2*n]   / netw;
    b.h = exp(x[index + 3*stride]) * biases[2*n+1] / neth;
    return b;
}

float overlap(float x1,float w1,float x2,float w2){
	float l1=x1-w1/2;
	float l2=x2-w2/2;
	float left=l1>l2? l1:l2;
	float r1=x1+w1/2;
	float r2=x2+w2/2;
	float right=r1<r2? r1:r2;
	return right-left;
}

float box_intersection(box a, box b)
{
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);
    if(w < 0 || h < 0) return 0;
    float area = w*h;
    return area;
}

float box_union(box a, box b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;
    return u;
}

float box_iou(box a, box b)
{
    float I = box_intersection(a, b);
    float U = box_union(a, b);
    if (I == 0 || U == 0) {
        return 0;
    }
    return I / U;
}

int nms_comparator(const void *pa, const void *pb)
{
    detection a = *(detection *)pa;
    detection b = *(detection *)pb;
    float diff = 0;
    if(b.sort_class >= 0){
        diff = a.prob[b.sort_class] - b.prob[b.sort_class];
    } else {
        diff = a.objectness - b.objectness;
    }
    if(diff < 0) return 1;
    else if(diff > 0) return -1;
    return 0;
}

int do_nms_sort(detection *dets, int total, int classes, float thresh)
{
    //1) delete the box which objectness is 0
    int i, j, k;
    k = total-1;
    for(i = 0; i <= k; ++i){
        if(dets[i].objectness == 0){
            detection swap = dets[i];
            dets[i] = dets[k];
            dets[k] = swap;
            --k;
            --i;
        }
    }
    total = k+1;
	//cout<<"total after OBJ_THRESH: "<<total<<"\n";
    //2) delete the box by nms
    for(k = 0; k < classes; ++k){
        for(i = 0; i < total; ++i){
            dets[i].sort_class = k;
        }
        qsort(dets, total, sizeof(detection), nms_comparator);
        for(i = 0; i < total; ++i){
            if(dets[i].prob[k] == 0) continue;
            box a = dets[i].bbox;
            for(j = i+1; j < total; ++j){
                box b = dets[j].bbox;
                //get IOU
                if (box_iou(a, b) > thresh)
                {
                    dets[j].prob[k] = 0;
                }
            }
        }
    }
	return total;
}

void get_network_boxes(float *predictions, int netw,int neth,int GRID,int* masks, float* anchors, int box_off, detection* dets,int nanchor,int nclasses,float OBJ_THRESH)
{//yolo  from : "forward_yolo_layer" function in yolo_layer.c  and  "activate_array" function in activations.c 
	int lw=GRID;
	int lh=GRID;
	//int nboxes=GRID*GRID*nanchor;
	int LISTSIZE=4+1+nclasses;
	//darkent output: box   grid  anchor
	//1 anchor: 7*7*x+7*7*y+7*7*w+7*7*w+7*7*obj+7*7*classes1+7*7*classes2..+7*7*classes80,and 3 anchor for 1 box
	//x 、 y、obj、classes do logisic
	//xy do logistic
	for(int n=0;n<nanchor;n++)
	{
		int index=n*lw*lh*LISTSIZE;
		int index_end=index+2*lw*lh;
		for(int i=index;i<index_end;i++)
			predictions[i]=1./(1.+exp(-predictions[i]));
	}
	//obj、classes do logisic
	for(int n=0;n<nanchor;n++)
	{
		int index=n*lw*lh*LISTSIZE+4*lw*lh;
		int index_end=index+(1+nclasses)*lw*lh;
		for(int i=index;i<index_end;i++)
		{
			predictions[i]=1./(1.+exp(-predictions[i]));
		}
	}
	//dets
	int count=box_off;  //box_off box index
	for(int i=0;i<lw*lh;i++)
	{
		int row=i/lw;
		int col=i%lw;
		for(int n=0;n<nanchor;n++)
		{
			int box_index=n*lw*lh*LISTSIZE+i;            //
			int obj_index=box_index+4*lw*lh;
			float objectness=predictions[obj_index];
			//if(objectness<OBJ_THRESH) continue;
			if (objectness > OBJ_THRESH)
            {
			    dets[count].objectness=objectness;
			    dets[count].classes=nclasses;
			    dets[count].bbox=get_yolo_box(predictions,anchors,masks[n],box_index,col,row,lw,lh,netw,neth,lw*lh);
			    for(int j=0;j<nclasses;j++)
			    {
				    int class_index=box_index+(5+j)*lw*lh;
				    float prob=objectness*predictions[class_index];
				    dets[count].prob[j] = (prob > OBJ_THRESH) ? prob : 0;

				    //printf("classIndex:%d %f(%f) %f %f %f %f \n",j,prob,objectness,dets[count].bbox.x,dets[count].bbox.y,dets[count].bbox.w,dets[count].bbox.h);
			    }
			    ++count;
			    //printf("count:%d ---------------\n",count);
			}
		}
	}
}

void outputs_transform(YoloModelData* modelData,rknn_output rknn_outputs[], int net_width, int net_height, detection* dets,int nclasses)
{
    for(int index=0;index<modelData->nyolo;index++)
    {
        float* output_temp=(float*)rknn_outputs[index].buf;
        if(index==0)
        {//
	        get_network_boxes(output_temp,net_width,net_height,modelData->GRID0,modelData->masks_0,modelData->anchorsList,0,dets,modelData->nanchor,nclasses,modelData->OBJ_THRESH);
	        //printf("GRID0-----------------\n");
	        continue;
        }
        if(index==1)
        {//
	        get_network_boxes(output_temp,net_width,net_height,modelData->GRID1,modelData->masks_1,modelData->anchorsList,modelData->nboxes_0,dets,modelData->nanchor,nclasses,modelData->OBJ_THRESH);
	        //printf("GRID0-----------------\n");
	        continue;
        }
        if(index==2)
        {//
	        get_network_boxes(output_temp,net_width,net_height,modelData->GRID2,modelData->masks_2,modelData->anchorsList,modelData->nboxes_0+modelData->nboxes_1,dets,modelData->nanchor,nclasses,modelData->OBJ_THRESH);
	        //printf("GRID0-----------------\n");
	        continue;
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------------------------

int postProcessYolo(RknnYoloEng* yoloEng,RknnDatas *pRknn, int width, int heigh, detect_result_group_t *group,rknn_output rknn_outputs[])
{
	if(yoloEng== NULL || pRknn==NULL || group == NULL )
	{
		return -1;
	}

	int MODEL_INPUT_SIZE = pRknn->inputSize;
	int NUM_CLASS = pRknn->numClass;

	float NMS_THRESH = yoloEng->modelData.NMS_THRESH;

	// 4)Process output
	detection* dets = NULL;
	dets =(detection*) calloc(yoloEng->modelData.nboxes_total,sizeof(detection));
	for(int i = 0; i < yoloEng->modelData.nboxes_total; ++i)
	{
		dets[i].prob = (float*) calloc(NUM_CLASS,sizeof(float));
		dets[i].objectness = 0;
	}

	outputs_transform(&(yoloEng->modelData) , rknn_outputs, MODEL_INPUT_SIZE, MODEL_INPUT_SIZE, dets,NUM_CLASS);

	int nboxes_left=do_nms_sort(dets,  yoloEng->modelData.nboxes_total, NUM_CLASS, NMS_THRESH);
	printf("total Box:%d \n",nboxes_left);
	int last_count = 0;
	group->count = 0;
	for(int i=0;i<nboxes_left;i++)
	{
		//char labelstr[4096]={0};
		int topclass=-1;            //
		float topclass_score=0;      //
		if(dets[i].objectness==0) continue;
		printf("++++++++++++++++++++++boxIndex:%d   thresh:%f   NUM_CLASS:\n",i,dets[i].objectness,NUM_CLASS);
		
		for(int j=0;j<NUM_CLASS;j++)
		{
			if(dets[i].prob[j]< yoloEng->modelData.OBJ_THRESH)
			    continue;

			//class 
			if(topclass_score<dets[i].prob[j])
			{
				topclass_score=dets[i].prob[j];
				topclass=j;
			}
			printf("classIndex:%d %s: %.02f%% \n",j, yoloEng->labels[j],dets[i].prob[j]*100);
		}

		if(topclass==-1)
			 continue;
		//
		float score_threshold = pRknn->threshold;
		if (topclass_score < score_threshold )
		{
			printf("props:%f    score_threshold:%f\r\n",topclass_score , score_threshold);
		    continue;
		}

		char *label =  yoloEng->labels[topclass];
	    //
		box b=dets[i].bbox;
		int x1 =(b.x-b.w/2.)*width;
		int x2=(b.x+b.w/2.)*width;
		int y1=(b.y-b.h/2.)*heigh;
		int y2=(b.y+b.h/2.)*heigh;

	    if(x1  < 0) x1  = 0;
	    if(x2> width-1) x2 = width-1;
	    if(y1 < 0) y1 = 0;
	    if(y2 > heigh-1) y2 = heigh-1;
	    printf("%s:%d:%d:%d:%d:%f\n", label, x1, y1, x2, y2, topclass_score);

	    group->results[last_count].box.left   = x1;
	    group->results[last_count].box.top	  = y1;
	    group->results[last_count].box.right  = x2;
	    group->results[last_count].box.bottom = y2;
	    group->results[last_count].prop = topclass_score;
		
		strcpy(group->results[last_count].name,label);
		printf("yolo result: (%4d, %4d, %4d, %4d), %4.2f, %s\n", x1, y1, x2, y2, topclass_score, label);
		last_count++;
	}
	group->count = last_count;

	//释放 dets
	free_detections(dets,yoloEng->modelData.nboxes_total);

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------
cyolo_engine::cyolo_engine()
{
	
}

cyolo_engine::~cyolo_engine()
{
	RknnDeinit();
}

// yolo
RknnRet cyolo_engine::RknnInit(RknnDatas *pRknn)
{
	if(pRknn==NULL) {return RKNN_ERR;}
	 //1、辅助模型数据
	memcpy(&m_rknnData, pRknn,sizeof(RknnDatas));

	int MODEL_INPUT_SIZE = m_rknnData.inputSize;
	int NUM_CLASS = m_rknnData.numClass;

	if( (0 == MODEL_INPUT_SIZE) ||(0 == NUM_CLASS) ){
		printf("MODEL_INPUT_SIZE:%d\r\n",MODEL_INPUT_SIZE);
		printf("NUM_CLASS:%d\r\n",NUM_CLASS);
		printf("RknnInit json get error.\n");
		return RKNN_ERR;
	}


	//1、get model info from config.txt-------------------------------------------------------------------
	printf("Loading CONF_FILE_PATH %s ...\n",m_rknnData.priboxPath.c_str());
	int ret=GetConfigs(&(m_rknnYoloEng.modelData),m_rknnData.priboxPath.c_str());
	if(ret<0)
	{
		return RKNN_ERR;
	}

	//2、load model-------------------------------------------------------------------
	printf("Loading model %s ...\n",m_rknnData.modelPath.c_str());
    // Load model
    FILE *fp = fopen(m_rknnData.modelPath.c_str(), "rb");
    if(fp == NULL) {
    	printf("fopen %s fail!\n", m_rknnData.modelPath.c_str());
        return RKNN_ERR;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    void *model = malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
    	printf("fread %s fail!\n", m_rknnData.modelPath.c_str());
        free(model);
        return RKNN_ERR;
    }
	if(NULL != fp){
		fclose(fp);
		fp = NULL;
	}

    //2)init model ------------------------------
    ret = rknn_init(&m_rknnYoloEng.ctx,model,model_len,RKNN_FLAG_PRIOR_MEDIUM);
    if(ret < 0)
    {
    	 printf("rknn_init fail! ret=%d\n", ret);
    	 return RKNN_ERR;
    }

    //3)Inference variate---------------------------------
    if(NULL != model){
    	free(model);
    	model =  NULL;
    }

	// load labes
	printf("loadLabelName %s\r\n",m_rknnData.labelPath.c_str());
	readLines(m_rknnData.labelPath.c_str(),m_rknnYoloEng.labels, m_rknnData.numClass,false);
    if(ret < 0) {
        printf("loadLabelName fail! ret=%d\n", ret);
        return RKNN_ERR;
    }

	return RKNN_SUCCESS;
}


// Deinit yolo======================================================================================================================
RknnRet  cyolo_engine::RknnDeinit()
{
	printf("RknnDeinit yolo ------------------------------------\n");
    if(m_rknnYoloEng.ctx >= 0)
    {
        rknn_destroy(m_rknnYoloEng.ctx);
    }
    //
	return RKNN_SUCCESS;
}

// inferenct ======================================================================================================================
RknnRet  cyolo_engine::Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID)
{
	if(srcimg.empty() || inputImg.empty()|| detect_result_group== NULL) 
	{
		return RKNN_ERR;
	}

	// 1) ----------------------------
	printf("1 set the inputs of model engine ------------------------------------\n");
	rknn_input inputs[1]={0};
	inputs[0].index = 0;
	inputs[0].buf = inputImg.data;
	inputs[0].size = inputImg.cols*inputImg.rows*inputImg.channels();
	//inputs[0].pass_through = false;//rmwei
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	int ret = rknn_inputs_set(m_rknnYoloEng.ctx, 1, inputs);
	if(ret < 0) {
		printf("rknn_input_set fail! ret=%d\n", ret);
		return RKNN_ERR;
	}
	
	// 2)------------------------
	printf("2 model engine run ------------------------------------\n");
	ret = rknn_run(m_rknnYoloEng.ctx, nullptr);
	if(ret < 0) {
		printf("rknn_run fail! ret=%d\n", ret);
		return RKNN_ERR;
	}

	// 3) -------------------------------------
	printf("3 rknn_outputs_get Process output ------------------------------------nyolo:%d\n",m_rknnYoloEng.modelData.nyolo);
    rknn_output_attr  o_num;
	memset(o_num.outputs, 0, sizeof(o_num.outputs));
	for(int index=0;index<m_rknnYoloEng.modelData.nyolo;index++)
    {
    	o_num.outputs_attr[index].index = index;
    	ret = rknn_query(m_rknnYoloEng.ctx, RKNN_QUERY_OUTPUT_ATTR, &(o_num.outputs_attr[index]), sizeof(o_num.outputs_attr[index]));
    	if(ret < 0)
    	{
    		printf("rknn_query fail! ret=%d\n", ret);
    		return RKNN_ERR;
    	}
    }
	for(int index=0;index<m_rknnYoloEng.modelData.nyolo;index++)
	{
		o_num.outputs[index].want_float = 1;
	}
	//printf ("o_num.outputs:%d\r\n",o_num.outputs);
	ret = rknn_outputs_get(m_rknnYoloEng.ctx, m_rknnYoloEng.modelData.nyolo, o_num.outputs, nullptr);
	if(ret < 0) {
		printf("rknn_outputs_get fail! ret=%d\n", ret);
		return RKNN_ERR;
	}

	// Process output----
	printf("rknn_outputs_get Process output ------------------------------------\n");
	ret =postProcessYolo(&m_rknnYoloEng, &m_rknnData, srcimg.cols, srcimg.rows, detect_result_group,o_num.outputs);
	if(ret < 0) {
		printf("rknn_outputs_get fail! ret=%d\n", ret);
		return RKNN_ERR;
	}
	//
	rknn_outputs_release(m_rknnYoloEng.ctx, m_rknnYoloEng.modelData.nyolo, o_num.outputs);
	memset(inputs[0].buf, 0, sizeof(inputImg.cols*inputImg.rows*inputImg.channels()));

	printf("########detect_result_group count:%d \n",detect_result_group->count);
	return RKNN_SUCCESS;
}


