#ifndef XGE_NO_AUDIO
#include <limits.h>
/* Music/Stream keep their public nominal struct tags. Project their identical
 * scalar members at proved offsets, rather than dereference an incompatible
 * Sound struct lvalue. Each access is the actual member's C scalar type. */
#define XGE_AUDIO_RECORD_LAYOUT(Type) \
    _Static_assert(sizeof(Type)==sizeof(xge_sound_t) && \
        offsetof(Type,iRefCount)==offsetof(xge_sound_t,iRefCount) && \
        offsetof(Type,iType)==offsetof(xge_sound_t,iType) && \
        offsetof(Type,iFlags)==offsetof(xge_sound_t,iFlags) && \
        offsetof(Type,pBackend)==offsetof(xge_sound_t,pBackend) && \
        _Generic(&((Type*)0)->iRefCount,int*:1,default:0) && \
        _Generic(&((Type*)0)->iType,int*:1,default:0) && \
        _Generic(&((Type*)0)->iFlags,uint32_t*:1,default:0) && \
        _Generic(&((Type*)0)->pBackend,void**:1,default:0),"audio record scalar ABI")
XGE_AUDIO_RECORD_LAYOUT(xge_sound_t);
XGE_AUDIO_RECORD_LAYOUT(xge_music_t);
XGE_AUDIO_RECORD_LAYOUT(xge_stream_t);
#define XGE_AUDIO_REFS(p) (*(int*)((unsigned char*)(p)+offsetof(xge_sound_t,iRefCount)))
#define XGE_AUDIO_TYPE(p) (*(int*)((unsigned char*)(p)+offsetof(xge_sound_t,iType)))
#define XGE_AUDIO_FLAGS(p) (*(uint32_t*)((unsigned char*)(p)+offsetof(xge_sound_t,iFlags)))
#define XGE_AUDIO_BACKEND(p) (*(void**)((unsigned char*)(p)+offsetof(xge_sound_t,pBackend)))
/* Public records remain small. Their private backends own the parent links;
 * a group handle may disappear while voices still retain its mixer node. */
typedef struct __xge_audio_group {
    ma_sound_group group;
    uint32_t references;
} __xge_audio_group;
typedef struct __xge_audio_sound {
    ma_sound sound; /* Existing control calls keep their first-member ABI. */
    __xge_audio_group* group;
} __xge_audio_sound;
static bool g_xgeAudioTransition;
#define XGE_SOUND_OWNED_RECORD 0x20000000u

