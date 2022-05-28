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
    if ((json_string == NULL) || (Det.count == 0) || filePath == NULL || taskID == NULL) {
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
    m_rknnData.modelDir = modelUrl;
    m_rknnData.threshold = threshold;
}

rknn_ai::~rknn_ai() {

}

RknnRet rknn_ai::init_model_engine() {
    //1.get model info
    RknnRet ret = RKNN_ERR;
    ret = get_model_info(&m_rknnData);
    if (ret != RKNN_SUCCESS) {
        SPDLOG_ERROR("get model info is error");
        return ret;
    }

    //2.check model info
    ret = check_model_info(&m_rknnData);
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
    printf("================modelAlgType: %s\r\n", m_rknnData.modelAlgType.c_str());
    if (strcmp(m_rknnData.modelAlgType.c_str(), SSD) == 0) {
        //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cssd_engine());
        m_aiEngine_api.reset(new cssd_engine());
        ret = m_aiEngine_api->RknnInit(&m_rknnData);
    }

    if (strcmp(m_rknnData.modelAlgType.c_str(), Yolo) == 0) {
        //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cyolo_engine());
        m_aiEngine_api.reset(new cyolo_engine());
        ret = m_aiEngine_api->RknnInit(&m_rknnData);
    }

    if (strcmp(m_rknnData.modelAlgType.c_str(), GddModel) == 0) {
//		m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cgdd_engine());
        m_aiEngine_api.reset(new cgdd_engine());
        ret = m_aiEngine_api->RknnInit(&m_rknnData);
    }

    if (strcmp(m_rknnData.modelAlgType.c_str(), HikangModel) == 0) {
        //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new chik_engine());
        m_aiEngine_api.reset(new chik_engine());
        ret = m_aiEngine_api->RknnInit(&m_rknnData);
    }

//	// 内部算法
    if (strcmp(m_rknnData.modelAlgType.c_str(), BuiltIn) == 0) {
        if (strcmp(m_rknnData.modelPath.c_str(), BodyPosture) == 0) {
            //m_aiEngine_api = std::shared_ptr<aiEngine_api>(new cpose_engine());
            m_aiEngine_api.reset(new cpose_engine());
            ret = m_aiEngine_api->RknnInit(&m_rknnData);
        }
    }

    return ret;
}

