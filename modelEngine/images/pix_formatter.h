//
// Created by notI on 2022/4/29.
//

#ifndef AI_CLIB_PIX_FORMATTER_H
#define AI_CLIB_PIX_FORMATTER_H

#include <mutex>
#include "RockchipRga.h"
#include "RockchipFileOps.h"
#include "rga.h"

/** 图片参数 */
typedef struct ImageSpec{
    uint8_t *data{};
    int width{};
    int height{};
    RgaSURF_FORMAT pix_format{};
} ImageSpec;

/** 像素格式转换 */
class pix_formatter final {
private:
    pix_formatter();

    mutable RockchipRga rkRga_;
    std::mutex chip_lock_;
public:
    pix_formatter &operator=(const pix_formatter &) = delete;

    pix_formatter(const pix_formatter &) = delete;

    virtual ~pix_formatter();

    /** Meyers' Singleton */
    static pix_formatter &get_formatter();

    bool is_ready() const {
        return rkRga_.RkRgaIsReady();
    };

    int yuv_2_bgr(ImageSpec &in_spec, ImageSpec &out_spec);
};


#endif //AI_CLIB_PIX_FORMATTER_H
