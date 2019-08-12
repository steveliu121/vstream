#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <poll.h>
#include <errno.h>

#define USER_AUDIO_BUFFER_TIME_MAX		500000
#define USER_AUDIO_PERIOD_TIME_DEFAULT		30000
#define AUDIO_CAPTURE_CHN_ID			0x20
#define AUDIO_PLAYBACK_CHN_ID			0x21

struct audio_attr {
	char dev_node[24];
	int rate;
	int format;
	int channels;
	int period_frames;
};

struct audio_buffer {
	void *vm_addr;
	int size;
	int refcount;
};
struct audio_profile {
	snd_pcm_t *capture_hd;
	snd_pcm_t *playback_hd;
	int capture_chn;
	int playback_chn;
	struct audio_attr capture_attr;
	struct audio_attr playback_attr;
};
static struct audio_profile g_audio_profile;
/*TODO expand audio_profile with attr*/
/*TODO set param sync period_frames*/
/*snd_pcm_hw_params_get_period_size*/

static int __audio_buffer_poll(const int chn, const int timeout)
{
	int count;
	struct pollfd *ufds;
	unsigned short revents;
	snd_pcm_t *hd;
	char *chn_name;

	hd = (chn == AUDIO_CAPTURE_CHN_ID) ?
		g_audio_profile.capture_hd : g_audio_profile.playback_hd;
	chn_name = (chn == AUDIO_CAPTURE_CHN_ID) ? "capture" : "playback";

	count = snd_pcm_poll_descriptors_count(hd);
	if (count <= 0) {
		printf("audio %s chn poll descriptors invalid\n", chn_name);
		return -1;
	}

	ufds = calloc(count, sizeof(struct pollfd));
	if (!ufds) {
		printf("no enough memory\n");
		return -1;
	}

	ret = snd_pcm_poll_descriptors(hd, ufds, count);
	if (ret < 0) {
		printf("audio %s chn obtain poll descriptors fail\n", chn_name);
		goto exit;
	}

	ret = poll(ufds, count, timeout);
	if (ret > 0) {
		ret = snd_pcm_poll_descriptors_revents(hd, ufds, count , &revents);
		if (!ret && (revents & ((chn == AUDIO_CAPTURE_CHN_ID) ? POLLIN : POLLOUT)))
			goto out;

		ret = -1;
		printf("audio %s process poll revents error\n", chn_name);
		goto exit;
	}
	if (ret == 0) {
		ret = -1;
		/* nothing polled */
		goto out;
	}

exit:
	free(ufds)
	if (snd_pcm_state(hd) == SND_PCM_STATE_XRUN) {
		if (chn == AUDIO_CAPTURE_CHN_ID)
			printf("audio capture chn overrun!!\n");
		else
			printf("audio playback chn underrun!!\n");
		if (snd_pcm_prepare(hd) < 0);
			printf("audio %s chn cannot recovery from xrun!!\n", chn_name);
	}
	return ret;

out:
	free(ufds);
	return ret;
}

