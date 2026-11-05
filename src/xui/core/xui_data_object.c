#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui.h"
#include <limits.h>
#include <string.h>
#include <stdatomic.h>
#include <assert.h>
#include <stdlib.h>
#include "xge_data_lifetime_internal.h"

#define XUI_DATA_OBJECT_MAGIC 0x5855444Fu


/* A slot owns one immutable payload generation. A provider invocation owns a
 * second real reference until its complete C return; replacement never leaves
 * a reader holding a pointer into the movable slot array. Mutable operations
 * retain their existing caller-serialized C contract. Atomic references only
 * protect actual lifetime, not arbitrary concurrent container mutations. */
typedef struct xui_data_generation_t {
	atomic_uint iRefs;
	unsigned iFlags;
	char* sFormat;
	void* pValue;
	union {size_t valueBytes;size_t formatBytes;} tSize;
	union { xui_data_provider_proc raw; xui_data_provider_owned_proc owned; } tRead;
	xui_data_provider_free_proc onFree;
	void* pUser;
} xui_data_generation_t;

struct xui_data_object_t {
	uint32_t iMagic;
	atomic_uint iRefCount;
	xui_data_generation_t** pSlots;
	int iCount;
	int iCapacity;
	atomic_uint iActivity;
	atomic_uint bDestroying;
};


enum { XUI_DATA_MANAGED=2u,XUI_DATA_OWNED_PROVIDER=4u };
typedef struct xui_data_operation_t { xui_data_object pObject;xerror* pAmbient;bool bOwned; } xui_data_operation_t;
static xge_data_lifetime_t* __xuiDataObjectRoot(xui_data_object object)
{return object && (atomic_load_explicit(&object->bDestroying,memory_order_relaxed)&XUI_DATA_MANAGED) ? (xge_data_lifetime_t*)(object+1):NULL;}
static xge_data_lifetime_t* __xuiDataGenerationRoot(xui_data_generation_t* generation)
{return generation && (generation->iFlags&XUI_DATA_MANAGED) ? (xge_data_lifetime_t*)(generation+1):NULL;}
static xui_callback_owner_t* __xuiDataGenerationOwner(xui_data_generation_t* generation)
{return generation && (generation->iFlags&XUI_DATA_OWNED_PROVIDER) ? (xui_callback_owner_t*)(__xuiDataGenerationRoot(generation)+1):NULL;}
static bool __xuiDataObjectReadable(const void* data)
{const struct xui_data_object_t* object=data;return !(atomic_load_explicit(&object->bDestroying,memory_order_acquire)&1u);}
static bool __xuiDataObjectTrace(const void*,xrtownershipvisitor,void*);
static bool __xuiDataGenerationTrace(const void*,xrtownershipvisitor,void*);
static int __xuiDataObjectFinalize(void*,bool*);
static int __xuiDataGenerationFinish(void*);
static const xge_data_lifetime_type_t __xuiDataObjectType={sizeof(struct xui_data_object_t),offsetof(struct xui_data_object_t,iRefCount),__xuiDataObjectReadable,__xuiDataObjectTrace,__xuiDataObjectFinalize,NULL};
static const xge_data_lifetime_type_t __xuiDataGenerationType={sizeof(xui_data_generation_t),offsetof(xui_data_generation_t,iRefs),NULL,__xuiDataGenerationTrace,NULL,__xuiDataGenerationFinish};
#define DO_FN(name) __xuiDataOwned##name
#define DO_PUBLIC(name) xuiDataObject##name
#define DO_STATUS(name) XUI_##name
#define DO_API XUI_API
#define DO_ROOT(object) __xuiDataObjectRoot(object)
#define DO_HANDLE xui_data_object
#define DO_OBJECT_TYPE __xuiDataObjectType
#define DO_GENERATION_TYPE __xuiDataGenerationType
#include "xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE

