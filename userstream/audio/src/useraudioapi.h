#ifndef __USER_AUDIO_API_H
#define __USER_AUDIO_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct audio_attr {
	char dev_node[24];
	uint32_t rate;
	uint32_t format;
	uint32_t channels;
	unsigned long period_frames;
};

struct audio_buffer {
	void *vm_addr;
	int size;
	int refcount;
};

int user_audio_capture_chn_create(struct audio_attr *attr);
int user_audio_capture_chn_release(const int chn);
int user_audio_capture_chn_enable(const int chn);
int user_audio_capture_chn_disable(const int chn);
int user_audio_capture_buffer_poll(const int chn);
int user_audio_capture_buffer_recv(const int chn, struct audio_buffer *buffer);
int user_audio_capture_get_volume(int volume);
int user_audio_capture_set_volume(int volume);
int user_audio_playback_chn_create(struct audio_attr *attr);
int user_audio_playback_chn_release(const int chn);
int user_audio_playback_chn_enable(const int chn);
int user_audio_playback_chn_disable(const int chn);
int user_audio_playback_buffer_poll(const int chn);
int user_audio_playback_buffer_send(const int chn, struct audio_buffer *buffer);
int user_audio_playback_get_volume(int volume);
int user_audio_playback_set_volume(int volume);

void user_audio_buffer_get(struct audio_buffer *buffer);
void user_audio_buffer_put(struct audio_buffer *buffer);

#ifdef __cplusplus
}
#endif
#endif
