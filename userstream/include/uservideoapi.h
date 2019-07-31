#ifndef __USER_VIDEO_API_H
#define __USER_VIDEO_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum pixfmt {
	USER_PIX_FMT_RGB24 = 0,
	USER_PIX_FMT_BGR24,
	USER_PIX_FMT_BGR32,
	/* YUV 4:2:0 */
	/* YUV420P*/
	USER_PIX_FMT_YU12,
	USER_PIX_FMT_YV12,
	/* YUV420SP*/
	USER_PIX_FMT_NV12,
	USER_PIX_FMT_NV21,
	/* YUV 4:2:2 */
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

enum bitrate_type {
	USER_BITRATE_CBR = 0,
	USER_BITRATE_VBR,
};

struct h264_attr {
	uint32_t width;
	uint32_t height;
	enum pixfmt fmt;
	uint32_t fps;
	uint32_t gop;
	uint32_t bitrate;
	enum bitrate_type bitrate_mode;
	uint32_t profile;
	uint32_t level;
};

struct h264_buffer {
	void *vm_addr;
	int size;
	int index;
	int refcount;
	void *priv;
};

int  user_isp_chn_create(struct isp_attr *attr);
void user_isp_chn_release(const int chn);
void user_isp_chn_enable(const int chn);
void user_isp_chn_disable(const int chn);
int  user_isp_buffer_poll(const int ch);
int  user_isp_buffer_recv(const int chn, struct isp_buffer *buffer);
void user_isp_buffer_get(struct isp_buffer *buffer);
void user_isp_buffer_put(struct isp_buffer *buffer);

int user_h264_chn_create(struct h264_attr *attr);
int user_h264_chn_release(const int chn);
int user_h264_chn_enable(const int chn);
int user_h264_chn_disable(const int chn);
int user_h264_buffer_poll(const int chn);
int user_h264_buffer_recv(const int chn, struct h264_buffer *buffer);
int user_h264_buffer_send(const int chn, struct h264_buffer *buffer);
void user_h264_buffer_get(struct h264_buffer *buffer);
void user_h264_buffer_put(struct h264_buffer *buffer);


#ifdef __cplusplus
}
#endif
#endif

