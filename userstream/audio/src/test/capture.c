#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <poll.h>

/* status/state (state sync ptr get state;status sync ptr and wake up poll get state)*/
unsigned long g_period_frames;

static int __audio_buffer_poll(snd_pcm_t *hd, const int timeout)
{
	int count;
	struct pollfd *ufds = NULL;
	unsigned short revents;
	char *chn_name;
	int ret;
	snd_pcm_status_t *status  = NULL;
	snd_pcm_state_t state;
	snd_pcm_uframes_t frames;

	chn_name = "capture";

	ret = snd_pcm_status_malloc(&status);
	if (ret < 0) {
		printf("audio %s chn malloc status fail\n", chn_name);
		return -1;
	}

	ret = snd_pcm_status(hd, status);
	if (ret < 0) {
		printf("audio %s chn get stauts fail\n", chn_name);
		return ret;
	}

	state = snd_pcm_status_get_state(status);


	frames = snd_pcm_status_get_avail(status);
	if (frames > g_period_frames) {
		ret = 0;
		goto exit;
	}

	count = snd_pcm_poll_descriptors_count(hd);
	if (count <= 0) {
		printf("audio %s chn poll descriptors invalid\n", chn_name);
		return -1;
	}

	ufds = calloc(count, sizeof(struct pollfd));
	if (!ufds) {
		printf("no enough memory\n");
		goto exit;
	}

	ret = snd_pcm_poll_descriptors(hd, ufds, count);
	if (ret < 0) {
		printf("audio %s chn obtain poll descriptors fail\n", chn_name);
		goto exit;
	}

	printf("%s%d count:%d, chn:%s\n", __func__, __LINE__, count , chn_name);
	printf("poll_fd:[%d], pollevent[%s]\n", ufds->fd, (ufds->events & POLLIN) ? "pollin" : "pollout");
	ret = poll(ufds, count, timeout);
	if (ret > 0) {
		printf("%s%d polled\n", __func__, __LINE__);
		ret = snd_pcm_poll_descriptors_revents(hd, ufds, count , &revents);
		if (!ret && (revents & POLLIN))
			goto exit;

		ret = -1;
		printf("audio %s process poll revents error\n", chn_name);
		goto exit;
	}
	if (ret == 0) {
		ret = -1;
	printf("%s%d polled nothing\n", __func__, __LINE__);
		/* nothing polled */
		goto exit;
	}

	printf("%s%d polled error\n", __func__, __LINE__);

exit:
	if (ufds)
		free(ufds);

	/* the will lead to update hw ptr and wake up poll_wait */
	ret = snd_pcm_status(hd, status);
	if (ret < 0) {
		printf("audio %s chn get stauts fail\n", chn_name);
		return ret;
	}

	state = snd_pcm_status_get_state(status);
	snd_pcm_status_free(status);

	if ((state == SND_PCM_STATE_XRUN) || (state != SND_PCM_STATE_RUNNING)) {
		printf("audio capture chn overrun!!\n");
		if (snd_pcm_prepare(hd) < 0) {
			printf("audio %s chn cannot recovery from xrun!!\n", chn_name);
			return ret;
		}
		if (snd_pcm_state(hd) == SND_PCM_STATE_PREPARED) {
			if ((ret = snd_pcm_start(hd)) < 0) {
				fprintf (stderr, "cannot start audio interface for use (%s)\n",
					 snd_strerror (ret));
			}
		}
	}

	return ret;
}
int main (int argc, char *argv[])
{
	int i;
	int err;
	short buf[160];
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;

	if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
			 argv[1],
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_nonblock(capture_handle, 0)) < 0) {
		fprintf (stderr, "cannot set pcm block (%s)\n",
			 snd_strerror (err));
		exit (1);
	}


	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	int rate = 8000;
	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	unsigned int period_time = 20000;
	if ((err = snd_pcm_hw_params_set_period_time_near(capture_handle, hw_params,
					&period_time, NULL)) < 0) {
		fprintf (stderr, "cannot set period time (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	unsigned int buffer_time = 500000;
	if ((err = snd_pcm_hw_params_get_buffer_time_max(hw_params,
				&buffer_time, NULL)) < 0) {
		fprintf (stderr, "cannot get buffer time (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	printf("buffertime %d\n", buffer_time);

	if ((err = snd_pcm_hw_params_get_buffer_time_min(hw_params,
				&buffer_time, NULL)) < 0) {
		fprintf (stderr, "cannot get buffer time (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	printf("buffertime %d\n", buffer_time);
	buffer_time = 500000;

	if ((err = snd_pcm_hw_params_set_buffer_time_near(capture_handle, hw_params,
				&buffer_time, NULL)) < 0) {
		fprintf (stderr, "cannot set buffer time (%s)\n",
			 snd_strerror (err));
		exit (1);
	}


	unsigned long period_frames;
	err = snd_pcm_hw_params_get_period_size(hw_params, &period_frames, NULL);
	if (err < 0) {
		printf("audio get period_frames fail\n");
		exit(1);
	}
	printf("~~~~~~period_frames[%lu]\n", period_frames);
	g_period_frames = period_frames;

	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	/* if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) { */
	/* 	fprintf (stderr, "cannot allocate software parameter structure (%s)\n", */
	/* 		 snd_strerror (err)); */
	/* 	exit (1); */
	/* } */

	/* /1* get the current swparams *1/ */
	/* err = snd_pcm_sw_params_current(capture_handle, sw_params); */
	/* if (err < 0) { */
	/* 	printf("Unable to determine current swparams for capture: %s\n", snd_strerror(err)); */
	/* 	exit(1); */
	/* } */
	/* /1* start the transfer when the buffer is almost full: *1/ */
	/* /1* (buffer_size / avail_min) * avail_min *1/ */
	/* err = snd_pcm_sw_params_set_start_threshold(capture_handle, sw_params, 80); */
	/* if (err < 0) { */
	/* 	printf("Unable to set start threshold mode for capture: %s\n", snd_strerror(err)); */
	/* 	exit(1); */
	/* } */
	/* /1* allow the transfer when at least period_size samples can be processed *1/ */
	/* /1* or disable this mechanism when period event is enabled (aka interrupt like style processing) *1/ */
	/* err = snd_pcm_sw_params_set_avail_min(capture_handle, sw_params, 80); */
	/* if (err < 0) { */
	/* 	printf("Unable to set avail min for capture: %s\n", snd_strerror(err)); */
	/* 	exit(1); */
	/* } */
	/* /1* write the parameters to the playback device *1/ */
	/* err = snd_pcm_sw_params(capture_handle, sw_params); */
	/* if (err < 0) { */
	/* 	printf("Unable to set sw params for capture: %s\n", snd_strerror(err)); */
	/* 	exit(1); */
	/* } */

	/* snd_pcm_sw_params_free(sw_params); */

	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		exit (1);
	}


	int ret;
	/* int init = 0; */

	/* if ((err = snd_pcm_readi (capture_handle, buf, 80)) != 80) { */
	/* 	fprintf (stderr, "read from audio interface failed (%s)\n", */
	/* 		 snd_strerror (err)); */
	/* 	exit (1); */
	/* } */

	/* TODO check if state == prepare */
	if ((err = snd_pcm_start(capture_handle)) < 0) {
		fprintf (stderr, "cannot start audio interface for use (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	for (i = 0; i < 5000; ++i) {
		/* if (init) { */
		ret = __audio_buffer_poll(capture_handle, 10000);
		if (ret < 0) {
			usleep(5000);
			continue;
		}
		/* snd_pcm_hwsync(capture_handle); */
		/* ret = snd_pcm_wait(capture_handle, 10000); */
		/* if (ret == -EPIPE) { */
		/* 	snd_pcm_prepare(capture_handle); */
		/* 	continue; */
		/* } */
		/* if (ret == 0) */
		/* 	continue; */

		/* } */

		/* init = 1; */

		if ((err = snd_pcm_readi (capture_handle, buf, 80)) != 80) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
				 snd_strerror (err));
			goto exit;
			/* exit (1); */
			/* if ((err == -EPIPE) && (snd_pcm_prepare(capture_handle) < 0)) { */
			/* 	printf("audio chn cannot recovery from xrun!!\n"); */
			/* 	exit(1); */
			/* } */

			/* init = 0; */
		}

		/* if (snd_pcm_state(capture_handle) == SND_PCM_STATE_RUNNING) */
			/* init = 1; */
		/* snd_pcm_wait(capture_handle, 100); */
		printf("get one frame\n");
	}

exit:
	sleep(20);
	snd_pcm_close (capture_handle);
	exit (0);
}
