import os
import sys
import cv2
import json
import time
import threading

from ctypes import cdll, byref, c_int, c_ubyte, c_float, c_char_p, c_void_p, c_uint32


class AILib:
    def __init__(self):
        self.ai_lib = cdll.LoadLibrary("/usr/lib/librknnx_api.so")
        # 1)creat_rknn_model_engine 函数说明
        # 輸出
        self.ai_lib.creat_rknn_model_engine.restype = c_void_p
        # 输入
        self.ai_lib.creat_rknn_model_engine.argtypes = [c_char_p, c_float]

        # 2)init_rknn_model_engine 函数说明
        self.ai_lib.init_rknn_model_engine.restype = c_int
        self.ai_lib.init_rknn_model_engine.argtypes = [c_void_p]

        self.ai_lib.delete_rknn_model_engine.restype = c_int
        self.ai_lib.delete_rknn_model_engine.argtypes = [c_void_p]

        self.ai_lib.rknn_model_engine_inference.restype = c_char_p
        self.ai_lib.rknn_model_engine_inference.argtypes = [c_void_p, c_char_p, c_uint32, c_char_p, c_char_p, c_int,
                                                            c_int]

        # 5)RknnRet rknnx_get_device_info(char* deviceType ,char* filePath);
        self.ai_lib.rknnx_get_device_info.restype = c_int
        self.ai_lib.rknnx_get_device_info.argtypes = [c_char_p, c_char_p]

        # 识别线程列表
        self.thread_list = []
        self.modelEngPidList = []

    # 创建模型识别引擎
    def create_model_engine(self, modelType, threshold, modelEngPidList):

        if modelType == "gdd" or modelType == "all":
            modelEngPid1 = self.ai_lib.creat_rknn_model_engine("./output_rknn_car".encode(), threshold)
            ret = self.ai_lib.init_rknn_model_engine(modelEngPid1)
            if ret < 0:
                print("init rknn model error")
            else:
                modelEngPidList.append(modelEngPid1)
                # print("modelEngPid1:", modelEngPid1)
            # modelEngPidList.append(modelEngPid1)

        if modelType == "yolo" or modelType == "all":
            modelEngPid2 = self.ai_lib.creat_rknn_model_engine("./yolo".encode(), threshold)
            ret = self.ai_lib.init_rknn_model_engine(modelEngPid2)
            if ret < 0:
                print("init rknn model error")
            else:
                modelEngPidList.append(modelEngPid2)
                # print("modelEngPid2:", modelEngPid2)

        if modelType == "ssd" or modelType == "all":
            modelEngPid3 = self.ai_lib.creat_rknn_model_engine("./ssd".encode(), threshold)
            ret = self.ai_lib.init_rknn_model_engine(modelEngPid3)
            if ret < 0:
                print("init rknn model error")
            else:
                modelEngPidList.append(modelEngPid3)
                # print("modelEngPid3:", modelEngPid3)

        if modelType == "pose" or modelType == "all":
            modelEngPid4 = self.ai_lib.creat_rknn_model_engine("./pose".encode(), threshold)
            ret = self.ai_lib.init_rknn_model_engine(modelEngPid4)
            if ret < 0:
                print("init rknn model error")
            else:
                modelEngPidList.append(modelEngPid4)
                # print("modelEngPid4:", modelEngPid4)

        if modelType == "hik" or modelType == "all":
            modelEngPid5 = self.ai_lib.creat_rknn_model_engine("./hik".encode(), threshold)
            ret = self.ai_lib.init_rknn_model_engine(modelEngPid5)
            if ret < 0:
                print("init rknn model error")
            else:
                modelEngPidList.append(modelEngPid5)
                # print("modelEngPid5:", modelEngPid5)

    #
    def delete_model_engine(self, modelEngPidList):
        # 3)释放识别引擎 delete_rknn_model_engine 函数
        # 说明

        for engIndex in range(len(modelEngPidList)):
            ret = self.ai_lib.delete_rknn_model_engine(modelEngPidList[engIndex])
            if ret < 0:
                print("delete rknn model error:,", modelEngPidList[engIndex])
            else:
                print("delete rknn model success:", modelEngPidList[engIndex])

    def create_camera_thread(self, cameraIndex, modelEngPidList):
        my_thread = threading.Thread(target=self.image_detection, args=(cameraIndex, modelEngPidList))
        my_thread.start()
        self.thread_list.append(my_thread)

    def image_detection(self, cameraIndex, modelEngPidList):
        count = 0
        while 1:
            for index in range(7):
                image_path = "./%s.jpg" % (index + 1)
                with open(image_path, 'rb') as f:
                    image_buf = f.read()
                    image_buf_size = len(image_buf)
                    image_buf_type = "JPG"

                    taskID = str(index)

                    for engIndex in range(len(modelEngPidList)):
                        result = self.ai_lib.rknn_model_engine_inference(
                            modelEngPidList[engIndex], image_buf, image_buf_size, image_buf_type.encode(), taskID.encode()
                            , 1920, 1080
                        )
                        if result is not None:
                            resultData = json.loads(c_char_p(result).value)
            count = count + 1
            if count > 2:
                break
            # time.sleep(1)


# ---------------------------------------------------------------------------------------
if __name__ == '__main__':
    ai_clib = AILib()
    for i in range(4):
        print("start=========================================================", i)
        # 1）初始化模型引擎
        modelType = "yolo"  # gdd yolo ssd pose hik
        threshold = c_float(1)
        threshold.value = 0.5
        print("creatModelEng")
        ai_clib.create_model_engine(modelType, threshold, ai_clib.modelEngPidList)

        # 2)
        print("imageDetection")
        cameraIndex = 1
        ai_clib.create_camera_thread(cameraIndex, ai_clib.modelEngPidList)
    print("WAIT...🔖")
    [thd.join() for thd in ai_clib.thread_list]
    [ai_clib.delete_model_engine([pid]) for pid in ai_clib.modelEngPidList]
    print("FINISH😆")
