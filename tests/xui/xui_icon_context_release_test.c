#include "xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct icon_context_release_state {
    xui_context context;
    xui_lifetime_link_t watch;
    int callbacks;
    int contextGone;
} icon_context_release_state;
#define ICON_RELEASE_CHECK(condition) do { if(!(condition)) { \
    fprintf(stderr,"icon Context release failed at line %d\n",__LINE__);return 1; } } while(0)
static void iconContextGone(void* user)
{((icon_context_release_state*)user)->contextGone=1;}
static int iconDrawNoop(xui_icon icon,xui_painter painter,xui_rect_t rect,const xui_icon_draw_desc_t* desc,void* user)
{(void)icon;(void)painter;(void)rect;(void)desc;(void)user;return XUI_OK;}
static void iconDestroyRequestsContext(xui_icon icon,void* user)
{
    icon_context_release_state* state=user;
    if(++state->callbacks!=1){fputs("REENTERED_ICON_FINALIZER\n",stderr);exit(73);}
    const char* name=xuiIconGetName(icon);
    if(!name || strcmp(name,"icon")){fputs("ICON_METADATA_NOT_ALIVE\n",stderr);exit(74);}
    xuiDestroy(state->context);
    /* Exit before returning to an unsafe old caller: this is a controlled
     * failure, not a deliberate read of already-freed native storage. */
    if(state->contextGone){fputs("CONTEXT_DESTROYED_INSIDE_ICON_RELEASE\n",stderr);fflush(stderr);exit(73);}
    state->context=NULL;
}
static int xuiIconContextReleaseCase(int mode)
{
    icon_context_release_state state={0};xui_icon_category category=NULL;xui_icon icon=NULL;
    ICON_RELEASE_CHECK(xuiCreate(&state.context)==XUI_OK);
    ICON_RELEASE_CHECK(xuiContextLifetimeWatch(state.context,&state.watch,iconContextGone,&state)==XUI_OK);
    ICON_RELEASE_CHECK(xuiIconCategoryCreate(state.context,"icons",NULL,&category)==XUI_OK);
    xui_icon_custom_desc_t custom={0};custom.iSize=sizeof(custom);custom.onDraw=iconDrawNoop;
    custom.onDestroy=iconDestroyRequestsContext;custom.pUser=&state;
    ICON_RELEASE_CHECK(xuiIconAddCustom(category,"icon",&custom,NULL,&icon)==XUI_OK);
    xui_icon_id id=xuiIconGetId(icon);
    switch(mode){
    case 0:ICON_RELEASE_CHECK(xuiIconCategoryRemove(state.context,"icons")==XUI_OK);break;
    case 1:xuiIconCategoryClear(state.context);break;
    case 2:
        ICON_RELEASE_CHECK(xuiIconCategoryAddRef(category)==XUI_OK);
        ICON_RELEASE_CHECK(xuiIconCategoryRemove(state.context,"icons")==XUI_OK);
        ICON_RELEASE_CHECK(state.callbacks==0);
        ICON_RELEASE_CHECK(xuiIconCategoryRelease(category)==XUI_OK);break;
    case 3:ICON_RELEASE_CHECK(xuiIconRemoveById(category,id)==XUI_OK);break;
    case 4:ICON_RELEASE_CHECK(xuiIconRemove(category,"icon")==XUI_OK);break;
    case 5:xuiIconClear(category);break;
    case 6:
        ICON_RELEASE_CHECK(xuiIconAddRef(icon)==XUI_OK);
        ICON_RELEASE_CHECK(xuiIconRemoveById(category,id)==XUI_OK);
        ICON_RELEASE_CHECK(state.callbacks==0);
        ICON_RELEASE_CHECK(xuiIconRelease(icon)==XUI_OK);break;
    default:return 1;
    }
    ICON_RELEASE_CHECK(state.callbacks==1 && state.contextGone && !state.context);
    ICON_RELEASE_CHECK(!state.watch.ppPrevious && !state.watch.pNext && !state.watch.onInvalidate && !state.watch.pUser);
    return 0;
}
#ifndef XUI_ICON_CONTEXT_RELEASE_NO_MAIN
int main(void)
{
    for(int mode=0;mode<7;++mode)if(xuiIconContextReleaseCase(mode))return 1;
    puts("Icon release: seven Context-close finalizer paths passed");return 0;
}
#endif
