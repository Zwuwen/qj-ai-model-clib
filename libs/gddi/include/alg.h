#pragma once
#include <memory>
#include <assert.h>
#include "res.h"

typedef std::string ChannelID;

class Tensor
{
public:
    int width_;
    int height_;
    int channel_;
    float* buffer_;
    
    Tensor(unsigned int width,unsigned int height,unsigned int channel);
    Tensor(unsigned int width);
    ~Tensor();
};

typedef enum
{
    UN_PROCESS = 0,         /* have no do any Process*/
    PRE_PROCESS,            /* preprocess end*/
    PROCESS,                /* Process end*/
    POST_PROCESS,           /* post Process end*/
    PROCESS_END,            /* all Process end*/
} TransferBufferStatus;

typedef enum
{
    CLASSIFY_TASK = 0,      /* classify task */  
    DETECT_TASK = 1         /* detect task */  
} Task;

typedef enum
{
    GDDI_SUCCESS = 0,                   /* success */ 
    GDDI_ERR = -1, 
    GDDI_ERR_DECRYPT_MODE = -2,         /* model decrypt error */  
    GDDI_ERR_INPUT_IMG = -3,            /* input img error*/
    GDDI_ERR_OVERDUE = -4,              /* model over time*/
    GDDI_ERR_INPUT_SIZE = -5,           /* batch api input must be batch size.*/
    GDDI_ERR_MALLOC = -6,               /* malloc mem err*/
    GDDI_ERR_RKNN = -7
} GddiRet;

class TransferBuffer
{
public:
    /****************************
     * func: TransferBuffer
     * param: channel_id_[input]: exp: '0' or any string.
     *        rows[input]: img rows.
     *        cols[input]: img cols.
     *        data[input]: use opencv CV_8UC8 BRG type.
     *        img_id[framId]: exp:0
     * return:
     * des: create TransferBuffer class
    *****************************/
    TransferBuffer(ChannelID channel_id, int rows, int cols, void* data, uint64_t img_id);
     /****************************
     * func: ~TransferBuffer
     * param: 
     * return:
     * des: release TransferBuffer class
    *****************************/
    ~TransferBuffer();
    TransferBufferStatus  transfer_buf_status_;
    ChannelID              channel_id_;
    std::shared_ptr<void>        input_img_;
    std::shared_ptr<void>        gpu_input_img_;
    std::vector<std::shared_ptr<Tensor>> tensors_vector_;
    float* input_buf_;
};

class AlgOutput
{
public:
    AlgOutput(){};
    ~AlgOutput(){};
    Task task_;
    ChannelID channel_id_;   /* channel ID*/
    bool has_result_;        /* if has result,has_result_ is true.*/
    std::shared_ptr<void> data_;            /* reuslt detail.*/
};

class AlgImpl
{
public:
    /****************************
     * func: AlgImpl
     * param: 
     * return:
     * des: create AlgImpl class
    *****************************/
    AlgImpl();
    /****************************
     * func: ~AlgImpl
     * param: 
     * return:
     * des: release AlgImpl class
    *****************************/
    ~AlgImpl();               

    /****************************
     * func: Init
     * param: device_id[input]: 0.
     *        config[input]: alg param input.
     *        model_name[input]: model patch.
     *        batch[input]: batch num.
     * return:GddiRet
     * des: release AlgImpl class
    *****************************/
    GddiRet Init(const int device_id, const std::string& config,const std::string model_name,unsigned int batch);
    /****************************
     * func: GetVersion
     * param: 
     * return: version.
     * des: get alg version
    *****************************/
    static std::string GetVersion();
    /****************************
     * func: getGxtFile
     * param: gxt_file_path[in]: gxt file save path.
     * return: void.
     * des: create Gxt File that is to give gddi release model.
    *****************************/
    static GddiRet getGxtFile(std::string gxt_file_path);
    /****************************
     * func: PreProcess
     * param: TransferBuffer*[input]:input to alg.
     * return: GddiRet
     * des: alg pre Process.
    *****************************/
    GddiRet PreProcess(std::shared_ptr<TransferBuffer> transfer_buf);
    /****************************
     * func: Process
     * param: TransferBuffer*[input]:input to alg.
     * return: GddiRet
     * des: alg Process.
    *****************************/
    GddiRet Process(std::shared_ptr<TransferBuffer>transfer_buf);

    /****************************
     * func: PostProcess
     * param: TransferBuffer*[input]:input to alg.
     * param: NN_OUTPUT[output]:alg output.
     * return: GddiRet
     * des: alg post Process.
    *****************************/
    GddiRet PostProcess(std::shared_ptr<TransferBuffer> transfer_buf, AlgOutput &res);

    /****************************
     * func: Reset
     * return: GddiRet
     * des: Reset alg.
    *****************************/
    GddiRet Reset();

    /****************************
     * func: PreProcess
     * param: transfer_buffer_vector[input]:input to alg.
     * return: GddiRet
     * des: alg pre Process for a batch.
    *****************************/
    GddiRet PreProcessBatch(std::vector<std::shared_ptr<TransferBuffer> > transfer_buffer_vector);

    /****************************
     * func: Process
     * param: transfer_buffer_vector[input]:input to alg.
     * return: GddiRet
     * des: alg Process for a batch.
    *****************************/
    GddiRet ProcessBatch(std::vector<std::shared_ptr<TransferBuffer> > transfer_buffer_vector);

    /****************************
     * func: PostProcess
     * param: transfer_buffer_vector[input]:input to alg.
     * param: alg_res_vector[output]:alg output vector.
     * return: GddiRet
     * des: alg post Process.
    *****************************/
    GddiRet PostProcessBatch(std::vector<std::shared_ptr<TransferBuffer> > transfer_buffer_vector, std::vector<AlgOutput> &alg_res_vector);

private:
    std::shared_ptr<void> data_;
};