char *rknn_ai::model_engine_inference(
        uint8_t *imageBuf, uint32_t imageBufSize, char *imageBufType, char *taskID, int width, int height
) {
    detect_result_group_t detect_result_group{};
    cv::Mat image;      //the src image
    cv::Mat inputImag;  //the image put into model
    if (strcmp(imageBufType, YUV420P) == 0) {
//#define YUV_TEST
        cout << "start yuv" << endl;
        pix_formatter &formatter = pix_formatter::get_formatter();
        if (!formatter.is_ready()) {
            cout << "rga is not ready!" << endl;
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
        auto out_data = std::make_shared<std::array<uint8_t, 1920 * 1080 * 3>>();
        out_spec.width = 1920;
        out_spec.height = 1080;
        out_spec.pix_format = RK_FORMAT_BGR_888;
        out_spec.data = out_data->data();

        auto start = chrono::system_clock::now();
        if (formatter.yuv_2_bgr(in_spec, out_spec)) {
            auto end = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            SPDLOG_TRACE("yuv->bgr cost {} s",double(duration.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den);
//            /** 写入BGR文件 */
//            if(out_spec.data){
//                cout<<"写入OUT_BGR.bin"<<endl;
//               ofstream bgr_file ("OUT_BGR.bin", ios::out | ios::binary);
//               bgr_file.write((char*)out_spec.data, 1920*1080*3);
//            }
        } else {
            return nullptr;
        }

        /*
        m_aiEngine_api->Inferenct(out_spec, &detect_result_group, taskID);
        if(detect_result_group.count>0){
            // wrap input buffer
            Mat img(out_spec.height, out_spec.width, CV_8UC3, (unsigned char*)out_spec.data);
            image = img;
        }
        */

        Mat img(out_spec.height, out_spec.width, CV_8UC3, (unsigned char *) out_spec.data);
        image = img;
        if (m_rknnData.inputSize != 0) {
            cv::resize(image, inputImag, cv::Size(m_rknnData.inputSize, m_rknnData.inputSize), 0, 0, cv::INTER_LINEAR);
        } else {
            inputImag = image;
        }
        m_aiEngine_api->Inferenct(image, inputImag, &detect_result_group, taskID);
//        static int  write_flag = 0;
//        if(detect_result_group.count>0){
//            /** 写入BGR文件 */
//            const int image_size = 1920*1080+(1920*1080>>1);
//            if(out_spec.data){
//                char bgr_file_name [64]={0};
//                char yuv_file_name [64]={0};
//                sprintf(bgr_file_name,"/data/OUT_BGR_%d.bin",write_flag);
//                cout<<"写入:"<<bgr_file_name<<endl;
//                ofstream bgr_file (bgr_file_name, ios::out | ios::binary);
//                bgr_file.write((char*)out_spec.data, 1920*1080*3);
//
//                sprintf(yuv_file_name,"/data/OUT_YUV_%d.bin",write_flag);
//                cout<<"写入:"<<yuv_file_name<<endl;
//                ofstream yuv_file (yuv_file_name, ios::out | ios::binary);
//                yuv_file.write((char*)in_spec.data, image_size);
//                write_flag++;
//            }
//        }
    } else if (strcmp(imageBufType, JPG) == 0) {
        std::vector<char> vec_data(imageBuf, imageBuf + imageBufSize);
        image = cv::imdecode(vec_data, CV_LOAD_IMAGE_COLOR);
        //
        cv::imwrite("test.jpg", image);
        // 清空  vec_data
        std::vector<char>().swap(vec_data);
        if (m_rknnData.inputSize != 0) {
            cv::resize(image, inputImag, cv::Size(m_rknnData.inputSize, m_rknnData.inputSize), 0, 0, cv::INTER_LINEAR);
        } else {
            inputImag = image;
        }
        m_aiEngine_api->Inferenct(image, inputImag, &detect_result_group, taskID);
    }
    for (int i = 0; i < detect_result_group.count; i++) {
        int x1 = detect_result_group.results[i].box.left;
        int y1 = detect_result_group.results[i].box.top;
        int x2 = detect_result_group.results[i].box.right;
        int y2 = detect_result_group.results[i].box.bottom;

        float prop = detect_result_group.results[i].prop;
        char *label = detect_result_group.results[i].name;
//        printf("result: (%4d, %4d, %4d, %4d), %4.2f, %s\n", x1, y1, x2, y2, prop, label);
        SPDLOG_INFO("Detect result:({:4d}, {:4d}, {:4d}, {:4d}), {:4.2f}, {}",x1, y1, x2, y2, prop, label);
        //绘制
        //rectangle(image, Point(x1, y1), Point(x2, y2), Scalar(0, 0, 255, 255), 6);
//        string temp = to_string(prop) + "_" + label;
//		printf("draw result:%s\r\n",temp.c_str());
        //putText(image, temp.c_str(), Point(x1, y1 - 12), 1, 2, Scalar(0, 255, 0, 255));
    }

    char *JsonMessge = nullptr;
    if (detect_result_group.count > 0) {
        time_t nSeconds = 0;
        time((time_t *) &nSeconds);
        string saveFileName = "/userdata/images/" + std::to_string(nSeconds) + "_" + std::to_string(rand()) + ".jpg";
        printf("%s\r\n", saveFileName.c_str());
        imwrite(saveFileName.c_str(), image);
        struct_to_cJSON(&JsonMessge, saveFileName.c_str(), detect_result_group, taskID);
    }
    return JsonMessge;

}

RknnRet rknn_ai::deInint_model_engine() {
    RknnRet ret = RKNN_ERR;
    //根据不同模型属性进行不同处理：模型初始化引擎
    if (m_aiEngine_api)
        ret = m_aiEngine_api->RknnDeinit();

    return ret;
}