/* Engine code residency and title ownership, including a bounded real window. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Owner {unsigned refs,holds,drops,mode;} Owner;
static bool count(const void* raw,size_t* output){*output=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw)
{
    Owner* owner=(Owner*)raw;++owner->holds;
    if(owner->mode==2)return false;
    ++owner->refs;
    if(owner->mode==3)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"init-hold",31,"first owner failure"));
    if(owner->mode==4){assert(xgeInit(NULL)==XGE_ERROR_BUSY);assert(xgeUnitChecked()==XGE_ERROR_BUSY);assert(!xgeGetTitle());}
    return true;
}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;assert(owner->refs==2);--owner->refs;++owner->drops;
    if(owner->mode==4){assert(xgeInit(NULL)==XGE_ERROR_INVALID_STATE);assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);}
    return owner->mode!=3 && owner->mode!=8;
}
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
    (void)unused;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
static int32 foreign_thread(void* unused)
{
    (void)unused;assert(xrtMemDebugFailAfter(0));
    assert(xgeSetTitleChecked("wrong thread")==XGE_ERROR_INVALID_STATE && !xgeGetTitle());
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();return 0;
}
static int calls;
static void window_title(const char* expected)
{
#if defined(_WIN32)
    char title[256];assert(GetWindowTextA((HWND)xgePlatformNativeHandle(),title,sizeof(title))>0);
    assert(!strcmp(title,expected));
#elif defined(__linux__)
    Display* display=XOpenDisplay(NULL);char* title=NULL;assert(display);
    assert(XFetchName(display,(Window)(uintptr_t)xgePlatformNativeHandle(),&title) && title);
    assert(!strcmp(title,expected));XFree(title);XCloseDisplay(display);
#else
    (void)expected;
#endif
}
static int frame(void* unused)
{
    (void)unused;++calls;window_title("original engine title");
    char title[]="updated running title";assert(xgeSetTitleChecked(title)==XGE_OK);
    memset(title,'?',sizeof(title)-1);window_title("updated running title");
    assert(!strcmp(xgeGetTitle(),"updated running title"));return 1;
}
int main(int argc,char** argv)
{
    assert(argc==2);unsigned mode=(unsigned)atoi(argv[1]);
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
    assert(xrtMemDebugEnable(true));
    /* Warm process-owned facilities before measuring per-engine cleanup. */
    assert(xgeInit(NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    Owner owner={1,0,0,mode};xge_callback_owner_t code={{&owner,&ops},hold,drop};
    char title[128]="original engine title";xge_desc_t desc={0};
    desc.iWidth=128;desc.iHeight=96;desc.sTitle=title;desc.iRunMode=mode==7 ? XGE_RUN_GAME_LOOP:XGE_RUN_MANUAL;
    if(mode==1){
        for(int budget=0;budget<6;++budget){
            assert(xrtMemDebugFailAfter(budget));int status=xgeInitWithCode(&desc,&code);
            xrtMemDebugFailClear();xrtClearError();
            assert(status==XGE_OK || status==XGE_ERROR_OUT_OF_MEMORY);
            if(status==XGE_OK){assert(owner.refs==2);assert(xgeUnitChecked()==XGE_OK);}
            assert(owner.refs==1 && owner.holds==owner.drops && !xgeGetTitle());
            xrtMemDebugSnapshot(&after);assert(after.LiveBytes==before.LiveBytes && after.LiveCount==before.LiveCount);
        }
    }else if(mode==2 || mode==3){
        assert(xgeInitWithCode(&desc,&code)==XGE_ERROR_INVALID_STATE);
        assert(owner.refs==1 && owner.holds==1 && owner.drops==(mode==3));
        if(mode==3)assert(xrtErrorCode(xrtGetError())==31);
        xrtClearError();assert(!xgeGetTitle() && xgeUnitChecked()==XGE_OK);
    }else{
        assert(xgeInitWithCode(&desc,&code)==XGE_OK && owner.refs==2 && owner.holds==1);
        strcpy(title,"mutated caller buffer");assert(!strcmp(xgeGetTitle(),"original engine title"));
        if(mode==0){
            assert(xgeSetTitleChecked(NULL)==XGE_ERROR_INVALID_ARGUMENT);
            const char* original=xgeGetTitle();assert(xrtMemDebugFailAfter(0));
            assert(xgeSetTitleChecked("replacement")==XGE_ERROR_OUT_OF_MEMORY);
            xrtMemDebugFailClear();xrtClearError();assert(xgeGetTitle()==original);
            assert(xgeSetTitleChecked(original)==XGE_OK && !strcmp(xgeGetTitle(),"original engine title"));
            char next[]="before run";assert(xgeSetTitleChecked(next)==XGE_OK);next[0]='?';assert(!strcmp(xgeGetTitle(),"before run"));
        }else if(mode==5){
            xge_texture texture=NULL;assert(xgeTextureCreateRGBAOwned(&texture,1,1,NULL,0)==XGE_OK);
            assert(xgeUnitChecked()==XGE_ERROR_BUSY && owner.refs==2 && !owner.drops);xgeTextureFree(texture);
        }else if(mode==6){
            xthread* thread=xrtThreadCreate(foreign_thread,NULL,0);assert(thread);
            assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK && !xrtThreadExitCode(thread));xrtThreadDestroy(thread);
        }else if(mode==7){assert(xgeRun(frame,NULL)==XGE_OK && calls==1 && owner.refs==2);}
        int status=xgeUnitChecked();assert(status==(mode==8 ? XGE_ERROR_INVALID_STATE:XGE_OK));
        xrtClearError();assert(owner.refs==1 && owner.drops==1 && !xgeGetTitle());
        assert(xgeUnitChecked()==XGE_OK && owner.drops==1);
    }
    xrtMemDebugSnapshot(&after);assert(after.LiveBytes==before.LiveBytes && after.LiveCount==before.LiveCount);
    printf("engine initialization ownership mode %u: PASS\n",mode);return 0;
}