static int __xgeAudioControlAccess(void)
{
    (void)xrtSpinLock(&g_xgeGraphicsLifetime.lock);
    int result=XGE_OK;
    if(g_xgeGraphicsLifetime.creatorThread && g_xgeGraphicsLifetime.creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    else if(g_xgeGraphicsLifetime.closing || g_xgeGraphicsLifetime.initializing || g_xgeAudioTransition)result=XGE_ERROR_BUSY;
    else if(!g_xge.bInitialized)result=XGE_ERROR_NOT_INITIALIZED;
    (void)xrtSpinUnlock(&g_xgeGraphicsLifetime.lock);return result;
}
static void __xgeAudioReport(int result)
{if(result!=XGE_OK && !xrtGetError())xrtSetErrorKind(XERR_STATE);}
static int __xgeAudioGroupRetain(__xge_audio_group* group)
{
    if(!group)return XGE_OK;
    if(!group->references || group->references==UINT32_MAX)return XGE_ERROR_INVALID_STATE;
    ++group->references;return XGE_OK;
}
static void __xgeAudioGroupRelease(__xge_audio_group* group)
{
    if(!group || --group->references)return;
    ma_sound_group_uninit(&group->group);xrtFree(group);
    (void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);
}
#include "xge_audio_clip.inc"
static xge_sound __xgeAudioAsSound(void* pObject)
{
	return (xge_sound)pObject;
}

static int __xgeAudioLoadPath(void* pObject, const char* sPath, int iType, uint32_t iFlags, xge_audio_group pGroup)
{
	xge_sound pSound;
	ma_sound* pMaSound;
	__xge_audio_sound* backend;
	ma_sound_group* pMaGroup;
	char* sFullPath;
	ma_uint32 iMaFlags;
	ma_result iMaResult;

	if ( (pObject == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;
	if ( !g_xge.bAudioInitialized ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	pSound = __xgeAudioAsSound(pObject);
	if(XGE_AUDIO_BACKEND(pSound) || XGE_AUDIO_REFS(pSound))return XGE_ERROR_INVALID_STATE;
	if(g_xge.iAudioCount==INT_MAX)return XGE_ERROR_OUT_OF_MEMORY;
	if(pGroup && !pGroup->pBackend)return XGE_ERROR_INVALID_ARGUMENT;
	access=__xgeAudioOwnerAcquire(&g_xgeGraphicsLifetime);if(access!=XGE_OK)return access;
	/* miniaudio initializes its complete sound; only the appended owner field
	 * is written here after successful initialization. Avoid a second clear. */
	backend=xrtMalloc(sizeof(*backend));pMaSound=backend ? &backend->sound:NULL;
	if ( backend == NULL ) {
		(void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	access = __xgePathResolve(sPath,&sFullPath);
	if ( access != XGE_OK ) {
		xrtFree(backend);(void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);
		__xgeLogFormat(XGE_LOG_WARN, "audio", "audio path resolve failed: %s", sPath);
		return access;
	}
	iMaFlags = iFlags;
	pMaGroup = NULL;
	if ( pGroup != NULL ) {
		pMaGroup = (ma_sound_group*)pGroup->pBackend;
		access=__xgeAudioGroupRetain((__xge_audio_group*)pMaGroup);
		if(access!=XGE_OK){xrtFree(sFullPath);xrtFree(backend);(void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);return access;}
	}
	#if defined(_WIN32) || defined(_WIN64)
	{
		int iWideLength = MultiByteToWideChar(CP_UTF8, 0, sFullPath, -1, NULL, 0);
		wchar_t* sWidePath = NULL;
		if ( iWideLength > 0 ) {
			sWidePath = (wchar_t*)xrtMalloc((size_t)iWideLength * sizeof(*sWidePath));
			if ( sWidePath != NULL ) {
				(void)MultiByteToWideChar(CP_UTF8, 0, sFullPath, -1, sWidePath, iWideLength);
				iMaResult = ma_sound_init_from_file_w((ma_engine*)g_xge.pAudioEngine, sWidePath, iMaFlags, pMaGroup, NULL, pMaSound);
				xrtFree(sWidePath);
			} else {
				iMaResult = MA_OUT_OF_MEMORY;
			}
		} else {
			iMaResult = MA_INVALID_FILE;
		}
	}
	#else
	iMaResult = ma_sound_init_from_file((ma_engine*)g_xge.pAudioEngine, sFullPath, iMaFlags, pMaGroup, NULL, pMaSound);
	#endif
	if ( iMaResult != MA_SUCCESS ) {
		__xgeLogFormat(XGE_LOG_WARN, "audio", "audio load failed: %s", sFullPath);
		xrtFree(sFullPath);
		__xgeAudioGroupRelease((__xge_audio_group*)pMaGroup);
		xrtFree(backend);(void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);
		return iMaResult==MA_OUT_OF_MEMORY ? XGE_ERROR_OUT_OF_MEMORY:XGE_ERROR_AUDIO_FAILED;
	}
	xrtFree(sFullPath);
	backend->group=(__xge_audio_group*)pMaGroup;
	XGE_AUDIO_REFS(pSound) = 1;
	XGE_AUDIO_TYPE(pSound) = iType;
	XGE_AUDIO_FLAGS(pSound) = iFlags;
	XGE_AUDIO_BACKEND(pSound) = pMaSound;
	g_xge.iAudioCount++;
	return XGE_OK;
}

static int __xgeAudioLoadFallback(void* pObject, int iType, uint32_t iFlags, xge_audio_group pGroup, int iOriginalError)
{
	xge_sound pSound;
	int iRet;

	if ( (iType != XGE_AUDIO_SOUND) || (g_xge.sFallbackSoundPath == NULL) ) {
		return iOriginalError;
	}
	iRet = __xgeAudioLoadPath(pObject, g_xge.sFallbackSoundPath, iType, iFlags, pGroup);
	if ( iRet != XGE_OK ) {
		return iOriginalError;
	}
	pSound = __xgeAudioAsSound(pObject);
	XGE_AUDIO_FLAGS(pSound) |= XGE_SOUND_FALLBACK;
	return XGE_OK;
}

static int __xgeAudioLoad(void* pObject, const char* sPath, int iType, uint32_t iFlags, xge_audio_group pGroup)
{
	int iRet;

	iRet = __xgeAudioLoadPath(pObject, sPath, iType, iFlags, pGroup);
	if ( iRet == XGE_ERROR_AUDIO_FAILED || iRet == XGE_ERROR_RESOURCE_FAILED || iRet == XGE_ERROR_UNSUPPORTED ) {
		return __xgeAudioLoadFallback(pObject, iType, iFlags, pGroup, iRet);
	}
	return iRet;
}

int xgeAudioInit(void)
{
	ma_allocation_callbacks tAlloc;
	ma_engine_config tConfig;
	ma_engine* pEngine;

	int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;
	if ( g_xge.bAudioInitialized ) {
		return XGE_OK;
	}
	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	pEngine = (ma_engine*)xrtMalloc(sizeof(*pEngine));
	if ( pEngine == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(&tAlloc, 0, sizeof(tAlloc));
	tAlloc.onMalloc = __xgeMaMalloc;
	tAlloc.onRealloc = __xgeMaRealloc;
	tAlloc.onFree = __xgeMaFree;
	tConfig = ma_engine_config_init();
	tConfig.allocationCallbacks = tAlloc;
	g_xgeAudioTransition=true;
	ma_result initialized=ma_engine_init(&tConfig, pEngine);
	if ( initialized != MA_SUCCESS ) {
		g_xgeAudioTransition=false;
		xrtFree(pEngine);
		__xgeLogError("audio", "audio engine init failed.");
		return initialized==MA_OUT_OF_MEMORY ? XGE_ERROR_OUT_OF_MEMORY:XGE_ERROR_AUDIO_FAILED;
	}
	g_xge.pAudioEngine = pEngine;
	g_xge.bAudioInitialized = 1;
	g_xgeAudioTransition=false;
	xgeAudioListenerSet(&g_xge.tAudioListener);
	return XGE_OK;
}

/* Core Unit already holds global admission closed; do not reopen it here. */
static void __xgeAudioUnit(void)
{
	if ( g_xge.bAudioInitialized == 0 ) {
		return;
	}
	g_xgeAudioTransition=true;
	ma_engine_uninit((ma_engine*)g_xge.pAudioEngine);
	xrtFree(g_xge.pAudioEngine);
	g_xge.pAudioEngine = NULL;
	g_xge.bAudioInitialized = 0;
	g_xgeAudioTransition=false;
}

int xgeAudioUnitChecked(void)
{
    int access=__xgeAudioControlAccess();
    if(access==XGE_ERROR_NOT_INITIALIZED)return XGE_OK;
    if(access!=XGE_OK)return access;
    if(__xgeAudioOwnerCount(&g_xgeGraphicsLifetime))return XGE_ERROR_BUSY;
    __xgeAudioUnit();return XGE_OK;
}
void xgeAudioUnit(void){__xgeAudioReport(xgeAudioUnitChecked());}

int xgeAudioIsReady(void)
{
	int access=__xgeAudioControlAccess();
	if(access!=XGE_OK){if(access!=XGE_ERROR_NOT_INITIALIZED)__xgeAudioReport(access);return 0;}
	return g_xge.bAudioInitialized;
}

void xgeAudioSetVolume(float fVolume)
{
	int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}
	if ( g_xge.bAudioInitialized ) {
		ma_engine_set_volume((ma_engine*)g_xge.pAudioEngine, fVolume);
	}
}

float xgeAudioGetVolume(void)
{
	int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return 0;}
	if ( g_xge.bAudioInitialized == 0 ) {
		return 0.0f;
	}
	return ma_engine_get_volume((ma_engine*)g_xge.pAudioEngine);
}

void xgeAudioListenerSet(const xge_audio_listener_t* pListener)
{
	if ( pListener == NULL ) {
		return;
	}
	int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}
	g_xge.tAudioListener = *pListener;
	if ( g_xge.bAudioInitialized ) {
		ma_engine_listener_set_position((ma_engine*)g_xge.pAudioEngine, 0, pListener->tPosition.fX, pListener->tPosition.fY, pListener->tPosition.fZ);
		ma_engine_listener_set_direction((ma_engine*)g_xge.pAudioEngine, 0, pListener->tForward.fX, pListener->tForward.fY, pListener->tForward.fZ);
		ma_engine_listener_set_world_up((ma_engine*)g_xge.pAudioEngine, 0, pListener->tUp.fX, pListener->tUp.fY, pListener->tUp.fZ);
	}
}

xge_audio_listener_t xgeAudioListenerGet(void)
{
	int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return (xge_audio_listener_t){0};}
	return g_xge.tAudioListener;
}

int xgeAudioGroupInit(xge_audio_group pGroup)
{
	ma_sound_group* pMaGroup;
	__xge_audio_group* backend;

	if ( pGroup == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;
	if ( !g_xge.bAudioInitialized ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if(pGroup->pBackend)return XGE_ERROR_INVALID_STATE;
	access=__xgeAudioOwnerAcquire(&g_xgeGraphicsLifetime);if(access!=XGE_OK)return access;
	/* The native group initializer clears its own sound node. */
	backend=xrtMalloc(sizeof(*backend));pMaGroup=backend ? &backend->group:NULL;
	if ( backend == NULL ) {
		(void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	ma_result initialized=ma_sound_group_init((ma_engine*)g_xge.pAudioEngine, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, pMaGroup);
	if ( initialized != MA_SUCCESS ) {
		xrtFree(backend);(void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);
		return initialized==MA_OUT_OF_MEMORY ? XGE_ERROR_OUT_OF_MEMORY:XGE_ERROR_AUDIO_FAILED;
	}
	pGroup->fVolume = 1.0f;
	pGroup->pBackend = pMaGroup;
	backend->references=1;
	return XGE_OK;
}

int xgeAudioGroupFreeChecked(xge_audio_group pGroup)
{
	if ( pGroup == NULL ) {
		return XGE_OK;
	}
	if ( pGroup->pBackend != NULL ) {
		int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;
		__xgeAudioGroupRelease((__xge_audio_group*)pGroup->pBackend);
	}
	memset(pGroup, 0, sizeof(*pGroup));
	return XGE_OK;
}
void xgeAudioGroupFree(xge_audio_group group){__xgeAudioReport(xgeAudioGroupFreeChecked(group));}

void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume)
{
	if ( pGroup == NULL ) {
		return;
	}
	if(pGroup->pBackend){int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}}
	pGroup->fVolume = fVolume;
	if ( pGroup->pBackend != NULL ) {
		ma_sound_group_set_volume((ma_sound_group*)pGroup->pBackend, fVolume);
	}
}

float xgeAudioGroupGetVolume(xge_audio_group pGroup)
{
	if ( pGroup == NULL ) {
		return 0.0f;
	}
	if ( pGroup->pBackend != NULL ) {
		int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return 0;}
		pGroup->fVolume = ma_sound_group_get_volume((ma_sound_group*)pGroup->pBackend);
	}
	return pGroup->fVolume;
}

void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds)
{
	if ( (pGroup != NULL) && (pGroup->pBackend != NULL) ) {
		int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}
		pGroup->fVolume = fTo;
		ma_sound_group_set_fade_in_milliseconds((ma_sound_group*)pGroup->pBackend, fFrom, fTo, (ma_uint64)iMilliseconds);
	}
}

int xgeSoundLoad(xge_sound pSound, const char* sPath)
{
	return xgeSoundLoadGroup(pSound, sPath, NULL);
}

int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup)
{
	return __xgeAudioLoad(pSound, sPath, XGE_AUDIO_SOUND, 0, pGroup);
}

int xgeSoundFallbackSet(const char* sPath)
{
	char* sCopy;

	if ( sPath == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sCopy = __xgeStrDup(sPath);
	if ( sCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeSoundFallbackClear();
	g_xge.sFallbackSoundPath = sCopy;
	return XGE_OK;
}

int xgeSoundFallbackGet(xge_sound pSound)
{
	int iRet;

	if ( pSound == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.sFallbackSoundPath == NULL ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iRet = __xgeAudioLoadPath(pSound, g_xge.sFallbackSoundPath, XGE_AUDIO_SOUND, 0, NULL);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	XGE_AUDIO_FLAGS(pSound) |= XGE_SOUND_FALLBACK;
	return XGE_OK;
}

void xgeSoundFallbackClear(void)
{
	if ( g_xge.sFallbackSoundPath != NULL ) {
		xrtFree(g_xge.sFallbackSoundPath);
		g_xge.sFallbackSoundPath = NULL;
	}
}

int xgeSoundPlay(xge_sound pSound)
{
	if ( (pSound == NULL) || (XGE_AUDIO_BACKEND(pSound) == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;
	return ma_sound_start((ma_sound*)XGE_AUDIO_BACKEND(pSound)) == MA_SUCCESS ? XGE_OK : XGE_ERROR_AUDIO_FAILED;
}

int xgeSoundStop(xge_sound pSound)
{
	if ( (pSound == NULL) || (XGE_AUDIO_BACKEND(pSound) == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;
	if ( ma_sound_stop((ma_sound*)XGE_AUDIO_BACKEND(pSound)) != MA_SUCCESS ) {
		return XGE_ERROR_AUDIO_FAILED;
	}
	ma_sound_seek_to_pcm_frame((ma_sound*)XGE_AUDIO_BACKEND(pSound), 0);
	return XGE_OK;
}

int xgeSoundPause(xge_sound pSound)
{
	if ( (pSound == NULL) || (XGE_AUDIO_BACKEND(pSound) == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;
	return ma_sound_stop((ma_sound*)XGE_AUDIO_BACKEND(pSound)) == MA_SUCCESS ? XGE_OK : XGE_ERROR_AUDIO_FAILED;
}

int xgeSoundResume(xge_sound pSound)
{
	return xgeSoundPlay(pSound);
}

int xgeSoundAddRef(xge_sound pSound)
{
	if(!pSound)return 0;
	/* A held record's backend is stable. Reject another control thread before
	 * reading its mutable, creator-thread reference count. */
	if(XGE_AUDIO_BACKEND(pSound) && __xgeAudioControlAccess()!=XGE_OK)return 0;
	/* Empty/dead records cannot be resurrected; overflow must not report a
	 * retained reference which was never acquired. Match Font's contract. */
	if ( XGE_AUDIO_REFS(pSound) <= 0 || XGE_AUDIO_REFS(pSound) == INT_MAX ) {
		return 0;
	}
	XGE_AUDIO_REFS(pSound)++;
	return XGE_AUDIO_REFS(pSound);
}

int xgeSoundFreeChecked(xge_sound pSound)
{
	if ( pSound == NULL ) {
		return XGE_OK;
	}
	if(XGE_AUDIO_BACKEND(pSound)){int access=__xgeAudioControlAccess();if(access!=XGE_OK)return access;}
	if ( XGE_AUDIO_REFS(pSound) > 1 ) {
		XGE_AUDIO_REFS(pSound)--;
		return XGE_OK;
	}
	bool owned=(XGE_AUDIO_FLAGS(pSound)&XGE_SOUND_OWNED_RECORD)!=0;
	if ( XGE_AUDIO_BACKEND(pSound) != NULL ) {
		__xge_audio_sound* backend=(__xge_audio_sound*)XGE_AUDIO_BACKEND(pSound);
		ma_sound_uninit((ma_sound*)XGE_AUDIO_BACKEND(pSound));
		if ( XGE_AUDIO_FLAGS(pSound) & XGE_SOUND_CLIP ) {
			__xge_clip_sound* pClipSound = (__xge_clip_sound*)XGE_AUDIO_BACKEND(pSound);
			ma_audio_buffer_ref_uninit(&pClipSound->buffer);
			xgeAudioClipFree(pClipSound->clip);
		}
		__xgeAudioGroupRelease(backend->group);
		xrtFree(XGE_AUDIO_BACKEND(pSound));
		--g_xge.iAudioCount;(void)__xgeAudioOwnerRelease(&g_xgeGraphicsLifetime);
	}
	if(owned)xrtFree(pSound);else memset(pSound, 0, sizeof(*pSound));
	return XGE_OK;
}
void xgeSoundFree(xge_sound sound){__xgeAudioReport(xgeSoundFreeChecked(sound));}

int xgeSoundCreateOwned(xge_sound* out,const char* path,xge_audio_group group)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;if(!path)return XGE_ERROR_INVALID_ARGUMENT;
    xge_sound sound=xrtCalloc(1,sizeof(*sound));if(!sound)return XGE_ERROR_OUT_OF_MEMORY;
    int result=xgeSoundLoadGroup(sound,path,group);
    if(result!=XGE_OK){xrtFree(sound);return result;}
    sound->iFlags|=XGE_SOUND_OWNED_RECORD;*out=sound;return XGE_OK;
}
int xgeSoundCreateFromClipOwned(xge_sound* out,const xge_sound_clip_desc_t* desc)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;if(!desc)return XGE_ERROR_INVALID_ARGUMENT;
    xge_sound sound=xrtCalloc(1,sizeof(*sound));if(!sound)return XGE_ERROR_OUT_OF_MEMORY;
    int result=xgeSoundFromClip(sound,desc);
    if(result!=XGE_OK){xrtFree(sound);return result;}
    sound->iFlags|=XGE_SOUND_OWNED_RECORD;*out=sound;return XGE_OK;
}
int xgeMusicCreateOwned(xge_music* out,const char* path,xge_audio_group group)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;if(!path)return XGE_ERROR_INVALID_ARGUMENT;
    xge_music music=xrtCalloc(1,sizeof(*music));if(!music)return XGE_ERROR_OUT_OF_MEMORY;
    int result=xgeMusicLoadGroup(music,path,group);
    if(result!=XGE_OK){xrtFree(music);return result;}
    music->iFlags|=XGE_SOUND_OWNED_RECORD;*out=music;return XGE_OK;
}
int xgeStreamCreateOwned(xge_stream* out,const char* path,xge_audio_group group)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;if(!path)return XGE_ERROR_INVALID_ARGUMENT;
    xge_stream stream=xrtCalloc(1,sizeof(*stream));if(!stream)return XGE_ERROR_OUT_OF_MEMORY;
    int result=xgeStreamOpenGroup(stream,path,group);
    if(result!=XGE_OK){xrtFree(stream);return result;}
    stream->iFlags|=XGE_SOUND_OWNED_RECORD;*out=stream;return XGE_OK;
}

void xgeSoundSetLoop(xge_sound pSound, int bLoop)
{
	if ( (pSound != NULL) && (XGE_AUDIO_BACKEND(pSound) != NULL) ) {
		int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}
		ma_sound_set_looping((ma_sound*)XGE_AUDIO_BACKEND(pSound), bLoop ? MA_TRUE : MA_FALSE);
	}
}

void xgeSoundSetVolume(xge_sound pSound, float fVolume)
{
	if ( (pSound != NULL) && (XGE_AUDIO_BACKEND(pSound) != NULL) ) {
		int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}
		ma_sound_set_volume((ma_sound*)XGE_AUDIO_BACKEND(pSound), fVolume);
	}
}

void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ)
{
	if ( (pSound != NULL) && (XGE_AUDIO_BACKEND(pSound) != NULL) ) {
		int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}
		ma_sound_set_positioning((ma_sound*)XGE_AUDIO_BACKEND(pSound), ma_positioning_absolute);
		ma_sound_set_position((ma_sound*)XGE_AUDIO_BACKEND(pSound), fX, fY, fZ);
	}
}

void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds)
{
	if ( (pSound != NULL) && (XGE_AUDIO_BACKEND(pSound) != NULL) ) {
		int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return;}
		ma_sound_set_fade_in_milliseconds((ma_sound*)XGE_AUDIO_BACKEND(pSound), fFrom, fTo, (ma_uint64)iMilliseconds);
	}
}

