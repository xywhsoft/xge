/* Production Pass algorithm with a CPU renderer double. No window/GPU/GL
 * loader/device exists. Stub counters prove state ordering and single Flush. */
#include "include/xge.h"
#include "include/xui.h"
#include "src/xui/core/xui_proxy_resource.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned GLuint;
#define GL_FRAMEBUFFER 1
#define GL_SCISSOR_TEST 2
static struct {int bInitialized,bSokolRunning,iWidth,iHeight;uint32_t iCurrentFramebufferId;
 xge_camera_t tCamera;int bViewportEnabled,bClipEnabled;xge_rect_t tViewportRect,tClipRect;} g_xge;
static xatomic32 g_xgeCallbackShutdown;
static unsigned flushes,binds,viewports,applies,clears;
static int failFlush,failFramebuffer;static bool flushDiagnostic;
static xge_pass_t injected;
static bool probeFlush;
static void drawOperationsDenied(void);
static void bindFramebuffer(unsigned target,GLuint value){(void)target;(void)value;++binds;}
static void viewport(int x,int y,int w,int h){(void)x;(void)y;(void)w;(void)h;++viewports;}
static void disable(unsigned value){(void)value;}
static void (*xge_glBindFramebuffer)(unsigned,GLuint)=bindFramebuffer;
static void (*xge_glViewport)(int,int,int,int)=viewport;
static void (*xge_glDisable)(unsigned)=disable;
static void __xgeViewportApplyCurrent(void){++applies;}
static void __xgeClipApplyCurrent(void){++applies;}
static int __xgeRenderTargetEnsureFramebuffer(xge_render_target target){(void)target;return failFramebuffer;}
static void __xgeCallbackOutcome(int next,int* result,xerror** first)
{xerror* e=xrtTakeError();if(next==XGE_OK && !e)return;if(*result==XGE_OK){*result=next?next:XGE_ERROR_INVALID_STATE;*first=e;}else xrtErrorFree(e);}
xge_camera_t xgeCameraDefault(float width,float height){(void)width;(void)height;return (xge_camera_t){0};}
void xgeClear(uint32_t color){(void)color;++clears;}
int xgeFlush(void)
{
 ++flushes;drawOperationsDenied();
 if(probeFlush){assert(!xgePassIsIdle() && !xgePassIsCurrent(&injected));assert(xgePassBegin(&injected)==XGE_ERROR_INVALID_STATE);}
 if(flushDiagnostic)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"flush",-79,"secondary"));
 return failFlush;
}
#include "src/xge/render/xge_pass.inl"
#ifdef XGE_PASS_DRAW_METHODS
static unsigned renderCalls,queryCalls;
static xui_draw_context operationDraw;
static int __xuiProxyXgeDrawPayloadFinalize(xui_proxy proxy,void* raw){(void)proxy;(void)raw;return XGE_OK;}
static bool __xuiProxyXgeSurfaceTargetValid(xui_surface surface){return surface!=NULL;}
static int __xuiProxyXgeClearRectLocal(xui_rect_t rect,uint32_t color){(void)rect;(void)color;++renderCalls;return XGE_OK;}
static int __xuiProxyXgeSurfaceDraw(xui_proxy proxy,xui_surface surface,xui_rect_t a,xui_rect_t b,uint32_t color,uint32_t flags)
{(void)proxy;(void)surface;(void)a;(void)b;(void)color;(void)flags;++renderCalls;return XGE_OK;}
void xgeShapePointPx(float x,float y,float size,uint32_t color){(void)x;(void)y;(void)size;(void)color;++renderCalls;}
static bool countedCurrent(const xge_pass_t* pass){++queryCalls;return xgePassIsCurrent(pass);}
#define xgePassIsCurrent countedCurrent
#include XGE_PASS_DRAW_METHODS
#undef xgePassIsCurrent
static void drawOperations(bool permitted)
{
    unsigned before=renderCalls,queries=queryCalls;int expected=permitted ? XGE_OK:XGE_ERROR_INVALID_ARGUMENT;
    xui_proxy_t proxy={0};xui_rect_t rect={0,0,1,1};
    assert(__xuiProxyXgeDrawClearRect(&proxy,operationDraw,rect,UINT32_MAX)==expected);
    assert(__xuiProxyXgeDrawSurface(&proxy,operationDraw,(void*)(uintptr_t)1,rect,rect,UINT32_MAX,0)==expected);
    assert(__xuiProxyXgeDrawPoint(&proxy,operationDraw,1,1,2,UINT32_MAX)==expected);
    assert(renderCalls==before+(permitted?3:0) && queryCalls==queries+3);
}
static void drawOperationsDenied(void){if(operationDraw)drawOperations(false);}
static void productionDrawOperations(void)
{
    struct xui_draw_context_t draw={0};operationDraw=&draw;
    draw.iMagic=draw.tOwned.tResource.iMagic=XUI_PROXY_XGE_DRAW_MAGIC;draw.tOwned.pTarget=(void*)(uintptr_t)1;
    xge_render_target_t target={.iWidth=10,.iHeight=20,.iFlags=XGE_RENDER_TARGET_WINDOW};
    xgePassInit(&draw.tPass,&target,0,0);drawOperations(false);
    assert(xgePassBegin(&draw.tPass)==XGE_OK);drawOperations(true);assert(draw.bDirty);
    xge_pass_t child;xgePassInit(&child,&target,0,0);assert(xgePassBegin(&child)==XGE_OK);drawOperations(false);
    assert(xgePassEnd(&child)==XGE_OK);drawOperations(true);
    /* The exact TargetEnd body preserves a failed NULL primary over a later
     * failing Flush diagnostic, then consumes the pass just once. */
    flushDiagnostic=true;failFlush=-77;
    assert(__xuiProxyXgeTargetEnd(&draw.tPass,-66)==-66 && !xrtGetError() && xgePassIsIdle());
    flushDiagnostic=false;failFlush=0;operationDraw=NULL;
}
#else
static void drawOperationsDenied(void){}
static void productionDrawOperations(void){}
#endif
static xge_render_target_t target={.iWidth=10,.iHeight=20,.iFlags=XGE_RENDER_TARGET_WINDOW};
static void init(xge_pass_t* pass){xgePassInit(pass,&target,0,0);}
static int32 worker(void* raw)
{assert(!xgePassIsIdle() && !xgePassIsCurrent(raw));return 0;}
static void passOrder(void)
{
 xge_pass_t a,b;init(&a);init(&b);init(&injected);g_xge.bInitialized=g_xge.bSokolRunning=1;
 g_xge.iWidth=31;g_xge.iHeight=41;g_xge.iCurrentFramebufferId=9;
 probeFlush=true;assert(xgePassBegin(&a)==XGE_OK && flushes==1);
 assert(xgePassIsCurrent(&a) && !xgePassIsIdle());
 xge_pass_t saved=a;init(&a);assert(!memcmp(&saved,&a,sizeof(a)));xrtClearError();
 assert(xgePassBegin(&b)==XGE_OK && flushes==2 && xgePassIsCurrent(&b));
 assert(xgePassEnd(&a)==XGE_ERROR_INVALID_STATE && flushes==2 && xgePassIsCurrent(&b));
 xthread* thread=xrtThreadCreate(worker,&b,0);assert(thread);assert(xrtThreadWait(thread)==XWAIT_OK);xrtThreadDestroy(thread);
 assert(xgePassEnd(&b)==XGE_OK && flushes==3 && xgePassIsCurrent(&a));
 assert(xgePassEnd(&a)==XGE_OK && flushes==4 && xgePassIsIdle());
 assert(g_xge.iWidth==31 && g_xge.iHeight==41 && g_xge.iCurrentFramebufferId==9);
 assert(xgePassEnd(&a)==XGE_ERROR_INVALID_STATE && flushes==4);
 /* Top End after backend stop consumes exactly once and restores CPU state. */
 assert(xgePassBegin(&a)==XGE_OK);unsigned before=flushes;g_xge.bSokolRunning=0;
 assert(xgePassEnd(&a)==XGE_ERROR_NOT_INITIALIZED && xgePassIsIdle() && !a.bActive && flushes==before);
 assert(g_xge.iWidth==31 && g_xge.iHeight==41 && xgePassEnd(&a)==XGE_ERROR_INVALID_STATE);
 g_xge.bSokolRunning=1;failFlush=-71;
 assert(xgePassBegin(&a)==-71 && xgePassIsIdle() && !a.bActive && !xrtGetError());failFlush=0;
 assert(xgePassBegin(&a)==XGE_OK);failFlush=-72;
 assert(xgePassEnd(&a)==-72 && xgePassIsIdle() && !xrtGetError());failFlush=0;
 assert(xgePassBegin(&a)==XGE_OK);flushDiagnostic=true;failFlush=-73;
 assert(xgePassEnd(&a)==-73 && xrtErrorCode(xrtGetError())==-79 && xgePassIsIdle());xrtClearError();flushDiagnostic=false;failFlush=0;
 probeFlush=false;
}
typedef struct DrawPayload {xge_pass_t pass;unsigned* ended;} DrawPayload;
static bool drawReady(const void* payload){const DrawPayload* p=payload;return xgePassIsCurrent(&p->pass);}
static int drawEnd(xui_proxy proxy,void* payload){(void)proxy;DrawPayload* p=payload;++*p->ended;return xgePassEnd(&p->pass);}
static void ownedInterleave(void)
{
 xui_proxy_t table={0};table.iSize=sizeof(table);table.iVersion=XUI_PROXY_VERSION;xui_proxy proxy=NULL;
 assert(xuiInternalProxyCreateOwned(&table,NULL,true,0,&proxy)==XUI_OK);xui_draw_context draw=NULL;DrawPayload* payload=NULL;unsigned ended=0;
 assert(xuiDrawCreateOwnedEx(proxy,NULL,sizeof(*payload),drawEnd,drawReady,&draw,(void**)&payload)==XUI_OK);
 payload->ended=&ended;init(&payload->pass);assert(xgePassBegin(&payload->pass)==XGE_OK);
 xrtownershipref ref=xuiDrawOwnership(draw);const xrtownershipfinishv2* completion=xuiProxyResourceOwnershipCompletionV2(ref);assert(completion && completion->V1.size==48);
 const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);assert(adapter->Hold(ref.Data));
 xui_draw_scope_t scope={0};assert(xuiDrawCallScopeBegin(draw,&scope)==XUI_OK);
 xge_pass_t child;init(&child);assert(xgePassBegin(&child)==XGE_OK);
 xerror* sentinel=xrtErrorCreate(XERR_STATE,"sentinel",-80,"keep");assert(sentinel);xrtSetErrorTake(sentinel);
 assert(xrtMemDebugFailAfter(0));bool consumed=true;
 assert(xuiDrawEndChecked(draw)==XUI_ERROR_BUSY && !xuiDrawEnded(draw));
 assert(xuiDrawReleaseOwnerChecked(draw,&consumed)==XUI_ERROR_BUSY && !consumed && !ended);
 assert(!completion->DropReady(ref.Data) && xrtGetError()==sentinel && !xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
 /* No End request was published; the regular input scope can finish safely. */
 assert(xuiDrawCallScopeEnd(&scope)==XUI_OK && !ended);
 assert(xgePassEnd(&child)==XGE_OK && xgePassIsCurrent(&payload->pass));
 assert(xuiDrawEndChecked(draw)==XUI_OK && ended==1 && xuiDrawEnded(draw) && xgePassIsIdle());
 assert(xuiDrawReleaseChecked(draw)==XUI_OK && completion->DropChecked(ref.Data));assert(xuiProxyReleaseChecked(proxy)==XUI_OK);
}
int main(void)
{
 assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 passOrder();productionDrawOperations();ownedInterleave();xrtClearError();xrtMemDebugSnapshot(&after);
 assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
 puts("Production Pass CPU-double ordering, single reservation Flush, stopped-backend restore and owned CanEnd: PASS");return 0;
}