static bool __xuiDataOwnerValid(const xui_callback_owner_t* owner)
{return owner && owner->tReference.Data && owner->tReference.Ops && owner->onHold && owner->onDrop;}
static int __xuiDataOwnerHold(const xui_callback_owner_t* owner,bool* held)
{
    *held=false;if(!__xuiDataOwnerValid(owner))return XUI_ERROR_INVALID_ARGUMENT;
    xerror* ambient=xrtTakeError();*held=owner->onHold(owner->tReference.Data);
    xerror* fresh=xrtTakeError();int status=*held && !fresh ? XUI_OK:XUI_ERROR_CALLBACK_FAILED;
    if(ambient){xrtErrorFree(fresh);fresh=ambient;}xrtSetErrorTake(fresh);return status;
}
static int __xuiDataOwnerDrop(xui_callback_owner_t owner)
{return !owner.onDrop || owner.onDrop(owner.tReference.Data) ? XUI_OK:XUI_ERROR_CALLBACK_FAILED;}
static int __xuiDataGenerationReleaseChecked(xui_data_generation_t* generation);
static bool __xuiDataObjectTrace(const void* data,xrtownershipvisitor visitor,void* user)
{
    const struct xui_data_object_t* object=data;
    for(int i=0;i<object->iCount;++i){
        xge_data_lifetime_t* root=__xuiDataGenerationRoot(object->pSlots[i]);
        if(!root || !visitor(__xuiDataOwnedReference(root),user))return false;
    }
    return true;
}
static bool __xuiDataGenerationTrace(const void* data,xrtownershipvisitor visitor,void* user)
{
    xui_callback_owner_t* owner=__xuiDataGenerationOwner((xui_data_generation_t*)data);
    return !owner || !owner->onDrop || visitor(owner->tReference,user);
}
static int __xuiDataObjectFinalize(void* data,bool* complete)
{
    xui_data_object object=data;xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    atomic_fetch_or_explicit(&object->bDestroying,1u,memory_order_release);
    xui_data_generation_t** slots=object->pSlots;int count=object->iCount,capacity=object->iCapacity;
    object->pSlots=NULL;object->iCount=object->iCapacity=0;__xuiDataOwnedMutationEnd(&mutation);
    xge_data_outcome_t result={0};__xuiDataOwnedOutcomeBegin(&result,XUI_OK);int remaining=0;
    for(int i=0;i<count;++i){
        bool consumed=false;int status=__xuiDataOwnedRelease(__xuiDataGenerationRoot(slots[i]),&consumed);
        __xuiDataOwnedOutcomeAdd(&result,status);
        if(!consumed)slots[remaining++]=slots[i];
    }
    if(remaining){
        if(!xrtOwnershipMutationBegin(&mutation))abort();
        object->pSlots=slots;object->iCount=remaining;object->iCapacity=capacity;__xuiDataOwnedMutationEnd(&mutation);
        if(result.iStatus==XUI_OK)result.iStatus=XUI_STATUS_DEFERRED;
    }else xrtFree(slots);
    *complete=remaining==0;int status=result.iStatus;xrtSetErrorTake(result.pError);return status;
}
static int __xuiDataGenerationFinish(void* data)
{
    xui_callback_owner_t* slot=__xuiDataGenerationOwner(data);if(!slot)return XUI_OK;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    xui_callback_owner_t owner=*slot;memset(slot,0,sizeof(*slot));__xuiDataOwnedMutationEnd(&mutation);
    return __xuiDataOwnerDrop(owner);
}

