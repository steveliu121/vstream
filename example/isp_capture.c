#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include <userispapi.h>

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
	printf("\t-m\tcapture mjpeg\n");
	printf("\t-r <widthxheight>\tset resolution\n");
	printf("\t-y\tcatpure yuv\n");
}

/* XXX crop & framerate not supported by current driver */
int main(int argc, char *argv[])
{
	int i;
	int ret;
	int ch;
	int isp_chn;
	int width = 1280;
	int height = 720;
	int fps = 20;
	struct isp_attr attr;
	struct isp_buffer buffer;
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

	attr.width = width;
	attr.height = height;
	attr.bufnum = 3;
	attr.fps = fps;
	strcpy(attr.dev_name, DEVICE_NAME);
	if (o_yuv)
		attr.fmt = USER_PIX_FMT_YUYV;
	else
		attr.fmt = USER_PIX_FMT_MJPEG;


	isp_chn = user_isp_chn_create(&attr);
	if (isp_chn < 0)
		return -1;

	user_isp_chn_enable(isp_chn);

	int count = 10;
	char mjpeg_file[24];
	FILE *fp_mjpeg;
	FILE *fp_yuv;

	i = 0;

	if (o_yuv) {
		count = 1200;
		fp_yuv = fopen("yuv", "w+");
	}

	while (count-- && !g_exit) {
		ret = user_isp_buffer_poll(isp_chn);
		if (!ret) {
			usleep(10000);
			continue;
		}

		user_isp_buffer_recv(isp_chn, &buffer);

		if (o_mjpeg) {
			sprintf(mjpeg_file, "mjpeg_%d", i++);
			fp_mjpeg = fopen(mjpeg_file, "w+");

			fwrite(buffer.vm_addr, 1,
					buffer.size, fp_mjpeg);
			fclose(fp_mjpeg);
		}

		if (o_yuv)
			fwrite(buffer.vm_addr, 1,
					buffer.size, fp_yuv);
		user_isp_buffer_put(&buffer);
	}

	if (o_yuv)
		fclose(fp_yuv);

	user_isp_chn_disable(isp_chn);

	user_isp_chn_release(isp_chn);

	return 0;
}