/* audio capture/playback based on alsa-lib */
static int __audio_set_params(snd_pcm_t *handle, struct audio_attr *attr)
{
	snd_pcm_hw_params_t *hwparams;
	unsigned int buffer_time;
	unsigned int period_time;
	int ret, format, rate;

	/* init hwparams */
	snd_pcm_hw_params_alloca(&hwparams);
	ret = snd_pcm_hw_params_any(handle, hwparams);
	if (ret < 0) {
		printf("no configurations available\n");
		goto exit;
	}

	/* set access type */
	ret = snd_pcm_hw_params_set_access(handle, hwparams,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		printf("access type not available\n");
		goto exit;
	}

	/* set format */
	switch (attr->format) {
	case 8:
		format = SND_PCM_FORMAT_U8;
		break;
	case 16:
		format = SND_PCM_FORMAT_S16_LE;
		break;
	case 24:
		format = SND_PCM_FORMAT_S24_3LE;
		break;
	default:
		printf("unsupported sample format\n");
		ret = -1;
		goto exit;
	}
	ret = snd_pcm_hw_params_set_format(handle, hwparams, format);
	if (ret < 0) {
		printf("sample format not available\n");
		goto exit;
	}

	/* set channels */
	ret = snd_pcm_hw_params_set_channels(handle, hwparams,
					     attr->channels);
	if (ret < 0) {
		printf("channels count not available\n");
		goto exit;
	}

	/* set sample rate */
	rate = attr->rate;
	ret = snd_pcm_hw_params_set_rate_near(handle, hwparams,
					      &attr->rate, 0);
	if (ret < 0) {
		printf("sample rate not available\n");
		goto exit;
	}
	if (rate != attr->rate)
		printf("The rate %d Hz is not supported by your hardware.\n"
			"==> Using %d Hz instead.\n", rate, attr->rate);

	/* set buffer time */
	ret = snd_pcm_hw_params_get_buffer_time_max(hwparams,
						    &buffer_time, 0);
	if (ret < 0) {
		printf("fail to get buffer time max\n");
		goto exit;
	}
	if (buffer_time <= 0) {
		printf("buffer time max <= 0\n");
		ret = -1;
		goto exit;
	}
	if (buffer_time > USER_AUDIO_BUFFER_TIME_MAX)
		buffer_time = USER_AUDIO_BUFFER_TIME_MAX;

	/* set period time */
	if (attr->period_frames > 0) {
		unsigned long period_frames;

		ret = snd_pcm_hw_params_set_period_size_near(handle, hwparams,
							    &period_frames, 0);
		printf("audio period_frames:[%d]\n", period_frames);
	} else {
		period_time = USER_AUDIO_PERIOD_TIME_DEFAULT;
		ret = snd_pcm_hw_params_set_period_time_near(handle, hwparams,
							     &period_time, 0);
		printf("audio period_time:[%d]\n", period_time);
	}
	if (ret < 0) {
		printf("period size not available\n");
		goto exit;
	}

	/* set buffer time */
	ret = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams,
						     &buffer_time, 0);
	if (ret < 0) {
		printf("buffer time not available\n");
		goto exit;
	}

	/* set hw_params */
	ret = snd_pcm_hw_params(handle, hwparams);
	if (ret < 0) {
		printf("unable to install hw params\n");
		goto exit;
	}

exit:
	snd_pcm_hw_params_free(hwparams);

	return ret;
}

int user_audio_capture_chn_create(struct audio_attr *attr)
{
	int ret;

	ret = snd_pcm_open(&g_audio_profile.capture_hd, attr->dev_node,
				SND_PCM_STREAM_CAPTURE, 0);
	if (ret < 0) {
		printf("create audio capture chn fail\n");
		goto exit;
	}

	ret = __audio_set_params(g_audio_profile.capture_hd, attr);

exit:
	if (ret < 0)
		return ret;
	return AUDIO_CAPTURE_CHN_ID;
}

int user_audio_capture_chn_release(const int chn)
{
	if (chn != AUDIO_CAPTURE_CHN_ID) {
		printf("audio capture chn not exist\n");
		return -1;
	}

	snd_pcm_close(g_audio_profile.capture_hd);

	return 0;
}

int user_audio_capture_chn_enable(const int chn)
{
	int ret;

	if (chn != AUDIO_CAPTURE_CHN_ID) {
		printf("audio capture chn not exist\n");
		return -1;
	}

	ret = snd_pcm_prepare(g_audio_profile.capture_hd);
	if (ret < 0) {
		printf("audio capture chn enable fail\n");
		return -1;
	}

	return 0;
}

int user_audio_capture_chn_disable(const int chn)
{
	int ret;

	if (chn != AUDIO_CAPTURE_CHN_ID) {
		printf("audio capture chn not exist\n");
		return -1;
	}

	ret = snd_pcm_drop(g_audio_profile.capture_hd);
	if (ret < 0) {
		printf("audio capture chn disable fail\n");
		return -1;
	}

	return 0;
}

int user_audio_capture_buffer_poll(const int chn)
{
	int timeout = 0;

	if (chn != AUDIO_CAPTURE_CHN_ID) {
		printf("audio capture chn not exist\n");
		return -1;
	}

	return __audio_buffer_poll(chn, 0);
}

