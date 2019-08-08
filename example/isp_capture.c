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
	printf("\t-m\tcapture mjpeg\n");
	printf("\t-r <widthxheight>\tset resolution\n");
	printf("\t-y\tcatpure isp(yuv/rgb)\n");
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
	int o_isp = 0;
	int o_mjpeg = 0;

	while ((ch = getopt(argc, argv, "hmr:i")) != -1) {
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
		case 'i':
			o_isp = 1;
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
	if (o_isp)
		attr.fmt = USER_PIX_FMT_BGR24;
	else
		attr.fmt = USER_PIX_FMT_MJPEG;


	isp_chn = user_isp_chn_create(&attr);
	if (isp_chn < 0)
		return -1;

	user_isp_chn_enable(isp_chn);

	int count = 10;
	char mjpeg_file[24];
	char isp_file[24];
	FILE *fp_mjpeg;
	FILE *fp_isp;

	i = 0;

	while (count && !g_exit) {
		ret = user_isp_buffer_poll(isp_chn);
		if (ret) {
			usleep(10000);
			continue;
		}

		ret = user_isp_buffer_recv(isp_chn, &buffer);
		if (ret) {
			usleep(10000);
			continue;
		}

		if (o_mjpeg) {
			sprintf(mjpeg_file, "mjpeg_%d", i++);
			fp_mjpeg = fopen(mjpeg_file, "w+");

			fwrite(buffer.vm_addr, 1,
					buffer.size, fp_mjpeg);
			fclose(fp_mjpeg);
		}

		if (o_isp) {
			sprintf(isp_file, "isp_%d.raw", i++);
			fp_isp = fopen(isp_file, "w+");

			fwrite(buffer.vm_addr, 1,
					buffer.size, fp_isp);
			fclose(fp_isp);
		}

	printf("one frame[%d]\n", buffer.size);
		user_isp_buffer_put(&buffer);
	count--;
	}

	user_isp_chn_disable(isp_chn);

	user_isp_chn_release(isp_chn);

	return 0;
}
