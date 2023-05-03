#define MINIMP4_IMPLEMENTATION
#include "minimp4.h"
#define ENABLE_AUDIO 1
#if ENABLE_AUDIO
#include "aacenc_lib.h"
#include "aacdecoder_lib.h"
#include "g711.h"
#define AUDIO_RATE 8000
#endif

#define VIDEO_FPS 20
//#define CODEC_H264 0
//#define CODEC_H265 1

static uint8_t *preload(const char *path, ssize_t *data_size)
{
    FILE *file = fopen(path, "rb");
    uint8_t *data = NULL;
    *data_size = 0;
    if (!file)
    {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END))
    {
    	if(file) fclose(file);
        return NULL;
    }
    *data_size = (ssize_t)ftell(file);
    if (*data_size < 0)
    {
    	if(file) fclose(file);
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET))
    {
    	if(file) fclose(file);
        return NULL;
    }
    data = (unsigned char*)malloc(*data_size);
    if (!data)
    {
    	if(file) fclose(file);
        return NULL;
    }
    if ((ssize_t)fread(data, 1, *data_size, file) != *data_size)
    {
    	if(file) fclose(file);
    	if(data) free(data);
        return NULL;
    }
    fclose(file);
    return data;
}

static ssize_t get_nal_size(uint8_t *buf, ssize_t size)
{
    ssize_t pos = 3;
    while ((size - pos) > 3)
    {
        if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 1)
            return pos;
        if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 0 && buf[pos + 3] == 1)
            return pos;
        pos++;
    }
    return size;
}

static int write_callback(int64_t offset, const void *buffer, size_t size, void *token)
{
    FILE *f = (FILE*)token;
    fseek(f, offset, SEEK_SET);
    return fwrite(buffer, 1, size, f) != size;
}

typedef struct
{
    uint8_t *buffer;
    ssize_t size;
} INPUT_BUFFER;

