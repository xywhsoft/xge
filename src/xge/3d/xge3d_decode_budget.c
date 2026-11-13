/* Standalone CPU quota operations. Keep these outside the glTF/model TU so
 * budget-only programs do not select parser/model code or require MODEL. */
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#if XGE_ENABLE_3D
int xge3dDecodeBudgetCreate(uint64_t bytes,xge3d_decode_budget **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!bytes||bytes>SIZE_MAX)return XGE_ERROR_INVALID_ARGUMENT;
    return xrtAllocationBudgetCreate((size_t)bytes,out)?XGE_OK:XGE_ERROR_OUT_OF_MEMORY;
}
int xge3dDecodeBudgetShareOwned(xge3d_decode_budget *budget,xge3d_decode_budget **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!budget)return XGE_ERROR_INVALID_ARGUMENT;
    if(!xrtAllocationBudgetAddRef(budget))return XGE_ERROR_INVALID_STATE;
    *out=budget;return XGE_OK;
}
void xge3dDecodeBudgetFree(xge3d_decode_budget *budget){xrtAllocationBudgetFree(budget);}
int xge3dDecodeBudgetInfo(xge3d_decode_budget *budget,xge3d_decode_budget_info_t *out)
{
    if(!out||out->size!=sizeof(*out))return XGE_ERROR_INVALID_ARGUMENT;
    xallocationbudgetinfo info={.Size=sizeof(info)};
    if(!xrtAllocationBudgetInfo(budget,&info))return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_decode_budget_info_t){sizeof(*out),info.Limit,info.LiveBytes,info.PeakBytes,info.LiveCount,info.Rejections};return XGE_OK;
}
int xge3dDecodeBudgetSetLimit(xge3d_decode_budget *budget,uint64_t bytes)
{return bytes&&bytes<=SIZE_MAX&&xrtAllocationBudgetSetLimit(budget,(size_t)bytes)?XGE_OK:XGE_ERROR_INVALID_ARGUMENT;}
#endif
