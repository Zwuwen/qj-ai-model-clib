#ifndef  __RKNN_AI_H__
#define  __RKNN_AI_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include "gdd/gdd.h"
#include "hik/hikang.h"
#include "ssd/ssd.h"
#include "yolo/yolo.h"

#include "rknnPose/rknn_pose_body_async.h"

class rknn_ai {
public:
    rknn_ai(char *modelUrl, float threshold);
    ~rknn_ai();
    RknnDatas rknn_data;
private:
    std::shared_ptr<aiEngine_api> m_aiEngine_api;
    void detect(detect_result_group_t &detect_result_group, Mat &image, Mat &inputImag, char *taskID);
    uint8_t bgr_buf[1920*1080*3]{};
    std::mutex vpu_lock_{};
protected:
    char *event_result{};
public:
    RknnRet init_model_engine();
    char *model_engine_inference(uint8_t *imageBuf, uint32_t imageBufSize, char *imageBufType, char *taskID, int, int);
};

#endif



