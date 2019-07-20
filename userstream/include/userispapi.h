#ifndef __USER_ISP_API_H
#define __USER_ISP_API_H

#ifdef __cplusplus
extern "C" {
#endif

enum pixfmt {
	USER_PIX_FMT_RGB24 = 0,
	USER_PIX_FMT_BGR24,
	USER_PIX_FMT_BGR32,
	USER_PIX_FMT_YU12,
	USER_PIX_FMT_YV12,
	USER_PIX_FMT_NV12,
	USER_PIX_FMT_NV21,
	USER_PIX_FMT_YUYV,
	USER_PIX_FMT_YVYU,
	USER_PIX_FMT_VYUY,
	USER_PIX_FMT_UYVY,
	USER_PIX_FMT_MJPEG,
	USER_PIX_FMT_JPEG,
	USER_PIX_FMT_H264,
};

struct isp_attr {
	int width;
	int height;
	int fps;
	enum pixfmt fmt;
	int bufnum;
	char dev_name[24];
};

struct isp_buffer {
	void *vm_addr;
	int size;
	int index;
	int refcount;
};

int  user_isp_chn_create(struct isp_attr *attr);
void user_isp_chn_release(const int chn);
void user_isp_chn_enable(const int chn);
void user_isp_chn_disable(const int chn);
int  user_isp_buffer_poll(const int ch);
int  user_isp_buffer_recv(const int chn, struct isp_buffer *buffer);
void user_isp_buffer_get(struct isp_buffer *buf);
void user_isp_buffer_put(struct isp_buffer *buf);

#ifdef __cplusplus
}
#endif
#endif