int pps_dua_convert_to_mp4(int codec_type, char *video_path, char *audio_path, char *mp4_file_path, int video_max_frame, int audio_max_frame, int resolution_type)
{
    int sequential_mode = 0;
    int fragmentation_mode = 0;
    int ret = 0;
    ssize_t h264_size;
    uint8_t *alloc_buf;
    uint8_t *buf_h264 = alloc_buf = preload(video_path, &h264_size);
    if (!buf_h264)
    {
        printf("error: can't open h264/h265 file\n");
        return -1;
    }
    /* Remove temp file after load succesfully */
	unlink(video_path);

    FILE *fout = fopen(mp4_file_path, "wb");
    if (!fout)
    {
        printf("error: can't open output file\n");
        if(buf_h264) free(buf_h264);
        return -1;
    }

    MP4E_mux_t *mux;
    mp4_h26x_writer_t mp4wr;
    mux = MP4E_open(sequential_mode, fragmentation_mode, fout, write_callback);
    if(resolution_type == 0) // FULL HD
    {
    	ret = mp4_h26x_write_init(&mp4wr, mux, 1920, 1080, codec_type); // codec_type: 0 (H264), 1 (H265)
    }
    else	//SD
    {
    	ret = mp4_h26x_write_init(&mp4wr, mux, 640, 360, codec_type); // codec_type: 0 (H264), 1 (H265)
    }
    if (MP4E_STATUS_OK != ret)
    {
        printf("error: mp4_h26x_write_init failed\n");
        if(buf_h264) free(buf_h264);
        if(fout) fclose(fout);
        return -1;
    }

	ssize_t pcmu_size;
	uint8_t *buf_pcmu  = (uint8_t *)preload(audio_path, &pcmu_size);
	if (!buf_pcmu)
	{
		printf("error: can't open pcmu file\n");
        if(buf_h264) free(buf_h264);
        if(fout) fclose(fout);
        MP4E_close(mux);
        mp4_h26x_write_close(&mp4wr);
		return -1;
	}
	/* Remove temp file after load succesfully */
	unlink(audio_path);

    ssize_t pcm_size = pcmu_size*2;
    int16_t *alloc_pcm;
    int16_t *buf_pcm  = alloc_pcm = (int16_t *) malloc(pcm_size * sizeof(char));
	if (!buf_pcm)
	{
		printf("error: can't malloc pcm buffer\n");
        if(buf_h264) free(buf_h264);
        if(fout) fclose(fout);
        MP4E_close(mux);
        mp4_h26x_write_close(&mp4wr);
        if(buf_pcmu) free(buf_pcmu);
		return -1;
	}

	convert_ulaw_buf_2_pcm_buf(buf_pcmu,buf_pcm,pcmu_size);
	free(buf_pcmu);

    uint32_t sample = 0, total_samples = pcm_size/2;
    uint64_t ts = 0, ats = 0;
    HANDLE_AACENCODER aacenc;
    AACENC_InfoStruct info;
    aacEncOpen(&aacenc, 0, 0);
    aacEncoder_SetParam(aacenc, AACENC_TRANSMUX, 0);
    aacEncoder_SetParam(aacenc, AACENC_AFTERBURNER, 1);
    aacEncoder_SetParam(aacenc, AACENC_BITRATE, 64000);
    aacEncoder_SetParam(aacenc, AACENC_SAMPLERATE, AUDIO_RATE);
    aacEncoder_SetParam(aacenc, AACENC_CHANNELMODE, 1);
    aacEncEncode(aacenc, NULL, NULL, NULL, NULL);
    aacEncInfo(aacenc, &info);

    MP4E_track_t tr;
    tr.track_media_kind = e_audio;
    tr.language[0] = 'u';
    tr.language[1] = 'n';
    tr.language[2] = 'd';
    tr.language[3] = 0;
    tr.object_type_indication = MP4_OBJECT_TYPE_AUDIO_ISO_IEC_14496_3;
    tr.time_scale = 90000;
    tr.default_duration = 0;
    tr.u.a.channelcount = 1;
    int audio_track_id = MP4E_add_track(mux, &tr);
    MP4E_set_dsi(mux, audio_track_id, info.confBuf, info.confSize);

    int video_f_count = 0;
    int audio_f_count = 0;

    ssize_t nal_size;
    AACENC_BufDesc in_buf, out_buf;
    AACENC_InArgs  in_args;
    AACENC_OutArgs out_args;
    uint8_t buf[2048];

    while (h264_size > 0 && video_f_count < video_max_frame && audio_f_count < audio_max_frame)
    {
        nal_size = get_nal_size(buf_h264, h264_size);
        if (nal_size < 4)
        {
            buf_h264  += 1;
            h264_size -= 1;
            continue;
        }
        if (MP4E_STATUS_OK != mp4_h26x_write_nal(&mp4wr, buf_h264, nal_size, 90000/VIDEO_FPS))
        {
            printf("error: mp4_h26x_write_nal failed\n");
            goto exit;
        }
        video_f_count++;
        buf_h264  += nal_size;
        h264_size -= nal_size;

        ts += 90000/VIDEO_FPS;
        while (ats < ts && audio_f_count < audio_max_frame)
        {
        	memset(buf,0,2048);
            if (total_samples < 1024)
            {
                buf_pcm = alloc_pcm;
                total_samples = pcm_size/2;
            }
            in_args.numInSamples = 1024;
            void *in_ptr = buf_pcm, *out_ptr = buf;
            int in_size          = 2*in_args.numInSamples;
            int in_element_size  = 2;
            int in_identifier    = IN_AUDIO_DATA;
            int out_size         = sizeof(buf);
            int out_identifier   = OUT_BITSTREAM_DATA;
            int out_element_size = 1;

            in_buf.numBufs            = 1;
            in_buf.bufs               = &in_ptr;
            in_buf.bufferIdentifiers  = &in_identifier;
            in_buf.bufSizes           = &in_size;
            in_buf.bufElSizes         = &in_element_size;
            out_buf.numBufs           = 1;
            out_buf.bufs              = &out_ptr;
            out_buf.bufferIdentifiers = &out_identifier;
            out_buf.bufSizes          = &out_size;
            out_buf.bufElSizes        = &out_element_size;

            if (AACENC_OK != aacEncEncode(aacenc, &in_buf, &out_buf, &in_args, &out_args))
            {
                printf("error: aac encode fail\n");
                goto exit;
            }
            sample  += in_args.numInSamples;
            buf_pcm += in_args.numInSamples;
            total_samples -= in_args.numInSamples;
            ats = (uint64_t)sample*90000/AUDIO_RATE;
            audio_f_count++;

            if (MP4E_STATUS_OK != MP4E_put_sample(mux, audio_track_id, buf, out_args.numOutBytes, 1024*90000/AUDIO_RATE, MP4E_SAMPLE_RANDOM_ACCESS))
            {
                printf("error: MP4E_put_sample failed\n");
                goto exit;
            }
        }
    }
exit:
    if (alloc_pcm)
        free(alloc_pcm);
    aacEncClose(&aacenc);
    if (alloc_buf)
        free(alloc_buf);
    MP4E_close(mux);
    mp4_h26x_write_close(&mp4wr);
    if (fout)
        fclose(fout);

    if(h264_size <= 0 || audio_f_count >= audio_max_frame || video_f_count >= video_max_frame)
    	return 1;
    else
    	return -1;
}

//int main(){
//	pps_dua_convert_to_mp4(CODEC_H264,"raw.h264", "output.g711", "out.mp4", 200, 78);
//	return 0;
//}
