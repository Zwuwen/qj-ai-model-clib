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

//    /********** apply for src buffer and dst buffer **********/
//    ret = rkRga_.RkRgaGetAllocBuffer(&bo_src,  in_spec.width, in_spec.height, 32);
//    if(ret) return false;
//    ret = rkRga_.RkRgaGetAllocBuffer(&bo_dst, out_spec.width, out_spec.height, 32);
//    if(ret) return false;
//
//    /********** map buffer_address to userspace **********/
//    rkRga_.RkRgaGetMmap(&bo_src);
//    rkRga_.RkRgaGetMmap(&bo_dst);
    /********** read data from *.bin file **********/
    memcpy(bo_src.ptr,in_spec.data,in_spec.height * in_spec.width * bpp);

    /********** rga_info_t Init **********/
    int tmp_fd = src.fd;
    memset(&src, 0, sizeof(rga_info_t));
//    src.fd = -1;
    src.mmuFlag = 1;
    src.fd = tmp_fd;

    tmp_fd = dst.fd;
    memset(&dst, 0, sizeof(rga_info_t));
//    dst.fd = -1;
    dst.mmuFlag = 1;
    dst.fd = tmp_fd;

//    /********** get src_Fd **********/
//    ret = rkRga_.RkRgaGetBufferFd(&bo_src, &src.fd);
//    printf("src.fd =%d \n", src.fd);
//    if (ret) return false;
//    /********** get dst_Fd **********/
//    ret = rkRga_.RkRgaGetBufferFd(&bo_dst, &dst.fd);
//    printf("dst.fd =%d \n", dst.fd);
//    if (ret) return false;
//        printf("rgaGetdstFd error : %s\n", strerror(errno));

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

    if(out_spec.data){
        memcpy(out_spec.data,bo_dst.ptr,out_spec.height * out_spec.width * 3);
    }else{
        out_spec.data = (uint8_t*)bo_dst.ptr;
    }
//    rkRga_.RkRgaUnmap(&bo_src);
//    rkRga_.RkRgaUnmap(&bo_dst);
//    rkRga_.RkRgaFreeBuffer(bo_src.fd,&bo_src);
//    rkRga_.RkRgaFreeBuffer(bo_dst.fd,&bo_dst);
//    rkRga_.RkRgaFree(&bo_src);
//    rkRga_.RkRgaFree(&bo_dst);
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

    /********** rga_info_t Init **********/
    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.mmuFlag = 1;

    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.mmuFlag = 1;

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

bool pix_formatter::free_buffer() {
    rkRga_.RkRgaUnmap(&bo_src);
    rkRga_.RkRgaUnmap(&bo_dst);
    rkRga_.RkRgaFreeBuffer(bo_src.fd,&bo_src);
    rkRga_.RkRgaFreeBuffer(bo_dst.fd,&bo_dst);
    rkRga_.RkRgaFree(&bo_src);
    rkRga_.RkRgaFree(&bo_dst);
    return true;
}
