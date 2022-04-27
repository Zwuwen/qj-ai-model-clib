CROSS_COMPILE	?= aarch64-linux-
CC		= ${CROSS_COMPILE}gcc
C++		= ${CROSS_COMPILE}g++
STRIP	= ${CROSS_COMPILE}strip
LD		= ${CROSS_COMPILE}ld
AR		=$(CROSS_COMPILE)ar

RKNN_API=./libs/librknn_api
OPENCV=./libs/opencv
CURL=./libs/curl
ROCK_X=./libs/rock_x
OPENSSL = ./libs/openssl
RGA = ./libs/rga
GDDI = ./libs/gddi

INCLUDEDIR=-I./include/ 
INCLUDEDIR+=-I./common/json/
INCLUDEDIR+=-I./common/config/
INCLUDEDIR+=-I${RKNN_API}/include
INCLUDEDIR+=-I${OPENCV}/include
INCLUDEDIR+=-I${CURL}/include
INCLUDEDIR+=-I${ROCK_X}/include
INCLUDEDIR+=-I${OPENSSL}/include
INCLUDEDIR+=-I${RGA}/include
INCLUDEDIR+=-I${GDDI}/include 

#C 编译器选型
CFLAGS	+= -g -Wall -fPIC
CFLAGS	+= -std=c++11
# 调试工具
#CFLAGS  += -fsanitize=address -fno-omit-frame-pointer

#LDFLAGS  链接器从哪里寻找库文件
LIBDIR	= -L${RKNN_API}/lib64/ -L${OPENCV}/lib64/ -L${CURL}/lib/ -L./libs -L${ROCK_X}/lib64/   -L${RGA}/lib/  -L${OPENSSL}/lib/ -L${GDDI}/lib/
LDFLAGS	+=  -lrt -ldl -lm  -s -w -Wl,-gc-sections
LDFLAGS	+= -lrknn_api 
LDFLAGS	+= -lrockx 
LDFLAGS	+= -lopencv_imgcodecs 
LDFLAGS	+= -lopencv_imgproc 
LDFLAGS	+= -lopencv_core
LDFLAGS	+= -lopencv_highgui  
LDFLAGS	+= -lcurl 
LDFLAGS	+= -Wl,-rpath,./lib -lpthread
LDFLAGS	+= -lgddi_rockchip_sdk
LDFLAGS	+= -lrga

OBJ := $(patsubst %.cpp,%.o,$(wildcard ./modelEngine/*.cpp))
OBJ += $(patsubst %.cpp,%.o,$(wildcard ./modelEngine/gdd/*.cpp))
OBJ += $(patsubst %.cpp,%.o,$(wildcard ./modelEngine/hik/*.cpp))
OBJ += $(patsubst %.cpp,%.o,$(wildcard ./modelEngine/rknnPose/*.cpp))
OBJ += $(patsubst %.cpp,%.o,$(wildcard ./modelEngine/ssd/*.cpp))
OBJ += $(patsubst %.cpp,%.o,$(wildcard ./modelEngine/yolo/*.cpp))
OBJ += $(patsubst %.cpp,%.o,$(wildcard ./common/config/*.cpp))
OBJ += $(patsubst %.c,%.o,$(wildcard ./common/json/*.c))

all:${OBJ}
	$(CC) $^ ${LIBDIR} $(LDFLAGS) -shared -fPIC -o ./librknnx_api.so
	$(AR) rcs $@ $^ 

%.o:%.cpp
	$(C++) -std=c++11 -c $< -o $@ ${INCLUDEDIR}  ${CFLAGS}
%.o:%.c
	$(CC) -std=c++11 -c $< -o $@ ${INCLUDEDIR}  ${CFLAGS}

clean:
	rm -rf *.o ${OBJ} $@ ${TARG}
