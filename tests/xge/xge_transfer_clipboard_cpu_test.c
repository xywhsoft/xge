/* Production single-XGE consumer with typed fake OLE publication. This never
 * creates a window, initializes a renderer, or reads/writes the OS clipboard. */
#if defined(_WIN32)
#include <windows.h>
#include <ole2.h>
#include <assert.h>
#include <string.h>
#define XRT_MODULE_ALL
#include "xge.h"

static IDataObject* cpuClipboard;
static HGLOBAL cpuReadHandle;
static int cpuInitResult,cpuSetResult,cpuFlushResult,cpuWrapResult;
static int cpuInitCalls,cpuUnitCalls,cpuSetCalls,cpuFlushCalls,cpuWrapCalls;
static int cpuExtraRefs;
static int cpuDragCalls,cpuDragCancel,cpuEventCancel,cpuEventComplete,cpuTraceEdges;
static int cpuProviderCalls;
static IDataObject* cpuDragHeld;
static xn_transfer_native_owner_v1 cpuOwner;
static bool cpuVisit(xrtownershipref ref,void* user)
{(void)user;size_t count=0;assert(ref.Data && ref.Ops && ref.Ops->Count(ref.Data,&count) && count>0);cpuTraceEdges++;return true;}
static HRESULT WINAPI cpuDoDragDrop(IDataObject* data,IDropSource* source,DWORD allowed,DWORD* effect)
{
    cpuDragCalls++;assert(allowed&DROPEFFECT_COPY);
    assert(source->lpVtbl->QueryContinueDrag(source,TRUE,0)==DRAGDROP_S_CANCEL);
    data->lpVtbl->AddRef(data);cpuDragHeld=data;
    if(cpuDragCancel>=2){
        FORMATETC format={0};format.cfFormat=0xc003;format.dwAspect=DVASPECT_CONTENT;format.lindex=-1;format.tymed=TYMED_HGLOBAL;
        STGMEDIUM output={0};assert(FAILED(data->lpVtbl->GetData(data,&format,&output)));
        if(cpuDragCancel==3)assert(FAILED(data->lpVtbl->GetData(data,&format,&output)));
        *effect=0;return DRAGDROP_S_CANCEL;
    }
    if(cpuDragCancel)assert(xgeDragCancel()==XGE_OK);
    *effect=DROPEFFECT_COPY;return cpuDragCancel ? DRAGDROP_S_CANCEL:DRAGDROP_S_DROP;
}
static HRESULT WINAPI cpuOleInitialize(void* reserved){(void)reserved;cpuInitCalls++;return (HRESULT)cpuInitResult;}
static void WINAPI cpuOleUninitialize(void){cpuUnitCalls++;}
static HRESULT WINAPI cpuOleSetClipboard(IDataObject* data)
{
    cpuSetCalls++;if(FAILED((HRESULT)cpuSetResult))return (HRESULT)cpuSetResult;
    if(cpuClipboard)cpuClipboard->lpVtbl->Release(cpuClipboard);
    cpuClipboard=data;data->lpVtbl->AddRef(data);
    for(int i=0;i<cpuExtraRefs;++i)data->lpVtbl->AddRef(data);
    return S_OK;
}
static HRESULT WINAPI cpuOleFlushClipboard(void){cpuFlushCalls++;return (HRESULT)cpuFlushResult;}
static DWORD WINAPI cpuClipboardSequence(void){return 0;}
static BOOL WINAPI cpuOpenClipboard(HWND owner){assert(owner==(HWND)1);return TRUE;}
static BOOL WINAPI cpuCloseClipboard(void){if(cpuReadHandle){GlobalFree(cpuReadHandle);cpuReadHandle=NULL;}return TRUE;}
static BOOL WINAPI cpuFormatAvailable(UINT format)
{FORMATETC query={0};query.cfFormat=(CLIPFORMAT)format;query.dwAspect=DVASPECT_CONTENT;query.lindex=-1;query.tymed=TYMED_HGLOBAL;return cpuClipboard && cpuClipboard->lpVtbl->QueryGetData(cpuClipboard,&query)==S_OK;}
static HANDLE WINAPI cpuGetClipboardData(UINT format)
{FORMATETC query={0};query.cfFormat=(CLIPFORMAT)format;query.dwAspect=DVASPECT_CONTENT;query.lindex=-1;query.tymed=TYMED_HGLOBAL;STGMEDIUM medium={0};if(!cpuClipboard || cpuClipboard->lpVtbl->GetData(cpuClipboard,&query,&medium)!=S_OK)return NULL;assert(!cpuReadHandle && medium.tymed==TYMED_HGLOBAL && !medium.pUnkForRelease);cpuReadHandle=medium.hGlobal;return cpuReadHandle;}
static UINT WINAPI cpuRegisterClipboardFormat(LPCSTR name)
{return !strcmp(name,"HTML Format") ? 0xc001:!strcmp(name,"PNG") ? 0xc002:0xc003;}
static int cpuWrap(const xn_transfer_services_v1* services,void* native,const xn_transfer_native_owner_v1* owner,void** out)
{
    (void)services;cpuWrapCalls++;*out=NULL;
    assert(owner->iSize==sizeof(*owner) && owner->iAbiTag==XN_TRANSFER_NATIVE_OWNER_V1_TAG && owner->onReady(native));
    assert(owner->onTrace(native,cpuVisit,NULL));
    cpuOwner=*owner;if(cpuWrapResult)return cpuWrapResult;
    /* Explicit process-resident fixture: Native iface already has the exact
     * typed COM vtable and one owned ref, so no code carrier is necessary. */
    *out=native;return 0;
}
#define OleInitialize cpuOleInitialize
#define OleUninitialize cpuOleUninitialize
#define OleSetClipboard cpuOleSetClipboard
#define OleFlushClipboard cpuOleFlushClipboard
#define GetClipboardSequenceNumber cpuClipboardSequence
#define RegisterClipboardFormatA cpuRegisterClipboardFormat
#define DoDragDrop cpuDoDragDrop
#define OpenClipboard cpuOpenClipboard
#define CloseClipboard cpuCloseClipboard
#define IsClipboardFormatAvailable cpuFormatAvailable
#define GetClipboardData cpuGetClipboardData
#include "../../single/xge.c"
#undef OleInitialize
#undef OleUninitialize
#undef OleSetClipboard
#undef OleFlushClipboard
#undef GetClipboardSequenceNumber
#undef RegisterClipboardFormatA
#undef DoDragDrop
#undef OpenClipboard
#undef CloseClipboard
#undef IsClipboardFormatAvailable
#undef GetClipboardData

