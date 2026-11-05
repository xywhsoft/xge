#include "xui_config.h"
#if XUI_ENABLE_TABLE_VIEW
#include "xui_internal.h"
#include "xui_callback_hold_internal.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define XUI_TABLE_CELL_MAGIC 0x5843434cu

typedef struct xui_cell_payload_t {
    xstrview text,tooltip;
    xvalue* value;
    xui_callback_owner_t textOwner,renderOwner;
    xui_table_view_cell_output_t* source;
} xui_cell_payload_t;

static bool __xuiCellValid(const xui_table_view_cell_output_t* cell)
{return cell && cell->_iMagic==XUI_TABLE_CELL_MAGIC && !cell->_bRetiring && cell->_iReferences;}
static const xui_table_view_cell_output_t* __xuiCellSource(const xui_table_view_cell_output_t* cell)
{return cell->_pSource ? cell->_pSource:cell;}
static void __xuiCellScalars(xui_table_view_cell_output_t* out,const xui_table_view_cell_output_t* in)
{
    out->iType=in->iType;out->iRowSpan=in->iRowSpan;out->iColSpan=in->iColSpan;
    out->bDisabled=in->bDisabled;out->bEditing=in->bEditing;out->bInvalid=in->bInvalid;
    out->bDirty=in->bDirty;out->bHasStyle=in->bHasStyle;out->bReadOnly=in->bReadOnly;
    out->iBackgroundColor=in->iBackgroundColor;out->iTextColor=in->iTextColor;out->iGridColor=in->iGridColor;
}
static void __xuiCellDefaultScalars(xui_table_view_cell_output_t* cell)
{
    const xui_table_view_cell_output_t defaults={.iRowSpan=1,.iColSpan=1};
    __xuiCellScalars(cell,&defaults);
}
static void __xuiCellRecordOwnOutcome(xui_table_cell_scope_t* scope,int status,const xerror* error)
{
    if(!scope || scope->pSelf!=scope || scope->iResult!=XUI_OK || (status==XUI_OK && !error))return;
    scope->iResult=status!=XUI_OK ? status:XUI_ERROR_CALLBACK_FAILED;scope->_pFirstError=xrtErrorRef(error);
}
static void __xuiCellRecordOutcome(xui_table_view_cell_output_t* cell,int status,const xerror* error)
{
    for(xui_table_cell_scope_t* scope=cell->_pScopes;scope;scope=scope->pNext)
        __xuiCellRecordOwnOutcome(scope,status,error);
}
static void __xuiCellRecordFailure(xui_table_view_cell_output_t* cell,int status)
{__xuiCellRecordOutcome(cell,status,xrtGetError());}
/* Preserve the first failure, including a NULL diagnostic. A later Drop must
 * never replace it merely because the first error could not allocate text. */
static void __xuiCellOutcome(int status,int* result,xerror** first)
{
    xerror* error=xrtTakeError();
    if(status==XUI_OK && !error)return;
    if(*result==XUI_OK){*result=status!=XUI_OK ? status:XUI_ERROR_CALLBACK_FAILED;*first=error;}
    else xrtErrorFree(error);
}
static int __xuiCellDrop(xui_callback_owner_t owner)
{return !owner.onDrop || owner.onDrop(owner.tReference.Data) ? XUI_OK:XUI_ERROR_CALLBACK_FAILED;}
static void __xuiCellDetach(xui_table_view_cell_output_t* cell,xui_cell_payload_t* payload)
{
    payload->text=cell->_tText;payload->tooltip=cell->_tTooltip;payload->value=cell->_pValue;
    payload->textOwner=cell->_tTextOwner;payload->renderOwner=cell->_tRenderOwner;
    payload->source=cell->_pSource;
    cell->_tText=(xstrview){0};cell->_tTooltip=(xstrview){0};cell->_pValue=NULL;
    cell->_onText=NULL;cell->_pTextUser=NULL;cell->_tTextOwner=(xui_callback_owner_t){0};
    cell->_onRender=NULL;cell->_pRenderUser=NULL;cell->_tRenderOwner=(xui_callback_owner_t){0};
    cell->_pSource=NULL;
}
static int __xuiCellUnpinUse(xui_table_view_cell_output_t* cell);
static int __xuiCellDropPayload(xui_cell_payload_t payload)
{
    int result=XUI_OK;xerror* first=NULL;
    xrtFree((void*)payload.text.Data);__xuiCellOutcome(XUI_OK,&result,&first);
    xrtFree((void*)payload.tooltip.Data);__xuiCellOutcome(XUI_OK,&result,&first);
    xrtValueRelease(payload.value);__xuiCellOutcome(XUI_OK,&result,&first);
    __xuiCellOutcome(__xuiCellDrop(payload.textOwner),&result,&first);
    __xuiCellOutcome(__xuiCellDrop(payload.renderOwner),&result,&first);
    if(payload.source)__xuiCellOutcome(__xuiCellUnpinUse(payload.source),&result,&first);
    xrtSetErrorTake(first);return result;
}
/* Entry has already published the zero count and retiring flag under mutation.
 * User invalidation/Drop and physical free are deliberately outside the gate. */
