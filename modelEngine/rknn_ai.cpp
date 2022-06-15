#include "rknn_ai.h"
#include "pix_formatter.h"
#include "Loggers.h"

RknnRet get_model_info(RknnDatas *pRknn) {
    if (pRknn == nullptr) { return RKNN_ERR; }

    //1.get model information form module url
    //1)the path of model file
    pRknn->jsonPath = pRknn->modelDir + "/" + "video.json";
    printf("jsonPath:%s\r\n", pRknn->jsonPath.c_str());

    cJSON *json_cfg = cJSON_Loade(pRknn->jsonPath.c_str());
    if (nullptr == json_cfg) {
        printf("cJSON_Loade:%s error\r\n", pRknn->jsonPath.c_str());
        return RKNN_ERR;
    }

    //the information of model
    int len;
    char temp[128] = {0};
    //2)ModelAlgType
    ModelAlgType_get(json_cfg, temp, &len);
    pRknn->modelAlgType = temp;
    printf("##############modelAlgType:%s\r\n", pRknn->modelAlgType.c_str());

    //3)ModelPath
    memset(temp, 0, len);
    ModelPath_get(json_cfg, temp, &len);
    pRknn->modelPath = pRknn->modelDir + "/" + temp;
    if (0 == strcmp(pRknn->modelAlgType.c_str(), BuiltIn)) {//rknn 1080 internal algorithm
        pRknn->modelPath = temp;
    }
    printf("##############modelPath:%s\r\n", pRknn->modelPath.c_str());

    //4)Labelsmap
    memset(temp, 0, len);
    Labelsmap_get(json_cfg, temp, &len);
    pRknn->labelPath = pRknn->modelDir + "/" + temp;
    printf("##############labelPath:%s\r\n", pRknn->labelPath.c_str());

    //5)Priorsbox
    memset(temp, 0, len);
    Priorsbox_get(json_cfg, temp, &len);
    pRknn->priboxPath = pRknn->modelDir + "/" + temp;
    printf("##############priboxPath:%s\r\n", pRknn->priboxPath.c_str());

    //6)
    InputSize_get(json_cfg, &pRknn->inputSize);
    NumResults_get(json_cfg, &pRknn->numResults);
    NumClass_get(json_cfg, &pRknn->numClass);

    json_exit(json_cfg);
    return RKNN_SUCCESS;
}

RknnRet check_model_info(RknnDatas *pRknn) {
    if (pRknn == NULL) { return RKNN_ERR; }

    //1)check model type
    if (0 != strcmp(pRknn->modelAlgType.c_str(), SSD) &&
        0 != strcmp(pRknn->modelAlgType.c_str(), Yolo) &&
        0 != strcmp(pRknn->modelAlgType.c_str(), GddModel) &&
        0 != strcmp(pRknn->modelAlgType.c_str(), HikangModel) &&
        0 != strcmp(pRknn->modelAlgType.c_str(), BuiltIn)) {
        printf("%s model type is not support!!!!\r\n", pRknn->modelAlgType.c_str());
        return RKNN_ERR;
    }

    //2)check whether the  model file   exists
    if (0 != (access(pRknn->modelPath.c_str(), F_OK)) && 0 != strcmp(pRknn->modelAlgType.c_str(), BuiltIn)) {
        printf("%s not exist!!!!\r\n", pRknn->modelPath.c_str());
        return RKNN_ERR;
    }
    if (0 != (access(pRknn->labelPath.c_str(), F_OK))) {
        printf("%s not exist!!!!\r\n", pRknn->labelPath.c_str());
        return RKNN_ERR;
    }
    if (0 != (access(pRknn->priboxPath.c_str(), F_OK))) {
        printf("%s not exist!!!!\r\n", pRknn->priboxPath.c_str());
        return RKNN_ERR;
    }

    return RKNN_SUCCESS;
}


