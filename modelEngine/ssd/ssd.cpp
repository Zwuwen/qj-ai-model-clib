#include "ssd.h"

float MIN_SCORE = 0.4f;
float NMS_THRESHOLD = 0.45f;


//get model boxpriors infomation=========================================================================================
int loadBoxPriors(RknnDatas *pRknn, RknnSSDEng *ssdEng)
{
    const char *d = " ";
	char *lines[4];
//	float (*boxPriors)[pRknn->numResults] = (float (*)[pRknn->numResults])ssdEng->box_priors;
	readLines(pRknn->priboxPath.c_str(), lines, 4,true);
    for (int i = 0; i < 4; i++) {
        char *line_str = lines[i];
        char *p;
        p = strtok(line_str, d);
        int priorIndex = 0;
        while (p) {
            float number = (float)(atof(p));
            ssdEng->box_priors[i][priorIndex++] = number;
            p=strtok(nullptr, d);
        }
        if (priorIndex != pRknn->numResults) {
			printf("error\n");
            return -1;
        }
    }
    return 0;
}

// ssd --------------------------------------------------------------------------
float CalculateOverlap(float xmin0, float ymin0, float xmax0, float ymax0, float xmin1, float ymin1, float xmax1, float ymax1)
{
    float w = fmax(0.f, fmin(xmax0, xmax1) - fmax(xmin0, xmin1));
    float h = fmax(0.f, fmin(ymax0, ymax1) - fmax(ymin0, ymin1));
    float i = w * h;
    float u = (xmax0 - xmin0) * (ymax0 - ymin0) + (xmax1 - xmin1) * (ymax1 - ymin1) - i;
    return u <= 0.f ? 0.f : (i / u);
}

float unexpit(float y) {
	return -1.0 * logf((1.0 / y) - 1.0);
}

float expit(float x)
{
    return (float) (1.0 / (1.0 + expf(-x)));
}

void decodeCenterSizeBoxes(float* predictions, RknnDatas *pRknn,RknnSSDEng* ssdEng)
{
	float (*boxPriors)[pRknn->numResults] = (float (*)[pRknn->numResults])ssdEng->box_priors;

    for (int i = 0; i < pRknn->numResults; ++i) {
        float ycenter = predictions[i*4+0] / Y_SCALE * boxPriors[2][i] + boxPriors[0][i];
        float xcenter = predictions[i*4+1] / X_SCALE * boxPriors[3][i] + boxPriors[1][i];
        float h = (float) expf(predictions[i*4 + 2] / H_SCALE) * boxPriors[2][i];
        float w = (float) expf(predictions[i*4 + 3] / W_SCALE) * boxPriors[3][i];

        float ymin = ycenter - h / 2.0f;
        float xmin = xcenter - w / 2.0f;
        float ymax = ycenter + h / 2.0f;
        float xmax = xcenter + w / 2.0f;

        predictions[i*4 + 0] = ymin;
        predictions[i*4 + 1] = xmin;
        predictions[i*4 + 2] = ymax;
        predictions[i*4 + 3] = xmax;
    }
}

int filterValidResult(float * outputClasses, void *pOutput, RknnDatas *pRknn, float *props)
{
    int validCount = 0;
	float min_score = unexpit(MIN_SCORE);
	int (*output)[pRknn->numResults] = (int (*)[pRknn->numResults])pOutput;
    // Scale them back to the input size.
    for (int i = 0; i < pRknn->numResults; ++i) {
        float topClassScore = (float)(-1000.0);
        int topClassScoreIndex = -1;

        // Skip the first catch-all class.
        for (int j = 1; j < pRknn->numClass; ++j) {
			// x and expit(x) has same monotonicity
			// so compare x and comare expit(x) is same
            //float score = expit(outputClasses[i*numClasses+j]);
            float score = outputClasses[i*pRknn->numClass+j];

            if (score > topClassScore) {
                topClassScoreIndex = j;
                topClassScore = score;
            }
        }

        if (topClassScore >= min_score) {
            output[0][validCount] = i;
            output[1][validCount] = topClassScoreIndex;
            props[validCount] = expit(outputClasses[i*pRknn->numClass+topClassScoreIndex]);
            ++validCount;
        }
    }

    return validCount;
}

int nms(RknnDatas *pRknn,int validCount, float* outputLocations, void *pOutput/*int (*output)[NUM_RESULTS]*/)
{
	int (*output)[pRknn->numResults] = (int (*)[pRknn->numResults])pOutput;
    for (int i=0; i < validCount; ++i) {
        if (output[0][i] == -1) {
            continue;
        }
        int n = output[0][i];
        for (int j=i + 1; j<validCount; ++j) {
            int m = output[0][j];
            if (m == -1) {
                continue;
            }
            float xmin0 = outputLocations[n*4 + 1];
            float ymin0 = outputLocations[n*4 + 0];
            float xmax0 = outputLocations[n*4 + 3];
            float ymax0 = outputLocations[n*4 + 2];

            float xmin1 = outputLocations[m*4 + 1];
            float ymin1 = outputLocations[m*4 + 0];
            float xmax1 = outputLocations[m*4 + 3];
            float ymax1 = outputLocations[m*4 + 2];

            float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1, xmax1, ymax1);

            if (iou >= NMS_THRESHOLD) {
                output[0][j] = -1;
            }
        }
    }
    return 0;
}

