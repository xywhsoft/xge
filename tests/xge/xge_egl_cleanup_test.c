/* Actual shared/static engine + real Mesa EGL. Faults affect only one cleanup
 * attempt; retry must finish exactly once and retain every still-owned handle. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if XGE_ENABLE_EGL
#include <EGL/egl.h>
#include "xge_egl_fault_preload.h"
#endif
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
    (void)unused;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif
int main(int argc,char** argv)
{
    assert(argc==3 || argc==4);int mode=atoi(argv[1]),stage=atoi(argv[2]);
#if !defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
    xge_egl_context_t context={0};
    assert(xgeEGLUnitChecked(NULL)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeEGLUnitChecked(&context)==XGE_OK);
    xge_egl_desc_t desc={0};desc.iWidth=desc.iHeight=4;desc.bPBuffer=1;
#if XGE_ENABLE_EGL
    assert(mode==0 || mode==6 || mode==7);
    assert(stage>=1 && stage<=4 && (mode==0 || stage>=(mode==6 ? 3:2)));
    xgeEglFaultSet(mode);
    if(mode)xgeEglCleanupFaultArm(stage);
    int result=xgeEGLInit(&context,&desc);
    if(!mode){
        assert(result==XGE_OK && context.bInitialized && context.bCleanupPending);
        assert(eglGetCurrentContext()==(EGLContext)context.pContext);
        xgeEglCleanupFaultArm(stage);
        /* Legacy void cleanup must retain duty too, without discarding it. */
        if(argc==4)assert(xgeEGLUnitChecked(&context)==XGE_ERROR_BACKEND_FAILED);
        else xgeEGLUnit(&context);
    }else assert(result==XGE_ERROR_BACKEND_FAILED);
    assert(!context.bInitialized && context.bCleanupPending && context.pDisplay);
    assert(context.iLastError==EGL_BAD_ACCESS);
    const char* names[]={"","eglUnbind","eglDestroyContext","eglDestroySurface","eglTerminate"};
    assert(!strcmp(context.sLastStage,names[stage]));
    assert((context.pContext!=NULL)==(mode!=6 && stage<=2));
    assert((context.pSurface!=NULL)==(stage<=3));
    assert(xgeEGLMakeCurrent(&context)==XGE_ERROR_NOT_INITIALIZED);
    int contexts=0,surfaces=0,terminated=0;
    xgeEglFaultCounts(&contexts,&surfaces,&terminated);
    assert(contexts==(mode!=6 && stage>=3) && surfaces==(stage>=4) && !terminated);
    assert(xgeEGLUnitChecked(&context)==XGE_OK);
    assert(eglGetCurrentContext()==EGL_NO_CONTEXT);
    xgeEglFaultCounts(&contexts,&surfaces,&terminated);
    assert(contexts==(mode!=6) && surfaces==1 && terminated==1);
    assert(xgeEGLUnitChecked(&context)==XGE_OK);
    int c2,s2,t2;xgeEglFaultCounts(&c2,&s2,&t2);
    assert(c2==contexts && s2==surfaces && t2==terminated);
#else
    (void)mode;(void)stage;
    assert(xgeEGLInit(&context,&desc)==XGE_ERROR_UNSUPPORTED);
    assert(!context.bInitialized && !context.bCleanupPending);
    assert(xgeEGLUnitChecked(&context)==XGE_OK);
#endif
    assert(!context.bInitialized && !context.bCleanupPending && !context.pDisplay &&
        !context.pSurface && !context.pContext && !xrtGetError());
    puts("EGL checked reverse-order cleanup, retained duty and exactly-once retry passed");return 0;
}
