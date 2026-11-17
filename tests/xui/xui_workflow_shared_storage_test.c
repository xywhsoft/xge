#define XRT_MODULE_ALL
#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static xvalue* object_with_count(int count)
{
    xvalue* value=xrtValueObject();
    assert(value && xuiXrtValueObjectSetInt(value,"count",0,count));
    return value;
}

static xvalue* schema_field(xvalue* schema)
{
    xvalue* fields=xuiXrtValueObjectGet(schema,"fields",0);
    return xrtValueCount(fields)==1 ? xuiXrtValueArrayGet(fields,0):NULL;
}

static void schema_storage(void)
{
    xvalue* defaultValue=object_with_count(1);
    xvalue* options=xrtValueArray();
    xvalue* children=xrtValueArray();
    assert(options && children);
    xui_workflow_config_field_desc_t desc={0};
    desc.iSize=sizeof(desc);
    desc.sId="settings";
    desc.iKind=XUI_WORKFLOW_CONFIG_FIELD_OBJECT;
    desc.bHasDefault=1;
    desc.pDefaultValue=defaultValue;
    desc.pOptions=options;
    desc.pChildren=children;

    xvalue* cloned=NULL;
    assert(xuiWorkflowConfigSchemaCreate(&cloned)==XUI_OK);
    assert(xuiWorkflowConfigSchemaAddField(cloned,&desc)==XUI_OK);
    xvalue* field=schema_field(cloned);
    assert(field && xuiXrtValueObjectGet(field,"default",0)!=defaultValue);
    assert(xuiXrtValueObjectGet(field,"options",0)!=options);
    assert(xuiXrtValueObjectGet(field,"children",0)!=children);
    xrtValueRelease(cloned);

    int failures=0,successes=0;
    for(int budget=0;budget<64;++budget){
        xvalue* shared=NULL;
        assert(xuiWorkflowConfigSchemaCreate(&shared)==XUI_OK);
        assert(xrtMemDebugFailAfter(budget));
        int status=xuiWorkflowConfigSchemaAddFieldShared(shared,&desc);
        bool failed=xrtMemDebugFailTriggered();
        xrtMemDebugFailClear();
        assert(status==(failed ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        xrtClearError();
        field=schema_field(shared);
        if(failed){
            assert(!field);
            ++failures;
        }else{
            assert(field && xuiXrtValueObjectGet(field,"default",0)==defaultValue);
            assert(xuiXrtValueObjectGet(field,"options",0)==options);
            assert(xuiXrtValueObjectGet(field,"children",0)==children);
            ++successes;
        }
        xrtValueRelease(shared);
        assert(xuiXrtValueObjectGetInt(defaultValue,"count",0)==1);
    }
    assert(failures && successes);
    xrtValueRelease(defaultValue);
    xrtValueRelease(options);
    xrtValueRelease(children);
}

static int make_ports(xui_workflow workflow,const char* node,const char* type,
    xvalue* config,xui_flow_port_desc_t* ports,int capacity,int* count,void* context)
{
    (void)workflow;
    (void)node;
    (void)type;
    (void)context;
    int number=(int)xuiXrtValueObjectGetInt(config,"count",0);
    if(number<0)return XUI_ERROR_INVALID_ARGUMENT;
    assert(number<=2 && capacity>=number);
    for(int i=0;i<number;++i){
        ports[i].iSize=sizeof(*ports);
        ports[i].sId=i==0 ? "first":"second";
        ports[i].iDirection=XUI_FLOW_PORT_OUTPUT;
        ports[i].iKind=XUI_FLOW_PORT_CONTROL;
    }
    *count=number;
    return XUI_OK;
}

static void check_node(xui_workflow workflow,xvalue* expected,int count,const char* summary)
{
    xvalue* value=NULL;
    assert(xuiWorkflowGetNodeConfigShared(workflow,"node",&value)==XUI_OK);
    assert(value==expected);
    xrtValueRelease(value);
    xui_flow_graph graph=xuiWorkflowGetGraph(workflow);
    assert(xuiFlowGraphGetNodePortCount(graph,0)==count);
    xui_flow_node_info_t info={0};
    info.iSize=sizeof(info);
    assert(xuiFlowGraphGetNode(graph,0,&info)==XUI_OK);
    assert(info.sSummary && !strcmp(info.sSummary,summary));
}

static void config_storage(int owned)
{
    xui_workflow workflow=NULL;
    assert((owned ? xuiWorkflowCreateOwned(&workflow):xuiWorkflowCreate(&workflow))==XUI_OK);
    xvalue* schema=NULL;
    assert(xuiWorkflowConfigSchemaCreate(&schema)==XUI_OK);
    xui_workflow_config_field_desc_t field={0};
    field.iSize=sizeof(field);
    field.sId="count";
    field.iKind=XUI_WORKFLOW_CONFIG_FIELD_INT;
    assert(xuiWorkflowConfigSchemaAddFieldShared(schema,&field)==XUI_OK);
    xui_workflow_node_type_desc_t type={0};
    type.iSize=sizeof(type);
    type.sId="test";
    type.pConfigSchema=schema;
    type.onDynamicPorts=make_ports;
    assert(xuiWorkflowRegisterNodeTypeShared(workflow,&type,NULL)==XUI_OK);
    xrtValueRelease(schema);
    assert(xuiWorkflowAddNode(workflow,"test","node",NULL,0,0,NULL)==XUI_OK);

    xvalue* old=object_with_count(1);
    xvalue* next=object_with_count(2);
    xvalue* invalid=object_with_count(-1);
    assert(xuiWorkflowSetNodeConfig(workflow,"node",old)==XUI_OK);
    xvalue* clone=NULL;
    assert(xuiWorkflowGetNodeConfigShared(workflow,"node",&clone)==XUI_OK);
    assert(clone && clone!=old);
    check_node(workflow,clone,1,"count=1");
    xrtValueRelease(clone);

    int failures=0,successes=0;
    for(int budget=0;budget<96;++budget){
        assert(xuiWorkflowSetNodeConfigShared(workflow,"node",old)==XUI_OK);
        assert(xrtMemDebugFailAfter(budget));
        int status=xuiWorkflowSetNodeConfigShared(workflow,"node",next);
        bool failed=xrtMemDebugFailTriggered();
        xrtMemDebugFailClear();
        assert(status==(failed ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        xrtClearError();
        if(failed){
            check_node(workflow,old,1,"count=1");
            ++failures;
        }else{
            check_node(workflow,next,2,"count=2");
            ++successes;
        }
    }
    assert(failures && successes);
    assert(xuiWorkflowSetNodeConfigShared(workflow,"node",next)==XUI_OK);
    assert(xuiWorkflowSetNodeConfigShared(workflow,"node",invalid)==XUI_ERROR_INVALID_ARGUMENT);
    xrtClearError();
    check_node(workflow,next,2,"count=2");
    assert(xuiWorkflowGetNodeConfig(workflow,"node",&clone)==XUI_OK);
    assert(clone && clone!=next && xuiXrtValueObjectGetInt(clone,"count",0)==2);
    xrtValueRelease(clone);
    xvalue* result=NULL;
    assert(xuiWorkflowGetNodeConfigShared(workflow,"node",&result)==XUI_OK && result==next);
    xrtValueRelease(old);
    xrtValueRelease(next);
    xrtValueRelease(invalid);
    assert(xuiWorkflowDestroyChecked(workflow)==XUI_OK);
    assert(xuiXrtValueObjectGetInt(result,"count",0)==2);
    xrtValueRelease(result);
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    schema_storage();
    config_storage(0);
    config_storage(1);
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(!after.DoubleFreeCount && !after.InvalidFreeCount && !after.UseAfterFreeCount);
    assert(!xrtGetError());
    puts("WORKFLOW_SHARED_STORAGE_OK");
    return 0;
}