void sort(RknnDatas *pRknn,void *pOutput /*int output[][NUM_RESULTS]*/, float* props, int sz) {
    int i = 0;
    int j = 0;

	if (sz < 2) {
		return;
	}
	int (*output)[pRknn->numResults] = (int (*)[pRknn->numResults])pOutput;
#if 1
    for(i = 0; i < sz-1; i++) {

		int top = i;
		for (j=i+1; j<sz; j++) {
            if(props[top] < props[j]) {
				top = j;
			}
		}

		if (i != top) {
			int tmp1 = output[0][i];
			int tmp2 = output[1][i];
			float prop = props[i];
			output[0][i] = output[0][top];
			output[1][i] = output[1][top];
			props[i] = props[top];
			output[0][top] = tmp1;
			output[1][top] = tmp2;
			props[top] = prop;
		}
	}
#endif
}

int postProcessSSD(RknnSSDEng* ssdEng,RknnDatas *pRknn,float * predictions, float *output_classes, int width, int heigh, detect_result_group_t *group)
{
	int output[2][pRknn->numResults] = {0};
    float props[pRknn->numResults]= {0};

    decodeCenterSizeBoxes(predictions,pRknn,ssdEng);

	int validCount = filterValidResult(output_classes, (void *)output, pRknn, props);
	
    if (validCount > OBJ_NUMB_MAX_SIZE) {
        printf("validCount too much !!\n");
        return -1;
    }
	//printf("validCount:%d\r\n",validCount);
	sort(pRknn,(void *)output, props, validCount);

    /* detect nest box */
    nms(pRknn,validCount, predictions, (void *)output);

    int last_count = 0;
    group->count = 0;
    /* box valid detect target */
    float score_threshold = pRknn->threshold;

    for (int i = 0; i < validCount; ++i) {
        if (output[0][i] == -1|| ( props[i] < score_threshold )) {
        	//printf("props[i]:%f    score_threshold:%f\r\n",props[i] , score_threshold);
            continue;
        }
        int n = output[0][i];
        int topClassScoreIndex = output[1][i];

        int x1 = (int)(predictions[n * 4 + 1] * width);
        int y1 = (int)(predictions[n * 4 + 0] * heigh);
        int x2 = (int)(predictions[n * 4 + 3] * width);
        int y2 = (int)(predictions[n * 4 + 2] * heigh);
        // There are a bug show toothbrush always
        if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0)
            continue;

        if(x1  < 0) x1  = 0;
	    if(x2> width-1) x2 = width-1;
	    if(y1 < 0) y1 = 0;
	    if(y2 > heigh-1) y2 = heigh-1;

        char *label = ssdEng->labels[topClassScoreIndex];

        group->results[last_count].box.left   = x1;
        group->results[last_count].box.top    = y1;
        group->results[last_count].box.right  = x2;
        group->results[last_count].box.bottom = y2;
        group->results[last_count].prop = props[i];
        strcpy(group->results[last_count].name,label);

		printf("ssd result: (%4d, %4d, %4d, %4d), %4.2f, %s\n", x1, y1, x2, y2, props[i], label);
        last_count++;
    }

    group->count = last_count;
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------
cssd_engine::cssd_engine()
{
	
}

cssd_engine::~cssd_engine()
{
	RknnDeinit();
}


// init ssd  model engine 
RknnRet cssd_engine::RknnInit(RknnDatas *pRknn)
{
   if(pRknn==NULL) {return RKNN_ERR ;}
   //1、辅助模型数据
	 m_rknnData = *pRknn;

    //1. check the model info-------------------------------------------------------
	int MODEL_INPUT_SIZE = m_rknnData.inputSize;
	int NUM_RESULTS = m_rknnData.numResults;
	int NUM_CLASS = m_rknnData.numClass;
	if( (0 == MODEL_INPUT_SIZE) || (0 == NUM_RESULTS)||(0 == NUM_CLASS) ){
		printf("MODEL_INPUT_SIZE:%d\r\n",MODEL_INPUT_SIZE);
		printf("NUM_RESULTS:%d\r\n",NUM_RESULTS);
		printf("NUM_CLASS:%d\r\n",NUM_CLASS);
		printf("RknnInit json get error.\n");
		return RKNN_ERR;
	}

	printf("Loading model %s ...\n",m_rknnData.modelPath.c_str());
    //2. Load model------------------------------------------------------------------
    FILE *fp = fopen(m_rknnData.modelPath.c_str(), "rb");
    if(fp == NULL) {
        printf("fopen %s fail!\n", m_rknnData.modelPath.c_str());
        return RKNN_ERR;
    }
    fseek(fp, 0, SEEK_END);
    unsigned int model_len = ftell(fp);
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

    int ret = 0;
    ret = rknn_init(&m_rknnSSDEng.ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM);
    if(ret < 0) 
    {
        printf("rknn_init fail! ret=%d\n", ret);
        return RKNN_ERR;
    }

	if(NULL != model){
		free(model);
		model =  NULL;
	}

	//3.get labes information---------------------------------------------------------------
	printf("loadLabelName %s\r\n",m_rknnData.labelPath.c_str());
	ret= readLines(m_rknnData.labelPath.c_str(),m_rknnSSDEng.labels, m_rknnData.numClass,true);
    if(ret < 0) 
    {
        printf("loadLabelName fail! ret=%d\n", ret);
        return RKNN_ERR;
    }

	//4.get box priors information
    printf("loadBoxPriors %s\r\n",m_rknnData.priboxPath.c_str());
	ret=loadBoxPriors(&m_rknnData, &m_rknnSSDEng);
    if(ret < 0) 
    {
        printf("loadBoxPriors fail! ret=%d\n", ret);
        return RKNN_ERR;
    }
	return RKNN_SUCCESS;
}