RknnRet struct_to_cJSON(char **json_string, const char *filePath, detect_result_group_t Det, char *taskID) {
    if ((json_string == nullptr) || (Det.count == 0) || filePath == nullptr || taskID == nullptr) {
        printf("%s: input is invalid", __func__);
    }

    cJSON *root = cJSON_CreateObject();

    if (!root) {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return RKNN_ERR;
    } else {
        cJSON *ImagePath = cJSON_CreateString(filePath);
        cJSON_AddItemToObject(root, "imagePath", ImagePath);

        cJSON *TaskID = cJSON_CreateString(taskID);
        cJSON_AddItemToObject(root, "taskID", TaskID);

        cJSON *Objs = cJSON_CreateArray();
        printf("Det.count:%d\r\n", Det.count);
        for (int i = 0; i < Det.count; i++) {
            cJSON *objInf = cJSON_CreateObject();

            cJSON *jsClassName = cJSON_CreateString(Det.results[i].name);
            cJSON_AddItemToObject(objInf, "className", jsClassName);

            cJSON *similarity = cJSON_CreateNumber(Det.results[i].prop);
            cJSON_AddItemToObject(objInf, "similarity", similarity);

            cJSON *x1 = cJSON_CreateNumber(Det.results[i].box.left);
            cJSON_AddItemToObject(objInf, "X1", x1);

            cJSON *y1 = cJSON_CreateNumber(Det.results[i].box.top);
            cJSON_AddItemToObject(objInf, "Y1", y1);

            cJSON *x2 = cJSON_CreateNumber(Det.results[i].box.right);
            cJSON_AddItemToObject(objInf, "X2", x2);

            cJSON *y2 = cJSON_CreateNumber(Det.results[i].box.bottom);
            cJSON_AddItemToObject(objInf, "Y2", y2);

            cJSON_AddItemToArray(Objs, objInf);
        }

        cJSON_AddItemToObject(root, "alarmList", Objs);
        *json_string = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
    }
    return RKNN_SUCCESS;
}


//=================================================================================================
rknn_ai::rknn_ai(char *modelUrl, float threshold) {
    rknn_data.modelDir = modelUrl;
    rknn_data.threshold = threshold;
}

rknn_ai::~rknn_ai(){
    delete event_result;
};

RknnRet rknn_ai::init_model_engine() {
    std::lock_guard<std::mutex> lk(vpu_lock_);
    //1.get model info
    RknnRet ret = RKNN_ERR;
    ret = get_model_info(&rknn_data);
    if (ret != RKNN_SUCCESS) {
        SPDLOG_ERROR("get model info is error");
        return ret;
    }

    //2.check model info
    ret = check_model_info(&rknn_data);
    if (ret != RKNN_SUCCESS) {
        SPDLOG_ERROR("model info is error");
        return ret;
    }

    //3.creat model engine
    //根据不同模型属性进行不同处理：模型初始化引擎
    //若p为智能指针对象(如：shared_ptr< int> p),成员函数reset使用：
    //p.reset(q) //q为智能指针要指向的新对象 ,会令智能指针p中存放指针q，即p指向q的空间，而且会释放原来的空间。（默认是delete）
    // 参考：https://blog.csdn.net/lzn211/article/details/109147985

    ret = RKNN_ERR;
    printf("================modelAlgType: %s\r\n", rknn_data.modelAlgType.c_str());
    if (strcmp(rknn_data.modelAlgType.c_str(), SSD) == 0) {
        //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cssd_engine());
        m_aiEngine_api.reset(new cssd_engine());
        ret = m_aiEngine_api->RknnInit(&rknn_data);
    }

    if (strcmp(rknn_data.modelAlgType.c_str(), Yolo) == 0) {
        //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cyolo_engine());
        m_aiEngine_api.reset(new cyolo_engine());
        ret = m_aiEngine_api->RknnInit(&rknn_data);
    }

    if (strcmp(rknn_data.modelAlgType.c_str(), GddModel) == 0) {
//		m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cgdd_engine());
        m_aiEngine_api.reset(new cgdd_engine());
        ret = m_aiEngine_api->RknnInit(&rknn_data);
    }

    if (strcmp(rknn_data.modelAlgType.c_str(), HikangModel) == 0) {
        //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new chik_engine());
        m_aiEngine_api.reset(new chik_engine());
        ret = m_aiEngine_api->RknnInit(&rknn_data);
    }

//	// 内部算法
    if (strcmp(rknn_data.modelAlgType.c_str(), BuiltIn) == 0) {
        if (strcmp(rknn_data.modelPath.c_str(), BodyPosture) == 0) {
            //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cpose_engine());
            m_aiEngine_api.reset(new cpose_engine());
            ret = m_aiEngine_api->RknnInit(&rknn_data);
        }
    }

    return ret;
}

