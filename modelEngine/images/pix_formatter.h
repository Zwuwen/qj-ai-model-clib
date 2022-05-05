//
// Created by notI on 2022/4/29.
//

#ifndef AI_CLIB_PIX_FORMATTER_H
#define AI_CLIB_PIX_FORMATTER_H

#include <mutex>
#include "RockchipRga.h"
#include "RockchipFileOps.h"
#include "rga.h"
#include <chrono>
/** 图片参数 */
typedef struct ImageSpec{
    /** 像素数据 */
    uint8_t *data{};
    /** 像素宽度 */
    int width{};
    /** 像素高度 */
    int height{};
    /** 像素格式 */
    RgaSURF_FORMAT pix_format{};
} ImageSpec;

/** 像素格式转换 */
class pix_formatter final {
private:
    pix_formatter();

    mutable RockchipRga rkRga_;
    std::mutex chip_lock_;
    /** 转换中使用*/
    bo_t bo_src{}, bo_dst{};
    rga_info_t src{};
    rga_info_t dst{};
    /** 内存管理 */
    bool alloc_buffer();
    bool free_buffer();
    bool init_info_t();
    void clear_info_t();
public:
    pix_formatter &operator=(const pix_formatter &) = delete;

    pix_formatter(const pix_formatter &) = delete;

    virtual ~pix_formatter();

    /** Meyers' Singleton */
    static pix_formatter &get_formatter();

    bool is_ready() const {
        return rkRga_.RkRgaIsReady();
    };

    bool yuv_2_bgr(ImageSpec &in_spec, ImageSpec &out_spec);
};


#endif //AI_CLIB_PIX_FORMATTER_H