XUI_API bool xuiDataObjectIsOwned(xui_data_object object)
{return object && object->iMagic==XUI_DATA_OBJECT_MAGIC && __xuiDataOwnedKnown(__xuiDataObjectRoot(object));}
XUI_API int xuiDataObjectCreateOwned(xui_data_object* output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;xui_data_object object=xrtCalloc(1,sizeof(*object)+sizeof(xge_data_lifetime_t));
    if(!object)return XUI_ERROR_OUT_OF_MEMORY;
    object->iMagic=XUI_DATA_OBJECT_MAGIC;atomic_init(&object->iRefCount,1u);atomic_init(&object->iActivity,0u);
    atomic_init(&object->bDestroying,XUI_DATA_MANAGED);
    __xuiDataOwnedInit(__xuiDataObjectRoot(object),&__xuiDataObjectType,xrtThreadCurrentId());
    *output=object;return XUI_OK;
}
XUI_API int xuiDataObjectShareOwned(xui_data_object object,xui_data_object* output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;if(!xuiDataObjectIsOwned(object))return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiDataOwnedAcquire(__xuiDataObjectRoot(object),true,false);if(status==XUI_OK)*output=object;return status;
}
XUI_API int xuiDataObjectReleaseOwnerChecked(xui_data_object object,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;if(!xuiDataObjectIsOwned(object))return XUI_ERROR_INVALID_ARGUMENT;
    return __xuiDataOwnedRelease(__xuiDataObjectRoot(object),consumed);
}
XUI_API int xuiDataObjectReleaseChecked(xui_data_object object)
{bool consumed=false;return xuiDataObjectReleaseOwnerChecked(object,&consumed);}

static int __xuiDataObjectValid(xui_data_object pData)
{
	return pData != NULL && pData->iMagic == XUI_DATA_OBJECT_MAGIC &&
		atomic_load_explicit(&pData->iRefCount,memory_order_acquire)>0u &&
		!(atomic_load_explicit(&pData->bDestroying,memory_order_acquire)&1u) &&
		(!__xuiDataObjectRoot(pData) || __xuiDataOwnedReadable(__xuiDataObjectRoot(pData)));
}

static char* __xuiDataStringCopy(const char* sText)
{
	if(!sText || !sText[0])return NULL;
	size_t length=strlen(sText);
	if(length==SIZE_MAX)return NULL;
	char* copy=(char*)xrtMalloc(length+1u);
	if(copy)memcpy(copy,sText,length+1u);
	return copy;
}

static bool __xuiDataGenerationHold(xui_data_generation_t* generation)
{
	if(__xuiDataGenerationRoot(generation))return __xuiDataOwnedAcquire(__xuiDataGenerationRoot(generation),true,true)==XUI_OK;
	unsigned refs=atomic_load_explicit(&generation->iRefs,memory_order_relaxed);
	do {if(!refs || refs==UINT_MAX)return false;}
	while(!atomic_compare_exchange_weak_explicit(&generation->iRefs,&refs,refs+1u,memory_order_relaxed,memory_order_relaxed));
	return true;
}

static int __xuiDataGenerationReleaseChecked(xui_data_generation_t* generation)
{
    if(!generation)return XUI_OK;
    if(__xuiDataGenerationRoot(generation)){bool consumed=false;return __xuiDataOwnedRelease(__xuiDataGenerationRoot(generation),&consumed);}
    if(atomic_fetch_sub_explicit(&generation->iRefs,1u,memory_order_acq_rel)!=1u)return XUI_OK;
    if(generation->onFree)generation->onFree(generation->pUser);
    xrtFree(generation);return XUI_OK;
}
static void __xuiDataGenerationDrop(xui_data_generation_t* generation){(void)__xuiDataGenerationReleaseChecked(generation);}
static int __xuiDataGenerationSnapshotEnd(xui_data_generation_t* generation)
{return __xuiDataGenerationRoot(generation) ? __xuiDataOwnedActivityEnd(__xuiDataGenerationRoot(generation)):__xuiDataGenerationReleaseChecked(generation);}

static int __xuiDataSlotFind(xui_data_object pData,const char* format)
{
	if(!__xuiDataObjectValid(pData) || !format)return -1;
	for(int i=0;i<pData->iCount;++i)
		if(strcmp(pData->pSlots[i]->sFormat,format)==0)return i;
	return -1;
}