// deinit ssd  model engine  ======================================================================================================================
RknnRet cssd_engine::RknnDeinit()
{
    printf("RknnDeinit ssd ------------------------------------\n");
    if(m_rknnSSDEng.ctx >= 0)
    {
        rknn_destroy(m_rknnSSDEng.ctx);
    }
	return RKNN_SUCCESS;
}


//ssd model engine inferenct======================================================================================================================
RknnRet cssd_engine::Inferenct(cv::Mat &srcimg,cv::Mat &inputImg,detect_result_group_t *detect_result_group,char* taskID)
{
	if(srcimg.empty() || inputImg.empty() || detect_result_group==NULL) 
    {
		return RKNN_ERR;
	}

	// 1) rknn engine input----------------------------
	rknn_input inputs[1]={0};
	inputs[0].index = 0;
	inputs[0].buf = inputImg.data;
	inputs[0].size = inputImg.cols*inputImg.rows*inputImg.channels();
	//inputs[0].pass_through = false;  //rmwei
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	int ret = rknn_inputs_set(m_rknnSSDEng.ctx, 1, inputs);
	if(ret < 0) {
		printf("rknn_input_set fail! ret=%d\n", ret);
		return RKNN_ERR;
	}
	// 2) rknn engine run------------------------
	ret = rknn_run(m_rknnSSDEng.ctx, nullptr);
	if(ret < 0) {
		printf("rknn_run fail! ret=%d\n", ret);
		return RKNN_ERR;
	}
	// 3) rknn engine output-------------------------------------
    rknn_output_attr  o_num;
	memset(o_num.outputs, 0, sizeof(o_num.outputs));
	o_num.outputs[0].want_float = 1;
	o_num.outputs[1].want_float = 1;

    o_num.outputs_attr[0].index = 0;
    ret = rknn_query(m_rknnSSDEng.ctx, RKNN_QUERY_OUTPUT_ATTR, &(o_num.outputs_attr[0]), sizeof(o_num.outputs_attr[0]));
    if(ret < 0) 
    {
        printf("rknn_query fail! ret=%d\n", ret);
        return RKNN_ERR;
    }

    o_num.outputs_attr[1].index = 1;
    ret = rknn_query(m_rknnSSDEng.ctx, RKNN_QUERY_OUTPUT_ATTR, &(o_num.outputs_attr[1]), sizeof(o_num.outputs_attr[1]));
    if(ret < 0) {
        printf("rknn_query fail! ret=%d\n", ret);
        return RKNN_ERR;
    }

	ret = rknn_outputs_get(m_rknnSSDEng.ctx, 2, o_num.outputs, nullptr);
	if(ret < 0) {
		printf("rknn_outputs_get fail! ret=%d\n", ret);
		return RKNN_ERR;
	}
	// 4)Process output----
	if(o_num.outputs[0].size == o_num.outputs_attr[0].n_elems*sizeof(float) &&
	   o_num.outputs[1].size == o_num.outputs_attr[1].n_elems*sizeof(float))
	{
		//----------------------------------------------
		float* predictions = (float*)o_num.outputs[0].buf;
		float* outputClasses = (float*)o_num.outputs[1].buf;
		// 
		ret=postProcessSSD(&m_rknnSSDEng ,&m_rknnData,predictions,outputClasses, srcimg.cols, srcimg.rows, detect_result_group);
	}
	else
	{
		printf("rknn_outputs_get fail! get outputs_size = [%d, %d], but expect [%lu, %lu]!\n",
			o_num.outputs[0].size, o_num.outputs[1].size, 
            o_num.outputs_attr[0].n_elems*sizeof(float), o_num.outputs_attr[1].n_elems*sizeof(float));
	}

	// 5)rknn model engine release
	rknn_outputs_release(m_rknnSSDEng.ctx, 2, o_num.outputs);
    printf("########detect_result_group count:%d \n",detect_result_group->count);
	return RKNN_SUCCESS;
}


RknnRet cssd_engine::Inferenct(ImageSpec &bgr_data,detect_result_group_t *detect_result_group,char* taskID){

	return RKNN_SUCCESS;
}