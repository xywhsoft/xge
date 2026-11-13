#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct icon_watch_state {
    xui_context context;xui_icon_category category;xui_icon icon;
    xui_lifetime_link_t categoryLink,iconLink,peerLink;
    int categoryGone,iconGone,peerGone,customGone,requestClose,errorOnDestroy;
} icon_watch_state;
static void watchPeer(void* user){++((icon_watch_state*)user)->peerGone;}
static void watchIcon(void* user)
{
    icon_watch_state* s=user;++s->iconGone;
    assert(!s->iconLink.ppPrevious && !s->iconLink.pNext && !s->iconLink.onInvalidate && !s->iconLink.pUser);
    xuiLifetimeUnwatch(&s->peerLink);
    xui_lifetime_link_t denied={0};
    assert(xuiIconLifetimeWatch(s->icon,&denied,watchPeer,s)==XUI_ERROR_INVALID_STATE);
    assert(xuiIconAddRef(s->icon)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiIconRelease(s->icon)==XUI_ERROR_INVALID_ARGUMENT);
}
static void watchCategory(void* user)
{
    icon_watch_state* s=user;++s->categoryGone;
    assert(!s->categoryLink.ppPrevious && !s->categoryLink.pNext && !s->categoryLink.onInvalidate && !s->categoryLink.pUser);
    xui_lifetime_link_t denied={0};
    assert(xuiIconCategoryLifetimeWatch(s->category,&denied,watchPeer,s)==XUI_ERROR_INVALID_STATE);
    assert(xuiIconCategoryAddRef(s->category)==XUI_ERROR_INVALID_ARGUMENT);
    if(s->requestClose){
        xuiDestroy(s->context);
        xui_icon_category rejected=NULL;
        assert(xuiIconCategoryCreate(s->context,"closing",NULL,&rejected)==XUI_ERROR_INVALID_STATE && !rejected);
    }
}
static int watchDraw(xui_icon a,xui_painter b,xui_rect_t c,const xui_icon_draw_desc_t*d,void*e)
{(void)a;(void)b;(void)c;(void)d;(void)e;return XUI_OK;}
static void watchCustomDestroy(xui_icon icon,void* user)
{
    icon_watch_state* s=user;++s->customGone;
    assert(s->iconGone==1 && s->peerGone==0);
    assert(!strcmp(xuiIconGetName(icon),"icon"));
    assert(xuiIconGetId(icon)!=XUI_ICON_ID_INVALID);
    if(s->errorOnDestroy)xrtSetErrorKind(XERR_TYPE);
}
static void setup(icon_watch_state* s)
{
    assert(xuiCreate(&s->context)==XUI_OK);
    assert(xuiIconCategoryCreate(s->context,"category",NULL,&s->category)==XUI_OK);
    xui_icon_custom_desc_t custom={0};custom.iSize=sizeof(custom);custom.onDraw=watchDraw;custom.onDestroy=watchCustomDestroy;custom.pUser=s;
    assert(xuiIconAddCustom(s->category,"icon",&custom,NULL,&s->icon)==XUI_OK);
    assert(xuiIconCategoryLifetimeWatch(s->category,&s->categoryLink,watchCategory,s)==XUI_OK);
    assert(xuiIconLifetimeWatch(s->icon,&s->peerLink,watchPeer,s)==XUI_OK);
    assert(xuiIconLifetimeWatch(s->icon,&s->iconLink,watchIcon,s)==XUI_OK);
    assert(xuiIconLifetimeWatch(s->icon,&s->categoryLink,watchPeer,s)==XUI_ERROR_INVALID_STATE);
}
int main(void)
{
    icon_watch_state s={0};setup(&s);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i){
        xui_lifetime_link_t a={0},b={0};
        assert(xuiIconCategoryLifetimeWatch(s.category,&a,watchPeer,&s)==XUI_OK);
        assert(xuiIconLifetimeWatch(s.icon,&b,watchPeer,&s)==XUI_OK);
        xuiLifetimeUnwatch(&a);xuiLifetimeUnwatch(&b);
    }
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(xuiIconRemove(s.category,"icon")==XUI_OK);assert(s.iconGone==1 && !s.categoryGone && s.customGone==1);
    xuiDestroy(s.context);assert(s.categoryGone==1);

    memset(&s,0,sizeof(s));setup(&s);
    assert(xuiIconCategoryAddRef(s.category)==XUI_OK && xuiIconAddRef(s.icon)==XUI_OK);
    assert(xuiIconCategoryRemove(s.context,"category")==XUI_OK);assert(!s.categoryGone && !s.iconGone);
    assert(xuiIconCategoryRelease(s.category)==XUI_OK);assert(s.categoryGone==1 && !s.iconGone);
    assert(xuiIconRelease(s.icon)==XUI_OK);assert(s.iconGone==1 && s.customGone==1);xuiDestroy(s.context);

    memset(&s,0,sizeof(s));setup(&s);xui_icon alias=NULL;
    assert(xuiIconAddAlias(s.category,"alias",s.icon,NULL,&alias)==XUI_OK);
    assert(xuiIconRemove(s.category,"icon")==XUI_OK);assert(!s.iconGone);
    assert(xuiIconRemove(s.category,"alias")==XUI_OK);assert(s.iconGone==1 && s.customGone==1);xuiDestroy(s.context);

    memset(&s,0,sizeof(s));setup(&s);
    assert(xuiIconCategoryAddRef(s.category)==XUI_OK && xuiIconAddRef(s.icon)==XUI_OK);
    xuiDestroy(s.context);assert(s.categoryGone==1 && s.iconGone==1 && s.customGone==1);

    memset(&s,0,sizeof(s));setup(&s);s.requestClose=1;
    assert(xuiIconCategoryRemove(s.context,"category")==XUI_OK);
    assert(s.categoryGone==1 && s.iconGone==1 && s.customGone==1);
    memset(&s,0,sizeof(s));setup(&s);bool accepted=false;
    assert(xuiIconRemoveOwnerChecked(s.icon,&accepted)==XUI_OK && accepted);
    assert(s.iconGone==1 && !s.categoryGone && s.customGone==1);
    accepted=false;assert(xuiIconCategoryRemoveOwnerChecked(s.category,&accepted)==XUI_OK && accepted);
    assert(s.categoryGone==1);xuiDestroy(s.context);

    memset(&s,0,sizeof(s));setup(&s);
    assert(xuiIconCategoryAddRef(s.category)==XUI_OK && xuiIconAddRef(s.icon)==XUI_OK);
    accepted=false;assert(xuiIconCategoryRemoveOwnerChecked(s.category,&accepted)==XUI_OK && accepted);
    accepted=false;assert(xuiIconCategoryRemoveOwnerChecked(s.category,&accepted)==XUI_OK && accepted);
    assert(!s.categoryGone && !s.iconGone);
    assert(xuiIconCategoryRelease(s.category)==XUI_OK && s.categoryGone==1);
    accepted=false;assert(xuiIconRemoveOwnerChecked(s.icon,&accepted)==XUI_OK && accepted);
    assert(!s.iconGone);assert(xuiIconRelease(s.icon)==XUI_OK && s.iconGone==1);xuiDestroy(s.context);
    memset(&s,0,sizeof(s));setup(&s);s.errorOnDestroy=1;
    accepted=false;assert(xuiIconRemoveOwnerChecked(s.icon,&accepted)==XUI_ERROR_CALLBACK_FAILED && accepted);
    assert(s.iconGone==1 && xrtGetError()!=NULL);xrtClearError();xuiDestroy(s.context);

    memset(&s,0,sizeof(s));setup(&s);s.errorOnDestroy=1;
    accepted=false;assert(xuiIconCategoryRemoveOwnerChecked(s.category,&accepted)==XUI_ERROR_CALLBACK_FAILED && accepted);
    assert(s.iconGone==1 && s.categoryGone==1 && xrtGetError()!=NULL);xrtClearError();xuiDestroy(s.context);
    puts("Icon watches: physical expiry, peer detach, retained/alias refs, Context close and 1000 zero-heap registrations passed");return 0;
}
