/* CPU-only, real native publishers. Every installed callback owns distinct
 * userdata and ownership data; no function pointer is erased or cast. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xge_host_xrt.h"
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct HostEntry {const char* name;xge_host_xrt_proc address;} HostEntry;
static const HostEntry entries[]={
#define XGE_HOST_XRT_SLOT(name) {#name,(xge_host_xrt_proc)name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;size_t first=0,last=sizeof(entries)/sizeof(*entries);while(first<last){size_t middle=first+(last-first)/2;int order=strcmp(name,entries[middle].name);if(order<0)last=middle;else if(order>0)first=middle+1;else return entries[middle].address;}return NULL;}

typedef struct Paint {xvalue* value;bool ready;int failure;unsigned* dropped;} Paint;
static bool ready(const void* raw){return ((const Paint*)raw)->ready;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{const Paint* paint=raw;return visit(xrtValueOwnership(paint->value),user);}
static int drop(void* raw)
{
    Paint* paint=raw;int status=paint->failure;++*paint->dropped;
    xrtValueRelease(paint->value);xrtFree(paint);return status;
}
static const xui_text_paint_ownership_v1_t protocol={
    sizeof(protocol),XUI_TEXT_PAINT_OWNERSHIP_V1_TAG,ready,trace,drop
};
static unsigned edges;
static bool visit(xrtownershipref ref,void* user){(void)user;assert(ref.Data && ref.Ops);++edges;return true;}
static void contract(void)
{
    for(int failure=0;failure<2;++failure){
        unsigned dropped=0;Paint* paint=xrtCalloc(1,sizeof(*paint));assert(paint);
        paint->value=xrtValueInt(17);assert(paint->value);paint->dropped=&dropped;paint->failure=failure ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
        xui_text_shape_t shape={0};shape.iSize=sizeof(shape);shape.pPaint=paint;shape.pPaintOwnership=&protocol;
        shape.pClusters=xrtCalloc(2,sizeof(*shape.pClusters));shape.pCarets=xrtCalloc(2,sizeof(*shape.pCarets));assert(shape.pClusters && shape.pCarets);
        xui_text_shape_t before=shape;
        assert(xuiTextShapeFreeChecked(&shape)==XUI_STATUS_DEFERRED && !memcmp(&shape,&before,sizeof(shape)) && !dropped && !xrtGetError());
        paint->ready=true;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
        edges=0;assert(shape.pPaintOwnership->TraceRaw(shape.pPaint,visit,NULL) && edges==1);assert(xrtOwnershipScopeEnd(&freeze));
        if(failure)assert(xrtMemDebugFailAfter(0));
        assert(xuiTextShapeFreeChecked(&shape)==(failure ? XUI_ERROR_CALLBACK_FAILED:XUI_OK));
        if(failure)assert(xrtMemDebugFailAfter(-1));
        assert(dropped==1 && !shape.pPaint && !shape.pClusters && !shape.pCarets && !shape.pPaintOwnership && !xrtGetError());
        assert(xuiTextShapeFreeChecked(&shape)==XUI_OK && dropped==1);
    }
}
static int retryDrop(void* raw,bool* consumed)
{
    Paint* paint=raw;*consumed=paint->ready;
    if(!*consumed)return XUI_ERROR_BUSY;
    return drop(paint);
}
static void raw_retry_and_malformed(void)
{
    unsigned dropped=0;Paint* paint=xrtCalloc(1,sizeof(*paint));assert(paint);
    paint->value=xrtValueInt(8);paint->dropped=&dropped;
    xui_text_shape_t shape={0};shape.iSize=sizeof(shape);shape.pPaint=paint;shape.paintFreeChecked=retryDrop;
    assert(xuiTextShapeFreeChecked(&shape)==XUI_ERROR_BUSY && shape.pPaint==paint && shape.paintFreeChecked==retryDrop && !dropped);
    paint->ready=true;
    xui_text_paint_ownership_v1_t bad=protocol;bad.size=sizeof(bad)+1;shape.pPaintOwnership=&bad;
    xui_text_shape_t before=shape;assert(xuiTextShapeFreeChecked(&shape)==XUI_ERROR_INVALID_ARGUMENT && !memcmp(&before,&shape,sizeof(shape)));
    bad=protocol;bad.TraceRaw=NULL;assert(xuiTextShapeFreeChecked(&shape)==XUI_ERROR_INVALID_ARGUMENT && !dropped);
    shape.pPaintOwnership=NULL;assert(xuiTextShapeFreeChecked(&shape)==XUI_OK && dropped==1 && !shape.pPaint);
}
int main(void)
{
    const xge_host_xrt_contract_t contractValue=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contractValue,resolve,NULL,NULL)==XGE_HOST_XRT_OK);assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);contract();raw_retry_and_malformed();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("typed custom paint: real value edge, exact metadata, Ready DEFERRED preserves full output, consumed false-NULL/OOM and raw retry passed");return 0;
}