int xgeSoundIsPlaying(xge_sound pSound)
{
	if ( (pSound == NULL) || (XGE_AUDIO_BACKEND(pSound) == NULL) ) {
		return 0;
	}
	int access=__xgeAudioControlAccess();if(access!=XGE_OK){__xgeAudioReport(access);return 0;}
	return ma_sound_is_playing((ma_sound*)XGE_AUDIO_BACKEND(pSound)) ? 1 : 0;
}

int xgeMusicLoad(xge_music pMusic, const char* sPath)
{
	return xgeMusicLoadGroup(pMusic, sPath, NULL);
}

int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup)
{
	return __xgeAudioLoad(pMusic, sPath, XGE_AUDIO_MUSIC, MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION, pGroup);
}

int xgeMusicPlay(xge_music pMusic)
{
	return xgeSoundPlay((xge_sound)pMusic);
}

int xgeMusicStop(xge_music pMusic)
{
	return xgeSoundStop((xge_sound)pMusic);
}

int xgeMusicPause(xge_music pMusic)
{
	return xgeSoundPause((xge_sound)pMusic);
}

int xgeMusicResume(xge_music pMusic)
{
	return xgeSoundResume((xge_sound)pMusic);
}

void xgeMusicFree(xge_music pMusic)
{
	xgeSoundFree((xge_sound)pMusic);
}

