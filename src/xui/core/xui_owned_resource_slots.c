#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_proxy_resource.h"
#include <stdlib.h>
#include <string.h>

void xuiInternalContextRecordFailure(xui_context context,int status)
{
    if(status==XUI_OK || !context)return;
    for(xui_call_scope_t* scope=context->pCallScopes;scope;scope=scope->pNext)
        if(scope->iResult==XUI_OK)scope->iResult=status;
}
static void outcome(int status,int* result,xerror** first)
{
    xerror* error=xrtTakeError();
    if(status==XUI_OK && !error)return;
    if(*result==XUI_OK){*result=status!=XUI_OK ? status:XUI_ERROR_CALLBACK_FAILED;*first=error;}
    else xrtErrorFree(error);
}
int xuiInternalFontRelease(xui_context context,xui_font font)
{
    int result=xrtGetError() ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
    xerror* first=result!=XUI_OK ? xrtTakeError():NULL;
    xuiInternalContextRecordFailure(context,result);
    if(font)outcome(xuiFontReleaseChecked(font),&result,&first);
    xuiInternalContextRecordFailure(context,result);xrtSetErrorTake(first);return result;
}
static int fontValidate(xui_context context,xui_font font)
{
    if(!context->pOwnedProxy || !font)return XUI_OK;
    xui_proxy proxy=xuiFontGetProxy(font);
    if(!proxy || proxy->_pCapability!=context->pOwnedProxy->_pCapability ||
       !xuiInternalProxyResourceReady(font,XUI_OWNED_FONT))return XUI_ERROR_INVALID_ARGUMENT;
    return XUI_OK;
}
int xuiInternalFontAdmit(xui_context context,xui_font font)
{
    int status=fontValidate(context,font);
    return status!=XUI_OK || !context->pOwnedProxy || !font ? status:xuiFontAddRef(font);
}
int xuiInternalFontSlotPrepare(xui_context context,xui_font* slot,xui_font font,bool admitted,bool preserveOld,xui_font_slot_update_t* update)
{
    if(!context || !slot || !update || update->prepared)return XUI_ERROR_INVALID_ARGUMENT;
    if(!context->pOwnedProxy){
        update->slot=slot;update->current=font;update->prepared=true;return XUI_OK;
    }
    /* Reserve this exact real field before any Hold or arbitrary user code.
     * Different fields/widgets can nest; the same field is a checked BUSY. */
    for(xui_font_slot_update_t* active=context->pActiveFontUpdates;active;active=active->next)
        if(active->slot==slot)return XUI_ERROR_BUSY;
    int status=xuiCallScopeBegin(context,&update->scope);if(status!=XUI_OK)return status;
    update->context=context;update->slot=slot;update->current=font;update->previous=*slot;
    update->preserveOld=preserveOld;update->ownsSlots=admitted;update->prepared=true;
    update->next=context->pActiveFontUpdates;context->pActiveFontUpdates=update;
    status=fontValidate(context,font);
    if(status==XUI_OK && admitted && font){status=xuiFontAddRef(font);update->newHeld=status==XUI_OK;}
    if(status==XUI_OK && admitted && preserveOld && *slot){
        status=xuiInternalFontAdmit(context,*slot);
        if(status==XUI_OK){update->guarded=*slot;update->guardHeld=true;}
    }
    return status==XUI_OK ? status:xuiInternalFontSlotEnd(update,true,status);
}
int xuiInternalFontSlotPublish(xui_font_slot_update_t* update)
{
    if(!update || !update->prepared || update->published)return XUI_ERROR_INVALID_STATE;
    if(update->context){
        if(update->context->bDestroyPending || update->context->bDestroying ||
           (update->widget && !xuiInternalWidgetIsValid(update->widget)) ||
           (update->preserveOld && *update->slot!=update->previous))return XUI_ERROR_INVALID_STATE;
        xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
        update->previous=*update->slot;*update->slot=update->current;update->published=true;
        if(!xrtOwnershipScopeEnd(&mutation))abort();
    }else{update->previous=*update->slot;*update->slot=update->current;update->published=true;}
    return XUI_OK;
}
int xuiInternalFontSlotBegin(xui_context context,xui_font* slot,xui_font font,bool admitted,xui_font_slot_update_t* update)
{
    int status=xuiInternalFontSlotPrepare(context,slot,font,admitted,false,update);
    if(status==XUI_OK)status=xuiInternalFontSlotPublish(update);
    return status==XUI_OK ? status:xuiInternalFontSlotEnd(update,true,status);
}
int xuiInternalFontSlotRestore(xui_font_slot_update_t* update)
{
    if(!update || !update->prepared || !update->published || update->restored)return XUI_ERROR_INVALID_STATE;
    if(*update->slot!=update->current)return XUI_ERROR_INVALID_STATE;
    if(update->context){
        xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
        *update->slot=update->previous;update->restored=true;
        if(!xrtOwnershipScopeEnd(&mutation))abort();
    }else{*update->slot=update->previous;update->restored=true;}
    return XUI_OK;
}
int xuiInternalFontSlotEnd(xui_font_slot_update_t* update,bool commit,int status)
{
    if(!update || !update->slot)return status;
    if(!update->context){
        if(update->published && !commit && !update->restored)(void)xuiInternalFontSlotRestore(update);
        memset(update,0,sizeof(*update));return status;
    }
    xui_context context=update->context;
    if(status==XUI_OK && xrtGetError())status=XUI_ERROR_CALLBACK_FAILED;
    xerror* first=status!=XUI_OK ? xrtTakeError():NULL;
    xuiInternalContextRecordFailure(context,status);
    if(update->published && !commit && !update->restored)outcome(xuiInternalFontSlotRestore(update),&status,&first);
    xui_font retired=!update->published || update->restored ? update->current:update->previous;
    if(update->ownsSlots && retired && (update->published || update->newHeld))outcome(xuiFontReleaseChecked(retired),&status,&first);
    if(update->guardHeld)outcome(xuiFontReleaseChecked(update->guarded),&status,&first);
    xuiInternalContextRecordFailure(context,status);
    if(context->pActiveFontUpdates!=update)abort();
    context->pActiveFontUpdates=update->next;
    outcome(xuiCallScopeEnd(&update->scope),&status,&first);
    memset(update,0,sizeof(*update));xrtSetErrorTake(first);return status;
}
bool xuiInternalContextTraceResources(xui_context context,xrtownershipvisitor visit,void* user)
{
    if(!context->pOwnedProxy)return true;
    if(!visit(xuiProxyOwnership(context->pOwnedProxy),user))return false;
    if(context->pDefaultFont && !visit(xuiFontOwnership(context->pDefaultFont),user))return false;
    for(int i=0;i<context->iFontCount;++i)
        if(context->pFonts[i].pFont && !visit(xuiFontOwnership(context->pFonts[i].pFont),user))return false;
    return true;
}
#endif