#undef NDEBUG
#include <assert.h>

static void cpuReset(void)
{
    if(cpuClipboard){IDataObject* old=cpuClipboard;cpuClipboard=NULL;
        for(int i=0;i<cpuExtraRefs;++i)old->lpVtbl->Release(old);
        old->lpVtbl->Release(old);}
    cpuInitResult=cpuSetResult=cpuFlushResult=cpuWrapResult=0;cpuExtraRefs=0;
    cpuInitCalls=cpuUnitCalls=cpuSetCalls=cpuFlushCalls=cpuWrapCalls=0;
    g_xgeWin32OleInitialized=0;g_xge.bSokolRunning=0;g_xge.bInitialized=0;xrtClearError();
}
static uint32_t cpuDragEvent(const xge_drag_event_t* event,void* user)
{(void)user;assert(event->pData && xgeDataObjectIsOwned(event->pData));if(event->iType==XGE_DRAG_EVENT_CANCEL)cpuEventCancel++;if(event->iType==XGE_DRAG_EVENT_COMPLETE)cpuEventComplete++;return XGE_DRAG_EFFECT_COPY;}
static bool cpuProviderCount(const void* data,size_t* count){*count=*(const size_t*)data;return true;}
static bool cpuProviderTrace(const void* data,xrtownershipvisitor visit,void* user){(void)data;(void)visit;(void)user;return true;}
static bool cpuProviderHold(const void* data){++*(size_t*)data;return true;}
static bool cpuProviderDrop(const void* data){--*(size_t*)data;return true;}
static int cpuProviderRead(const xstrview* format,xge_data_output_t* output,void* user)
{(void)format;(void)output;(void)user;if(++cpuProviderCalls==2)xrtSetErrorKind(XERR_ARGUMENT);return 1;}
static void cpuDragCases(const xn_transfer_services_v1* services)
{
    for(int cancel=0;cancel<2;++cancel){
        cpuReset();cpuDragCalls=cpuEventCancel=cpuEventComplete=cpuTraceEdges=0;cpuDragCancel=cancel;
        g_xgeWin32OleInitialized=1;g_xgeNativeDragOut=1;
        xge_data_object data=NULL;assert(xgeDataObjectCreateOwned(&data)==XGE_OK);
        assert(xgeDataObjectSet(data,"application/x-native-test","a\0b",3)==XGE_OK);
        assert(xgeDragBegin(data,XGE_DRAG_EFFECT_COPY,0)==XGE_ERROR_UNSUPPORTED && !xgeDragIsActive());
        assert(xgeDragEventCallbackSet(cpuDragEvent,NULL)==XGE_OK);
        assert(xgeDragBeginWithTransfer(data,XGE_DRAG_EFFECT_COPY,0,services)==XGE_OK);
        assert(cpuDragCalls==1 && cpuTraceEdges==1 && !xgeDragIsActive());
        assert(cpuEventComplete==!cancel && cpuEventCancel==cancel);
        bool consumed=false;assert(xgeDataObjectReleaseOwnerChecked(data,&consumed)==XGE_OK && consumed);
        assert(cpuDragHeld);FORMATETC format={0};format.cfFormat=0xc003;format.dwAspect=DVASPECT_CONTENT;format.lindex=-1;format.tymed=TYMED_HGLOBAL;
        STGMEDIUM medium={0};assert(cpuDragHeld->lpVtbl->GetData(cpuDragHeld,&format,&medium)==S_OK);
        assert(!memcmp(GlobalLock(medium.hGlobal),"a\0b",3));GlobalUnlock(medium.hGlobal);ReleaseStgMedium(&medium);
        cpuDragHeld->lpVtbl->Release(cpuDragHeld);cpuDragHeld=NULL;
        assert(xgeDragEventCallbackSet(NULL,NULL)==XGE_OK);g_xgeNativeDragOut=0;
    }
    for(int action=2;action<4;++action){
        cpuReset();cpuDragCalls=cpuEventCancel=cpuEventComplete=cpuProviderCalls=0;cpuDragCancel=action;
        g_xgeWin32OleInitialized=1;g_xgeNativeDragOut=1;
        size_t references=1;static const xrtownershipops ops={cpuProviderCount,cpuProviderTrace};
        xge_callback_owner_t owner={{&references,&ops},cpuProviderHold,cpuProviderDrop};
        xge_data_object data=NULL;assert(xgeDataObjectCreateOwned(&data)==XGE_OK);
        assert(xgeDataObjectSetProviderOwned(data,"application/x-native-test",cpuProviderRead,NULL,&owner)==XGE_OK);
        assert(xgeDragBeginWithTransfer(data,XGE_DRAG_EFFECT_COPY,0,services)==XGE_ERROR_CALLBACK_FAILED);
        assert(cpuDragCalls==1 && cpuProviderCalls==action-1 && !xgeDragIsActive());
        assert(!xrtGetError()); /* A later fresh diagnostic cannot replace first NULL. */
        bool consumed=false;assert(xgeDataObjectReleaseOwnerChecked(data,&consumed)==XGE_OK && consumed);
        cpuDragHeld->lpVtbl->Release(cpuDragHeld);cpuDragHeld=NULL;assert(references==1);g_xgeNativeDragOut=0;
    }
}
static void cpuCheckData(void)
{
    assert(cpuClipboard);FORMATETC format={0};format.cfFormat=0xc003;
    format.dwAspect=DVASPECT_CONTENT;format.lindex=-1;format.tymed=TYMED_HGLOBAL;
    STGMEDIUM medium={0};assert(cpuClipboard->lpVtbl->GetData(cpuClipboard,&format,&medium)==S_OK);
    assert(medium.tymed==TYMED_HGLOBAL && !medium.pUnkForRelease);
    const void* bytes=GlobalLock(medium.hGlobal);assert(bytes && memcmp(bytes,"a\0b",3)==0);
    GlobalUnlock(medium.hGlobal);ReleaseStgMedium(&medium);
    IEnumFORMATETC* values=NULL;assert(cpuClipboard->lpVtbl->EnumFormatEtc(cpuClipboard,DATADIR_GET,&values)==S_OK);
    FORMATETC formats[3];ULONG count=0;assert(values->lpVtbl->Next(values,3,formats,&count)==S_FALSE && count==2);values->lpVtbl->Release(values);
}
int main(void)
{
    xn_transfer_services_v1 services={sizeof(services),XN_TRANSFER_SERVICES_V1_TAG,XN_TRANSFER_PROCESS_RESIDENT,NULL,NULL,cpuWrap,cpuWrap,cpuWrap};
    xge_clipboard_item_t items[]={{XGE_CLIPBOARD_FORMAT_TEXT_UTF8,"hello",5},{"application/x-native-test","a\0b",3}};
    for(int result=0;result<2;++result){
        cpuReset();cpuInitResult=result==0 ? S_OK:S_FALSE;cpuExtraRefs=2;
        assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_OK);
        assert(cpuInitCalls==1 && cpuUnitCalls==1 && cpuSetCalls==1 && cpuFlushCalls==1 && cpuWrapCalls==1);cpuCheckData();
    }
    cpuReset();cpuInitResult=(int)RPC_E_CHANGED_MODE;
    assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_ERROR_BACKEND_FAILED);
    assert(cpuInitCalls==1 && cpuUnitCalls==0 && cpuWrapCalls==0 && cpuSetCalls==0);
    cpuReset();cpuWrapResult=-1;
    assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_ERROR_BACKEND_FAILED);
    assert(cpuUnitCalls==1 && cpuSetCalls==0);
    cpuReset();cpuSetResult=(int)E_FAIL;
    assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_ERROR_BACKEND_FAILED);
    assert(cpuUnitCalls==1 && cpuSetCalls==1 && cpuFlushCalls==0 && !cpuClipboard);
    cpuReset();cpuFlushResult=(int)E_FAIL;
    assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_ERROR_BACKEND_FAILED);
    assert(cpuUnitCalls==1 && cpuSetCalls==1 && cpuFlushCalls==1);cpuCheckData();
    cpuReset();g_xgeWin32OleInitialized=1;
    assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_OK);
    assert(cpuInitCalls==0 && cpuUnitCalls==0 && cpuFlushCalls==0 && cpuSetCalls==1);cpuCheckData();
    cpuReset();xrtSetErrorKind(XERR_STATE);const xerror* ambient=xrtGetError();
    assert(ambient && xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_OK);
    assert(xrtGetError()==ambient && cpuSetCalls==1);
    cpuReset();assert(xrtMemDebugFailAfter(0));
    assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));assert(cpuInitCalls==0 && cpuWrapCalls==0 && cpuSetCalls==0);
    cpuReset();items[0].pData="a\0b";items[0].iDataSize=3;
    assert(xgeClipboardSetItemsWithTransfer(items,2,&services)==XGE_ERROR_INVALID_ARGUMENT && cpuSetCalls==0);
    cpuReset();unsigned char pixel[4]={12,34,56,78};void* png=NULL;size_t pngSize=0;
    assert(xgeImageEncodePNG(1,1,pixel,4,&png,&pngSize)==XGE_OK);
    xge_clipboard_item_t image={XGE_CLIPBOARD_FORMAT_IMAGE_PNG,png,pngSize};
    assert(xgeClipboardSetItemsWithTransfer(&image,1,&services)==XGE_OK);xrtFree(png);
    FORMATETC bitmap={0};bitmap.cfFormat=CF_DIBV5;bitmap.dwAspect=DVASPECT_CONTENT;bitmap.lindex=-1;bitmap.tymed=TYMED_HGLOBAL;
    STGMEDIUM dib={0};assert(cpuClipboard->lpVtbl->GetData(cpuClipboard,&bitmap,&dib)==S_OK);
    const BITMAPV5HEADER* info=GlobalLock(dib.hGlobal);assert(info && info->bV5Width==1 && info->bV5Height==-1 && info->bV5AlphaMask==0xff000000u);
    const unsigned char* packed=(const unsigned char*)(info+1);assert(packed[0]==56 && packed[1]==34 && packed[2]==12 && packed[3]==78);
    GlobalUnlock(dib.hGlobal);ReleaseStgMedium(&dib);
    cpuReset();xge_clipboard_item_t empty={"application/x-native-test",NULL,0};
    assert(xgeClipboardSetItemsWithTransfer(&empty,1,&services)==XGE_OK);
    FORMATETC binary={0};binary.cfFormat=0xc003;binary.dwAspect=DVASPECT_CONTENT;binary.lindex=-1;binary.tymed=TYMED_HGLOBAL;
    STGMEDIUM zero={0};assert(cpuClipboard->lpVtbl->GetData(cpuClipboard,&binary,&zero)==S_OK && zero.hGlobal && GlobalSize(zero.hGlobal)==0);
    ReleaseStgMedium(&zero);
    g_xge.bSokolRunning=1;g_xge.bInitialized=1;_sapp.valid=true;_sapp.win32.hwnd=(HWND)1;
    unsigned char untouched=0xab;assert(xgeClipboardGetData(empty.sFormat,&untouched,1)==0 && untouched==0xab);
    cpuReset();cpuDragCases(&services);cpuReset();return 0;
}
#else
int main(void){return 0;}
#endif
