#ifndef __USER_H264_H
#define __USER_H264_H

#include "bcm_host.h"
#include "ilclient.h"
#include <uservideoapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define H264_CHN_ID 0x10

struct h264_profile {
	int chn;
	struct h264_attr attr;
	COMPONENT_T *component;
	COMPONENT_T *component_list[5];
	ILCLIENT_T *client;
};

struct h264_profile g_h264_profile;

#ifdef __cplusplus
}
#endif
#endif
