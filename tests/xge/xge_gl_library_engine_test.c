/* Actual static XGE + one complete XRT, actual local GLX windows. Wrapping OS
 * entry points accounts for OUR loader references and injects close failures;
 * it never replaces GL code or invokes a fake function address. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <xge.h>
#include <xge_static_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void* handle;
static int references,opens,closes,failClose,reenter,frames,holds,drops,running;
static xge_texture texture;
static void reenterUnit(void)
{
    if(!reenter)return;
    reenter=0;
    /* Sokol and the candidate resolver each own one explicit OS reference. */
    assert(references==2 && holds==1 && !drops);
    int before=references;
    assert(xgeUnitChecked()==XGE_OK && !xrtGetError());
    assert(references==before && holds==1 && !drops);
}
#if defined(_WIN32)
typedef HMODULE (WINAPI *LoadFn)(LPCSTR);
typedef HMODULE (WINAPI *LoadExFn)(LPCSTR,HANDLE,DWORD);
typedef BOOL (WINAPI *FreeFn)(HMODULE);
typedef FARPROC (WINAPI *ProcFn)(HMODULE,LPCSTR);
typedef FARPROC (WINAPI *WglFn)(LPCSTR);
extern LoadFn __real___imp_LoadLibraryA;
extern LoadExFn __real___imp_LoadLibraryExA;
extern FreeFn __real___imp_FreeLibrary;
extern ProcFn __real___imp_GetProcAddress;
static WglFn realWgl;
static HMODULE track(const char* name,HMODULE library)
{
    if(library && name && !strcmp(name,"opengl32.dll")){
        assert(!handle || handle==(void*)library);handle=(void*)library;++references;++opens;
    }
    return library;
}
static HMODULE WINAPI loadLibrary(LPCSTR name)
{return track(name,__real___imp_LoadLibraryA(name));}
static HMODULE WINAPI loadLibraryEx(LPCSTR name,HANDLE file,DWORD flags)
{return track(name,__real___imp_LoadLibraryExA(name,file,flags));}
static BOOL WINAPI freeLibrary(HMODULE library)
{
    if((void*)library==handle){
        assert(references);if(failClose){SetLastError(ERROR_BUSY);return FALSE;}
        BOOL result=__real___imp_FreeLibrary(library);
        if(result){++closes;if(!--references)handle=NULL;}return result;
    }
    return __real___imp_FreeLibrary(library);
}
static FARPROC WINAPI wglProc(LPCSTR name)
{if(!strcmp(name,"glBindFramebuffer"))reenterUnit();return realWgl(name);}
static FARPROC WINAPI getProc(HMODULE library,LPCSTR name)
{
    FARPROC result=__real___imp_GetProcAddress(library,name);
    if((void*)library==handle && name && !strcmp(name,"wglGetProcAddress")){
        realWgl=(WglFn)result;return (FARPROC)wglProc;
    }
    return result;
}
LoadFn __wrap___imp_LoadLibraryA=loadLibrary;
LoadExFn __wrap___imp_LoadLibraryExA=loadLibraryEx;
FreeFn __wrap___imp_FreeLibrary=freeLibrary;
ProcFn __wrap___imp_GetProcAddress=getProc;
#else
void* __real_dlopen(const char*,int);
void* __real_dlsym(void*,const char*);
int __real_dlclose(void*);
void* __wrap_dlopen(const char* name,int flags)
{
    void* raw=__real_dlopen(name,flags);
    if(raw && name && (!strcmp(name,"libGL.so.1") || !strcmp(name,"libGL.so"))){
        assert(!handle || handle==raw);handle=raw;++references;++opens;
    }
    return raw;
}
int __wrap_dlclose(void* raw)
{
    if(raw==handle){
        assert(references);
        /* After Run only XGE's resolver reference remains. */
        if(failClose && references==1)return -1;
        int result=__real_dlclose(raw);if(!result){++closes;if(!--references)handle=NULL;}
        return result;
    }
    return __real_dlclose(raw);
}
void* __wrap_dlsym(void* raw,const char* name)
{
    if(!strcmp(name,"glBindFramebuffer"))reenterUnit();
    return __real_dlsym(raw,name);
}
#endif
static bool hold(const void* raw){assert(raw==&holds);++holds;return true;}
static bool drop(const void* raw)
{
    /* During Sokol cleanup its OWN reference still protects OS teardown.
     * XGE's separate resolver owner has already been retired. */
    assert(raw==&holds && holds==1);
    assert(references==(running ? 1:0));
    ++drops;--holds;
    xge_desc_t desc={0};
    assert(xgeInit(&desc)==XGE_ERROR_INVALID_STATE);
    assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
    return true;
}
static bool count(const void* raw,size_t* output)
{assert(raw==&holds);*output=(size_t)holds+1;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{assert(raw==&holds);(void)visit;(void)user;return true;}
static const xrtownershipops referenceOps={count,trace};
static int frame(void* raw)
{
    ++frames;
    if(raw){const unsigned char white[]={255,255,255,255};
        assert(xgeTextureCreateRGBAOwned(&texture,1,1,white,sizeof(white))==XGE_OK);}
    return 1;
}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0 && mode<=3);
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
    xge_callback_owner_t owner={0};owner.tReference.Data=&holds;owner.tReference.Ops=&referenceOps;owner.onHold=hold;owner.onDrop=drop;
    xge_desc_t desc={0};desc.iWidth=128;desc.iHeight=96;desc.sTitle="GL resolver ownership";
    if(mode==3){
        assert(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
        fprintf(stderr,"After Unit: resolver libraries=%d\n",references);
        assert(!references && opens==closes);return 0;
    }
    assert(xgeInitWithCode(&desc,&owner)==XGE_OK && holds==1 && !drops);
    reenter=mode==2;
    running=1;assert(xgeRun(frame,mode==1 ? &texture:NULL)==XGE_OK);running=0;
    if(mode==2){assert(!frames && !holds && drops==1 && !references && opens==closes);}
    else {
        assert(frames==1 && holds==1 && !drops && references==1);
        if(mode==1){
            assert(xgeUnitChecked()==XGE_ERROR_BUSY && references==1 && !drops);
            xgeTextureFree(texture);assert(!xgeGraphicsOwnedResourceCount());
        }
        failClose=1;
        for(int i=0;i<3;++i){
            assert(xgeUnitChecked()==XGE_ERROR_BACKEND_FAILED && references==1 && holds==1 && !drops);
            assert(xgeInit(&desc)==XGE_ERROR_BUSY);
        }
        failClose=0;assert(xgeUnitChecked()==XGE_OK && !references && !holds && drops==1 && opens==closes);
    }
    for(int i=0;i<3;++i){
        assert(xgeInitWithCode(&desc,&owner)==XGE_OK);
        assert(xgeRun(frame,NULL)==XGE_OK && references==1);
        assert(xgeUnitChecked()==XGE_OK && !holds && !references && opens==closes);
    }
    assert(drops==4 && !xrtGetError());
    printf("GL library engine mode %d: checked close retry, code retention, Busy and same-process restart passed\n",mode);
    return 0;
}
