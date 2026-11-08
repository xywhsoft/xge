#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

typedef struct icon_state {int calls,mode; xui_rect_t last;} icon_state;
static int icon(xui_widget widget,int id,xui_draw_context draw,xui_rect_t rect,int state,void* user)
{
    (void)id;(void)draw;(void)state;icon_state* data=user;++data->calls;data->last=rect;
    if(data->mode==1) {xuiWidgetDestroy(widget);return 1;}
    if(data->mode==2) {xuiTreeViewSetIconRenderer(widget,NULL,NULL);return 1;}
    return 1;
}
#define CHECK(e) do{if(!(e)){fprintf(stderr,"xui_tree_icon_test:%d %s\n",__LINE__,#e);failed=1;goto cleanup;}}while(0)
int main(void)
{
    xui_test_proxy_state_t proxy;xuiTestProxyInit(&proxy);xui_context context=NULL;
    xui_widget root=NULL,tree=NULL;xui_surface target=NULL;int failed=0;icon_state state={0};
    CHECK(xuiTestSurfaceCreate(&proxy,&target,400,200,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    CHECK(xuiCreate(&context)==XUI_OK&&xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    CHECK(xuiInputViewport(context,400,200)==XUI_OK&&xuiWidgetCreate(context,&root)==XUI_OK);
    CHECK(xuiSetRootWidget(context,root)==XUI_OK&&xuiWidgetSetRect(root,(xui_rect_t){0,0,400,200})==XUI_OK);
    xui_tree_view_node_t nodes[]={{.iId=1,.iParent=-1,.sText="Root",.bExpanded=1,.bEnabled=1,.bHasChildren=1,.bIconReserved=1},{.iId=2,.iParent=1,.sText="Child",.bEnabled=1,.bIconReserved=1}};
    xui_tree_view_desc_t desc={0};desc.iSize=sizeof(desc);desc.arrNodes=nodes;desc.iNodeCount=2;
    CHECK(xuiTreeViewCreate(context,&tree,&desc)==XUI_OK&&xuiWidgetAddChild(root,tree)==XUI_OK&&xuiWidgetSetRect(tree,(xui_rect_t){0,0,400,200})==XUI_OK);
    
    CHECK(xuiTreeViewSetIconRenderer(tree,icon,&state)==XUI_OK&&xuiLayout(context)==XUI_OK);
    xui_rect_i_t dirty={0,0,400,200};int first=xuiRender(context,target,&dirty,1);CHECK(first==XUI_OK&&state.calls>=2&&state.last.fW>0&&state.last.fH>0);
    xui_rect_t before=xuiTreeViewGetItemRect(tree,1);CHECK(xuiTreeViewSetIconRenderer(tree,NULL,NULL)==XUI_OK&&xuiLayout(context)==XUI_OK);xui_rect_t after=xuiTreeViewGetItemRect(tree,1);CHECK(!memcmp(&before,&after,sizeof(before))&&xuiTreeViewGetVisibleNodeId(tree,1)==2);
    state.mode=2;state.calls=0;CHECK(xuiTreeViewSetIconRenderer(tree,icon,&state)==XUI_OK&&xuiRender(context,target,&dirty,1)==XUI_OK&&state.calls==1);
    state.mode=1;state.calls=0;CHECK(xuiTreeViewSetIconRenderer(tree,icon,&state)==XUI_OK&&xuiRender(context,target,&dirty,1)==XUI_OK&&state.calls==1);tree=NULL;
cleanup:if(context)xuiDestroy(context);if(target)proxy.tProxy.surfaceDestroy(&proxy.tProxy,target);if(!failed)puts("xui_tree_icon_test passed: native row geometry retained, icon callback, unbind fallback, reentrant replacement and destruction");return failed;
}
