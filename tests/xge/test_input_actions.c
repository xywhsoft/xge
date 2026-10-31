#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"foundation %d: %s\n",__LINE__,#x);return 1;}}while(0)
#if XGE_ENABLE_INPUT_ACTIONS
static float read_action(const xge_action_binding_t *b,void *user){return ((float*)user)[b->code];}
static int actions(void)
{
    xge_actions *a=NULL;CHECK(!xgeActionsCreate(3,4,&a));float input[4]={0};
    xge_action_binding_t left={XGE_ACTION_KEY,0,0,-1},right={XGE_ACTION_KEY,0,1,1},axis={XGE_ACTION_GAMEPAD_AXIS,-1,2,1};
    CHECK(!xgeActionsBind(a,0,&left)&&!xgeActionsBind(a,0,&right)&&!xgeActionsBind(a,1,&axis)&&!xgeActionsSetDeadzone(a,1,.2f));
    input[0]=1;input[2]=.6f;CHECK(!xgeActionsUpdate(a,read_action,input));xge_action_state_t s,old;
    CHECK(!xgeActionsGet(a,0,&s)&&s.value==-1&&s.down&&s.pressed&&!s.released);
    CHECK(!xgeActionsGet(a,1,&s)&&fabsf(s.value-.5f)<1e-6f&&s.pressed);old=s;
    input[2]=NAN;CHECK(xgeActionsUpdate(a,read_action,input)==XGE_ERROR_INVALID_ARGUMENT);CHECK(!xgeActionsGet(a,1,&s)&&!memcmp(&s,&old,sizeof(s)));
    input[2]=.1f;input[1]=1;CHECK(!xgeActionsUpdate(a,read_action,input));CHECK(!xgeActionsGet(a,0,&s)&&!s.value&&!s.down&&s.released);
    CHECK(!xgeActionsGet(a,1,&s)&&!s.value&&s.released);
    input[0]=0;CHECK(!xgeActionsUpdate(a,read_action,input)&&!xgeActionsGet(a,0,&s)&&s.value==1&&s.pressed);
    CHECK(!xgeActionsSetEnabled(a,0,0)&&!xgeActionsUpdate(a,read_action,input)&&!xgeActionsGet(a,0,&s)&&!s.down&&!s.released);
    CHECK(!xgeActionsSetEnabled(a,0,1)&&!xgeActionsClearBindings(a,0));left.scale=1;CHECK(!xgeActionsBind(a,0,&left));
    input[0]=1;CHECK(!xgeActionsUpdate(a,read_action,input)&&!xgeActionsGet(a,0,&s)&&s.pressed);xgeActionsReset(a);
    CHECK(!xgeActionsGet(a,0,&s)&&!s.down&&!s.pressed&&!s.released);CHECK(xgeActionsSetDeadzone(a,1,1)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!xgeActionsBind(a,2,&left)&&!xgeActionsBind(a,2,&right));
    CHECK(xgeActionsBind(a,2,&left)==XGE_ERROR_BUFFER_TOO_SMALL);
    xallocationscope scope={0};xallocationbudget *quota=NULL;
    CHECK(xrtAllocationBudgetCreate(1,&quota)&&xrtAllocationBudgetEnter(quota,&scope));
    CHECK(!xgeActionsUpdate(a,read_action,input)&&!scope.Rejections);CHECK(xrtAllocationBudgetLeave(&scope));xrtAllocationBudgetFree(quota);
    xgeActionsFree(a);return 0;
}
#endif
int main(void){
#if XGE_ENABLE_INPUT_ACTIONS
    CHECK(!actions());
#endif
    puts("Input actions CPU contracts passed");return 0;
}
