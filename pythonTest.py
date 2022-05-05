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
        self.ai_lib.rknn_model_engine_inference.argtypes = [c_void_p, c_char_p, c_uint32, c_char_p, c_char_p]
        # 识别线程列表
        self.thread_list = []
        self.modelEngPidList = []

    # 创建模型识别引擎
    def creatModelEng(self, modelType, threshold, modelEngPidList):

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
    def deleteModelEng(self,modelEngPidList):
        # 3)释放识别引擎 delete_rknn_model_engine 函数
        # 说明

        for engIndex in range(len(modelEngPidList)):
            ret = self.ai_lib.delete_rknn_model_engine(modelEngPidList[engIndex])
            if ret < 0:
                print("delete rknn model error:,", modelEngPidList[engIndex])
            else:
                print("delete rknn model success:", modelEngPidList[engIndex])

    def creatCameraThread(self, cameraIndex, modelEngPidList):
        myThread = threading.Thread(target=self.imageDetection, args=(cameraIndex, modelEngPidList))
        myThread.start()
        self.thread_list.append(myThread)

    def imageDetection(self,cameraIndex, modelEngPidList):
        count = 0
        while 1:
            for index in range(1):
                imagePath = "./%s.jpg" % (index + 1)
                with open(imagePath, 'rb') as f:
                    imageBuf = f.read()
                    # print("imageBuf:", type(imageBuf))
                    imageBufSize = len(imageBuf)
                    imageBufType = "JPG"
                    # print("imageBuf:",imageBuf)
                    # print("imageBufSize:", imageBufSize)
                    jpg_buf = (c_ubyte * (imageBufSize))()

                    taskID = str(index)

                    # print(
                    #     "image:=============================================================================================",
                    #     index)

                    for engIndex in range(len(modelEngPidList)):
                        # print(
                        #     "engIndex:=============================================================================================",
                        #     engIndex)

                        result = self.ai_lib.rknn_model_engine_inference(
                            modelEngPidList[engIndex], imageBuf, imageBufSize, imageBufType.encode(), taskID.encode()
                        )
                        # print("result:", result)
                        if result is not None:
                            resultData = json.loads(c_char_p(result).value)
                            # print("resultData:", resultData)
            # break
            count = count + 1
            if count == 10:
                break
            # time.sleep(1)


# ---------------------------------------------------------------------------------------
if __name__ == '__main__':
    ai_clib = AILib()
    for i in range(1):
        print("start=========================================================", i)
        # 1）初始化模型引擎
        modelType = "gdd"  # gdd yolo ssd pose hik
        threshold = c_float(1)
        threshold.value = 0.5
        print("creatModelEng")
        ai_clib.creatModelEng(modelType, threshold, ai_clib.modelEngPidList)

        # 2)
        print("imageDetection")
        cameraIndex = 1
        ai_clib.creatCameraThread(cameraIndex, ai_clib.modelEngPidList)
    print("WAIT...🔖")
    [thd.join() for thd in ai_clib.thread_list]
    [ai_clib.deleteModelEng([pid]) for pid in ai_clib.modelEngPidList]
    print("FINISH😆")