static int __xuiDataSlotReserve(xui_data_object pData,int capacity)
{
	if(capacity<=pData->iCapacity)return XUI_OK;
	if(capacity<0 || (size_t)capacity>SIZE_MAX/sizeof(*pData->pSlots))return XUI_ERROR_OUT_OF_MEMORY;
	int next=pData->iCapacity>0 ? pData->iCapacity:4;
	while(next<capacity){if(next>INT_MAX/2){next=capacity;break;}next*=2;}
	if((size_t)next>SIZE_MAX/sizeof(*pData->pSlots))return XUI_ERROR_OUT_OF_MEMORY;
	xui_data_generation_t** slots=(xui_data_generation_t**)xrtRealloc(pData->pSlots,sizeof(*slots)*(size_t)next);
	if(!slots)return XUI_ERROR_OUT_OF_MEMORY;
	pData->pSlots=slots;pData->iCapacity=next;
	return XUI_OK;
}

XUI_API int xuiDataObjectCreate(xui_data_object* output)
{
	if(!output)return XUI_ERROR_INVALID_ARGUMENT;
	*output=NULL;
	xui_data_object object=(xui_data_object)xrtCalloc(1u,sizeof(*object));
	if(!object)return XUI_ERROR_OUT_OF_MEMORY;
	object->iMagic=XUI_DATA_OBJECT_MAGIC;
	atomic_init(&object->iRefCount,1u);atomic_init(&object->iActivity,0u);atomic_init(&object->bDestroying,0u);
	*output=object;return XUI_OK;
}

XUI_API int xuiDataObjectAddRef(xui_data_object object)
{
	if(__xuiDataObjectRoot(object))return __xuiDataOwnedAcquire(__xuiDataObjectRoot(object),false,false);
	if(!__xuiDataObjectValid(object))return XUI_ERROR_INVALID_ARGUMENT;
	unsigned refs=atomic_load_explicit(&object->iRefCount,memory_order_relaxed);
	do {if(!refs || refs>=INT_MAX)return XUI_ERROR_INVALID_ARGUMENT;}
	while(!atomic_compare_exchange_weak_explicit(&object->iRefCount,&refs,refs+1u,memory_order_relaxed,memory_order_relaxed));
	return XUI_OK;
}

XUI_API void xuiDataObjectRelease(xui_data_object object)
{
	if(__xuiDataObjectRoot(object)){bool consumed=false;(void)__xuiDataOwnedRelease(__xuiDataObjectRoot(object),&consumed);return;}
	if(!__xuiDataObjectValid(object))return;
	if(atomic_fetch_sub_explicit(&object->iRefCount,1u,memory_order_acq_rel)!=1u)return;
	/* No owning field remains attached while provider cleanup can reenter. A
	 * real active operation still owns a reference and cannot reach this tail. */
	atomic_store_explicit(&object->bDestroying,1u,memory_order_release);
	xui_data_generation_t** slots=object->pSlots;
	int count=object->iCount;
	object->pSlots=NULL;object->iCount=0;object->iCapacity=0;
	for(int i=0;i<count;++i)__xuiDataGenerationDrop(slots[i]);
	xrtFree(slots);object->iMagic=0u;xrtFree(object);
}

