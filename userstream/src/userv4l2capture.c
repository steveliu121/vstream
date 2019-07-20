#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include <userv4l2capture.h>

int user_v4l2_open(const char *device_name, int flag)
{
	int fd_video;

	fd_video = open(device_name, flag);
	if (fd_video < 0)
		printf("v4l2 open video device fail, %s\n", strerror(errno));

	return fd_video;
}

void user_v4l2_close(const int dev)
{
	close(dev);
}

/* timeout: xx ms*/
int user_v4l2_poll(const int dev, int timeout)
{
	int ret = 0;
	struct pollfd pfd;

	pfd.fd = dev;
	pfd.events = POLLIN;
	ret = poll(&pfd, 1, timeout);
	if (ret <= 0)
		return -1;

	if (pfd.revents & POLLIN)
		return 0;

	return -1;
}

int user_v4l2_query_cap(const int dev)
{
	int ret;
	struct v4l2_capability cap;

	ret = ioctl(dev, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("v4l2 ioctl QUERYCAP fail, %s\n", strerror(errno));
		return ret;
	}

	printf("\nv4l2 capability:\n");
	printf("\tdriver: %s, card: %s\n", cap.driver, cap.card);
	printf("\tbus_info:%s\n", cap.bus_info);
	printf("\tversion: %u.%u.%u\n",
			(cap.version >> 16) & 0xff,
			(cap.version >> 8) & 0xff,
			cap.version & 0xff);
	printf("\tcapabilities: 0x%8x\n\n", cap.capabilities);

	return ret;
}

/* enum fmt for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
void user_v4l2_enum_fmt(const int dev)
{
	struct v4l2_fmtdesc fmtdesc;

	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	printf("\nv4l2 support format:\n");

	while (ioctl(dev, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
		printf("\tdescription:%s, pixelformat:%c%c%c%c\n",
				fmtdesc.description,
				v4l2pixfmtstr(fmtdesc.pixelformat));
		fmtdesc.index++;
	}
	printf("\n");
}

/* get fmt for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
int user_v4l2_get_fmt(const int dev, struct v4l2_format *fmt)
{
	int ret;

	if (!fmt)
		return -1;

	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_G_FMT, fmt);
	if (ret < 0) {
		printf("v4l2 ioctl G_FMT fail, %s\n", strerror(errno));
		return ret;
	}

	printf("\nv4l2 current format:\n");
	printf("\twidth:%d, height:%d\n",
			fmt->fmt.pix.width, fmt->fmt.pix.height);
	printf("\tpixelformat:%c%c%c%c\n\n", v4l2pixfmtstr(fmt->fmt.pix.pixelformat));

	return ret;
}

int user_v4l2_set_fmt(const int dev, struct v4l2_format *fmt)
{
	int ret;

	if (!fmt)
		return -1;

	ret = ioctl(dev, VIDIOC_S_FMT, fmt);
	if (ret < 0) {
		printf("v4l2 ioctl S_FMT fail, %s\n", strerror(errno));
		return ret;
	}

	return ret;
}

/* get fmt for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
int user_v4l2_get_param(const int dev, struct v4l2_streamparm *parm)
{
	int ret;

	if (!parm)
		return -1;

	parm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_G_PARM, parm);
	if (ret < 0) {
		printf("v4l2 ioctl G_PARM fail, %s\n", strerror(errno));
		return ret;
	}

	printf("\nv4l2 streamparam:\n");
	printf("\tframerate:(%d / %d)\n\n",
			parm->parm.capture.timeperframe.numerator,
			parm->parm.capture.timeperframe.denominator);

	return ret;
}

/* set fmt for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* XXX not support yet */
int user_v4l2_set_param(const int dev, struct v4l2_streamparm *parm)
{
	int ret;

	if (!parm)
		return -1;

	parm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_S_PARM, parm);
	if (ret < 0) {
		printf("v4l2 ioctl S_PARM fail, %s\n", strerror(errno));
		return ret;
	}

	return ret;
}

