/* CPU-only quota ownership. No model file, network, window or GPU. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xge.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static xge3d_decode_budget_info_t info(xge3d_decode_budget* budget)
{
    xge3d_decode_budget_info_t value={.size=sizeof(value)};
    assert(xge3dDecodeBudgetInfo(budget,&value)==XGE_OK);return value;
}
int main(void)
{
    xge3d_decode_budget *a=(void*)(uintptr_t)1,*b=(void*)(uintptr_t)1;
    assert(xge3dDecodeBudgetCreate(0,&a)==XGE_ERROR_INVALID_ARGUMENT && !a);
    assert(xge3dDecodeBudgetCreate(64,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dDecodeBudgetShareOwned(NULL,&b)==XGE_ERROR_INVALID_ARGUMENT && !b);
    assert(xge3dDecodeBudgetCreate(64,&a)==XGE_OK && a);
    assert(xge3dDecodeBudgetShareOwned(a,NULL)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dDecodeBudgetShareOwned(a,&b)==XGE_OK && a==b);
    assert(info(a).limit==64 && !info(a).live_bytes);
    xge3d_decode_budget_info_t wrong={0};
    assert(xge3dDecodeBudgetInfo(a,&wrong)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dDecodeBudgetSetLimit(a,0)==XGE_ERROR_INVALID_ARGUMENT && info(a).limit==64);
    xrtClearError();
    xallocationscope scope={0};assert(xrtAllocationBudgetEnter(b,&scope));
    unsigned char* bytes=xrtMalloc(40);assert(bytes);
    bytes[0]=17;bytes[39]=91;
    assert(info(a).live_bytes==40 && info(a).live_allocations==1);
    assert(xge3dDecodeBudgetSetLimit(a,39)==XGE_ERROR_INVALID_ARGUMENT && info(a).limit==64);
    xrtClearError();
    uint64_t rejectedBefore=info(a).rejections;
    assert(!xrtMalloc(25));xrtClearError();
    assert(info(a).rejections>rejectedBefore && info(a).peak_bytes>=40);
    assert(xge3dDecodeBudgetSetLimit(a,128)==XGE_OK && info(b).limit==128);
    xge3dDecodeBudgetFree(a);a=NULL;
    assert(bytes[0]==17 && bytes[39]==91 && info(b).live_bytes==40);
    xge3dDecodeBudgetFree(b);b=NULL;
    /* The real active allocation scope still owns the budget here. */
    assert(info(scope.Budget).limit==128 && info(scope.Budget).live_bytes==40);
    xrtFree(bytes);assert(!info(scope.Budget).live_bytes);
    assert(xrtAllocationBudgetLeave(&scope));
    assert(xge3dDecodeBudgetCreate(UINT64_MAX,&a)==XGE_OK && info(a).limit==UINT64_MAX);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){
        assert(xge3dDecodeBudgetSetLimit(a,64+(uint64_t)(i%2))==XGE_OK);
        assert(info(a).limit==64+(uint64_t)(i%2));
        assert(xge3dDecodeBudgetShareOwned(a,&b)==XGE_OK && b==a);
        xge3dDecodeBudgetFree(b);b=NULL;
    }
    xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount &&
        before.ReallocCount==after.ReallocCount);
    xge3dDecodeBudgetFree(a);xge3dDecodeBudgetFree(NULL);xrtClearError();
    puts("3D DecodeBudget: independent owners, live charges, limits, rejection and 4000 zero-heap operations passed");
    return 0;
}
