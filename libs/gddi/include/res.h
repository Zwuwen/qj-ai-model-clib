#pragma once
#include <string>
#include <vector>

class ClassifyRes
{
private:
public:
    ClassifyRes(){
        class_id_ = -1;
        prob_ = 0;
    };
    ~ClassifyRes(){};
    int class_id_;                 //分类名称
    float prob_;                  //分类置信度
    std::vector<std::string> name_vector_;  //分类名称
};


class DetectRes
{
private:
public:
    DetectRes(){
        class_id_ = -1;
        prob_ = 0;
    };
    ~DetectRes(){};
    int class_id_;		// 类别
    float prob_;      // 概率
    float bbox_[4];    //框框 bbox_[0]: x0 bbox_[1]: y0 bbox_[2]: width bbox_[3]:height
    std::vector<std::string> name_vector_;  //分类名称
};