static int __xuiDataOperationBegin(xui_data_object object,xui_data_operation_t* call)
{
    memset(call,0,sizeof(*call));
    xge_data_lifetime_t* root=__xuiDataObjectRoot(object);
    if(root){
        call->bOwned=true;call->pAmbient=xrtTakeError();int status=__xuiDataOwnedAcquire(root,true,true);
        if(status!=XUI_OK){xrtErrorFree(call->pAmbient);call->pAmbient=NULL;return status;}
    }else{
        int status=xuiDataObjectAddRef(object);if(status!=XUI_OK)return status;
        atomic_fetch_add_explicit(&object->iActivity,1u,memory_order_relaxed);
    }
    call->pObject=object;return XUI_OK;
}
static int __xuiDataOperationEnd(xui_data_operation_t* call,int status)
{
    xui_data_object object=call->pObject;call->pObject=NULL;
    if(call->bOwned){
        xge_data_outcome_t result={0};__xuiDataOwnedOutcomeBegin(&result,status);
        __xuiDataOwnedOutcomeAdd(&result,__xuiDataOwnedActivityEnd(__xuiDataObjectRoot(object)));
        return __xuiDataOwnedOutcomeEnd(&result,call->pAmbient);
    }
    xerror* first=status!=XUI_OK ? xrtTakeError():NULL;
    atomic_fetch_sub_explicit(&object->iActivity,1u,memory_order_release);xuiDataObjectRelease(object);
    if(status!=XUI_OK){xrtClearError();xrtSetErrorTake(first);}
    return status;
}

static xui_data_generation_t* __xuiDataGenerationCreate(xui_data_object object,const char* format,const void* value,size_t size,bool provider)
{
    xge_data_lifetime_t* owner=__xuiDataObjectRoot(object);size_t extra=owner ? sizeof(xge_data_lifetime_t)+(provider ? sizeof(xui_callback_owner_t):0u):0u;
    size_t length=strlen(format);size_t header=sizeof(xui_data_generation_t)+extra;
    if(length>=SIZE_MAX-header || size>SIZE_MAX-header-length-1u)return NULL;
    xui_data_generation_t* generation=xrtCalloc(1,header+length+1u+size);if(!generation)return NULL;
    atomic_init(&generation->iRefs,1u);
    if(owner){generation->iFlags=XUI_DATA_MANAGED|(provider ? XUI_DATA_OWNED_PROVIDER:0u);__xuiDataOwnedInit(__xuiDataGenerationRoot(generation),&__xuiDataGenerationType,owner->iCreator);}
    generation->sFormat=(char*)generation+header;memcpy(generation->sFormat,format,length+1u);
    if(size){generation->pValue=generation->sFormat+length+1u;memcpy(generation->pValue,value,size);}
    if(provider)generation->tSize.formatBytes=length;else generation->tSize.valueBytes=size;return generation;
}

static int __xuiDataPublish(xui_data_object object,xui_data_generation_t* next,bool* published)
{
    *published=false;xge_data_lifetime_t* root=__xuiDataObjectRoot(object);xrtownershipscope mutation={0};
    if(root){if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;int status=__xuiDataOwnedAdmit(root);if(status!=XUI_OK){__xuiDataOwnedMutationEnd(&mutation);return status;}}
    int index=__xuiDataSlotFind(object,next->sFormat);xui_data_generation_t* old=NULL;int status=XUI_OK;
    if(index<0){
        if(object->iCount==INT_MAX)status=XUI_ERROR_OUT_OF_MEMORY;
        else status=__xuiDataSlotReserve(object,object->iCount+1);
        if(status==XUI_OK)index=object->iCount++;
    }else old=object->pSlots[index];
    if(status==XUI_OK){object->pSlots[index]=next;*published=true;}
    if(root)__xuiDataOwnedMutationEnd(&mutation);
    if(status==XUI_OK)status=__xuiDataGenerationReleaseChecked(old);
    return status;
}

XUI_API int xuiDataObjectSet(xui_data_object object,const char* format,const void* value,size_t size)
{
	if(!format || !format[0] || (size && !value))return XUI_ERROR_INVALID_ARGUMENT;
	xui_data_operation_t call={0};int status=__xuiDataOperationBegin(object,&call);
	if(status!=XUI_OK)return status;
	xui_data_generation_t* next=__xuiDataGenerationCreate(object,format,value,size,false);
	if(!next)status=XUI_ERROR_OUT_OF_MEMORY;
	else {
		bool published=false;status=__xuiDataPublish(object,next,&published);
		if(!published)__xuiDataGenerationDrop(next);
	}
	return __xuiDataOperationEnd(&call,status);
}

