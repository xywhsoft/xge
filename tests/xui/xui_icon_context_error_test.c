/* The new release receipt must preserve both callback errors and an older
 * ambient diagnostic while a finalizer requests Context destruction. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct icon_error_state {
    xui_context context;xui_lifetime_link_t watch;xerror* expected;
    int order;int gone;int callbacks;
} icon_error_state;
static void iconErrorGone(void* user){((icon_error_state*)user)->gone=1;}
static int iconErrorDraw(xui_icon a,xui_painter b,xui_rect_t c,const xui_icon_draw_desc_t* d,void* e)
{(void)a;(void)b;(void)c;(void)d;(void)e;return XUI_OK;}
static void iconWriteError(icon_error_state* state)
{xrtSetErrorKind(XERR_TYPE);state->expected=xrtErrorRef(xrtGetError());assert(state->expected);}
static void iconErrorDestroy(xui_icon icon,void* user)
{
    icon_error_state* state=user;(void)icon;assert(++state->callbacks==1);
    if(state->order==1)iconWriteError(state);
    xuiDestroy(state->context);
    if(state->gone){fputs("PREMATURE_CONTEXT_IN_ERROR_CALLBACK\n",stderr);exit(73);}
    if(state->order==2)iconWriteError(state);
}
static int iconErrorCase(int useVoid,int ambientPresent,int order)
{
    icon_error_state state={0};state.order=order;xui_icon_category category=NULL;xui_icon icon=NULL;
    assert(xrtGetError()==NULL && xuiCreate(&state.context)==XUI_OK);
    assert(xuiContextLifetimeWatch(state.context,&state.watch,iconErrorGone,&state)==XUI_OK);
    assert(xuiIconCategoryCreate(state.context,"errors",NULL,&category)==XUI_OK);
    xui_icon_custom_desc_t custom={0};custom.iSize=sizeof(custom);custom.onDraw=iconErrorDraw;
    custom.onDestroy=iconErrorDestroy;custom.pUser=&state;
    assert(xuiIconAddCustom(category,"icon",&custom,NULL,&icon)==XUI_OK);
    xerror* ambient=NULL;if(ambientPresent){xrtSetErrorKind(XERR_RANGE);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    int status=XUI_OK;
    if(useVoid)xuiIconCategoryClear(state.context);
    else status=xuiIconCategoryRemove(state.context,"errors");
    xerror* error=xrtTakeError();assert(state.callbacks==1 && state.gone);
    if(order){assert(error==state.expected);if(!useVoid)assert(status==XUI_ERROR_CALLBACK_FAILED);}
    else assert(status==XUI_OK && error==ambient);
    xrtErrorFree(error);xrtErrorFree(state.expected);xrtErrorFree(ambient);return 0;
}
int main(void)
{
    for(int useVoid=0;useVoid<2;++useVoid)
        for(int ambient=0;ambient<2;++ambient)
            for(int order=0;order<3;++order)if(iconErrorCase(useVoid,ambient,order))return 1;
    puts("Icon release: 12 ambient/callback diagnostic and Context-close cases passed");return 0;
}
