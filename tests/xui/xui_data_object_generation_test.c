#include "xui_owned_model_host_test.h"
#include "xge.h"
#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Keep the two authoritative public callback types exact. The identical
 * scenarios below intentionally do not cast one family's callback to another. */
#define DATA_TEST(P,U) \
typedef struct P##_generation_test { \
    P##_data_object object; int action,inRead,freed,releaseOwner,secondary; \
} P##_generation_test; \
static void P##_generation_free(void* user) { \
    P##_generation_test* state=user;assert(!state->inRead);++state->freed; \
    if(state->action==3){ \
        assert(P##DataObjectSet(state->object,"application/test","nested",6u)==U##_OK); \
        for(int i=0;i<40;++i){char name[40];snprintf(name,sizeof(name),"nested/%d",i); \
            assert(P##DataObjectSet(state->object,name,&i,sizeof(i))==U##_OK);} \
    }else if(state->action==4){ \
        assert(P##DataObjectSet(state->object,"invalid","x",1u)==U##_ERROR_INVALID_ARGUMENT); \
        assert(P##DataObjectAddRef(state->object)==U##_ERROR_INVALID_ARGUMENT); \
    } \
    if(state->secondary)xrtSetErrorKind(XERR_STATE); \
} \
static int P##_generation_read(const char* format,void* output,size_t capacity,size_t* size,void* user) { \
    P##_generation_test* state=user;state->inRead=1;assert(strcmp(format,"application/test")==0); \
    if(state->action==1 || state->action==2 || state->action==5){ \
        assert(P##DataObjectSet(state->object,format,"new",3u)==U##_OK); \
        for(int i=0;i<40;++i){char name[40];snprintf(name,sizeof(name),"added/%d",i); \
            assert(P##DataObjectSet(state->object,name,&i,sizeof(i))==U##_OK);} \
        assert(state->freed==0); \
    } \
    if(state->releaseOwner){P##DataObjectRelease(state->object);assert(state->freed==0);} \
    *size=3u;int result=U##_OK; \
    if(output){if(capacity<3u)result=U##_ERROR_BUFFER_TOO_SMALL;else memcpy(output,"old",3u);} \
    if(state->action==5){xrtClearError();result=U##_ERROR_BACKEND_FAILED;} \
    state->inRead=0;return result; \
} \
static void P##_generation_run(void) { \
    for(int action=1;action<=5;++action){ \
        P##_generation_test state={0};state.action=action;state.releaseOwner=action==2;state.secondary=action==5; \
        assert(P##DataObjectCreate(&state.object)==U##_OK); \
        assert(P##DataObjectSetProvider(state.object,"application/test",P##_generation_read,P##_generation_free,&state)==U##_OK); \
        if(action==3){ \
            assert(P##DataObjectSet(state.object,"application/test","outer",5u)==U##_OK); \
            assert(state.freed==1);char value[10]={0};size_t size=0; \
            assert(P##DataObjectGet(state.object,"application/test",value,sizeof(value),&size)==U##_OK); \
            assert(size==6u && memcmp(value,"nested",6u)==0); \
        }else if(action==4){ \
            P##DataObjectRelease(state.object);assert(state.freed==1);continue; \
        }else { \
            char value[8]={0};size_t size=0;int status=P##DataObjectGet(state.object,"application/test",value,sizeof(value),&size); \
            assert(status==(action==5 ? U##_ERROR_BACKEND_FAILED:U##_OK)); \
            assert(size==3u && memcmp(value,"old",3u)==0 && state.freed==1); \
            if(action==5)assert(xrtGetError()==NULL); \
        } \
        if(!state.releaseOwner)P##DataObjectRelease(state.object); \
    } \
    P##_data_object object=NULL;assert(P##DataObjectCreate(&object)==U##_OK); \
    const unsigned char binary[]={0,255,0,128};assert(P##DataObjectSet(object,"binary",binary,sizeof(binary))==U##_OK); \
    size_t size=0;assert(P##DataObjectGet(object,"binary",NULL,0,&size)==U##_OK && size==sizeof(binary)); \
    unsigned char copy[4]={0};assert(P##DataObjectGet(object,"binary",copy,2u,&size)==U##_ERROR_BUFFER_TOO_SMALL); \
    assert(copy[0]==0 && copy[1]==0); \
    assert(P##DataObjectGet(object,"binary",copy,sizeof(copy),&size)==U##_OK && memcmp(copy,binary,sizeof(copy))==0); \
    for(int budget=0;budget<4;++budget){ \
        assert(xrtMemDebugFailAfter(budget));int status=P##DataObjectSet(object,"binary","replacement",11u); \
        assert(xrtMemDebugFailAfter(-1));xrtClearError(); \
        if(status==U##_OK)break; \
        assert(status==U##_ERROR_OUT_OF_MEMORY); \
        assert(P##DataObjectGet(object,"binary",copy,sizeof(copy),&size)==U##_OK && size==sizeof(binary) && memcmp(copy,binary,sizeof(copy))==0); \
    } \
    for(int budget=0;budget<4;++budget){ \
        P##_generation_test state={0};state.object=object; \
        assert(xrtMemDebugFailAfter(budget));int status=P##DataObjectSetProvider(object,"application/test",P##_generation_read,P##_generation_free,&state); \
        assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(state.freed==0); \
        if(status==U##_OK){assert(P##DataObjectSet(object,"application/test","done",4u)==U##_OK && state.freed==1);break;} \
        assert(status==U##_ERROR_OUT_OF_MEMORY && !P##DataObjectHas(object,"application/test")); \
    } \
    P##DataObjectRelease(object); \
}

DATA_TEST(xge,XGE)
DATA_TEST(xui,XUI)
#undef DATA_TEST

int main(void)
{
    modelHostBind();xrtClearError();
    xge_generation_run();xui_generation_run();
    puts("DataObject exact typed stable generation/reentry/binary tests passed");
    return 0;
}
