/* Video encode demo using OpenMAX IL though the ilcient helper library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <userh264.h>


int user_h264_chn_create(struct h264_attr *attr)
{
	OMX_VIDEO_PARAM_PORTFORMATTYPE format;
	/*TODO*/
	OMX_PARAM_PORTDEFINITIONTYPE definition;
	OMX_VIDEO_PARAM_BITRATETYPE bitratetype;
	COMPONENT_T *video_encode = NULL;
	COMPONENT_T *component_list[5];
	OMX_ERRORTYPE omx_ret;
	int il_ret;
	ILCLIENT_T *client = NULL;

	memset(component_list, 0, sizeof(component_list));

	bcm_host_init();
	client = ilclient_init();
	if (!client) {
		printf("ilclient create fail\n");
		goto out;
	}

	omx_ret = OMX_Init();
	if (omx_ret != OMX_ErrorNone) {
		printf("OMX init fail\n");
		goto out;
	}

	/* create video_encode */
	il_ret = ilclient_create_component(client, &video_encode, "video_encode",
				 ILCLIENT_DISABLE_ALL_PORTS |
				 ILCLIENT_ENABLE_INPUT_BUFFERS |
				 ILCLIENT_ENABLE_OUTPUT_BUFFERS);
	if (il_ret != 0) {
		printf("IL create video_encode component fail[%x]\n", il_ret);
		goto out;
	}
	component_list[0] = video_encode;

	/* get current settings of video_encode component from port 200 */
	memset(&definition, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	definition.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	definition.nVersion.nVersion = OMX_VERSION;
	definition.nPortIndex = 200;

	omx_ret = OMX_GetParameter(ILC_GET_HANDLE(video_encode),
				OMX_IndexParamPortDefinition, &definition);
	if (omx_ret != OMX_ErrorNone) {
		printf("OMX get param for video_encode port 200 fail\n");
		goto out;
	}

	/* Port 200: in 1/1 115200 16 enabled,not pop.,not cont. 320x240 320x240 @1966080 20 */
	definition.format.video.nFrameWidth = attr->width;
	definition.format.video.nFrameHeight = attr->height;
	definition.format.video.xFramerate = attr->fps * (1 << 16);
	definition.format.video.nSliceHeight = attr->height;
	definition.format.video.nStride = attr->width;
	switch (attr->fmt) {
	case USER_PIX_FMT_BGR24:
		definition.format.video.eColorFormat = OMX_COLOR_Format24bitBGR888;
		break;
	case USER_PIX_FMT_BGR32:
		definition.format.video.eColorFormat = OMX_COLOR_Format32bitABGR8888;
		break;
	case USER_PIX_FMT_NV12:
		definition.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
		break;
	case USER_PIX_FMT_YUYV:
		definition.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;
		break;
	case USER_PIX_FMT_YVYU:
		definition.format.video.eColorFormat = OMX_COLOR_FormatYCrYCb;
		break;
	case USER_PIX_FMT_UYVY:
		definition.format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;
		break;
	case USER_PIX_FMT_VYUY:
		definition.format.video.eColorFormat = OMX_COLOR_FormatCrYCbY;
		break;
	default:
		printf("Unknown pixelformat !!!\n");
		goto out;
	}

	omx_ret = OMX_SetParameter(ILC_GET_HANDLE(video_encode),
				OMX_IndexParamPortDefinition, &definition);
	if (omx_ret != OMX_ErrorNone) {
		printf("OMX set param for video_encode input port 200 fail\n");
		goto out;
	}

	memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	format.nVersion.nVersion = OMX_VERSION;
	format.nPortIndex = 201;
	format.eCompressionFormat = OMX_VIDEO_CodingAVC;

	omx_ret = OMX_SetParameter(ILC_GET_HANDLE(video_encode),
				OMX_IndexParamVideoPortFormat, &format);
	if (omx_ret != OMX_ErrorNone) {
		printf("OMX set param for video_encode output port 201 fail\n");
		goto out;
	}

	/* set current bitrate */
	memset(&bitratetype, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
	bitratetype.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
	bitratetype.nVersion.nVersion = OMX_VERSION;
	switch (attr->bitrate_mode) {
	case  USER_BITRATE_CBR:
		bitratetype.eControlRate = OMX_Video_ControlRateConstant;
		break;
	case  USER_BITRATE_VBR:
		bitratetype.eControlRate = OMX_Video_ControlRateVariable;
		break;
	default:
		printf("Unknown bitrate mode !!!\n");
		goto out;
	}
	bitratetype.nTargetBitrate = attr->bitrate;
	bitratetype.nPortIndex = 201;
	omx_ret = OMX_SetParameter(ILC_GET_HANDLE(video_encode),
				OMX_IndexParamVideoBitrate, &bitratetype);
	if (omx_ret != OMX_ErrorNone) {
		printf("OMX set bitrate for video_encode output port 201 fail\n");
		goto out;
	}

	/* get current bitrate */
	memset(&bitratetype, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
	bitratetype.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
	bitratetype.nVersion.nVersion = OMX_VERSION;
	bitratetype.nPortIndex = 201;

	omx_ret = OMX_GetParameter(ILC_GET_HANDLE(video_encode),
			OMX_IndexParamVideoBitrate, &bitratetype);
	if (omx_ret != OMX_ErrorNone) {
		printf("OMX get bitrate for video_encode output port 201 fail\n");
		goto out;
	}
	printf("Current Bitrate=%u\n", bitratetype.nTargetBitrate);


	/* encode to idle */
	il_ret = ilclient_change_component_state(video_encode, OMX_StateIdle);
	if (il_ret == -1) {
		printf("ilclient change component state to idle fail\n");
		goto out;
	}

	/* enabling input port buffers for 200 */
	il_ret = ilclient_enable_port_buffers(video_encode, 200, NULL, NULL, NULL);
	if (il_ret != 0) {
		printf("enabling input port buffers for 200 fail\n");
		goto out;
	}

	/* enabling output port buffers for 201 */
	il_ret = ilclient_enable_port_buffers(video_encode, 201, NULL, NULL, NULL);
	if (il_ret != 0) {
		printf("enabling output port buffers for 201 fail\n");
		goto out;
	}

	g_h264_profile.chn = H264_CHN_ID;
	memcpy(&g_h264_profile.attr, attr, sizeof(struct h264_attr));
	g_h264_profile.component = video_encode;
	memcpy(g_h264_profile.component_list, component_list, sizeof(component_list));
	g_h264_profile.client = client;

	return H264_CHN_ID;

out:
	if (!component_list[0])
		ilclient_cleanup_components(component_list);
	OMX_Deinit();
	if (!client)
		ilclient_destroy(client);
	bcm_host_deinit();

	return -1;
}

int user_h264_chn_release(const int chn)
{
	if (chn != H264_CHN_ID) {
		printf("h264 chn not exist\n");
		return -1;
	}

	/* disabling port buffers for 200 and 201 */
	ilclient_disable_port_buffers(g_h264_profile.component, 200, NULL, NULL, NULL);
	ilclient_disable_port_buffers(g_h264_profile.component, 201, NULL, NULL, NULL);
	/* encode to loaded */
	ilclient_change_component_state(g_h264_profile.component,
							OMX_StateLoaded);

	if (!g_h264_profile.component_list[0])
		ilclient_cleanup_components(g_h264_profile.component_list);
	OMX_Deinit();
	if (!g_h264_profile.client)
		ilclient_destroy(g_h264_profile.client);
	bcm_host_deinit();

	return 0;
}

int user_h264_chn_enable(const int chn)
{
	if (chn != H264_CHN_ID) {
		printf("h264 chn not exist\n");
		return -1;
	}

	/* encode to executing */
	return ilclient_change_component_state(g_h264_profile.component,
							OMX_StateExecuting);
}

int user_h264_chn_disable(const int chn)
{
	if (chn != H264_CHN_ID) {
		printf("h264 chn not exist\n");
		return -1;
	}

	/* encode to idle */
	/* ilclient_state_transition(list, OMX_StateIdle); */
	return ilclient_change_component_state(g_h264_profile.component,
							OMX_StateIdle);
}

int user_h264_buffer_poll(const int chn)
{
	return 0;
}

/* TODO add h264 encode into loop */
int user_h264_buffer_recv(const int chn, struct h264_buffer *buffer)
{
	OMX_BUFFERHEADERTYPE *omx_buf = NULL;
	OMX_ERRORTYPE omx_ret;

	 omx_buf = ilclient_get_output_buffer(g_h264_profile.component, 201, 1);
	if (!omx_buf) {
		printf("h264 chn output is busy, get output buffer fail\n");
		return -1;
	}

	 omx_ret = OMX_FillThisBuffer(ILC_GET_HANDLE(g_h264_profile.component), omx_buf);
	 if (omx_ret != OMX_ErrorNone) {
		printf("h264 fill buffer fail\n");
		return -1;
	 }

	 buffer->vm_addr = omx_buf->pBuffer;
	 buffer->size = omx_buf->nFilledLen;
	 buffer->refcount = 1;
	 buffer->priv = omx_buf;

	 return 0;
}

int user_h264_buffer_send(const int chn, struct h264_buffer *buffer)
{
	OMX_BUFFERHEADERTYPE *omx_buf = NULL;
	OMX_ERRORTYPE omx_ret;

	if (chn != H264_CHN_ID) {
		printf("h264 chn not exist\n");
		return -1;
	}
	if (!buffer->vm_addr || !buffer->size) {
		printf("h264 input buffer is null\n");
		return -1;
	}

	omx_buf = ilclient_get_input_buffer(g_h264_profile.component, 200, 1);
	if (!omx_buf) {
		printf("h264 chn input is busy, get input buffer fail\n");
		return -1;
	}

	/* XXX memcpy a bad way cost too much mem */
	memcpy(omx_buf->pBuffer, buffer->vm_addr, buffer->size);
	omx_buf->nFilledLen = buffer->size;

	omx_ret = OMX_EmptyThisBuffer(ILC_GET_HANDLE(g_h264_profile.component), omx_buf);
	if (omx_ret != OMX_ErrorNone) {
		printf("h264 empty buffer fail\n");
		return -1;
	}

	return 0;
}

void user_h264_buffer_get(struct h264_buffer *buffer)
{
	buffer->refcount++;
}

void user_h264_buffer_put(struct h264_buffer *buffer)
{
	OMX_BUFFERHEADERTYPE *omx_buf;

	buffer->refcount--;
	if (buffer->refcount == 0) {
		omx_buf = (OMX_BUFFERHEADERTYPE *)buffer->priv;
		omx_buf->nFilledLen = 0;
	}
}

