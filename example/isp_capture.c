#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <userv4l2capture.h>

#define DEVICE_NAME "/dev/video0"
struct buffer_info buf_info[MAX_BUF_NUM];

int g_exit;

void sig_handle(int sig)
{
	g_exit = 1;
}

void print_usage(void)
{
	printf("Usage:\n");
	printf("\t-h\tprint this help message\n");
	printf("\t-m\tcapture mjpeg\n");
	printf("\t-r <widthxheight>\tset resolution\n");
	printf("\t-y\tcatpure yuv\n");
}

/* XXX crop & framerate not supported by current driver */
int main(int argc, char *argv[])
{
	int fd_video;
	struct v4l2_format fmt;
	struct v4l2_cropcap cropcap;
	struct v4l2_streamparm parm;
	int buf_num;
	int i;
	int ret;
	int ch;
	int width = 1280;
	int height = 720;
	int o_yuv = 0;
	int o_mjpeg = 0;

	while ((ch = getopt(argc, argv, "hmr:y")) != -1) {
		switch (ch) {
		case 'm':
			o_mjpeg = 1;
			break;
		case 'r':
			if (sscanf(optarg, "%dx%d", &width, &height) != 2) {
				printf("Error resolution format\n");
				return -1;
			}
			break;
		case 'y':
			o_yuv = 1;
			break;
		case 'h':
			print_usage();
			return 0;
		default:
			print_usage();
			return 0;
		}
	}

	if (argc == 1) {
		print_usage();
		return 0;
	}

	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);

	fd_video = user_v4l2_open(DEVICE_NAME, O_RDWR);
	if (fd_video < 0)
		return -1;

	user_v4l2_query_cap(fd_video);

	user_v4l2_enum_fmt(fd_video);

	user_v4l2_cropcap(fd_video, &cropcap);

	user_v4l2_get_fmt(fd_video, &fmt);

	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	if (o_yuv)
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	else
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

	user_v4l2_set_fmt(fd_video, &fmt);

	user_v4l2_get_fmt(fd_video, &fmt);

	user_v4l2_get_param(fd_video, &parm);

	buf_num = 3;
	user_v4l2_request_bufs(fd_video, buf_num);

	for (i = 0; i < buf_num; i++) {
		user_v4l2_query_map_buf(fd_video, i, &buf_info[i]);
		user_v4l2_qbuf(fd_video, i);
	}

	user_v4l2_streamon(fd_video);

	int count = 10;
	int buf_index;
	char mjpeg_file[24];
	FILE *fp_mjpeg;
	FILE *fp_yuv;

	i = 0;

	if (o_yuv) {
		count = 1200;
		fp_yuv = fopen("yuv", "w+");
	}

	while (count-- && !g_exit) {
		ret = user_v4l2_poll(fd_video, 5);
		if (!ret) {
			usleep(10000);
			continue;
		}

		buf_index = user_v4l2_dqbuf(fd_video);

		if (o_mjpeg) {
			sprintf(mjpeg_file, "mjpeg_%d", i++);
			fp_mjpeg = fopen(mjpeg_file, "w+");

			fwrite(buf_info[buf_index].vm_start, 1,
					buf_info[buf_index].size, fp_mjpeg);
			fclose(fp_mjpeg);
		}

		if (o_yuv)
			fwrite(buf_info[buf_index].vm_start, 1,
					buf_info[buf_index].size, fp_yuv);

		user_v4l2_qbuf(fd_video, buf_index);
	}

	if (o_yuv)
		fclose(fp_yuv);

	user_v4l2_streamoff(fd_video);

	for (i = 0; i < buf_num; i++)
		user_v4l2_unmap(&buf_info[i]);

	user_v4l2_close(fd_video);

	return 0;
}
