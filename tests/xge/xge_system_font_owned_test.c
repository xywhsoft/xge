#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));
    xge_system_font_desc_t desc={0};desc.iSize=sizeof(desc);desc.iRole=XGE_SYSTEM_FONT_MONOSPACE;desc.fPixelSize=16;
    xge_font font=NULL;assert(xgeFontLoadSystemOwned(&font,&desc)==XGE_OK);
    assert(font && font->fSize==16 && xgeFontIsOwned(font) && xgeGraphicsOwnedResourceCount()==1);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);xgeFontFree(font);assert(!xgeGraphicsOwnedResourceCount());
    assert(xgeUnitChecked()==XGE_OK);xrtClearError();
    for(int budget=0;budget<16;++budget){
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));font=(void*)(uintptr_t)1;
        int result=xgeFontLoadSystemOwned(&font,&desc);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert((result==XGE_OK)==(font!=NULL));xgeFontFree(font);
        assert(!xgeGraphicsOwnedResourceCount());assert(xgeUnitChecked()==XGE_OK);xrtClearError();
        xrtMemDebugSnapshot(&after);
        assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
        assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    }
    desc.iRole=INT_MAX;font=(void*)(uintptr_t)1;
    assert(xgeFontLoadSystemOwned(&font,&desc)==XGE_ERROR_INVALID_ARGUMENT && !font);
    assert(!xgeGraphicsOwnedResourceCount());assert(xgeUnitChecked()==XGE_OK);
    puts("System font: platform resolution, owned engine obligation, Busy and OOM rollback passed");return 0;
}
