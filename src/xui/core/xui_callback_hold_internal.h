#ifndef XUI_CALLBACK_HOLD_INTERNAL_H
#define XUI_CALLBACK_HOLD_INTERNAL_H
#include "xui_internal.h"

/* Raw Hold's bool remains the ownership fact. A fresh diagnostic rejects
 * business publication but does not undo the acquired reference. Callers
 * must Drop exactly once when held is true, even on checked failure. */
static inline int xuiInternalCallbackHoldFresh(const xui_callback_owner_t* owner,bool* held)
{
    int status=xuiInternalCallbackOwnerHold(owner);
    *held=status==XUI_OK && owner && owner->onHold;
    if(status==XUI_OK && xrtGetError()!=NULL)status=XUI_ERROR_CALLBACK_FAILED;
    return status;
}
/* Dispatch can already carry a business diagnostic (e.g. onError). Isolate
 * it from the Hold transaction: it cannot suppress a legitimate callback.
 * Restore the older diagnostic after selecting status from the fresh one. */
static inline int xuiInternalCallbackHoldIsolated(const xui_callback_owner_t* owner,bool* held)
{
    xerror* ambient=xrtTakeError();
    int status=xuiInternalCallbackOwnerHold(owner);
    *held=status==XUI_OK && owner && owner->onHold;
    xerror* fresh=xrtTakeError();
    if(status==XUI_OK && fresh)status=XUI_ERROR_CALLBACK_FAILED;
    if(ambient){xrtErrorFree(fresh);xrtSetErrorTake(ambient);}
    else xrtSetErrorTake(fresh);
    return status;
}
#endif
