#include <fcntl.h>
#include <string.h>

#include <linux/videodev2.h>

#include <userisp.h>
#include <userv4l2capture.h>

int user_isp_chn_create(struct isp_attr *attr)
{
	int fd_video = -1;
	struct v4l2_format fmt;
	struct v4l2_streamparm parm;
	int i;

	fd_video = user_v4l2_open(attr->dev_name, O_RDWR);
	if (fd_video < 0)
		return -1;

	user_v4l2_get_fmt(fd_video, &fmt);

	fmt.fmt.pix.width = attr->width;
	fmt.fmt.pix.height = attr->height;
	/* XXX supported by raspberrypi */
	switch (attr->fmt) {
		case USER_PIX_FMT_RGB24:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
			break;
		case USER_PIX_FMT_BGR24:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
			break;
		case USER_PIX_FMT_BGR32:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;
			break;
		/* YUV420P */
		case USER_PIX_FMT_YU12:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
			break;
		case USER_PIX_FMT_YV12:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420;
			break;
		/* YUV420SP */
		case USER_PIX_FMT_NV12:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
			break;
		case USER_PIX_FMT_NV21:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV21;
			break;
		case USER_PIX_FMT_YUYV:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
			break;
		case USER_PIX_FMT_YVYU:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU;
			break;
		case USER_PIX_FMT_VYUY:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_VYUY;
			break;
		case USER_PIX_FMT_UYVY:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
			break;
		case USER_PIX_FMT_MJPEG:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
			break;
		case USER_PIX_FMT_JPEG:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
			break;
		case USER_PIX_FMT_H264:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
			break;
	}

	user_v4l2_set_fmt(fd_video, &fmt);

	user_v4l2_get_param(fd_video, &parm);

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = attr->fps;

	user_v4l2_set_param(fd_video, &parm);

	user_v4l2_request_bufs(fd_video, attr->bufnum);

	for (i = 0; i < attr->bufnum; i++) {
		user_v4l2_query_map_buf(fd_video, i,
				&g_isp_profile.buf_info[i]);
		user_v4l2_qbuf(fd_video, i);
	}

	g_isp_profile.chn = fd_video;
	memcpy(&g_isp_profile.attr, attr, sizeof(*attr));

	return fd_video;
}

void user_isp_chn_release(const int chn)
{
	int i;

	for (i = 0; i < g_isp_profile.attr.bufnum; i++)
		user_v4l2_unmap(&g_isp_profile.buf_info[i]);

	user_v4l2_close(chn);
}

void user_isp_chn_enable(const int chn)
{
	user_v4l2_streamon(chn);
}

void user_isp_chn_disable(const int chn)
{
	user_v4l2_streamoff(chn);
}

int user_isp_buffer_poll(const int chn)
{
	return user_v4l2_poll(chn, 0);
}

int user_isp_buffer_recv(const int chn, struct isp_buffer *buffer)
{
	int index;

	index = user_v4l2_dqbuf(chn);

	buffer->vm_addr = g_isp_profile.buf_info[index].vm_start;
	buffer->size = g_isp_profile.buf_info[index].size;
	buffer->index = index;
	buffer->refcount = 1;

	return 0;
}

void user_isp_buffer_get(struct isp_buffer *buf)
{
	buf->refcount++;
}

void user_isp_buffer_put(struct isp_buffer *buf)
{
	buf->refcount--;
	if (buf->refcount == 0)
		user_v4l2_qbuf(g_isp_profile.chn, buf->index);
}
