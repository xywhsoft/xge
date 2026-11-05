#ifndef XUI_CALLBACK_HOLD_INTERNAL_H
#define XUI_CALLBACK_HOLD_INTERNAL_H
#include "xui.h"

/* The owner record itself has no Widget/Context dependency. Pure Native
 * consumers (including the standalone Document model) share this exact raw
 * acquisition rule without linking UI event/layout implementation code. */
static inline bool xuiInternalCallbackOwnerRawValid(const xui_callback_owner_t* owner)
{
    return !owner ||
        ((!owner->tReference.Data && !owner->tReference.Ops && !owner->onHold && !owner->onDrop) ||
         (owner->tReference.Data && owner->tReference.Ops && owner->onHold && owner->onDrop));
}
static inline int xuiInternalCallbackOwnerRawHold(const xui_callback_owner_t* owner)
{
    if(!xuiInternalCallbackOwnerRawValid(owner))return XUI_ERROR_INVALID_ARGUMENT;
    return !owner || !owner->onHold || owner->onHold(owner->tReference.Data) ? XUI_OK:XUI_ERROR_CALLBACK_FAILED;
}

/* Raw Hold's bool remains the ownership fact. A fresh diagnostic rejects
 * business publication but does not undo the acquired reference. Callers
 * must Drop exactly once when held is true, even on checked failure. */
static inline int xuiInternalCallbackHoldFresh(const xui_callback_owner_t* owner,bool* held)
{
    int status=xuiInternalCallbackOwnerRawHold(owner);
    *held=status==XUI_OK && owner && owner->onHold;
    if(*held && xrtGetError()!=NULL)status=XUI_ERROR_CALLBACK_FAILED;
    return status;
}
/* Dispatch can already carry a business diagnostic (e.g. onError). Isolate
 * it from the Hold transaction: it cannot suppress a legitimate callback.
 * Restore the older diagnostic after selecting status from the fresh one. */
static inline int xuiInternalCallbackHoldIsolated(const xui_callback_owner_t* owner,bool* held)
{
    *held=false;
    if(!xuiInternalCallbackOwnerRawValid(owner))return XUI_ERROR_INVALID_ARGUMENT;
    if(!owner || !owner->onHold)return XUI_OK;
    xerror* ambient=xrtTakeError();
    int status=xuiInternalCallbackOwnerRawHold(owner);
    *held=status==XUI_OK && owner && owner->onHold;
    xerror* fresh=xrtTakeError();
    if(status==XUI_OK && fresh)status=XUI_ERROR_CALLBACK_FAILED;
    if(ambient){xrtErrorFree(fresh);xrtSetErrorTake(ambient);}
    else xrtSetErrorTake(fresh);
    return status;
}
#endif
