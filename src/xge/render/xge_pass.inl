#include "xge_pass_state.h"
bool xgePassIsIdle(void){return __xgePassStateIdle(&g_xgePassState);}
bool xgePassIsCurrent(const xge_pass_t* pass){return __xgePassStateCurrent(&g_xgePassState,pass);}
void xgePassInit(xge_pass pass,xge_render_target target,uint32_t flags,uint32_t color)
{
    if(!pass)return;
    if(!__xgePassStateMayInit(&g_xgePassState,pass)){xrtSetErrorKind(XERR_STATE);return;}
    memset(pass,0,sizeof(*pass));pass->pTarget=target;pass->iClearFlags=flags;pass->iClearColor=color;
}
int xgePassBegin(xge_pass pass)
{
    if(!pass || !pass->pTarget)return XGE_ERROR_INVALID_ARGUMENT;
    if(pass->bActive)return XGE_ERROR_ALREADY_INITIALIZED;
    if(pass->pTarget->_iActivePasses==INT32_MAX)return XGE_ERROR_BUSY;
    if(!__xgePassStateReserve(&g_xgePassState,pass))return XGE_ERROR_INVALID_STATE;
    ++pass->pTarget->_iActivePasses;
    xerror* previous=xrtTakeError();xerror* first=NULL;int status=XGE_OK;
    if(xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE))status=XGE_ERROR_INVALID_STATE;
    else if(!g_xge.bInitialized || !g_xge.bSokolRunning)status=XGE_ERROR_NOT_INITIALIZED;
    if(status==XGE_OK)__xgeCallbackOutcome(xgeFlush(),&status,&first);
    if(status==XGE_OK)__xgeCallbackOutcome(__xgeRenderTargetEnsureFramebuffer(pass->pTarget),&status,&first);
    if(status!=XGE_OK){
        --pass->pTarget->_iActivePasses;
        __xgePassStateAbort(&g_xgePassState,pass);xrtErrorFree(previous);xrtSetErrorTake(first);return status;
    }
    xge_render_target target=pass->pTarget;
    pass->iPrevWidth=g_xge.iWidth;pass->iPrevHeight=g_xge.iHeight;pass->iPrevFramebufferId=g_xge.iCurrentFramebufferId;
    pass->tPrevCamera=g_xge.tCamera;pass->bPrevViewportEnabled=g_xge.bViewportEnabled;pass->tPrevViewportRect=g_xge.tViewportRect;
    pass->bPrevClipEnabled=g_xge.bClipEnabled;pass->tPrevClipRect=g_xge.tClipRect;
    uint32_t framebuffer=(target->iFlags&XGE_RENDER_TARGET_WINDOW) ? 0:target->iFramebufferId;
    xge_glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)framebuffer);g_xge.iCurrentFramebufferId=framebuffer;
    g_xge.iWidth=target->iWidth;g_xge.iHeight=target->iHeight;
#if XGE_ENABLE_2D
    g_xge.tCamera=xgeCameraDefault((float)target->iWidth,(float)target->iHeight);
#endif
    g_xge.bViewportEnabled=0;memset(&g_xge.tViewportRect,0,sizeof(g_xge.tViewportRect));
    g_xge.bClipEnabled=0;memset(&g_xge.tClipRect,0,sizeof(g_xge.tClipRect));
    xge_glViewport(0,0,target->iWidth,target->iHeight);
    if(xge_glDisable)xge_glDisable(GL_SCISSOR_TEST);
    if(pass->iClearFlags&XGE_PASS_CLEAR_COLOR)xgeClear(pass->iClearColor);
    __xgeCallbackOutcome(XGE_OK,&status,&first);
    __xgePassStateActivate(&g_xgePassState,pass);
    if(status!=XGE_OK){
        /* A late native callback failed after the target switched. Consume
         * rollback exactly once before returning a failed Begin. */
        __xgeCallbackOutcome(xgePassEnd(pass),&status,&first);xrtErrorFree(previous);xrtSetErrorTake(first);return status;
    }
    xrtSetErrorTake(previous);return XGE_OK;
}
int xgePassEnd(xge_pass pass)
{
    if(!pass)return XGE_ERROR_INVALID_ARGUMENT;
    /* No Flush, state publication or error mutation may precede this check. */
    if(!__xgePassStateBeginEnd(&g_xgePassState,pass))return XGE_ERROR_INVALID_STATE;
    xerror* previous=xrtTakeError();xerror* first=NULL;int status=XGE_OK;
    if(g_xge.bSokolRunning)__xgeCallbackOutcome(xgeFlush(),&status,&first);
    else status=XGE_ERROR_NOT_INITIALIZED;
    if(g_xge.bSokolRunning && xge_glBindFramebuffer)xge_glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)pass->iPrevFramebufferId);
    g_xge.iCurrentFramebufferId=pass->iPrevFramebufferId;g_xge.iWidth=pass->iPrevWidth;g_xge.iHeight=pass->iPrevHeight;
    g_xge.tCamera=pass->tPrevCamera;g_xge.bViewportEnabled=pass->bPrevViewportEnabled;g_xge.tViewportRect=pass->tPrevViewportRect;
    g_xge.bClipEnabled=pass->bPrevClipEnabled;g_xge.tClipRect=pass->tPrevClipRect;
    __xgeCallbackOutcome(XGE_OK,&status,&first);
    __xgeViewportApplyCurrent();__xgeCallbackOutcome(XGE_OK,&status,&first);
    __xgeClipApplyCurrent();__xgeCallbackOutcome(XGE_OK,&status,&first);
    __xgePassStateFinishEnd(&g_xgePassState,pass);
    --pass->pTarget->_iActivePasses;
    if(status!=XGE_OK){xrtErrorFree(previous);xrtSetErrorTake(first);}else xrtSetErrorTake(previous);
    return status;
}
int xgePassCreateOwned(xge_pass* out,xge_render_target target,uint32_t flags,uint32_t color)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(xgeRenderTargetAddRef(target)<=0)return XGE_ERROR_INVALID_STATE;
    xge_pass pass=xrtCalloc(1,sizeof(*pass));
    if(!pass){xgeRenderTargetFree(target);return XGE_ERROR_OUT_OF_MEMORY;}
    xgePassInit(pass,target,flags,color);pass->_bOwnsRecord=true;*out=pass;return XGE_OK;
}
int xgePassReleaseOwnerChecked(xge_pass pass,bool* consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!pass || !pass->_bOwnsRecord)return XGE_ERROR_INVALID_ARGUMENT;
    if(pass->bActive && !xgePassIsCurrent(pass))return XGE_ERROR_BUSY;
    xerror* previous=xrtTakeError();xerror* first=NULL;int result=XGE_OK;
    if(pass->bActive)__xgeCallbackOutcome(xgePassEnd(pass),&result,&first);
    if(!pass->bActive){
        bool targetConsumed=false;
        __xgeCallbackOutcome(xgeRenderTargetReleaseOwnerChecked(pass->pTarget,&targetConsumed),&result,&first);
        if(targetConsumed){xrtFree(pass);*consumed=true;__xgeCallbackOutcome(XGE_OK,&result,&first);}
    }
    if(result!=XGE_OK){xrtErrorFree(previous);xrtSetErrorTake(first);}else xrtSetErrorTake(previous);
    return result;
}
void xgePassFree(xge_pass pass)
{
    if(!pass)return;
    bool consumed=false;
    if(xgePassReleaseOwnerChecked(pass,&consumed)!=XGE_OK && !xrtGetError())xrtSetErrorKind(XERR_STATE);
}