XUI_API int xuiDataObjectSetProvider(xui_data_object object,const char* format,
	xui_data_provider_proc read,xui_data_provider_free_proc release,void* user)
{
	if(!format || !format[0] || !read)return XUI_ERROR_INVALID_ARGUMENT;
	if(__xuiDataObjectRoot(object))return XUI_ERROR_INVALID_STATE;
	xui_data_operation_t call={0};int status=__xuiDataOperationBegin(object,&call);
	if(status!=XUI_OK)return status;
	xui_data_generation_t* next=__xuiDataGenerationCreate(object,format,NULL,0u,false);
	if(!next)status=XUI_ERROR_OUT_OF_MEMORY;
	else {
		next->tRead.raw=read;next->onFree=release;next->pUser=user;
		bool published=false;status=__xuiDataPublish(object,next,&published);
		if(!published){next->onFree=NULL;__xuiDataGenerationDrop(next);}
	}
	return __xuiDataOperationEnd(&call,status);
}

XUI_API int xuiDataObjectFormatCount(xui_data_object object)
{return __xuiDataObjectValid(object) ? object->iCount:0;}

XUI_API const char* xuiDataObjectFormatAt(xui_data_object object,int index)
{
	return __xuiDataObjectValid(object) && index>=0 && index<object->iCount ? object->pSlots[index]->sFormat:NULL;
}

XUI_API int xuiDataObjectHas(xui_data_object object,const char* format)
{return __xuiDataSlotFind(object,format)>=0;}

XUI_API int xuiDataObjectGet(xui_data_object object,const char* format,void* output,size_t capacity,size_t* outputSize)
{
    if(!outputSize || (capacity && !output))return XUI_ERROR_INVALID_ARGUMENT;
    *outputSize=0u;int index=__xuiDataSlotFind(object,format);
    xui_data_generation_t* generation=index<0 ? NULL:object->pSlots[index];
    bool owned=__xuiDataObjectRoot(object)!=NULL;
    bool provider=generation && ((generation->iFlags&XUI_DATA_OWNED_PROVIDER) || generation->tRead.raw!=NULL);
    if(generation && !provider){
        *outputSize=generation->tSize.valueBytes;
        if(output && capacity<generation->tSize.valueBytes)return XUI_ERROR_BUFFER_TOO_SMALL;
        if(output && generation->tSize.valueBytes)memcpy(output,generation->pValue,generation->tSize.valueBytes);
        return XUI_OK;
    }
    xui_data_operation_t call={0};int status=__xuiDataOperationBegin(object,&call);if(status!=XUI_OK)return status;
    if(!generation)status=XUI_ERROR_NOT_FOUND;
    else if(!__xuiDataGenerationHold(generation))status=XUI_ERROR_INVALID_STATE;
    else {
        if(generation->iFlags&XUI_DATA_OWNED_PROVIDER){
            xui_callback_owner_t owner=*__xuiDataGenerationOwner(generation);bool held=false;
            status=__xuiDataOwnerHold(&owner,&held);xge_data_outcome_t result={0};__xuiDataOwnedOutcomeBegin(&result,status);
            if(status==XUI_OK){
                xstrview view={generation->sFormat,generation->tSize.formatBytes};
                xui_data_output_t sink={NULL,output,capacity,0u};sink._pSelf=&sink;
                int callback=generation->tRead.owned(&view,&sink,generation->pUser);sink._pSelf=NULL;*outputSize=sink.iRequired;
                __xuiDataOwnedOutcomeAdd(&result,callback==0 ? XUI_OK:XUI_ERROR_CALLBACK_FAILED);
                if(result.iStatus==XUI_OK && output && sink.iRequired>capacity)
                    __xuiDataOwnedOutcomeAdd(&result,XUI_ERROR_BUFFER_TOO_SMALL);
            }
            if(held)__xuiDataOwnedOutcomeAdd(&result,__xuiDataOwnerDrop(owner));
            status=result.iStatus;xrtSetErrorTake(result.pError);
        }else if(generation->tRead.raw)status=generation->tRead.raw(generation->sFormat,output,capacity,outputSize,generation->pUser);
        else {
            *outputSize=generation->tSize.valueBytes;if(output && capacity<generation->tSize.valueBytes)status=XUI_ERROR_BUFFER_TOO_SMALL;
            else if(output && generation->tSize.valueBytes)memcpy(output,generation->pValue,generation->tSize.valueBytes);
        }
        if(owned){
            xge_data_outcome_t result={0};__xuiDataOwnedOutcomeBegin(&result,status);
            __xuiDataOwnedOutcomeAdd(&result,__xuiDataGenerationSnapshotEnd(generation));status=result.iStatus;xrtSetErrorTake(result.pError);
        }else {
            xerror* first=status!=XUI_OK ? xrtTakeError():NULL;__xuiDataGenerationSnapshotEnd(generation);
            if(status!=XUI_OK){xrtClearError();xrtSetErrorTake(first);}
        }
    }
    return __xuiDataOperationEnd(&call,status);
}