void xgeMusicSetLoop(xge_music pMusic, int bLoop)
{
	xgeSoundSetLoop((xge_sound)pMusic, bLoop);
}

void xgeMusicSetVolume(xge_music pMusic, float fVolume)
{
	xgeSoundSetVolume((xge_sound)pMusic, fVolume);
}

void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds)
{
	xgeSoundFade((xge_sound)pMusic, fFrom, fTo, iMilliseconds);
}

int xgeMusicIsPlaying(xge_music pMusic)
{
	return xgeSoundIsPlaying((xge_sound)pMusic);
}

int xgeStreamOpen(xge_stream pStream, const char* sPath)
{
	return xgeStreamOpenGroup(pStream, sPath, NULL);
}

int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup)
{
	return __xgeAudioLoad(pStream, sPath, XGE_AUDIO_STREAM, MA_SOUND_FLAG_STREAM, pGroup);
}

int xgeStreamPlay(xge_stream pStream)
{
	return xgeSoundPlay((xge_sound)pStream);
}

int xgeStreamStop(xge_stream pStream)
{
	return xgeSoundStop((xge_sound)pStream);
}

int xgeStreamPause(xge_stream pStream)
{
	return xgeSoundPause((xge_sound)pStream);
}

int xgeStreamResume(xge_stream pStream)
{
	return xgeSoundResume((xge_sound)pStream);
}

