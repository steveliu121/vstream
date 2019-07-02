#ifndef __USER_V4L2_CAPTURE_H
#define __USER_V4L2_CAPTURE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/videodev2.h>

#define v4l2pixfmtstr(x)	(char)((x) & 0xff), (char)(((x) >> 8) & 0xff), \
	(char)(((x) >> 16) & 0xff), (char)(((x) >> 24) & 0xff)
#define MAX_BUF_NUM 4

struct buffer_info {
	void * vm_start;
	unsigned int size;
};

int user_v4l2_open(const char *device_name, int flag);
void user_v4l2_close(const int dev);
int user_v4l2_poll(const int dev, int timeout);
int user_v4l2_query_cap(const int dev);
void user_v4l2_enum_fmt(const int dev);
int user_v4l2_get_fmt(const int dev, struct v4l2_format *fmt);
int user_v4l2_set_fmt(const int dev, struct v4l2_format *fmt);
int user_v4l2_get_param(const int dev, struct v4l2_streamparm *parm);
int user_v4l2_set_param(const int dev, struct v4l2_streamparm *parm);
int user_v4l2_cropcap(const int dev, struct v4l2_cropcap *cropcap);
int user_v4l2_get_crop(const int dev, struct v4l2_crop *crop);
int user_v4l2_set_crop(const int dev, struct v4l2_crop *crop);
int user_v4l2_request_bufs(const int dev, int buf_num);
int user_v4l2_query_map_buf(const int dev, int buf_index,
		struct buffer_info *buf_info);
int user_v4l2_unmap(struct buffer_info *buf_info);
int user_v4l2_qbuf(const int dev, int buf_index);
int user_v4l2_dqbuf(const int dev);
int user_v4l2_streamon(const int dev);
int user_v4l2_streamoff(const int dev);

#ifdef __cplusplus
{
#endif

#endif
