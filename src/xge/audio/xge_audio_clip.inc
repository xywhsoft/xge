/* CPU-only immutable decoded clips; each voice owns its own buffer cursor. */
struct xge_audio_clip {
    xatomic32 references;
    xge_audio_clip_info_t info;
    float *samples;
};
typedef struct __xge_clip_sound {
    ma_sound sound; /* First: existing sound control APIs keep their ABI. */
    ma_audio_buffer_ref buffer;
    xge_audio_clip *clip;
} __xge_clip_sound;

int xgeAudioClipDecodeMemory(const xge_audio_clip_decode_t *desc,xge_audio_clip **out)
{
    if(out)*out=NULL;
    if(!desc||!out||!desc->data||!desc->bytes)return XGE_ERROR_INVALID_ARGUMENT;
    uint64_t limit=desc->max_decoded_bytes?desc->max_decoded_bytes:64u*1024u*1024u;
    if(limit>512u*1024u*1024u||limit>SIZE_MAX)return XGE_ERROR_INVALID_ARGUMENT;
    if(desc->cancelled&&desc->cancelled(desc->user))return XGE_ERROR_INVALID_STATE;
    ma_decoder decoder;
    ma_decoder_config config=ma_decoder_config_init(ma_format_f32,0,0);
    if(ma_decoder_init_memory(desc->data,desc->bytes,&config,&decoder)!=MA_SUCCESS)return XGE_ERROR_AUDIO_FAILED;
    ma_format format;ma_uint32 channels=0,rate=0;int error=XGE_ERROR_AUDIO_FAILED;
    xge_audio_clip *clip=NULL;
    if(ma_decoder_get_data_format(&decoder,&format,&channels,&rate,NULL,0)!=MA_SUCCESS||
       format!=ma_format_f32||!channels||channels>32||!rate)goto done;
    uint64_t maximum=limit/(channels*sizeof(float));
    if(!maximum){error=XGE_ERROR_OUT_OF_MEMORY;goto done;}
    clip=xrtMalloc(sizeof(*clip));if(!clip){error=XGE_ERROR_OUT_OF_MEMORY;goto done;}
    memset(clip,0,sizeof(*clip));xrtAtomic32Init(&clip->references,1);
    uint64_t capacity=maximum<16384?maximum:16384,frames=0;
    clip->samples=xrtMalloc((size_t)capacity*channels*sizeof(float));
    if(!clip->samples){error=XGE_ERROR_OUT_OF_MEMORY;goto done;}
    for(;;){
        if(desc->cancelled&&desc->cancelled(desc->user)){error=XGE_ERROR_INVALID_STATE;goto done;}
        if(frames==capacity){
            if(capacity==maximum){float extra[32];ma_uint64 got=0;
                ma_result r=ma_decoder_read_pcm_frames(&decoder,extra,1,&got);
                if(!got&&(r==MA_SUCCESS||r==MA_AT_END))break;
                error=XGE_ERROR_OUT_OF_MEMORY;goto done;
            }
            uint64_t next=capacity>maximum/2?maximum:capacity*2;
            float *data=xrtRealloc(clip->samples,(size_t)next*channels*sizeof(float));
            if(!data){error=XGE_ERROR_OUT_OF_MEMORY;goto done;}clip->samples=data;capacity=next;
        }
        ma_uint64 count=capacity-frames,got=0;if(count>4096)count=4096;
        ma_result r=ma_decoder_read_pcm_frames(&decoder,clip->samples+(size_t)frames*channels,count,&got);
        if(got>count||(r!=MA_SUCCESS&&r!=MA_AT_END))goto done;
        frames+=got;if(!got||r==MA_AT_END)break;
    }
    if(!frames)goto done;
    if(desc->cancelled&&desc->cancelled(desc->user)){error=XGE_ERROR_INVALID_STATE;goto done;}
    clip->info=(xge_audio_clip_info_t){channels,rate,frames,frames*channels*sizeof(float)};
    /* Shrinking is optional; keep a valid clip if the allocator cannot shrink. */
    {float *data=xrtRealloc(clip->samples,(size_t)clip->info.bytes);if(data)clip->samples=data;}
    *out=clip;clip=NULL;error=XGE_OK;
done:
    ma_decoder_uninit(&decoder);if(clip){xrtFree(clip->samples);xrtFree(clip);}return error;
}
xge_audio_clip_info_t xgeAudioClipInfo(const xge_audio_clip *clip)
{return clip?clip->info:(xge_audio_clip_info_t){0};}
const float *xgeAudioClipSamples(const xge_audio_clip *clip){return clip?clip->samples:NULL;}
int xgeAudioClipAddRef(xge_audio_clip *clip)
{
    if(!clip)return XGE_ERROR_INVALID_ARGUMENT;
    uint32_t before=xrtAtomic32Load(&clip->references,XMEMORY_RELAXED);
    for(;;){if(!before||before==UINT32_MAX)return XGE_ERROR_INVALID_STATE;
        if(xrtAtomic32CompareExchange(&clip->references,&before,before+1,XMEMORY_RELAXED,XMEMORY_RELAXED))return XGE_OK;}
}
void xgeAudioClipFree(xge_audio_clip *clip)
{if(clip&&xrtAtomic32FetchSub(&clip->references,1,XMEMORY_ACQ_REL)==1){xrtFree(clip->samples);xrtFree(clip);}}
int xgeSoundFromClip(xge_sound sound,const xge_sound_clip_desc_t *desc)
{
    if(!sound||!desc||!desc->clip||(desc->spatial!=0&&desc->spatial!=1)||
       (desc->group&&!desc->group->pBackend))return XGE_ERROR_INVALID_ARGUMENT;
    if(sound->pBackend)return XGE_ERROR_INVALID_STATE;
    if(!xgeAudioIsReady())return XGE_ERROR_NOT_INITIALIZED;
    xge_audio_clip *clip=desc->clip;uint64_t first=desc->first_frame;
    if(first>=clip->info.frames)return XGE_ERROR_INVALID_ARGUMENT;
    uint64_t frames=desc->frame_count?desc->frame_count:clip->info.frames-first;
    if(frames>clip->info.frames-first)return XGE_ERROR_INVALID_ARGUMENT;
    __xge_clip_sound *voice=xrtMalloc(sizeof(*voice));if(!voice)return XGE_ERROR_OUT_OF_MEMORY;
    memset(voice,0,sizeof(*voice));int result=xgeAudioClipAddRef(clip);if(result){xrtFree(voice);return result;}
    ma_result r=ma_audio_buffer_ref_init(ma_format_f32,clip->info.channels,
        clip->samples+(size_t)first*clip->info.channels,frames,&voice->buffer);
    if(r==MA_SUCCESS){voice->buffer.sampleRate=clip->info.sample_rate;
        r=ma_sound_init_from_data_source((ma_engine*)g_xge.pAudioEngine,&voice->buffer,
            desc->spatial?0:MA_SOUND_FLAG_NO_SPATIALIZATION,
            desc->group?(ma_sound_group*)desc->group->pBackend:NULL,&voice->sound);
        if(r!=MA_SUCCESS)ma_audio_buffer_ref_uninit(&voice->buffer);
    }
    if(r!=MA_SUCCESS){xgeAudioClipFree(clip);xrtFree(voice);return XGE_ERROR_AUDIO_FAILED;}
    voice->clip=clip;*sound=(xge_sound_t){1,XGE_AUDIO_SOUND,XGE_SOUND_CLIP,voice};++g_xge.iAudioCount;return XGE_OK;
}