static int __xuiCellDestroy(xui_table_view_cell_output_t* cell)
{
    int heap=cell->_bHeap;xerror* first=xrtTakeError();int result=first ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
    xui_cell_payload_t payload={0};__xuiCellDetach(cell,&payload);
    xuiInternalLifetimeInvalidateAll(&cell->_pLifetimeLinks);
    __xuiCellOutcome(XUI_OK,&result,&first);
    __xuiCellOutcome(__xuiCellDropPayload(payload),&result,&first);
    memset(cell,0,sizeof(*cell));if(heap)xrtFree(cell);
    xrtSetErrorTake(first);return result;
}
static int __xuiCellRetainPin(xui_table_view_cell_output_t* cell,int kind)
{
    if(!__xuiCellValid(cell))return XUI_ERROR_INVALID_ARGUMENT;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    if(cell->_iReferences==SIZE_MAX || (kind==1 && cell->_iReadPins==SIZE_MAX) || (kind==2 && cell->_iUsePins==SIZE_MAX)){
        (void)xrtOwnershipScopeEnd(&mutation);return XUI_ERROR_INVALID_STATE;
    }
    ++cell->_iReferences;if(kind==1)++cell->_iReadPins;if(kind==2)++cell->_iUsePins;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return XUI_OK;
}
static int __xuiCellReleasePin(xui_table_view_cell_output_t* cell,int kind)
{
    if(!__xuiCellValid(cell) || (kind==1 && !cell->_iReadPins) || (kind==2 && !cell->_iUsePins))return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    if(kind==1)--cell->_iReadPins;
    if(kind==2)--cell->_iUsePins;
    bool destroy=--cell->_iReferences==0;if(destroy)cell->_bRetiring=1;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return destroy ? __xuiCellDestroy(cell):XUI_OK;
}
static int __xuiCellUnpinUse(xui_table_view_cell_output_t* cell)
{return __xuiCellReleasePin(cell,2);}
static int __xuiCellMutable(const xui_table_view_cell_output_t* cell,bool payload)
{
    if(!__xuiCellValid(cell))return XUI_ERROR_INVALID_ARGUMENT;
    if(cell->_pOwnershipClaim)return XUI_ERROR_INVALID_STATE;
    if(cell->_iReadPins || cell->_iUsePins || (payload && cell->_pSource))return XUI_ERROR_BUSY;
    return XUI_OK;
}
XUI_API void xuiTableViewCellOutputInit(xui_table_view_cell_output_t* cell)
{
    if(!cell)return;
    memset(cell,0,sizeof(*cell));cell->_iMagic=XUI_TABLE_CELL_MAGIC;
    cell->_iCreatorThread=xrtThreadCurrentId();
    cell->_iReferences=cell->_iExternalReferences=1;__xuiCellDefaultScalars(cell);
}
XUI_API int xuiTableViewCellOutputCreate(xui_table_view_cell_output_t** output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    xui_table_view_cell_output_t* cell=xrtMalloc(sizeof(*cell));
    if(!cell)return XUI_ERROR_OUT_OF_MEMORY;
    xuiTableViewCellOutputInit(cell);cell->_bHeap=1;*output=cell;return XUI_OK;
}
XUI_API int xuiTableViewCellOutputAddRef(xui_table_view_cell_output_t* cell)
{
    if(cell && cell->_iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!__xuiCellValid(cell) || !cell->_bHeap || cell->_pOwnershipClaim)return XUI_ERROR_INVALID_ARGUMENT;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    if(cell->_iReferences==SIZE_MAX || cell->_iExternalReferences==SIZE_MAX){
        (void)xrtOwnershipScopeEnd(&mutation);return XUI_ERROR_INVALID_STATE;
    }
    ++cell->_iReferences;++cell->_iExternalReferences;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return XUI_OK;
}
XUI_API int xuiTableViewCellOutputReleaseOwnerChecked(xui_table_view_cell_output_t* cell,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(cell && cell->_iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!__xuiCellValid(cell) || !cell->_bHeap || !cell->_iExternalReferences)return XUI_ERROR_INVALID_ARGUMENT;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    --cell->_iExternalReferences;bool destroy=--cell->_iReferences==0;if(destroy)cell->_bRetiring=1;
    *consumed=true;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return destroy ? __xuiCellDestroy(cell):XUI_OK;
}
XUI_API int xuiTableViewCellOutputReleaseChecked(xui_table_view_cell_output_t* cell)
{
    bool consumed=false;return xuiTableViewCellOutputReleaseOwnerChecked(cell,&consumed);
}
XUI_API int xuiTableViewCellOutputDisposeChecked(xui_table_view_cell_output_t* cell)
{
    if(!cell || !cell->_iMagic)return XUI_OK;
    if(!__xuiCellValid(cell) || cell->_bHeap)return XUI_ERROR_INVALID_ARGUMENT;
    if(cell->_iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(cell->_iReferences!=1 || cell->_iCalls || cell->_iReadPins)return XUI_ERROR_BUSY;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    cell->_iExternalReferences=cell->_iReferences=0;cell->_bRetiring=1;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return __xuiCellDestroy(cell);
}
XUI_API int xuiTableViewCellOutputCallScopeBegin(xui_table_view_cell_output_t* cell,xui_table_cell_scope_t* scope)
{
    if(!scope || scope->pCell || scope->pSelf || scope->pPrevious || scope->pNext || scope->iResult || scope->bEnding || scope->_pFirstError || !__xuiCellValid(cell) || cell->_pOwnershipClaim)return XUI_ERROR_INVALID_ARGUMENT;
    if(cell->_iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    if(cell->_iReferences==SIZE_MAX || cell->_iCalls==SIZE_MAX){(void)xrtOwnershipScopeEnd(&mutation);return XUI_ERROR_INVALID_STATE;}
    ++cell->_iReferences;++cell->_iCalls;
    scope->pCell=cell;scope->pSelf=scope;scope->pNext=cell->_pScopes;
    if(scope->pNext)scope->pNext->pPrevious=scope;
    cell->_pScopes=scope;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return XUI_OK;
}
XUI_API int xuiTableViewCellOutputCallScopeEnd(xui_table_cell_scope_t* scope)
{
    if(!scope)return XUI_ERROR_INVALID_ARGUMENT;
    if(!scope->pCell && !scope->pSelf && !scope->pPrevious && !scope->pNext &&
       scope->iResult==XUI_OK && !scope->bEnding && !scope->_pFirstError)return XUI_OK;
    if(scope->pSelf!=scope || scope->bEnding || !__xuiCellValid(scope->pCell))return XUI_ERROR_INVALID_STATE;
    xui_table_view_cell_output_t* cell=scope->pCell;
    if(cell->_iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    __xuiCellRecordOutcome(cell,XUI_OK,xrtGetError());
    int result=scope->iResult;xerror* first=scope->_pFirstError;scope->bEnding=1;
    if(scope->pPrevious)scope->pPrevious->pNext=scope->pNext;else cell->_pScopes=scope->pNext;
    if(scope->pNext)scope->pNext->pPrevious=scope->pPrevious;
    --cell->_iCalls;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    /* Keep the address-stable ending token until the final pin's consuming
     * cleanup returns. Its nullable primary owns a separate error reference. */
    xrtClearError();__xuiCellOutcome(__xuiCellReleasePin(cell,false),&result,&first);
    memset(scope,0,sizeof(*scope));xrtSetErrorTake(first);return result;
}
static int __xuiCellScopeEndOutcome(xui_table_cell_scope_t* scope,int status,xerror** first)
{
    __xuiCellRecordOwnOutcome(scope,status,*first);
    int ended=xuiTableViewCellOutputCallScopeEnd(scope);xerror* error=xrtTakeError();
    if(status!=XUI_OK){xrtErrorFree(error);return status;}
    if(ended!=XUI_OK){xrtErrorFree(*first);*first=error;return ended;}
    if(status==XUI_OK && error){*first=error;return XUI_ERROR_CALLBACK_FAILED;}
    xrtErrorFree(error);return status;
}

XUI_API int xuiTableViewCellOutputReset(xui_table_view_cell_output_t* cell)
{
    int result=__xuiCellMutable(cell,false);if(result!=XUI_OK)return result;
    xui_table_cell_scope_t scope={0};result=xuiTableViewCellOutputCallScopeBegin(cell,&scope);if(result!=XUI_OK)return result;
    xui_cell_payload_t old={0};xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){__xuiCellRecordOwnOutcome(&scope,XUI_ERROR_INVALID_STATE,xrtGetError());return xuiTableViewCellOutputCallScopeEnd(&scope);}
    __xuiCellDetach(cell,&old);__xuiCellDefaultScalars(cell);
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    result=__xuiCellDropPayload(old);__xuiCellRecordFailure(cell,result);
    xerror* first=result!=XUI_OK ? xrtTakeError():NULL;
    result=__xuiCellScopeEndOutcome(&scope,result,&first);
    xrtSetErrorTake(first);return result;
}
XUI_API int xuiTableViewCellOutputUse(xui_table_view_cell_output_t* cell,xui_table_view_cell_output_t* cached)
{
    int result=__xuiCellMutable(cell,false);if(result!=XUI_OK)return result;
    if(!__xuiCellValid(cached) || cached->_pOwnershipClaim)return XUI_ERROR_INVALID_ARGUMENT;
    if(cell->_iCreatorThread!=cached->_iCreatorThread)return XUI_ERROR_INVALID_STATE;
    if(cell==cached)return XUI_OK;
    xui_table_view_cell_output_t* source=(xui_table_view_cell_output_t*)__xuiCellSource(cached);
    /* Delegation is always a single edge to independently retainable storage.
     * Self-delegation would create a read-pin cycle, and automatic sources can
     * never escape the callback which owns their physical storage. */
    if(source==cell || !source->_bHeap)return XUI_ERROR_INVALID_ARGUMENT;
    xui_table_cell_scope_t scope={0};result=xuiTableViewCellOutputCallScopeBegin(cell,&scope);if(result!=XUI_OK)return result;
    result=__xuiCellRetainPin(source,2);
    if(result!=XUI_OK){__xuiCellRecordOwnOutcome(&scope,result,xrtGetError());return xuiTableViewCellOutputCallScopeEnd(&scope);}
    xui_cell_payload_t old={0};xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){
        __xuiCellRecordOwnOutcome(&scope,XUI_ERROR_INVALID_STATE,xrtGetError());
        (void)__xuiCellUnpinUse(source);return xuiTableViewCellOutputCallScopeEnd(&scope);
    }
    __xuiCellDetach(cell,&old);__xuiCellScalars(cell,cached);cell->_pSource=source;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    result=__xuiCellDropPayload(old);__xuiCellRecordFailure(cell,result);
    xerror* first=result!=XUI_OK ? xrtTakeError():NULL;
    result=__xuiCellScopeEndOutcome(&scope,result,&first);
    xrtSetErrorTake(first);return result;
}
static int __xuiCellSetText(xui_table_view_cell_output_t* cell,xstrview text,bool tooltip)
{
    int result=__xuiCellMutable(cell,true);if(result!=XUI_OK)return result;
    if(text.Size>(size_t)INT_MAX || (!text.Data && text.Size) || (text.Data && memchr(text.Data,0,text.Size)))return XUI_ERROR_INVALID_ARGUMENT;
    xstrview old=tooltip ? cell->_tTooltip:cell->_tText;
    if(old.Data==text.Data && old.Size==text.Size)return XUI_OK;
    char* copy=NULL;
    if(text.Size){copy=xrtMalloc(text.Size+1);if(!copy)return XUI_ERROR_OUT_OF_MEMORY;memcpy(copy,text.Data,text.Size);copy[text.Size]=0;}
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){xrtFree(copy);return XUI_ERROR_INVALID_STATE;}
    if(tooltip)cell->_tTooltip=(xstrview){copy,text.Size};else cell->_tText=(xstrview){copy,text.Size};
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    xrtFree((void*)old.Data);return XUI_OK;
}
XUI_API int xuiTableViewCellOutputSetText(xui_table_view_cell_output_t* cell,xstrview text)
{return __xuiCellSetText(cell,text,false);}
XUI_API int xuiTableViewCellOutputSetTooltip(xui_table_view_cell_output_t* cell,xstrview text)
{return __xuiCellSetText(cell,text,true);}
XUI_API xstrview xuiTableViewCellOutputGetText(const xui_table_view_cell_output_t* cell)
{return __xuiCellValid(cell) && !cell->_pOwnershipClaim ? __xuiCellSource(cell)->_tText:(xstrview){0};}
XUI_API xstrview xuiTableViewCellOutputGetTooltip(const xui_table_view_cell_output_t* cell)
{return __xuiCellValid(cell) && !cell->_pOwnershipClaim ? __xuiCellSource(cell)->_tTooltip:(xstrview){0};}
XUI_API const xvalue* xuiTableViewCellOutputGetValue(const xui_table_view_cell_output_t* cell)
{return __xuiCellValid(cell) && !cell->_pOwnershipClaim ? __xuiCellSource(cell)->_pValue:NULL;}
XUI_API int xuiTableViewCellOutputSetValue(xui_table_view_cell_output_t* cell,const xvalue* value)
{
    int result=__xuiCellMutable(cell,true);if(result!=XUI_OK)return result;
    xui_table_cell_scope_t scope={0};result=xuiTableViewCellOutputCallScopeBegin(cell,&scope);if(result!=XUI_OK)return result;
    xvalue* copy=value ? xrtValueClone(value):NULL;
    if(value && !copy){__xuiCellRecordOwnOutcome(&scope,XUI_ERROR_OUT_OF_MEMORY,xrtGetError());return xuiTableViewCellOutputCallScopeEnd(&scope);}
    /* Clone can invoke a value's native clone hook. Revalidate the live receiver. */
    result=__xuiCellMutable(cell,true);
    if(result!=XUI_OK){__xuiCellRecordOwnOutcome(&scope,result,xrtGetError());xrtValueRelease(copy);return xuiTableViewCellOutputCallScopeEnd(&scope);}
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){
        __xuiCellRecordOwnOutcome(&scope,XUI_ERROR_INVALID_STATE,xrtGetError());
        xrtValueRelease(copy);return xuiTableViewCellOutputCallScopeEnd(&scope);
    }
    xvalue* old=cell->_pValue;cell->_pValue=copy;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    xrtValueRelease(old);xerror* first=NULL;__xuiCellOutcome(XUI_OK,&result,&first);
    __xuiCellRecordOutcome(cell,result,first);result=__xuiCellScopeEndOutcome(&scope,result,&first);
    xrtSetErrorTake(first);return result;
}
static void __xuiCellCleanupOutcome(xui_table_view_cell_output_t* cell,int status,int* result,xerror** first)
{
    xerror* error=xrtTakeError();if(status==XUI_OK && !error)return;
    if(status==XUI_OK)status=XUI_ERROR_CALLBACK_FAILED;
    __xuiCellRecordOutcome(cell,status,*result!=XUI_OK ? *first:error);
    if(*result==XUI_OK){*result=status;*first=error;}else xrtErrorFree(error);
}
static int __xuiCellSetCallback(xui_table_view_cell_output_t* cell,
    xui_table_view_cell_text_owned_proc text,xui_table_view_cell_render_owned_proc render,
    bool formatter,void* user,const xui_callback_owner_t* incoming)
{
    int result=__xuiCellMutable(cell,true);if(result!=XUI_OK)return result;
    if(!xuiInternalCallbackOwnerValid(incoming) || (!(formatter ? text!=NULL:render!=NULL) && incoming && incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t owner=incoming ? *incoming:(xui_callback_owner_t){0};
    xui_table_cell_scope_t scope={0};result=xuiTableViewCellOutputCallScopeBegin(cell,&scope);if(result!=XUI_OK)return result;
    bool held=false;result=xuiInternalCallbackHoldFresh(&owner,&held);
    if(result==XUI_OK)result=__xuiCellMutable(cell,true);
    xerror* first=result!=XUI_OK ? xrtTakeError():NULL;
    __xuiCellRecordOwnOutcome(&scope,result,first);
    if(result!=XUI_OK){
        if(held)__xuiCellCleanupOutcome(cell,__xuiCellDrop(owner),&result,&first);
    }else{
        xrtownershipscope mutation={0};
        if(!xrtOwnershipMutationBegin(&mutation)){
            result=XUI_ERROR_INVALID_STATE;__xuiCellCleanupOutcome(cell,__xuiCellDrop(owner),&result,&first);
        }else{
            xui_callback_owner_t old=formatter ? cell->_tTextOwner:cell->_tRenderOwner;
            if(formatter){cell->_onText=text;cell->_pTextUser=user;cell->_tTextOwner=owner;}
            else{cell->_onRender=render;cell->_pRenderUser=user;cell->_tRenderOwner=owner;}
            if(!xrtOwnershipScopeEnd(&mutation))abort();
            __xuiCellCleanupOutcome(cell,__xuiCellDrop(old),&result,&first);
        }
    }
    __xuiCellRecordOwnOutcome(&scope,result,first);result=__xuiCellScopeEndOutcome(&scope,result,&first);
    xrtSetErrorTake(first);return result;
}
XUI_API int xuiTableViewCellOutputSetFormatterOwned(xui_table_view_cell_output_t* cell,xui_table_view_cell_text_owned_proc text,void* user,const xui_callback_owner_t* owner)
{return __xuiCellSetCallback(cell,text,NULL,true,user,owner);}
XUI_API int xuiTableViewCellOutputSetRendererOwned(xui_table_view_cell_output_t* cell,xui_table_view_cell_render_owned_proc render,void* user,const xui_callback_owner_t* owner)
{return __xuiCellSetCallback(cell,NULL,render,false,user,owner);}
static bool __xuiCellCount(const void* data,size_t* count)
{
    const xui_table_view_cell_output_t* cell=data;
    if(!__xuiCellValid(cell) || !cell->_bHeap || !count)return false;
    *count=cell->_iReferences;return true;
}
static bool __xuiCellTrace(const void* data,xrtownershipvisitor visit,void* user)
{
    const xui_table_view_cell_output_t* cell=data;
    if(!__xuiCellValid(cell) || !cell->_bHeap || !visit)return false;
    if(cell->_bOwnershipCleared)return true;
    if(cell->_pSource){xrtownershipref ref=xuiTableViewCellOutputOwnership(cell->_pSource);return ref.Data && visit(ref,user);}
    if(cell->_pValue){xrtownershipref ref=xrtValueOwnership(cell->_pValue);if(ref.Data && !visit(ref,user))return false;}
    if(cell->_tTextOwner.tReference.Data && !visit(cell->_tTextOwner.tReference,user))return false;
    if(cell->_tRenderOwner.tReference.Data && !visit(cell->_tRenderOwner.tReference,user))return false;
    return true;
}
static const xrtownershipops __xuiCellOps={__xuiCellCount,__xuiCellTrace};
/* Metadata discovery reads only identity fields, immutable while a caller's
 * real reference/snapshot protects storage. Busy/claim/clear stay separate. */
static bool __xuiCellIdentity(xrtownershipref ref)
{
    const xui_table_view_cell_output_t* cell=ref.Data;
    return ref.Ops==&__xuiCellOps && cell && cell->_iMagic==XUI_TABLE_CELL_MAGIC && cell->_bHeap;
}

XUI_API xrtownershipref xuiTableViewCellOutputOwnership(const xui_table_view_cell_output_t* cell)
{return __xuiCellValid(cell) && cell->_bHeap ? (xrtownershipref){cell,&__xuiCellOps}:(xrtownershipref){0};}
XUI_API bool xuiTableViewCellOutputBusy(const xui_table_view_cell_output_t* cell)
{return !__xuiCellValid(cell) || cell->_iCalls || cell->_iReadPins;}
XUI_API bool xuiTableViewCellOutputCanRetire(const xui_table_view_cell_output_t* cell)
{return __xuiCellValid(cell) && cell->_bHeap && !cell->_iCalls && !cell->_iReadPins && !cell->_pOwnershipClaim;}
XUI_API int xuiTableViewCellOutputLifetimeWatch(xui_table_view_cell_output_t* cell,xui_lifetime_link_t* link,xui_lifetime_invalidate_proc invalidate,void* user)
{
    if(!__xuiCellValid(cell) || !cell->_bHeap || cell->_pOwnershipClaim || !link || !invalidate)return XUI_ERROR_INVALID_ARGUMENT;
    if(link->ppPrevious || link->pNext || link->onInvalidate || link->pUser)return XUI_ERROR_INVALID_STATE;
    link->pNext=cell->_pLifetimeLinks;link->ppPrevious=&cell->_pLifetimeLinks;link->onInvalidate=invalidate;link->pUser=user;
    if(link->pNext)link->pNext->ppPrevious=&link->pNext;
    cell->_pLifetimeLinks=link;return XUI_OK;
}

/* Move owning fields between two explicit native records. This is the
 * transaction commit for GetCellOwned; no second text clone is introduced. */
int xuiInternalTableCellMove(xui_table_view_cell_output_t* target,xui_table_view_cell_output_t* source)
{
    int result=__xuiCellMutable(target,false);if(result!=XUI_OK)return result;
    result=__xuiCellMutable(source,false);if(result!=XUI_OK)return result;
    if(target->_iCreatorThread!=source->_iCreatorThread)return XUI_ERROR_INVALID_STATE;
    if(target==source)return XUI_OK;
    xui_table_cell_scope_t scope={0};result=xuiTableViewCellOutputCallScopeBegin(target,&scope);if(result!=XUI_OK)return result;
    xui_cell_payload_t old={0};xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){__xuiCellRecordOwnOutcome(&scope,XUI_ERROR_INVALID_STATE,xrtGetError());return xuiTableViewCellOutputCallScopeEnd(&scope);}
    __xuiCellDetach(target,&old);__xuiCellScalars(target,source);
    target->_tText=source->_tText;target->_tTooltip=source->_tTooltip;target->_pValue=source->_pValue;
    target->_onText=source->_onText;target->_pTextUser=source->_pTextUser;target->_tTextOwner=source->_tTextOwner;
    target->_onRender=source->_onRender;target->_pRenderUser=source->_pRenderUser;target->_tRenderOwner=source->_tRenderOwner;
    target->_pSource=source->_pSource;
    xui_cell_payload_t moved={0};__xuiCellDetach(source,&moved);__xuiCellDefaultScalars(source);
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    result=__xuiCellDropPayload(old);__xuiCellRecordFailure(target,result);
    xerror* first=result!=XUI_OK ? xrtTakeError():NULL;
    result=__xuiCellScopeEndOutcome(&scope,result,&first);
    xrtSetErrorTake(first);return result;
}

/* Complete native participant admission. Clear quarantines outgoing slots;
 * Finish consumes those retired slots outside freeze exactly once. Count still
 * reports real references (including plan holds and cache read pins). */
static bool __xuiCellAdapterHold(const void* data)
{return xuiTableViewCellOutputAddRef((xui_table_view_cell_output_t*)data)==XUI_OK;}
static void __xuiCellAdapterDrop(const void* data)
{(void)xuiTableViewCellOutputReleaseChecked((xui_table_view_cell_output_t*)data);}
static bool __xuiCellAdapterClaim(const void* data,const void* token)
{
    xui_table_view_cell_output_t* cell=(xui_table_view_cell_output_t*)data;
    if(!token || !__xuiCellValid(cell) || !cell->_bHeap || cell->_iCalls || cell->_iReadPins ||
       cell->_bOwnershipCleared || (cell->_pOwnershipClaim && cell->_pOwnershipClaim!=token))return false;
    cell->_pOwnershipClaim=token;return true;
}
static void __xuiCellAdapterRestore(const void* data,const void* token)
{
    xui_table_view_cell_output_t* cell=(xui_table_view_cell_output_t*)data;
    if(!token || cell->_pOwnershipClaim!=token || cell->_bOwnershipCleared)abort();
    cell->_pOwnershipClaim=NULL;
}
static void __xuiCellAdapterClear(const void* data,const void* token)
{
    xui_table_view_cell_output_t* cell=(xui_table_view_cell_output_t*)data;
    if(!token || cell->_pOwnershipClaim!=token || cell->_bOwnershipCleared)abort();
    cell->_bOwnershipCleared=1;
}
static bool __xuiCellAdapterFinish(const void* data,const void* token)
{
    xui_table_view_cell_output_t* cell=(xui_table_view_cell_output_t*)data;
    if(!token || cell->_pOwnershipClaim!=token || !cell->_bOwnershipCleared)abort();
    if(cell->_bOwnershipFinished)return true;
    xui_cell_payload_t retired={0};xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))return false;
    __xuiCellDetach(cell,&retired);cell->_bOwnershipFinished=1;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return __xuiCellDropPayload(retired)==XUI_OK;
}
static const xrtownershipadapterv1 __xuiCellAdapter={
    sizeof(xrtownershipadapterv1),__xuiCellAdapterHold,__xuiCellAdapterDrop,
    __xuiCellAdapterClaim,__xuiCellAdapterRestore,NULL,__xuiCellAdapterClear,__xuiCellAdapterFinish
};
/* The optional typed completion preserves a failed final snapshot release,
 * including false with no allocated diagnostic. One call consumes one Hold;
 * the caller must never also invoke the legacy void Adapter.Drop. */
