/* Real control publishers, exercised headlessly through accessibility actions. */
#define main xui_owned_event_regression_main
#include "xui_owned_callback_test.c"
#undef main

enum {CHOICE_KEEP=400,CHOICE_CLEAR,CHOICE_CLOSE,CHOICE_NEST};
typedef struct ChoiceUser {unsigned marker;Owner* owner;int kind;xui_widget peer;} ChoiceUser;
static int clear_choice(xui_widget w,int k)
{
    switch(k){case 0:return xuiButtonSetClick(w,NULL,NULL);case 1:return xuiCheckBoxSetChange(w,NULL,NULL);
    case 2:return xuiCheckCardSetChange(w,NULL,NULL);case 3:return xuiRadioSetChange(w,NULL,NULL);
    case 4:return xuiToggleSetChange(w,NULL,NULL);case 5:return xuiSliderSetChange(w,NULL,NULL);
    case 6:return xuiRangeSliderSetChange(w,NULL,NULL);default:return xuiScrollBarSetChange(w,NULL,NULL);}
}
static int fire_choice(xui_widget w,int kind,unsigned serial)
{
    if(kind==3)assert(xuiRadioSetChecked(w,0)==XUI_OK);
    if(kind<5)return xuiWidgetPerformAccessibleAction(w,1,XUI_ACCESSIBLE_ACTION_ACTIVATE,NULL);
    xui_accessible_value_t value={0};value.iSize=sizeof(value);value.iType=XUI_ACCESSIBLE_VALUE_NUMBER;value.fNumber=serial%2 ? 0.75:0.25;
    xui_accessible_node_t node={0};node.iSize=sizeof(node);
    assert(xuiWidgetGetAccessibleNode(w,kind==6 ? 1:0,&node)==XUI_OK);
    return xuiWidgetPerformAccessibleAction(w,node.iId,XUI_ACCESSIBLE_ACTION_SET_VALUE,&value);
}
static void choice_body(xui_widget w,void* raw)
{
    ChoiceUser* user=raw;assert(user->marker==0xCE11U);Owner* owner=user->owner;
    assert(owner->refs>=2 && !owner->released);bool nested=owner->inCall;owner->inCall=true;++owner->calls;
    if(owner->action==CHOICE_CLEAR)assert(clear_choice(w,user->kind)==XUI_OK);
    else if(owner->action==CHOICE_CLOSE)xuiDestroy(owner->context);
    else if(owner->action==CHOICE_NEST && !nested)assert(fire_choice(user->peer,user->kind,1)==XUI_OK);
    assert(owner->refs && !owner->released);owner->inCall=nested;
}
static void click_choice(xui_widget w,void* u){choice_body(w,u);}
static void int_choice(xui_widget w,int value,void* u){assert(value==0 || value==1);choice_body(w,u);}
static void float_choice(xui_widget w,float value,void* u){assert(value==0.25f || value==0.75f);choice_body(w,u);}
static void range_choice(xui_widget w,float start,float end,void* u)
{assert((start==0.25f || start==0.75f) && end>=start);choice_body(w,u);}
static int install_choice(xui_widget w,int k,ChoiceUser* user,const xui_callback_owner_t* ref)
{
    switch(k){case 0:return xuiButtonSetClickOwned(w,click_choice,user,ref);case 1:return xuiCheckBoxSetChangeOwned(w,int_choice,user,ref);
    case 2:return xuiCheckCardSetChangeOwned(w,int_choice,user,ref);case 3:return xuiRadioSetChangeOwned(w,int_choice,user,ref);
    case 4:return xuiToggleSetChangeOwned(w,int_choice,user,ref);case 5:return xuiSliderSetChangeOwned(w,float_choice,user,ref);
    case 6:return xuiRangeSliderSetChangeOwned(w,range_choice,user,ref);default:return xuiScrollBarSetChangeOwned(w,float_choice,user,ref);}
}
static xui_widget make_choice(xui_context c,int k)
{
    xui_widget w=NULL;int status;
    switch(k){case 0:status=xuiButtonCreate(c,&w,NULL);break;case 1:status=xuiCheckBoxCreate(c,&w,NULL);break;
    case 2:status=xuiCheckCardCreate(c,&w,NULL);break;case 3:status=xuiRadioCreate(c,&w,NULL);break;
    case 4:status=xuiToggleCreate(c,&w,NULL);break;case 5:status=xuiSliderCreate(c,&w,NULL);break;
    case 6:status=xuiRangeSliderCreate(c,&w,NULL);break;default:status=xuiScrollBarCreate(c,&w,NULL);break;}
    assert(status==XUI_OK && w);return w;
}
static void choice_lifetimes(void)
{
    for(int k=0;k<8;++k)for(int behavior=CHOICE_KEEP;behavior<=CHOICE_NEST;++behavior){
        xui_context c=NULL;assert(xuiCreate(&c)==XUI_OK);xui_widget w=make_choice(c,k);
        Owner owner={.refs=1,.context=c,.action=behavior};ChoiceUser user={0xCE11U,&owner,k,NULL};xui_callback_owner_t ref=descriptor(&owner);
        assert(install_choice(w,k,&user,&ref)==XUI_OK);
        if(behavior==CHOICE_NEST){user.peer=make_choice(c,k);assert(install_choice(user.peer,k,&user,&ref)==XUI_OK);}
        assert(drop(&owner));assert(fire_choice(w,k,0)==XUI_OK);assert(owner.calls==(behavior==CHOICE_NEST?2u:1u));
        if(behavior!=CHOICE_CLOSE)xuiDestroy(c);
        assert(!owner.refs && owner.released==1);
    }
}
static void choice_graph_warm(void)
{
    xui_context c=NULL;assert(xuiCreate(&c)==XUI_OK);Owner owner={.refs=1,.context=c,.action=CHOICE_KEEP};ChoiceUser users[8];xui_widget widgets[8];xui_callback_owner_t ref=descriptor(&owner);
    for(int k=0;k<8;++k){widgets[k]=make_choice(c,k);users[k]=(ChoiceUser){0xCE11U,&owner,k,NULL};assert(install_choice(widgets[k],k,&users[k],&ref)==XUI_OK);}
    xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(c,visit,&edges) && edges==8);assert(xrtOwnershipScopeEnd(&freeze));
    for(int k=0;k<8;++k){assert(fire_choice(widgets[k],k,0)==XUI_OK);assert(fire_choice(widgets[k],k,1)==XUI_OK);}
    unsigned calls=owner.calls;xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    for(unsigned n=0;n<200;++n)for(int k=0;k<8;++k)assert(fire_choice(widgets[k],k,n)==XUI_OK);
    xrtMemDebugSnapshot(&b);assert(owner.calls==calls+1600 && a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
    assert(drop(&owner));xuiDestroy(c);assert(owner.released==1);
}
static void choice_failures(int selected)
{
    for(int k=0;k<8;++k){
        if(selected>=0 && k!=selected)continue;
        xui_context c=NULL;assert(xuiCreate(&c)==XUI_OK);xui_widget w=make_choice(c,k);Owner owner={.refs=1,.context=c,.action=CHOICE_KEEP};ChoiceUser user={0xCE11U,&owner,k,NULL};xui_callback_owner_t ref=descriptor(&owner);
        for(int point=0;point<2;++point){assert(xrtMemDebugFailAfter(point));assert(install_choice(w,k,&user,&ref)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(owner.refs==1 && !w->pEventOwners);}
        assert(install_choice(w,k,&user,&ref)==XUI_OK);owner.failHold=true;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(fire_choice(w,k,0)==XUI_OK);
        assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && !owner.calls && !xrtGetError());owner.failHold=false;
        /* A failed, uncalled publisher still updates the native control's state. */
        owner.action=CHOICE_CLEAR;owner.failDrop=true;assert(drop(&owner));assert(xuiCallScopeBegin(c,&scope)==XUI_OK);
        assert(fire_choice(w,k,1)==XUI_OK);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && owner.calls==1 && !xrtGetError());
        assert(!owner.refs && owner.released==1);xuiDestroy(c);
    }
}
int main(int argc,char** argv)
{
    int selected=-1;assert(argc==1 || (argc==2 && argv[1][0]>='0' && argv[1][0]<='7' && !argv[1][1]));
    if(argc==2)selected=argv[1][0]-'0';
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    if(selected<1){choice_lifetimes();choice_graph_warm();}
    choice_failures(selected);xrtClearError();xrtMemDebugSnapshot(&b);
    assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);
    puts("eight owned control callbacks: real accessibility publishers, replacement, close, nested peers, graph, OOM, checked failure and 200 warm calls passed");return 0;
}
