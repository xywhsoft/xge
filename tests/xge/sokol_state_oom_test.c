/* Real Sokol startup allocator; no replacement backend/window implementation. */
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE
#include "extlibs/sokol/sokol_app.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int allowance=-1;
static unsigned allocated,released,failed,panics,inits,frames,cleanups;
static void* allocate(size_t size,void* user)
{
    (void)user;
    if(allowance==0){++failed;return NULL;}
    if(allowance>0)--allowance;
    void* result=malloc(size);assert(result);++allocated;return result;
}
static void release(void* data,void* user)
{(void)user;if(data){++released;free(data);}}
static void log_message(const char* tag,uint32_t level,uint32_t item,
    const char* message,uint32_t line,const char* file,void* user)
{
    (void)tag;(void)message;(void)line;(void)file;(void)user;
    if(level==0){assert(item==SAPP_LOGITEM_MALLOC_FAILED);++panics;}
}
static void initialize(void){++inits;}
static void frame(void){++frames;sapp_quit();}
static void cleanup(void){++cleanups;}
int main(int argc,char** argv)
{
    assert(argc==1 || (argc==2 && !strcmp(argv[1],"recover")));
    sapp_desc desc={0};desc.width=128;desc.height=96;desc.window_title="Startup allocator recovery";
    desc.enable_clipboard=true;desc.clipboard_size=32;
    desc.enable_dragndrop=true;desc.max_dropped_files=2;desc.max_dropped_file_path_length=16;
    desc.allocator.alloc_fn=allocate;desc.allocator.free_fn=release;desc.logger.func=log_message;
    desc.init_cb=initialize;desc.frame_cb=frame;desc.cleanup_cb=cleanup;
    for(unsigned cycle=0;cycle<4;++cycle)for(int budget=0;budget<2;++budget){
        unsigned before=allocated;allowance=budget;
        sapp_run(&desc);allowance=-1;
        assert(!sapp_isvalid() && !inits && !frames && !cleanups);
        assert(allocated==before+(unsigned)budget && allocated==released);
        assert(panics==cycle*2+(unsigned)budget+1 && failed==panics);
    }
#ifdef XGE_TEST_STATE_OOM_FIXED
    /* The native drop buffer uses an int extent: reject before multiplication. */
    unsigned before=allocated;desc.max_dropped_files=INT_MAX;desc.max_dropped_file_path_length=2;
    sapp_run(&desc);
    assert(!sapp_isvalid() && !inits && !frames && !cleanups);
    assert(panics==9 && failed==8 && allocated==before+1 && allocated==released);
    desc.max_dropped_files=2;desc.max_dropped_file_path_length=16;
#endif
    if(argc==2){
        sapp_run(&desc);
        assert(inits==1 && frames==1 && cleanups==1 && !sapp_isvalid());
        assert(allocated==released);
    }
    printf("Sokol state OOM: 8 allocation failures, balanced storage, recovery=%u\n",frames);
    return 0;
}