int user_audio_capture_buffer_recv(const int chn, struct audio_buffer *buffer)
{
	struct audio_buffer *buffer;
	int size;
	int channels, format, period_frames;

	if (chn != AUDIO_CAPTURE_CHN_ID) {
		printf("audio capture chn not exist\n");
		return -1;
	}

	buffer = (struct audio_buffer *)calloc(1, sizeof(*buffer));
	if (!buffer) {
		printf("no enough memory\n");
		goto exit;
	}

	channels = g_audio_profile.capture_attr.channels;
	format = g_audio_profile.capture_attr.format;
	period_frames = g_audio_profile.capture_attr.period_frames;
	size = channels * format * period_frames / 8;
	buffer->vm_addr = calloc(1, size);
	if (buffer->vm_addr) {
		printf("no enough memory\n");
		goto exit;
	}

	ret = snd_pcm_readi(g_audio_profile.capture_hd, buffer->vm_addr,
							buffer->period_frames);
	if (ret < 0) {
		if (ret = -EPIPE) {
			printf("audio capture chn readi overrun!!\n");
			ret = snd_pcm_prepare(g_audio_profile.capture_hd);
			if (ret < 0)
				printf("audio capture chn cannot recovery from xrun!!\n");
			goto exit;
		}

		printf("audio capture chn readi error[%s]!!\n", strerror(errno));
		goto exit;
	}

	period_frames = ret;
	buffer->size = channels * format * period_frames / 8;
	buffer->refcount = 1;

	return 0;

exit:
	if (buffer) {
		if (buffer->vm_addr)
			free(buffer->vm_addr);
		free(buffer);
		buffer = NULL;
	}

	return -1;
}

void user_audio_buffer_get(struct audio_buffer *buffer)
{
	buffer->refcount++;
}

void user_audio_buffer_put(struct audio_buffer *buffer)
{
	buffer->refcount--;
	if (buffer->refcount == 0) {
		free(buffer->vm_addr);
		free(buffer);
	}
}

int user_audio_playback_chn_create(struct audio_attr *attr)
{
	int ret;

	ret = snd_pcm_open(&g_audio_profile.playback_hd, attr->dev_node,
				SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		printf("create audio playback chn fail\n");
		goto exit;
	}

	ret = __audio_set_params(g_audio_profile.playback_hd, attr);

exit:
	if (ret < 0)
		return ret;
	return AUDIO_PLAYBACK_CHN_ID;
}

int user_audio_playback_chn_release(const int chn)
{
	if (chn != AUDIO_PLAYBACK_CHN_ID) {
		printf("audio playback chn not exist\n");
		return -1;
	}

	/* Stop PCM device and drop pending frames */
	/* snd_pcm_drop(pcm_handle); */

	/* Stop PCM device after pending frames have been played */
	snd_pcm_drain(g_audio_profile.playback_hd);
	snd_pcm_close(g_audio_profile.playback_hd);

	return 0;
}

int user_audio_playback_chn_enable(const int chn)
{
	int ret;

	if (chn != AUDIO_PLAYBACK_CHN_ID) {
		printf("audio playback chn not exist\n");
		return -1;
	}

	ret = snd_pcm_prepare(g_audio_profile.playback_hd);
	if (ret < 0) {
		printf("audio playback chn enable fail\n");
		return -1;
	}

	return 0;
}

int user_audio_playback_chn_disable(const int chn)
{
	int ret;

	if (chn != AUDIO_PLAYBACK_CHN_ID) {
		printf("audio playback chn not exist\n");
		return -1;
	}

	ret = snd_pcm_drop(g_audio_profile.playback_hd);
	if (ret < 0) {
		printf("audio playback chn disable fail\n");
		return -1;
	}

	return 0;
}

int user_audio_playback_buffer_poll(const int chn)
{
	int timeout = 0;

	if (chn != AUDIO_PLAYBACK_CHN_ID) {
		printf("audio playback chn not exist\n");
		return -1;
	}

	return __audio_buffer_poll(chn, 0);
}

int user_audio_playback_buffer_send(const int chn, struct audio_buffer *buffer)
{
	int ret;

	if (chn != AUDIO_PLAYBACK_CHN_ID) {
		printf("audio playback chn not exist\n");
		return -1;
	}

	ret = snd_pcm_writei(g_audio_profile.playback_hd, buffer->vm_addr, );
}