XUI_API int xuiDataOutputWrite(xui_data_output_t* output,size_t offset,const void* data,size_t size)
{
    if(!output || output->_pSelf!=output || (size && !data) || offset>SIZE_MAX-size)return XUI_ERROR_INVALID_ARGUMENT;
    size_t previous=output->iRequired,required=offset+size;if(required>previous)output->iRequired=required;
    if(!output->_pDestination)return XUI_OK;
    if(required>output->iCapacity)return XUI_OK;
    if(size)memmove((char*)output->_pDestination+offset,data,size);
    if(offset>previous)memset((char*)output->_pDestination+previous,0,offset-previous);
    return XUI_OK;
}
XUI_API int xuiDataOutputSet(xui_data_output_t* output,const void* data,size_t size)
{
    if(!output || output->_pSelf!=output || (size && !data))return XUI_ERROR_INVALID_ARGUMENT;
    output->iRequired=size;if(!output->_pDestination)return XUI_OK;
    if(size>output->iCapacity)return XUI_OK;
    if(size)memmove(output->_pDestination,data,size);
    return XUI_OK;
}
XUI_API int xuiDataObjectSetProviderOwned(xui_data_object object,const char* format,xui_data_provider_owned_proc read,void* user,const xui_callback_owner_t* incoming)
{
    if(!format || !format[0] || !read || !__xuiDataOwnerValid(incoming) || !xuiDataObjectIsOwned(object))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t owner=*incoming;xui_data_operation_t call={0};int status=__xuiDataOperationBegin(object,&call);if(status!=XUI_OK)return status;
    xui_data_generation_t* next=__xuiDataGenerationCreate(object,format,NULL,0u,true);bool published=false;
    if(!next)status=XUI_ERROR_OUT_OF_MEMORY;
    else {
        bool held=false;status=__xuiDataOwnerHold(&owner,&held);if(held)*__xuiDataGenerationOwner(next)=owner;
        if(status==XUI_OK)status=__xuiDataOwnedAdmit(__xuiDataObjectRoot(object));
        if(status==XUI_OK){next->tRead.owned=read;next->pUser=user;status=__xuiDataPublish(object,next,&published);}
        if(!published){xge_data_outcome_t result={0};__xuiDataOwnedOutcomeBegin(&result,status);__xuiDataOwnedOutcomeAdd(&result,__xuiDataGenerationReleaseChecked(next));status=result.iStatus;xrtSetErrorTake(result.pError);}
    }
    return __xuiDataOperationEnd(&call,status);
}


#endif
