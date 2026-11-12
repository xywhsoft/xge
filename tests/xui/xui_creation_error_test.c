/* Real native constructors, callbacks and controls; the proxy supplies CPU
 * font geometry only. No callback or error result is replaced by a stub. */
#include "xui_internal.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define CHECK(v) do { ++checks; if(!(v)){fprintf(stderr,"creation-error:%d:%s\n",__LINE__,#v);exit(1);} } while(0)
typedef struct state {xui_context context;xui_widget owner;int mode,init,destroy,dropError;} state;
typedef struct part_data {xui_widget part;} part_data;
static int init(xui_widget widget,void* data,const void* value,void* user)
{
    (void)data;(void)value;state* s=user;++s->init;
    CHECK(!xrtGetError());
    if(s->mode==1)return XUI_ERROR_UNSUPPORTED;
    if(s->mode==2){xrtSetErrorKind(XERR_RANGE);return XUI_OK;}
    if(s->mode==3){xrtSetErrorKind(XERR_RANGE);return XUI_ERROR_UNSUPPORTED;}
    if(s->mode==4)CHECK(xuiRequestClose(s->context)==XUI_OK);
    if(s->mode==5)xuiWidgetDestroy(widget);
    if(s->mode==6)xuiWidgetDestroy(s->owner);
    return XUI_OK;
}
static void destroy(xui_widget widget,void* data,void* user)
{(void)widget;(void)data;state* s=user;++s->destroy;if(s->dropError)xrtSetErrorKind(XERR_STATE);}
static void contextGone(void* value){*(xui_context*)value=NULL;}
static xui_widget_type type(xui_context c,const char* name,xui_widget_type parent,state* s)
{
    xui_widget_type result=NULL;
    xui_widget_type_desc_t d={.iSize=sizeof(d),.sName=name,.pParent=parent,
        .iTypeDataSize=sizeof(part_data),.pUser=s,.onInit=init,.onDestroy=destroy};
    CHECK(xuiWidgetRegisterType(c,&result,&d)==XUI_OK && result);return result;
}
static const xerror* seed(void)
{xrtSetErrorInfo(XERR_IO,"creation-test",73,"an earlier handled operation");CHECK(xrtGetError());return xrtGetError();}
static void constructors(int mode,int dropError,int nested)
{
    xui_context c=NULL;CHECK(xuiCreate(&c)==XUI_OK);xui_lifetime_link_t life={0};CHECK(xuiContextLifetimeWatch(c,&life,contextGone,&c)==XUI_OK);
    state parent={.context=c,.dropError=dropError},child={.context=c,.mode=mode};
    xui_widget_type p=type(c,"ambient.parent",NULL,&parent),t=type(c,"ambient.child",p,&child);
    xui_call_scope_t outer={0};const xerror* previous=seed();if(nested)CHECK(xuiCallScopeBegin(c,&outer)==XUI_OK);
    xui_widget output=(xui_widget)(uintptr_t)1;int status=xuiWidgetCreateTyped(c,t,&output,NULL);
    CHECK(parent.init==1 && child.init==1);
    if(mode==0){CHECK(status==XUI_OK && output && xrtGetError()==(nested?NULL:previous));xuiWidgetDestroy(output);if(!nested)CHECK(parent.destroy==1 && child.destroy==1);}
    else if(mode==1){CHECK(status==XUI_ERROR_UNSUPPORTED && !output && !xrtGetError());if(!nested)CHECK(parent.destroy==1 && !child.destroy);}
    else if(mode==2 || mode==3){CHECK(status==(mode==2?XUI_ERROR_CALLBACK_FAILED:XUI_ERROR_UNSUPPORTED) && !output && xrtErrorKind(xrtGetError())==XERR_RANGE);if(!nested)CHECK(parent.destroy==1 && child.destroy==(mode==2));}
    else {CHECK(status!=XUI_OK && !output && parent.destroy==1 && child.destroy==1);}
    if(nested){CHECK(!parent.destroy && !child.destroy);int ended=xuiCallScopeEnd(&outer);/* The C caller handled integer-only refusal; an outer receipt sees only a fresh TLS diagnostic. */CHECK(ended==(mode<2?XUI_OK:XUI_ERROR_CALLBACK_FAILED));if(mode<2)CHECK(xrtGetError()==previous);CHECK(parent.destroy==1 && child.destroy==(mode==0 || mode==2));}
    xrtClearError();xuiLifetimeUnwatch(&life);if(c)CHECK(xuiDestroyChecked(c)==XUI_OK);xrtClearError();
}
static void parent_diagnostic(void)
{
    xui_context c=NULL;CHECK(xuiCreate(&c)==XUI_OK);
    state parent={.context=c,.mode=2},child={.context=c};
    xui_widget_type p=type(c,"failure.parent",NULL,&parent),t=type(c,"unreached.child",p,&child);
    seed();xui_widget output=(xui_widget)(uintptr_t)1;
    CHECK(xuiWidgetCreateTyped(c,t,&output,NULL)==XUI_ERROR_CALLBACK_FAILED && !output && xrtErrorKind(xrtGetError())==XERR_RANGE);
    CHECK(parent.init==1 && child.init==0 && parent.destroy==1 && child.destroy==0);
    xrtClearError();CHECK(xuiDestroyChecked(c)==XUI_OK);xrtClearError();
}
static void parts(int closeOwner)
{
    xui_context c=NULL;CHECK(xuiCreate(&c)==XUI_OK);state ownerState={.context=c},child={.context=c,.mode=closeOwner?6:0};xui_widget_type ownerType=type(c,"part.owner",NULL,&ownerState),partType=type(c,"part.child",NULL,&child);xui_widget owner=NULL;CHECK(xuiWidgetCreateTyped(c,ownerType,&owner,NULL)==XUI_OK);child.owner=owner;part_data* data=xuiWidgetGetTypeData(owner);CHECK(data);
    const xerror* old=seed();int status=xuiInternalWidgetCreatePart(owner,partType,&data->part,NULL);
    if(closeOwner){CHECK(status!=XUI_OK && ownerState.destroy==1 && child.destroy==1);}
    else {CHECK(status==XUI_OK && data->part && xrtGetError()==old);xrtClearError();xuiWidgetDestroy(owner);CHECK(ownerState.destroy==1 && child.destroy==1);}
    xrtClearError();CHECK(xuiDestroyChecked(c)==XUI_OK);xrtClearError();
}
static void controls(void)
{
    xui_test_proxy_state_t proxy;xuiTestProxyInit(&proxy);xui_context c=NULL;xui_font font=NULL;CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&proxy.tProxy)==XUI_OK);CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK && xuiSetDefaultFont(c,font)==XUI_OK);
    const xerror* old=seed();xui_widget base=NULL;CHECK(xuiWidgetCreate(c,&base)==XUI_OK && base && xrtGetError()==old);
    xui_text_edit_desc_t td={.iSize=sizeof(td),.pFont=font,.sText="one\ntwo"};xui_widget text=NULL;CHECK(xuiTextEditCreate(c,&text,&td)==XUI_OK && text && xrtGetError()==old);CHECK(xuiTextEditSetText(text,"next\n中文")==XUI_OK && !strcmp(xuiTextEditGetText(text),"next\n中文") && xrtGetError()==old);CHECK(xuiTextEditSetSelection(text,0,2)==XUI_OK && xrtGetError()==old);(void)xuiTextEditGetCursorRect(text);CHECK(xrtGetError()==old);
    xui_code_edit_desc_t cd={.iSize=sizeof(cd),.pFont=font};xui_widget code=NULL;CHECK(xuiCodeEditCreate(c,&code,&cd)==XUI_OK && xrtGetError()==old);xui_rect_t rect={0};CHECK(xuiCodeEditGetOffsetRect(code,0,&rect)==XUI_OK && xrtGetError()==old);
    xui_message_list_desc_t md={.iSize=sizeof(md),.pFont=font};xui_widget messages=NULL;CHECK(xuiMessageListCreate(c,&messages,&md)==XUI_OK && xrtGetError()==old);xui_message_node_t node={.iSize=sizeof(node),.sId="one",.sText="body",.iType=XUI_MESSAGE_NODE_SYSTEM};CHECK(xuiMessageListSetNodes(messages,&node,1)==XUI_OK && xrtGetError()==old && xuiMessageListGetNodeCount(messages)==1);CHECK(xuiMessageListUpdateNodeText(messages,"one","changed")==XUI_OK && xrtGetError()==old);CHECK(!strcmp(xuiMessageListGetNode(messages,0)->sText,"changed"));(void)xuiMessageListGetNodeRect(messages,0);CHECK(xrtGetError()==old);
    xrtClearError();CHECK(xuiDestroyChecked(c)==XUI_OK);proxy.tProxy.fontDestroy(&proxy.tProxy,font);CHECK(!xrtGetError());
}
int main(void)
{
    controls();parent_diagnostic();for(int mode=0;mode<=5;++mode){constructors(mode,0,0);if(mode<4)constructors(mode,0,1);}constructors(1,1,0);constructors(2,1,0);parts(0);parts(1);printf("PASS creation error boundaries: %u checks\n",checks);return 0;
}
