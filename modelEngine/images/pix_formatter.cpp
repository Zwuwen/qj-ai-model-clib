//
// Created by notI on 2022/4/29.
//

#include <iostream>
#include "pix_formatter.h"
#include "RgaApi.h"

bool pix_formatter::yuv_2_bgr(ImageSpec &in_spec,ImageSpec &out_spec) {
    /** RAII */
    std::lock_guard<std::mutex> lk(chip_lock_);
    int ret = 0;
    bo_t bo_src, bo_dst;
    /** get bpp from format */
    float bpp = get_bpp_from_format(in_spec.pix_format);
//    std::cout<<"bpp: "<<bpp<<std::endl;
    /********** apply for src buffer and dst buffer **********/
//    std::cout<<in_spec.width<<","<<in_spec.height<<std::endl;
    ret = rkRga_.RkRgaGetAllocBuffer(&bo_src,  in_spec.width, in_spec.height, 32);
    std::cout<<"bosrc_ret: "<<ret<<std::endl;

    ret = rkRga_.RkRgaGetAllocBuffer(&bo_dst, out_spec.width, out_spec.height, 32);
    std::cout<<"bodst_ret: "<<ret<<std::endl;

    /********** map buffer_address to userspace **********/
    rkRga_.RkRgaGetMmap(&bo_src);
    rkRga_.RkRgaGetMmap(&bo_dst);
    /********** read data from *.bin file **********/
    memcpy(bo_src.ptr,in_spec.data,in_spec.height * in_spec.width * bpp);

    /********** rga_info_t Init **********/
    rga_info_t src;
    rga_info_t dst;

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.mmuFlag = 1;

    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.mmuFlag = 1;

    /********** get src_Fd **********/
    ret = rkRga_.RkRgaGetBufferFd(&bo_src, &src.fd);
//    printf("src.fd =%d \n", src.fd);
    if (ret) {
        return false;
//        printf("rgaGetsrcFd fail : %s\n", strerror(errno));
    }
    /********** get dst_Fd **********/
    ret = rkRga_.RkRgaGetBufferFd(&bo_dst, &dst.fd);
//    printf("dst.fd =%d \n", dst.fd);
    if (ret) {
        return false;
//        printf("rgaGetdstFd error : %s\n", strerror(errno));
    }
//        /********** if not fd, try to check phyAddr and virAddr **************/
//        if (src.fd <= 0 || dst.fd <= 0) {
//            /********** check phyAddr and virAddr ,if none to get virAddr **********/
//            if ((src.phyAddr != nullptr || src.virAddr != nullptr) || src.hnd != NULL) {
//                //ret = RkRgaGetHandleMapAddress( gbs->handle, &src.virAddr );
//                printf("src.virAddr =%p\n", src.virAddr);
//                if (!src.virAddr) {
//                    printf("err! src has not fd and address for render ,Stop!\n");
//                    break;
//                }
//            }
//            /********** check phyAddr and virAddr ,if none to get virAddr **********/
//            if ((dst.phyAddr != nullptr || dst.virAddr != nullptr) || dst.hnd != NULL) {
//                //ret = RkRgaGetHandleMapAddress( gbd->handle, &dst.virAddr );
//                printf("dst.virAddr =%p\n", dst.virAddr);
//                if (!dst.virAddr) {
//                    printf("err! dst has not fd and address for render ,Stop!\n");
//                    break;
//                }
//            }
//        }
    /********** set the rect_info **********/
    rga_set_rect( &src.rect, 0, 0,  in_spec.width, in_spec.height,
            in_spec.width/*stride*/, in_spec.height, in_spec.pix_format);
    rga_set_rect(&dst.rect, 0, 0, out_spec.width, out_spec.height,
                 out_spec.width/*stride*/, out_spec.height, out_spec.pix_format);

    /************ set the rga_mod ,rotation\composition\scale\copy .... **********/
    /********** call rga_Interface **********/
    ret = rkRga_.RkRgaBlit(&src, &dst, nullptr);
    if (ret) {
//        printf("rgaFillColor error : %s\n", strerror(errno));
        return false;
    }
//    std::cout<<"convert finished"<<std::endl;
//    for(int i=0;i<10;++i){
//        std::cout<<((char*)bo_src.ptr)[i]<<",";
//    }
//    std::cout<<std::endl;
//
//    for(int i=0;i<10;++i){
//        std::cout<<((char*)bo_dst.ptr)[i]<<",";
//    }
//    std::cout<<std::endl;

    memcpy(out_spec.data,bo_dst.ptr,in_spec.height * in_spec.width * 3);
    return true;
}

pix_formatter& pix_formatter::get_formatter() {
    static pix_formatter formatter;
    return formatter;
}

pix_formatter::pix_formatter() {
    rkRga_.RkRgaInit();
}

pix_formatter::~pix_formatter() {
    rkRga_.RkRgaDeInit();
}