void xgeStreamClose(xge_stream pStream)
{
	xgeSoundFree((xge_sound)pStream);
}

void xgeStreamSetLoop(xge_stream pStream, int bLoop)
{
	xgeSoundSetLoop((xge_sound)pStream, bLoop);
}

void xgeStreamSetVolume(xge_stream pStream, float fVolume)
{
	xgeSoundSetVolume((xge_sound)pStream, fVolume);
}

void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ)
{
	xgeSoundSetPosition((xge_sound)pStream, fX, fY, fZ);
}

void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds)
{
	xgeSoundFade((xge_sound)pStream, fFrom, fTo, iMilliseconds);
}

int xgeStreamIsPlaying(xge_stream pStream)
{
	return xgeSoundIsPlaying((xge_sound)pStream);
}
#undef XGE_AUDIO_RECORD_LAYOUT
#undef XGE_AUDIO_REFS
#undef XGE_AUDIO_TYPE
#undef XGE_AUDIO_FLAGS
#undef XGE_AUDIO_BACKEND
#undef XGE_SOUND_OWNED_RECORD
#else
int xgeAudioClipDecodeMemory(const xge_audio_clip_decode_t* d,xge_audio_clip** out) { (void)d;if(out)*out=NULL;return XGE_ERROR_UNSUPPORTED; }
xge_audio_clip_info_t xgeAudioClipInfo(const xge_audio_clip* c) { (void)c;return (xge_audio_clip_info_t){0}; }
const float* xgeAudioClipSamples(const xge_audio_clip* c) { (void)c;return NULL; }
int xgeAudioClipAddRef(xge_audio_clip* c) { (void)c;return XGE_ERROR_UNSUPPORTED; }
void xgeAudioClipFree(xge_audio_clip* c) { (void)c; }
int xgeSoundFromClip(xge_sound s,const xge_sound_clip_desc_t* d) { (void)s;(void)d;return XGE_ERROR_UNSUPPORTED; }
int xgeAudioInit(void) { return XGE_ERROR_UNSUPPORTED; }
static void __xgeAudioUnit(void) {}
void xgeAudioUnit(void) {}
int xgeAudioUnitChecked(void) {return XGE_ERROR_UNSUPPORTED;}
int xgeSoundFreeChecked(xge_sound sound) {(void)sound;return XGE_ERROR_UNSUPPORTED;}
int xgeAudioGroupFreeChecked(xge_audio_group group) {(void)group;return XGE_ERROR_UNSUPPORTED;}
int xgeSoundCreateOwned(xge_sound* out,const char* path,xge_audio_group group)
{(void)path;(void)group;if(out)*out=NULL;return XGE_ERROR_UNSUPPORTED;}
int xgeSoundCreateFromClipOwned(xge_sound* out,const xge_sound_clip_desc_t* desc)
{(void)desc;if(out)*out=NULL;return XGE_ERROR_UNSUPPORTED;}
int xgeMusicCreateOwned(xge_music* out,const char* path,xge_audio_group group)
{(void)path;(void)group;if(out)*out=NULL;return XGE_ERROR_UNSUPPORTED;}
int xgeStreamCreateOwned(xge_stream* out,const char* path,xge_audio_group group)
{(void)path;(void)group;if(out)*out=NULL;return XGE_ERROR_UNSUPPORTED;}
int xgeAudioIsReady(void) { return 0; }
void xgeAudioSetVolume(float fVolume) { (void)fVolume; }
float xgeAudioGetVolume(void) { return 0.0f; }
void xgeAudioListenerSet(const xge_audio_listener_t* pListener) { (void)pListener; }
xge_audio_listener_t xgeAudioListenerGet(void) { xge_audio_listener_t tListener; memset(&tListener, 0, sizeof(tListener)); return tListener; }
int xgeAudioGroupInit(xge_audio_group pGroup) { (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
void xgeAudioGroupFree(xge_audio_group pGroup) { (void)pGroup; }
void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume) { (void)pGroup; (void)fVolume; }
float xgeAudioGroupGetVolume(xge_audio_group pGroup) { (void)pGroup; return 0.0f; }
void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds) { (void)pGroup; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeSoundLoad(xge_sound pSound, const char* sPath) { (void)pSound; (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup) { (void)pSound; (void)sPath; (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundFallbackSet(const char* sPath) { (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundFallbackGet(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
void xgeSoundFallbackClear(void) {}
int xgeSoundPlay(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundStop(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundPause(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundResume(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundAddRef(xge_sound pSound) { (void)pSound; return 0; }
void xgeSoundFree(xge_sound pSound) { (void)pSound; }
void xgeSoundSetLoop(xge_sound pSound, int bLoop) { (void)pSound; (void)bLoop; }
void xgeSoundSetVolume(xge_sound pSound, float fVolume) { (void)pSound; (void)fVolume; }
void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ) { (void)pSound; (void)fX; (void)fY; (void)fZ; }
void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds) { (void)pSound; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeSoundIsPlaying(xge_sound pSound) { (void)pSound; return 0; }
int xgeMusicLoad(xge_music pMusic, const char* sPath) { (void)pMusic; (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup) { (void)pMusic; (void)sPath; (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicPlay(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicStop(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicPause(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicResume(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
void xgeMusicFree(xge_music pMusic) { (void)pMusic; }
void xgeMusicSetLoop(xge_music pMusic, int bLoop) { (void)pMusic; (void)bLoop; }
void xgeMusicSetVolume(xge_music pMusic, float fVolume) { (void)pMusic; (void)fVolume; }
void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds) { (void)pMusic; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeMusicIsPlaying(xge_music pMusic) { (void)pMusic; return 0; }
int xgeStreamOpen(xge_stream pStream, const char* sPath) { (void)pStream; (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup) { (void)pStream; (void)sPath; (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamPlay(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamStop(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamPause(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamResume(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
void xgeStreamClose(xge_stream pStream) { (void)pStream; }
void xgeStreamSetLoop(xge_stream pStream, int bLoop) { (void)pStream; (void)bLoop; }
void xgeStreamSetVolume(xge_stream pStream, float fVolume) { (void)pStream; (void)fVolume; }
void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ) { (void)pStream; (void)fX; (void)fY; (void)fZ; }
void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds) { (void)pStream; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeStreamIsPlaying(xge_stream pStream) { (void)pStream; return 0; }
#endif
