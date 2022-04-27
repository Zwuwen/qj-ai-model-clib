import os
import sys
import cv2
import json
import time
import threading

from ctypes import cdll, byref, c_int, c_ubyte, c_float, c_char_p, c_void_p, c_uint32


# 创建模型识别引擎
def creatModelEng(modelType, threshold, modelEngPidList):
    # 1)creat_rknn_model_engine 函数说明
    # 輸出
    aiLib.creat_rknn_model_engine.restype = c_void_p
    # 输入
    aiLib.creat_rknn_model_engine.argtypes = [c_char_p, c_float]

    # 2)init_rknn_model_engine 函数说明
    aiLib.init_rknn_model_engine.restype = c_int
    aiLib.init_rknn_model_engine.argtypes = [c_void_p]

    if modelType == "gdd" or modelType == "all":
        modelEngPid1 = aiLib.creat_rknn_model_engine("./output_rknn_car".encode(), threshold)
        print("modelEngPid2:", modelEngPid1)
        ret = aiLib.init_rknn_model_engine(modelEngPid1)
        if ret < 0:
            print("init rknn model error")
        else:
            modelEngPidList.append(modelEngPid1)
            print("modelEngPid1:", modelEngPid1)

    if modelType == "yolo" or modelType == "all":
        modelEngPid2 = aiLib.creat_rknn_model_engine("./yolo".encode(), threshold)
        ret = aiLib.init_rknn_model_engine(modelEngPid2)
        if ret < 0:
            print("init rknn model error")
        else:
            modelEngPidList.append(modelEngPid2)
            print("modelEngPid2:", modelEngPid2)

    if modelType == "ssd" or modelType == "all":
        modelEngPid3 = aiLib.creat_rknn_model_engine("./ssd".encode(), threshold)
        ret = aiLib.init_rknn_model_engine(modelEngPid3)
        if ret < 0:
            print("init rknn model error")
            os._exit()
        else:
            modelEngPidList.append(modelEngPid3)
            print("modelEngPid3:", modelEngPid3)

    if modelType == "pose" or modelType == "all":
        modelEngPid4 = aiLib.creat_rknn_model_engine("./pose".encode(), threshold)
        ret = aiLib.init_rknn_model_engine(modelEngPid4)
        if ret < 0:
            print("init rknn model error")
        else:
            modelEngPidList.append(modelEngPid4)
            print("modelEngPid4:", modelEngPid4)

    if modelType == "hik" or modelType == "all":
        modelEngPid5 = aiLib.creat_rknn_model_engine("./hik".encode(), threshold)
        ret = aiLib.init_rknn_model_engine(modelEngPid5)
        if ret < 0:
            print("init rknn model error")
        else:
            modelEngPidList.append(modelEngPid5)
            print("modelEngPid5:", modelEngPid5)


def del_model_engine(model_engine_id):
    print(f'del_model_engine({model_engine_id})')
    aiLib.delete_rknn_model_engine.restype = c_int
    aiLib.delete_rknn_model_engine.argtypes = [c_void_p]
    ret = aiLib.delete_rknn_model_engine(model_engine_id)
    if ret < 0:
        print("delete rknn model error:,", model_engine_id)
    else:
        print("delete rknn model success:", model_engine_id)


#
thread_list = []


def creatCameraThread(cameraIndex, modelEngPidList):
    myThread = threading.Thread(target=imageDetection, args=(cameraIndex, modelEngPidList))
    myThread.start()
    #
    # myThread.join()
    thread_list.append(myThread)


def imageDetection(cameraIndex, modelEngPidList):
    # 3)rknn_model_engine_inference 函数说明
    aiLib.rknn_model_engine_inference.restype = c_char_p
    aiLib.rknn_model_engine_inference.argtypes = [c_void_p, c_char_p, c_uint32, c_char_p, c_char_p]

    count = 0
    while 1:
        for index in range(1):
            imagePath = "./%s.jpg" % (index + 1)
            with open(imagePath, 'rb') as f:
                imageBuf = f.read()
                print("imageBuf:", type(imageBuf))
                imageBufSize = len(imageBuf)
                imageBufType = "JPG"
                # print("imageBuf:",imageBuf)
                print("imageBufSize:", imageBufSize)
                jpg_buf = (c_ubyte * (imageBufSize))()

                taskID = str(index)

                print(
                    "image:=============================================================================================",
                    index)

                for engIndex in range(len(modelEngPidList)):
                    print(
                        "engIndex:=============================================================================================",
                        engIndex)

                    result = aiLib.rknn_model_engine_inference(modelEngPidList[engIndex], imageBuf, imageBufSize,
                                                               imageBufType.encode(), taskID.encode())
                    print("result:", result)
                    if result != None:
                        resultData = json.loads(c_char_p(result).value)
                        print("resultData:", resultData)
        # break
        count = count + 1
        if count > 1:
            break
        time.sleep(1)


# ---------------------------------------------------------------------------------------
if __name__ == '__main__':
    # 加载库文件
    aiLib = cdll.LoadLibrary("/usr/lib/librknnx_api.so")
    modelEngPidList = []
    for i in range(2):
        print("start=========================================================", i)
        # 1）初始化模型引擎
        # modelType = "all"  # gdd yolo ssd pose hik
        modelType = "gdd"  # gdd yolo ssd pose hik
        threshold = c_float(1)
        threshold.value = 0.5
        print("creatModelEng")
        creatModelEng(modelType, threshold, modelEngPidList)
        # 2)
        print("imageDetection")
        cameraIndex = 1
        creatCameraThread(cameraIndex, modelEngPidList)
        # 3)
        print("end=========================================================", i)
    print(f'thread_list0:{thread_list},pidList:{modelEngPidList}')
    [thd.join() for thd in thread_list]
    print(f'thread_list1:{thread_list},pidList:{modelEngPidList}')
    [del_model_engine(id) for id in modelEngPidList]
    modelEngPidList.clear()
