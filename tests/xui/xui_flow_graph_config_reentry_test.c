#define XRT_MODULE_MEMORY_DEBUG
#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* The raw source handle is borrowed from this test; the independent Clone
 * owns a second payload. Only that clone's Drop may reenter the graph. */
typedef struct config_payload {
 xui_flow_graph graph;
 int onClone;
 int onDrop;
 int clone;
 int* dropped;
} config_payload;

static void add_node(xui_flow_graph graph,const char* id)
{
 xui_flow_node_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId=id;
 assert(xuiFlowGraphAddNode(graph,&desc,NULL)==XUI_OK);
}

static bool clone_config(void* raw,void** output,void* user)
{
 config_payload* source=raw;config_payload* copy;(void)user;*output=NULL;
 copy=xrtMalloc(sizeof(*copy));if(!copy)return false;
 *copy=*source;copy->clone=1;
 if(source->onClone){
  assert(xuiFlowGraphRemoveNode(source->graph,"one")==XUI_OK);
  if(source->onClone==2)add_node(source->graph,"one");
 }
 *output=copy;return true;
}

static void drop_config(void* raw,void* user)
{
 config_payload* payload=raw;(void)user;
 if(payload->clone){
  ++*payload->dropped;
  if(payload->onDrop)assert(xuiFlowGraphRemoveNode(payload->graph,"one")==XUI_OK);
  xrtFree(payload);
 }
}

static xvalue* handle(config_payload* payload)
{
 static const xvaluehandleops ops={clone_config,drop_config,NULL,NULL};
 void* raw=payload;xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);
 assert(value && raw==NULL);return value;
}

static void check_clone_change(int recreate)
{
 xui_flow_graph graph=NULL;xvalue *source,*found=NULL;
 int dropped=0;assert(xuiFlowGraphCreate(&graph)==XUI_OK);add_node(graph,"one");
 config_payload payload={graph,recreate?2:1,0,0,&dropped};source=handle(&payload);
 assert(xuiFlowGraphSetNodeConfig(graph,"one",source)==XUI_ERROR_INVALID_STATE);
 assert(xuiFlowGraphGetNodeCount(graph)==recreate && dropped==1);
 if(recreate){assert(xuiFlowGraphGetNodeConfig(graph,"one",&found)==XUI_OK && !found);}
 xrtValueRelease(source);xuiFlowGraphDestroy(graph);assert(!xrtGetError());
}

static void check_previous_drop(void)
{
 xui_flow_graph graph=NULL;xvalue *source,*next;int dropped=0;
 assert(xuiFlowGraphCreate(&graph)==XUI_OK);add_node(graph,"one");
 config_payload payload={graph,0,1,0,&dropped};source=handle(&payload);
 assert(xuiFlowGraphSetNodeConfig(graph,"one",source)==XUI_OK);xrtValueRelease(source);
 next=xrtValueNull();assert(next);
 assert(xuiFlowGraphSetNodeConfig(graph,"one",next)==XUI_OK);
 assert(dropped==1 && xuiFlowGraphGetNodeCount(graph)==0);
 xrtValueRelease(next);xuiFlowGraphDestroy(graph);assert(!xrtGetError());
}

int main(void)
{
 xmemdebugsnapshot before,after;assert(xrtMemDebugEnable(true));
 xrtMemDebugSnapshot(&before);
 for(int i=0;i<8;++i){check_clone_change(0);check_clone_change(1);check_previous_drop();}
 xrtMemDebugSnapshot(&after);
 assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
 assert(after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
 puts("FlowGraph config reentry: changed-node clone rejected, same-id replacement untouched, old Drop detached, exactly-once cleanup and heap balance passed");
 return 0;
}
