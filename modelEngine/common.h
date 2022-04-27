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
#include <sys/syscall.h> 
#include <memory>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "getConfigInfo.h"
#include "src_json.h"
#include "rknn_api.h"

#define YUV         "YUV"
#define JPG         "JPG"

#define SSD          "ssd"
#define Yolo         "yolo"
#define GddModel     "GddModel"
#define HikangModel  "HikangModel"
#define BuiltIn      "built-in"
#define BodyPosture  "BodyPosture"

#define DEBUG_INFO printf("[info] %s,%d\r\n", __FUNCTION__, __LINE__);


using namespace std;
using namespace cv;


typedef enum
{
    RKNN_SUCCESS = 0,                   /* success */ 
    RKNN_ERR = -1, 
} RknnRet;

#define OBJ_NAME_MAX_SIZE 32
#define OBJ_NUMB_MAX_SIZE 64

typedef struct _BOX_RECT {
  int left;
  int right;
  int top;
  int bottom;
} BOX_RECT;

typedef struct __detect_result_t {
  char name[OBJ_NAME_MAX_SIZE]= { '\0' };
  BOX_RECT box;
  float prop;
} detect_result_t;

typedef struct _detect_result_group_t {
    int count;
    detect_result_t results[OBJ_NUMB_MAX_SIZE];
} detect_result_group_t;

typedef struct 
{
	  rknn_output outputs[3];
	  rknn_tensor_attr outputs_attr[3];
} rknn_output_attr;

typedef struct
{
	float threshold=0;       //obj score threshold
	// rknn model info
	int inputSize=0;
	int numResults=0;        //NUM_RESULTS         10407
	int numClass=0;          //NUM_CLASS			2

	string modelDir="";       //  model URL
	string jsonPath="";       //  video.json  full path
	string modelPath="";      //  .rknn  full path
	string labelPath="";      //  label.txt  full path
	string priboxPath="";     //  pribox.txt  full path
	string modelAlgType="";   //  ssd、yolo、built-in 、GddModel、HikangModel
}RknnDatas;
//===========================================================================================
