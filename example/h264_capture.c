#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include <uservideoapi.h>

#define DEVICE_NAME "/dev/video0"

int g_exit;

void sig_handle(int sig)
{
	g_exit = 1;
}

void print_usage(void)
{
	printf("Usage:\n");
	printf("\t-h\tprint this help message\n");
	printf("\t-r <widthxheight>\tset resolution\n");
}

/* XXX crop & framerate not supported by current driver */
int main(int argc, char *argv[])
{
	int ret;
	int ch;
	int isp_chn = -1;
	int h264_chn = -1;
	int width = 1280;
	int height = 720;
	int fps = 20;
	struct isp_attr isp_attr;
	struct isp_buffer isp_buffer;
	struct h264_attr h264_attr;
	struct h264_buffer h264_buffer;
	FILE *fp_h264;


	while ((ch = getopt(argc, argv, "hr:")) != -1) {
		switch (ch) {
		case 'r':
			if (sscanf(optarg, "%dx%d", &width, &height) != 2) {
				printf("Error resolution format\n");
				return -1;
			}
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

	isp_attr.width = width;
	isp_attr.height = height;
	isp_attr.bufnum = 3;
	isp_attr.fps = fps;
	strcpy(isp_attr.dev_name, DEVICE_NAME);
	isp_attr.fmt = USER_PIX_FMT_RGB24;

	h264_attr.width = width;
	h264_attr.height = height;
	h264_attr.fmt = USER_PIX_FMT_BGR24;
	h264_attr.fps = fps;
	h264_attr.bitrate = 1000000;
	h264_attr.bitrate_mode = USER_BITRATE_VBR;


	isp_chn = user_isp_chn_create(&isp_attr);
	if (isp_chn < 0)
	    goto exit;

	h264_chn = user_h264_chn_create(&h264_attr);
	if (h264_chn < 0)
	    goto exit;

	user_isp_chn_enable(isp_chn);
	ret = user_h264_chn_enable(h264_chn);
	if (ret < 0)
	    goto exit;

	fp_h264 = fopen("out.h264", "w+");

	while (!g_exit) {
		ret = user_isp_buffer_poll(isp_chn);
		if (ret) {
			usleep(10000);
			continue;
		}

		user_isp_buffer_recv(isp_chn, &isp_buffer);

		h264_buffer.vm_addr = isp_buffer.vm_addr;
		h264_buffer.size = isp_buffer.size;
		user_h264_buffer_send(h264_chn, &h264_buffer);

		user_isp_buffer_put(&isp_buffer);

		/* TODO this api dose nothing */
		user_h264_buffer_poll(h264_chn);

		user_h264_buffer_recv(h264_chn, &h264_buffer);

		fwrite(h264_buffer.vm_addr, 1, h264_buffer.size, fp_h264);

		user_h264_buffer_put(&h264_buffer);
	}

	fclose(fp_h264);

exit:
	if (h264_chn >= 0) {
	    user_h264_chn_disable(h264_chn);
	    user_h264_chn_release(h264_chn);
	}

	if (isp_chn >= 0) {
	    user_isp_chn_disable(isp_chn);
	    user_isp_chn_release(isp_chn);
	}

	return 0;
}