char *rknn_ai::model_engine_inference(
        uint8_t *imageBuf, uint32_t imageBufSize, char *imageBufType, char *taskID, int width, int height
) {
    if(!m_aiEngine_api){
        SPDLOG_ERROR("m_aiEngine_api is null");
    }
    detect_result_group_t detect_result_group{};
    if (strcmp(imageBufType, YUV420P) == 0) {
//#define YUV_TEST
        pix_formatter &formatter = pix_formatter::get_formatter();
        if (!formatter.is_ready()) {
            SPDLOG_ERROR("rga is not ready");
            return nullptr;
        }
        ImageSpec in_spec, out_spec;
        in_spec.data = imageBuf;
        in_spec.width = width;
        in_spec.height = height;
        in_spec.pix_format = RK_FORMAT_YCbCr_420_P;
#ifdef YUV_TEST
        const int image_size = 1920*1080+(1920*1080>>1);
        auto in_data = std::make_shared<std::array<uint8_t ,image_size >>();
        in_spec.data = in_data->data();
        in_spec.width =1920;
        in_spec.height = 1080;
        in_spec.pix_format =RK_FORMAT_YCbCr_420_SP;
        /** 读取yuv文件 */
        ifstream yuv_file("/data/zxf/1.yuv",ios::in|ios::binary);
        if(!yuv_file){
            cout<<"open yuv file failed"<<endl;
        }else
        {
            if (!yuv_file.read((char*)in_spec.data, image_size)) {
                // Same effect as above
                cout<<"read yuv file failed"<<endl;
            }
        }
#endif
        out_spec.width = width;
        out_spec.height = height;
        out_spec.pix_format = RK_FORMAT_BGR_888;
        out_spec.data = bgr_buf;

        auto start = chrono::system_clock::now();
        if (formatter.yuv_2_bgr(in_spec, out_spec)) {
            auto end = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            SPDLOG_TRACE("yuv->bgr spends {} s",double(duration.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den);
        } else {
            return nullptr;
        }
        Mat image(out_spec.height, out_spec.width, CV_8UC3, (unsigned char *) out_spec.data);
        cv::Mat inputImag;  //the image put into model
        detect(detect_result_group, image, inputImag, taskID);
    } else if (strcmp(imageBufType, JPG) == 0) {
        std::vector<char> vec_data(imageBuf, imageBuf + imageBufSize);
        Mat image = cv::imdecode(vec_data, CV_LOAD_IMAGE_COLOR);
        cv::Mat inputImag;  //the image put into model
        detect(detect_result_group, image, inputImag, taskID);
    }
    return event_result;
}
/**
 * 检测上报识别事件
 * @param detect_result_group
 * @param image
 * @param taskID
 */
void rknn_ai::detect(detect_result_group_t&detect_result_group , cv::Mat& image, cv::Mat& inputImag, char*taskID ) {
    if (rknn_data.inputSize != 0) {
        cv::resize(image, inputImag, cv::Size(rknn_data.inputSize, rknn_data.inputSize), 0, 0, cv::INTER_LINEAR);
    } else {
        inputImag = image;
    }
    m_aiEngine_api->Inferenct(image, inputImag, &detect_result_group, taskID);

    if (event_result){
        cJSON_free(event_result);
        event_result = nullptr;
    }
    if (detect_result_group.count > 0) {
        time_t nSeconds = 0;
        time(&nSeconds);
        string saveFileName = "/userdata/images/" + std::to_string(nSeconds) + "_" + std::to_string(rand()) + ".jpg";
        imwrite(saveFileName.c_str(), image);
        struct_to_cJSON(&event_result, saveFileName.c_str(), detect_result_group, taskID);
    }
}
