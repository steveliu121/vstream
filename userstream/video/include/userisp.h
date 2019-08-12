#ifndef __USER_ISP_H
#define __USER_ISP_H

#include <uservideoapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ISP_BUF_NUM 4
struct isp_buffer_info {
	void * vm_start;
	unsigned int size;
};

struct isp_profile {
	int chn;
	struct isp_attr attr;
	struct isp_buffer_info buf_info[MAX_ISP_BUF_NUM];
};

struct isp_profile g_isp_profile;


#ifdef __cplusplus
}
#endif
#endif

