#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include <useraudioapi.h>

int g_exit;

void sig_handle(int sig)
{
	g_exit = 1;
}

void print_usage(void)
{
	printf("Usage:\n");
	printf("\t-h\tprint this help message\n");
	printf("\t-c <capture.pcm>\tcapture voice\n");
	printf("\t-d <dev>\tsound card\n");
	printf("\t-f <fromat>\tset format\n");
	printf("\t-n <channels>\tset channels\n");
	printf("\t-p <playback.pcm>\tplayback voice\n");
	printf("\t-r <samplerate>\tset sample rate\n");
}

int main(int argc, char *argv[])
{
	int i;
	int ret;
	int ch;
	int capture_chn = -1;
	int playback_chn = -1;
	char *dev, *capture_file, *playback_file;
	FILE *fp_capture, *fp_playback;
	int o_capture = 0, o_playback = 0;
	uint32_t format, channels, rate;
	unsigned long period_frames;
	struct audio_attr attr;
	struct audio_buffer buffer;

	format = 16;
	channels = 1;
	rate = 8000;
	sprintf(dev, "%s", "plughw:0,0");
	memset(&attr, 0, sizeof(attr));

	while ((ch = getopt(argc, argv, "hc:d:f:n:p:r:")) != -1) {
		switch (ch) {
		case 'c':
			o_capture = 1;
			sprintf(capture_file, "%s", optarg);
			break;
		case 'd':
			sprintf(dev, "%s", optarg);
			break;
		case 'f':
			format = atoi(optarg);
			break;
		case 'n':
			channels = atoi(optarg);
			break;
		case 'p':
			o_playback = 1;
			sprintf(playback_file, "%s", optarg);
			break;
		case 'r':
			rate = atoi(optarg);
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

	printf("dump audio attr:format[%d], rate[%d], channels[%d], dev[%s]\n",
			format, rate, channels, dev);
	attr.format = format;
	attr.channels = channels;
	attr.rate = rate;
	strcpy(attr.dev_node, dev);
	period_frames = rate / 50; /* period time: 20ms */
	attr.period_frames = period_frames;

	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);

	if (o_capture) {
		fp_capture = fopen(capture_file, "w+");
		capture_chn = user_audio_capture_chn_create(&attr);
		if (capture_chn < 0)
			goto exit;
		ret = user_audio_capture_chn_enable(capture_chn);
		if (ret < 0)
			goto exit;

		while (!g_exit) {
			ret = user_audio_capture_buffer_poll(capture_chn);
			if (ret < 0) {
				usleep(10000);
				continue;
			}

			ret = user_audio_capture_buffer_recv(capture_chn, &buffer);
			if (!ret) {
				fwrite(buffer.vm_addr, 1, buffer.size, fp_capture);
				user_audio_buffer_put(&buffer);
				printf("audio capture get one buffer\n");
			}
			usleep(5000);
		}

		user_audio_capture_chn_disable(capture_chn);
		user_audio_capture_chn_release(capture_chn);
		fclose(fp_capture);
	}

	if (o_playback) {
		fp_playback = fopen(playback_file, "r");
		playback_chn = user_audio_playback_chn_create(&attr);
		if (playback_chn < 0)
			goto exit;
		ret = user_audio_playback_chn_enable(playback_chn);
		if (ret < 0)
			goto exit;

		while (!g_exit) {
			ret = user_audio_playback_buffer_poll(playback_chn);
			if (ret < 0) {
				usleep(10000);
				continue;
			}

			ret = fread(buffer.vm_addr, 1, period_frames * channels * format / 8, fp_playback);
			if (ret = 0)
				break;
			buffer.size = ret;

			user_audio_playback_buffer_send(playback_chn, &buffer);
			printf("audio playback send one buffer\n");
			usleep(5000);
		}

		user_audio_playback_chn_disable(playback_chn);
		user_audio_playback_chn_release(playback_chn);
		fclose(fp_playback);
	}

	return 0;

exit:
	if (capture_chn != -1)
		user_audio_capture_chn_release(capture_chn);
	if (playback_chn != -1)
		user_audio_playback_chn_release(playback_chn);
	return 0;
}

