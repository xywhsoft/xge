/* Inject native-origin IME events into the production queues without a TIP. */
#include "single/xge.c"

#undef NDEBUG
#include <assert.h>

static int32 check_other_thread(void* unused)
{
    (void)unused;
    xge_ime_event_info_t info={.iType=-1};
    char output='?';
    assert(xgeImeEventRead(&info,&output,1)==XGE_ERROR_INVALID_STATE);
    assert(info.iType==-1 && output=='?');
    return 0;
}

int main(void)
{
    xge_ime_event_info_t info={.iType=-1};
    char output[8]={'?','?','?','?','?','?','?','?'};
    assert(xgeImeEventRead(&info,output,sizeof(output))==XGE_ERROR_NOT_INITIALIZED);
    assert(info.iType==-1 && output[0]=='?');
    xge_desc_t desc={.iRunMode=XGE_RUN_MANUAL};
    assert(xgeInit(&desc)==XGE_OK);

    assert(__xgeImeQueuePush(XGE_EVENT_IME_START,"old",3,-1,-1,-1));
    xge_ime_event_t legacy;
    assert(xgeImeEventGet(&legacy)==1 && legacy.iTextSize==3);
    assert(xgeInputEventPendingCount()==1);
    __xgeInputEventQueueReset();

    // Synthetic events of the same kind must not consume a native IME entry.
    xge_input_event_info_t synthetic={.iType=XGE_EVENT_IME_UPDATE,
        .iFlags=XGE_INPUT_EVENT_FLAG_NATIVE_IME};
    assert(xgeInputEventPostData(&synthetic,"local",5)==XGE_OK);
    const char text[]={'A','\0',(char)0xc3,(char)0xa9};
    assert(__xgeImeQueuePushOwnedRangeAttribute(XGE_EVENT_IME_UPDATE,text,sizeof(text),
        4,1,4,1,32,35,XGE_IME_ATTRIBUTE_INPUT_ERROR,17));
    xge_input_event_info_t ordered;
    assert(xgeInputEventRead(&ordered,output,sizeof(output))==XGE_INPUT_READ_READY);
    assert((ordered.iFlags & XGE_INPUT_EVENT_FLAG_SYNTHETIC)!=0 && memcmp(output,"local",5)==0);
    memset(output,'?',sizeof(output));

    xthread* worker=xrtThreadCreate(check_other_thread,NULL,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(xgeImeEventRead(NULL,output,sizeof(output))==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImeEventRead(&info,NULL,1)==XGE_ERROR_INVALID_ARGUMENT && info.iType==-1);

    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeImeEventRead(&info,output,3)==XGE_INPUT_READ_NEED_BUFFER);
    assert(info.iSize==sizeof(info) && info.iType==XGE_EVENT_IME_UPDATE);
    assert(info.iTextSize==sizeof(text) && info.iCursor==4 && info.iSelectStart==1 && info.iSelectEnd==4);
    assert(info.bReplacementRange==1 && info.iReplacementStart==32 && info.iReplacementEnd==35);
    assert(info.iAttribute==XGE_IME_ATTRIBUTE_INPUT_ERROR && output[0]=='?');
    uint64_t sequence=info.iSequence;
    assert(xgeImeEventRead(&info,output,sizeof(output))==XGE_INPUT_READ_READY);
    assert(info.iSequence==sequence && memcmp(output,text,sizeof(text))==0 && output[sizeof(text)]=='?');
    assert(memcmp(legacy.sText,"old",4)==0);
    assert(xgeImeEventRead(&info,NULL,0)==XGE_INPUT_READ_EMPTY);
    assert(info.iSize==sizeof(info) && info.iType==0 && info.iTextSize==0);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(after.AllocCount==before.AllocCount && after.ReallocCount==before.ReallocCount);
    assert(after.FreeCount==before.FreeCount+1);
    assert(xgeInputEventRead(&ordered,output,sizeof(output))==XGE_INPUT_READ_READY);
    assert(ordered.iSequence==sequence && xgeInternalInputImeOwner()==17);

    // A native event left in the compatibility queue across a frame cannot
    // hide the exact match for a later event, even as sequences wrap.
    g_xge.iInputEventSequence=UINT64_MAX-1;
    assert(__xgeImeQueuePush(XGE_EVENT_IME_UPDATE,"first",5,-1,-1,-1));
    __xgeInputEventQueueReset();
    assert(__xgeImeQueuePush(XGE_EVENT_IME_UPDATE,"second",6,-1,-1,-1));
    assert(__xgeImeQueuePush(XGE_EVENT_IME_END,"",0,0,0,0));
    assert(xgeInputEventRead(&ordered,output,sizeof(output))==XGE_INPUT_READ_READY);
    assert(ordered.iSequence==0 && memcmp(output,"second",6)==0);
    assert(xgeImeEventRead(&info,output,sizeof(output))==XGE_INPUT_READ_READY);
    assert(info.iSequence==UINT64_MAX && memcmp(output,"first",5)==0);
    assert(xgeImeEventRead(&info,NULL,0)==XGE_INPUT_READ_READY);
    assert(info.iSequence==1 && info.iType==XGE_EVENT_IME_END && info.iTextSize==0);
    assert(xgeImeEventGet(&legacy)==0);
    assert(xgeUnitChecked()==XGE_OK);
    puts("XGE_COUNTED_IME_NATIVE_OK");
    return 0;
}