/* Only immutable metadata is read here; a physical snapshot hold protects
 * storage. In particular, do not read mutable pin/call/reference counters. */
static bool __xuiCellAdapterDropReady(const void* data)
{return data && ((const xui_table_view_cell_output_t*)data)->_iCreatorThread==xrtThreadCurrentId();}
static xrtownershipfinishresult __xuiCellCompletionStep(const void* data,const void* token)
{
    if(!__xuiCellAdapterDropReady(data))return XRT_OWNERSHIP_FINISH_BUSY;
    return __xuiCellAdapterFinish(data,token) ? XRT_OWNERSHIP_FINISH_READY:XRT_OWNERSHIP_FINISH_FAILED;
}
static bool __xuiCellAdapterDropChecked(const void* data)
{return xuiTableViewCellOutputReleaseChecked((xui_table_view_cell_output_t*)data)==XUI_OK;}
static const xrtownershipfinishv2 __xuiCellCompletionV2={
    {sizeof(xrtownershipfinishv2),&__xuiCellAdapter,__xuiCellCompletionStep},
    XRT_OWNERSHIP_FINISH_V2_TAG,__xuiCellAdapterDropChecked,__xuiCellAdapterDropReady
};
XUI_API const xrtownershipfinishv2* xuiTableViewCellOutputOwnershipCompletionV2(xrtownershipref ref)
{
    return __xuiCellIdentity(ref) ? &__xuiCellCompletionV2:NULL;
}
static bool __xuiCellPreparationReady(const void* data)
{return !xuiTableViewCellOutputBusy(data);}
static xrtownershipprepareresult __xuiCellPreparationPrepare(const void* data,const void* token)
{
    const xui_table_view_cell_output_t* cell=data;
    if(!token || !__xuiCellValid(cell) || cell->_pOwnershipClaim!=token || cell->_bOwnershipCleared)return XRT_OWNERSHIP_PREPARE_FAILED;
    return xuiTableViewCellOutputBusy(cell) ? XRT_OWNERSHIP_PREPARE_BUSY:XRT_OWNERSHIP_PREPARE_READY;
}
static const xrtownershippreparationv1 __xuiCellPreparation={
    sizeof(xrtownershippreparationv1),&__xuiCellAdapter,__xuiCellPreparationReady,__xuiCellPreparationPrepare
};
XUI_API const xrtownershippreparationv1* xuiTableViewCellOutputOwnershipPreparationV1(xrtownershipref ref)
{return __xuiCellIdentity(ref) ? &__xuiCellPreparation:NULL;}
static bool __xuiCellQuiescenceBusy(const void* data)
{return xuiTableViewCellOutputBusy(data);}
static const xrtownershipquiescencev1 __xuiCellQuiescence={sizeof(xrtownershipquiescencev1),&__xuiCellOps,__xuiCellQuiescenceBusy};
XUI_API const xrtownershipadapterv1* xuiTableViewCellOutputOwnershipAdapterV1(xrtownershipref ref)
{
    return __xuiCellIdentity(ref) ? &__xuiCellAdapter:NULL;
}
XUI_API const xrtownershipquiescencev1* xuiTableViewCellOutputOwnershipQuiescenceV1(xrtownershipref ref)
{return __xuiCellIdentity(ref) ? &__xuiCellQuiescence:NULL;}