/* crop capture for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* ioctl only support reading */
int user_v4l2_cropcap(const int dev, struct v4l2_cropcap *cropcap)
{
	int ret;

	cropcap->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_CROPCAP, cropcap);
	if (ret < 0) {
		printf("v4l2 ioctl CROPCAP fail, %s\n", strerror(errno));
		return ret;
	}

	printf("\nv4l2 cropcap:\n");
	printf("\tbounds:(%d, %d), (%d x %d)\n",
			cropcap->bounds.left,
			cropcap->bounds.top,
			cropcap->bounds.width,
			cropcap->bounds.height);
	printf("\tdefrect:(%d, %d), (%d x %d)\n",
			cropcap->defrect.left,
			cropcap->defrect.top,
			cropcap->defrect.width,
			cropcap->defrect.height);
	printf("\tpixelaspect:(%d / %d)\n\n",
			cropcap->pixelaspect.numerator,
			cropcap->pixelaspect.denominator);

	return 0;
}

/* get crop for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* XXX not support yet */
int user_v4l2_get_crop(const int dev, struct v4l2_crop *crop)
{
	int ret;

	if (!crop)
		return -1;

	crop->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_G_CROP, crop);
	if (ret < 0) {
		printf("v4l2 ioctl G_CROP fail, %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

/* set crop for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* XXX not support yet */
int user_v4l2_set_crop(const int dev, struct v4l2_crop *crop)
{
	int ret;

	if (!crop)
		return -1;

	crop->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_S_CROP, crop);
	if (ret < 0) {
		printf("v4l2 ioctl S_CROP fail, %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

/* request buffers for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* and V4L2_MEMORY_MMAP only*/
int user_v4l2_request_bufs(const int dev, int buf_num)
{
	int ret;
	struct v4l2_requestbuffers reqbufs;

	if ((buf_num <= 0) || (buf_num > MAX_ISP_BUF_NUM))
		return -1;

	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbufs.memory = V4L2_MEMORY_MMAP;
	reqbufs.count = buf_num;

	ret = ioctl(dev, VIDIOC_REQBUFS, &reqbufs);
	if (ret < 0) {
		printf("v4l2 ioctl REQBUFS fail, %s\n", strerror(errno));
		return ret;
	}

	return ret;
}

/* query buffers for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* and V4L2_MEMORY_MMAP only*/
int user_v4l2_query_map_buf(const int dev, int buf_index,
		struct isp_buffer_info *buf_info)
{
	int ret;
	struct v4l2_buffer buffer;

	if (!buf_info)
		return -1;

	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.index = buf_index;
	buffer.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(dev, VIDIOC_QUERYBUF, &buffer);
	if (ret < 0) {
		printf("v4l2 ioctl QUERYBUF fail, %s\n", strerror(errno));
		return ret;
	}

	buf_info->size = buffer.length;

	buf_info->vm_start = mmap(NULL, buf_info->size, PROT_READ | PROT_WRITE,
			MAP_SHARED, dev, buffer.m.offset);
	if (buf_info->vm_start == MAP_FAILED) {
		printf("v4l2 mmap failed, %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int user_v4l2_unmap(struct isp_buffer_info *buf_info)
{
	if (buf_info->vm_start && buf_info->size)
		munmap(buf_info->vm_start, buf_info->size);
	else
		return -1;

	return 0;
}

/* queue buffers for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* and V4L2_MEMORY_MMAP only*/
int user_v4l2_qbuf(const int dev, int buf_index)
{
	int ret;
	struct v4l2_buffer buffer;

	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	buffer.index = buf_index;

	ret = ioctl(dev, VIDIOC_QBUF, &buffer);
	if (ret < 0) {
		printf("v4l2 ioctl QBUF fail, %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

/* dequeue buffers for type V4L2_BUF_TYPE_VIDEO_CAPTURE only */
/* and V4L2_MEMORY_MMAP only*/
/* return buf_index: >= 0 && <= MAX_BUF_NUM - 1 */
int user_v4l2_dqbuf(const int dev)
{
	int ret;
	struct v4l2_buffer buffer;

	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(dev, VIDIOC_DQBUF, &buffer);
	if (ret < 0) {
		printf("v4l2 ioctl DQBUF fail, %s\n", strerror(errno));
		return ret;
	}

	return buffer.index;
}

int user_v4l2_streamon(const int dev)
{
	int ret;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		printf("v4l2 ioctl STREAMON fail, %s\n", strerror(errno));
		return ret;
	}

	return 0;
}

int user_v4l2_streamoff(const int dev)
{
	int ret;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		printf("v4l2 ioctl STREAMOFF fail, %s\n", strerror(errno));
		return ret;
	}

	return 0;
}
