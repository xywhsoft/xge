/* Exact production generation/whole-snapshot algorithms, no OS backend. */
#include <assert.h>
#include <string.h>
#include "../../src/xge/input/xge_data_object.c"
#include "../../src/xui/core/xui_data_object.c"

typedef struct snapshot_owner_t {size_t refs;int calls,action,backend;void* source;} snapshot_owner_t;
static bool snapshotCount(const void* raw,size_t* out){*out=((const snapshot_owner_t*)raw)->refs;return true;}
static bool snapshotTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static bool snapshotHold(const void* raw){((snapshot_owner_t*)raw)->refs++;return true;}
static bool snapshotDrop(const void* raw){snapshot_owner_t* owner=(snapshot_owner_t*)raw;assert(owner->refs);owner->refs--;return true;}
static const xrtownershipops snapshotOps={snapshotCount,snapshotTrace};

#define SNAPSHOT_FAMILY(prefix,upper) \
static int prefix##Read(const xstrview* format,prefix##_data_output_t* out,void* raw) \
{ \
    snapshot_owner_t* owner=raw;owner->calls++; \
    if(owner->action==1 && !out->_pDestination)assert(prefix##DataObjectSet(owner->source,format->Data,"new",3)==upper##_OK); \
    if(owner->action==3)return 1; \
    static const unsigned char bytes[4]={'a',0,'b','c'}; \
    size_t size=owner->action==2 && out->_pDestination ? 4u:3u; \
    return prefix##DataOutputSet(out,bytes,size); \
} \
static int prefix##Consume(const prefix##_clipboard_item_t* items,int count,void* raw) \
{ \
    snapshot_owner_t* owner=raw;owner->backend++;assert(count==2); \
    assert(items[0].iDataSize==3 && memcmp(items[0].pData,"xyz",3)==0); \
    assert(items[0].pData==((prefix##_data_object)owner->source)->pSlots[0]->pValue); \
    assert(items[1].iDataSize==3 && memcmp(items[1].pData,"a\0b",3)==0);return upper##_OK; \
} \
static void prefix##Cases(void) \
{ \
    for(int action=0;action<4;++action){ \
        prefix##_data_object data=NULL;assert(prefix##DataObjectCreateOwned(&data)==upper##_OK); \
        snapshot_owner_t owner={1,0,action,0,data}; \
        prefix##_callback_owner_t capture={{&owner,&snapshotOps},snapshotHold,snapshotDrop}; \
        assert(prefix##DataObjectSet(data,"binary","xyz",3)==upper##_OK); \
        assert(prefix##DataObjectSetProviderOwned(data,"lazy",prefix##Read,&owner,&capture)==upper##_OK); \
        int status=__##prefix##DataSnapshotCopy(data,prefix##Consume,&owner); \
        assert(status==(action==0 ? upper##_OK:action==1 ? upper##_ERROR_INVALID_STATE:action==2 ? upper##_ERROR_BUFFER_TOO_SMALL:upper##_ERROR_CALLBACK_FAILED)); \
        assert(owner.backend==(action==0)); \
        assert(owner.calls==(action==0 || action==2 ? 2:1)); \
        xrtClearError();bool consumed=false;assert(prefix##DataObjectReleaseOwnerChecked(data,&consumed)==upper##_OK && consumed);assert(owner.refs==1); \
    } \
    prefix##_data_object raw=NULL;assert(prefix##DataObjectCreate(&raw)==upper##_OK); \
    assert(__##prefix##DataSnapshotCopy(raw,prefix##Consume,NULL)==upper##_ERROR_INVALID_ARGUMENT);prefix##DataObjectRelease(raw); \
    prefix##_data_object data=NULL;assert(prefix##DataObjectCreateOwned(&data)==upper##_OK); \
    snapshot_owner_t owner={1,0,0,0,data};prefix##_callback_owner_t capture={{&owner,&snapshotOps},snapshotHold,snapshotDrop}; \
    assert(prefix##DataObjectSet(data,"binary","xyz",3)==upper##_OK); \
    assert(prefix##DataObjectSetProviderOwned(data,"lazy",prefix##Read,&owner,&capture)==upper##_OK); \
    assert(xrtMemDebugFailAfter(0));assert(__##prefix##DataSnapshotCopy(data,prefix##Consume,&owner)==upper##_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1)); \
    assert(owner.backend==0 && owner.calls==1);xrtClearError(); \
    bool consumed=false;assert(prefix##DataObjectReleaseOwnerChecked(data,&consumed)==upper##_OK && consumed);assert(owner.refs==1); \
}

SNAPSHOT_FAMILY(xge,XGE)
SNAPSHOT_FAMILY(xui,XUI)
static void dragAccessorCases(void)
{
    xui_event_t event={0};xui_drag_event_data_t data={0};event.iSize=sizeof(event);event.pData=&data;
    assert(!xuiEventDragData(NULL));event.iType=XUI_EVENT_DRAG_ENTER;assert(!xuiEventDragData(&event));
    data.iSize=sizeof(data);
    const int types[]={XUI_EVENT_DRAG_ENTER,XUI_EVENT_DRAG_OVER,XUI_EVENT_DRAG_LEAVE,XUI_EVENT_DROP};
    for(size_t i=0;i<sizeof(types)/sizeof(types[0]);++i){event.iType=types[i];assert(xuiEventDragData(&event)==&data);}
    event.iSize=sizeof(event)-1;assert(!xuiEventDragData(&event));event.iSize=sizeof(event);
    data.iSize=sizeof(data)-1;assert(!xuiEventDragData(&event));data.iSize=sizeof(data);
    event.pData=NULL;assert(!xuiEventDragData(&event));
    /* A structurally valid transfer-shaped record must still be rejected for
     * geometric tags; this proves the discriminant independently of size. */
    xui_drag_event_data_t untyped={.iSize=sizeof(untyped)};event.pData=&untyped;
    const int geometric[]={XUI_EVENT_DRAG_BEGIN,XUI_EVENT_DRAG_MOVE,XUI_EVENT_DRAG_END,XUI_EVENT_DRAG_CANCEL};
    for(size_t i=0;i<sizeof(geometric)/sizeof(geometric[0]);++i){event.iType=geometric[i];assert(!xuiEventDragData(&event));}
}
int main(void){xgeCases();xuiCases();dragAccessorCases();return 0;}