int xuiInternalTableCellFormatBegin(xui_widget widget,int row,int column,
    const xui_table_view_cell_output_t* cell,xui_table_cell_format_t* format)
{
    if(!format || format->context || format->held || format->owned || format->text.Data ||
       !__xuiCellValid(cell) || !cell->_iReadPins || !xuiInternalWidgetIsValid(widget))return XUI_ERROR_INVALID_ARGUMENT;
    xui_context context=widget->pContext;
    if(context->iOperationDepth<=0 || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
    const xui_table_view_cell_output_t* source=__xuiCellSource(cell);
    format->context=context;
    if(source->_tText.Size || !source->_onText){format->text=source->_tText;return XUI_OK;}
    xui_table_view_cell_text_owned_proc selected=source->_onText;void* user=source->_pTextUser;
    format->owner=source->_tTextOwner;
    int result=xuiInternalCallbackHoldIsolated(&format->owner,&format->held);
    if(result!=XUI_OK)return result;
    if(!xuiInternalWidgetIsValid(widget) || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
    format->text=selected(widget,row,column,cell,user);format->owned=true;
    if(!xuiInternalWidgetIsValid(widget) || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
    if(format->text.Size>(size_t)INT_MAX || (!format->text.Data && format->text.Size) ||
       (format->text.Data && (memchr(format->text.Data,0,format->text.Size) || format->text.Data[format->text.Size]!=0)))return XUI_ERROR_INVALID_ARGUMENT;
    return XUI_OK;
}
int xuiInternalTableCellFormatEndOutcome(xui_table_cell_format_t* format,int result,xerror* first)
{
    xerror* ambient=xrtTakeError();
    if(result==XUI_OK && (first || ambient)){result=XUI_ERROR_CALLBACK_FAILED;if(!first){first=ambient;ambient=NULL;}}
    xrtErrorFree(ambient);
    if(format->owned){format->owned=false;xrtFree((void*)format->text.Data);format->text=(xstrview){0};}
    __xuiCellOutcome(XUI_OK,&result,&first);
    if(format->held){format->held=false;__xuiCellOutcome(xuiInternalCallbackOwnerDropOutcome(format->context,&format->owner,result,first),&result,&first);}
    memset(format,0,sizeof(*format));xrtSetErrorTake(first);return result;
}
int xuiInternalTableCellFormatEnd(xui_table_cell_format_t* format,int result)
{
    xerror* first=result!=XUI_OK ? xrtTakeError():NULL;
    return xuiInternalTableCellFormatEndOutcome(format,result,first);
}

/* Internal read lease covers the actual native formatter/draw consumer, not
 * just provider return. It is also valid for automatic provider output. */
int xuiInternalTableCellReadBegin(xui_table_view_cell_output_t* cell)
{return __xuiCellRetainPin(cell,true);}
int xuiInternalTableCellReadEnd(xui_table_view_cell_output_t* cell)
{return __xuiCellReleasePin(cell,1);}
const xui_table_view_cell_output_t* xuiInternalTableCellSource(const xui_table_view_cell_output_t* cell)
{return __xuiCellValid(cell) ? __xuiCellSource(cell):NULL;}
#endif
