#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <math.h>
#include <string.h>
#if XGE_ENABLE_INPUT_ACTIONS
typedef struct action_value { xge_action_state_t state;float deadzone,sample;int enabled; } action_value;
typedef struct action_binding { uint32_t action;xge_action_binding_t binding; } action_binding;
struct xge_actions { xarray values,bindings;uint32_t capacity; };
int xgeActionsCreate(uint32_t count,uint32_t capacity,xge_actions **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;
    if(!count||count>65536||!capacity||capacity>1048576){return XGE_ERROR_INVALID_ARGUMENT;}
    xge_actions *a=xrtCalloc(1,sizeof(*a));if(!a){return XGE_ERROR_OUT_OF_MEMORY;}
    xrtArrayInit(&a->values,sizeof(action_value));xrtArrayInit(&a->bindings,sizeof(action_binding));a->capacity=capacity;
    if(!xrtArrayResize(&a->values,count)||!xrtArrayReserve(&a->bindings,capacity)){xgeActionsFree(a);return XGE_ERROR_OUT_OF_MEMORY;}
    memset(a->values.Data,0,count*sizeof(action_value));for(uint32_t i=0;i<count;++i){((action_value*)a->values.Data)[i].enabled=1;}
    *out=a;return XGE_OK;
}
void xgeActionsFree(xge_actions *a){if(a){xrtArrayUnit(&a->values);xrtArrayUnit(&a->bindings);xrtFree(a);}}
static action_value *action_at(const xge_actions *a,uint32_t i){return a&&i<a->values.Count?(action_value*)a->values.Data+i:NULL;}
int xgeActionsBind(xge_actions *a,uint32_t action,const xge_action_binding_t *b)
{
    action_value *v=action_at(a,action);
    if(!v||!b||b->type<0||b->type>3||!isfinite(b->scale)||fabsf(b->scale)>16||b->device< -1||b->device>=XGE_GAMEPAD_MAX){return XGE_ERROR_INVALID_ARGUMENT;}
    if((b->type==XGE_ACTION_KEY&&b->code>=512)||(b->type==XGE_ACTION_MOUSE_BUTTON&&(!b->code||(b->code&~7u)))||
       (b->type==XGE_ACTION_GAMEPAD_BUTTON&&!b->code)||(b->type==XGE_ACTION_GAMEPAD_AXIS&&b->code>=XGE_GAMEPAD_AXIS_COUNT)){return XGE_ERROR_INVALID_ARGUMENT;}
    if(a->bindings.Count>=a->capacity){return XGE_ERROR_BUFFER_TOO_SMALL;}
    action_binding entry={action,*b};return xrtArrayPush(&a->bindings,&entry)?XGE_OK:XGE_ERROR_OUT_OF_MEMORY;
}
int xgeActionsClearBindings(xge_actions *a,uint32_t action)
{
    action_value *v=action_at(a,action);if(!v){return XGE_ERROR_INVALID_ARGUMENT;}
    action_binding *items=(action_binding*)a->bindings.Data;size_t n=0;for(size_t i=0;i<a->bindings.Count;++i){if(items[i].action!=action){items[n++]=items[i];}}
    a->bindings.Count=n;v->state=(xge_action_state_t){0};return XGE_OK;
}
int xgeActionsSetDeadzone(xge_actions *a,uint32_t action,float deadzone)
{action_value *v=action_at(a,action);if(!v||!isfinite(deadzone)||deadzone<0||deadzone>=1){return XGE_ERROR_INVALID_ARGUMENT;}v->deadzone=deadzone;return XGE_OK;}
int xgeActionsSetEnabled(xge_actions *a,uint32_t action,int enabled)
{action_value *v=action_at(a,action);if(!v||(enabled!=0&&enabled!=1)){return XGE_ERROR_INVALID_ARGUMENT;}v->enabled=enabled;if(!enabled){v->state=(xge_action_state_t){0};}return XGE_OK;}
static float action_poll(const xge_action_binding_t *b,void *user)
{
    (void)user;if(b->type==XGE_ACTION_KEY){return xgeInputKeyConsumed((int)b->code)?0:(float)xgeKeyDown((int)b->code);}
    if(b->type==XGE_ACTION_MOUSE_BUTTON){return (float)xgeMouseDown((int)b->code);}
    float value=0;for(int i=0;i<XGE_GAMEPAD_MAX;++i){if((b->device<0||b->device==i)&&xgeGamepadConnected(i)){
        float next=b->type==XGE_ACTION_GAMEPAD_BUTTON?(float)xgeGamepadButtonDown(i,b->code):xgeGamepadAxis(i,(int)b->code);
        if(fabsf(next)>fabsf(value)){value=next;}
    }}return value;
}
int xgeActionsUpdate(xge_actions *a,xge_action_read_proc read,void *user)
{
    if(!a){return XGE_ERROR_INVALID_ARGUMENT;}if(!read){read=action_poll;}
    action_value *values=(action_value*)a->values.Data;for(size_t i=0;i<a->values.Count;++i){values[i].sample=0;}
    for(size_t i=0;i<a->bindings.Count;++i){action_binding *b=xrtArrayGet(&a->bindings,i);if(!values[b->action].enabled){continue;}
        float sample=read(&b->binding,user);if(!isfinite(sample)||fabsf(sample)>1){return XGE_ERROR_INVALID_ARGUMENT;}
        values[b->action].sample+=sample*b->binding.scale;
    }
    for(size_t i=0;i<a->values.Count;++i){action_value *v=values+i;float x=fmaxf(-1,fminf(1,v->sample)),m=fabsf(x);
        x=v->enabled&&m>v->deadzone?copysignf((m-v->deadzone)/(1-v->deadzone),x):0;
        int down=x!=0,was=v->state.down;v->state=(xge_action_state_t){x,down,down&&!was,!down&&was};
    }return XGE_OK;
}
int xgeActionsGet(const xge_actions *a,uint32_t action,xge_action_state_t *out)
{action_value *v=action_at(a,action);if(!v||!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=v->state;return XGE_OK;}
void xgeActionsReset(xge_actions *a){if(a){for(size_t i=0;i<a->values.Count;++i){((action_value*)a->values.Data)[i].state=(xge_action_state_t){0};}}}
#endif
