//
// Created by notI on 2022/4/29.
//

#include <iostream>
#include "pix_formatter.h"
#include "RgaApi.h"
#include <sys/time.h>

bool pix_formatter::yuv_2_bgr(ImageSpec &in_spec,ImageSpec &out_spec) {
    /** RAII */
    std::lock_guard<std::mutex> lk(chip_lock_);
    int ret = 0;
    /** get bpp from format */
    float bpp = get_bpp_from_format(in_spec.pix_format);
    /** 参数检查 */
    if(bpp==0) return false;
    if(!in_spec.data) return false;
    /********** read data from *.bin file **********/
    memcpy(bo_src.ptr,in_spec.data,in_spec.height * in_spec.width * bpp);

    /********** rga_info_t Init **********/
    clear_info_t();
    /********** set the rect_info **********/
    rga_set_rect( &src.rect, 0, 0,  in_spec.width, in_spec.height,
            in_spec.width/*stride*/, in_spec.height, in_spec.pix_format);
    rga_set_rect(&dst.rect, 0, 0, out_spec.width, out_spec.height,
                 out_spec.width/*stride*/, out_spec.height, out_spec.pix_format);

    /************ set the rga_mod ,rotation\composition\scale\copy .... **********/
    /********** call rga_Interface **********/
    ret = rkRga_.RkRgaBlit(&src, &dst, nullptr);
    if (ret) {
        return false;
    }
    if(out_spec.data){
        memcpy(out_spec.data,bo_dst.ptr,out_spec.height * out_spec.width * 3);
    }else{
        out_spec.data = (uint8_t*)bo_dst.ptr;
    }
    return true;
}

pix_formatter& pix_formatter::get_formatter() {
    static pix_formatter formatter;
    return formatter;
}

pix_formatter::pix_formatter() {
    rkRga_.RkRgaInit();
    alloc_buffer();
}

pix_formatter::~pix_formatter() {
    rkRga_.RkRgaDeInit();
    free_buffer();
}

bool pix_formatter::alloc_buffer() {
    int ret= 0;
    /********** apply for src buffer and dst buffer **********/
    ret = rkRga_.RkRgaGetAllocBuffer(&bo_src,  3840, 2176, 32);
    if(ret) return false;
    ret = rkRga_.RkRgaGetAllocBuffer(&bo_dst, 3840, 2176, 32);
    if(ret) return false;

    /********** map buffer_address to userspace **********/
    rkRga_.RkRgaGetMmap(&bo_src);
    rkRga_.RkRgaGetMmap(&bo_dst);
    if(!init_info_t()) return false;
    return true;
}

bool pix_formatter::free_buffer() {
    rkRga_.RkRgaUnmap(&bo_src);
    rkRga_.RkRgaUnmap(&bo_dst);
    rkRga_.RkRgaFreeBuffer(bo_src.fd,&bo_src);
    rkRga_.RkRgaFreeBuffer(bo_dst.fd,&bo_dst);
    rkRga_.RkRgaFree(&bo_src);
    rkRga_.RkRgaFree(&bo_dst);
    return true;
}

bool pix_formatter::init_info_t() {
    int ret = 0;
    clear_info_t();
    /********** get src_Fd **********/
    ret = rkRga_.RkRgaGetBufferFd(&bo_src, &src.fd);
    printf("src.fd =%d \n", src.fd);
    if (ret) return false;
    /********** get dst_Fd **********/
    ret = rkRga_.RkRgaGetBufferFd(&bo_dst, &dst.fd);
    printf("dst.fd =%d \n", dst.fd);
    if (ret) return false;
    return true;
}

void pix_formatter::clear_info_t() {
    int tmp_fd = src.fd;
    memset(&src, 0, sizeof(rga_info_t));
    src.fd = tmp_fd;
    src.mmuFlag = 1;

    tmp_fd = dst.fd;
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = tmp_fd;
    dst.mmuFlag = 1;
}
