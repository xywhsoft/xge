/* Exercise the actual vendored X11 startup, including its allocator.
 * No replacement window/GL implementation and no XRT or language bridge. */
#define _POSIX_C_SOURCE 200809L
#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE
#include "extlibs/sokol/sokol_app.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned allocations,frees,panics,inits,frames,cleanups;
static void* allocate(size_t size,void* user)
{(void)user;void* p=malloc(size);assert(p);++allocations;return p;}
static void release(void* p,void* user)
{(void)user;if(p){++frees;free(p);}}
static void log_message(const char* tag,uint32_t level,uint32_t item,
    const char* message,uint32_t line,const char* file,void* user)
{
    (void)tag;(void)message;(void)line;(void)file;(void)user;
    if(level==0){assert(item==SAPP_LOGITEM_LINUX_X11_OPEN_DISPLAY_FAILED);++panics;}
}
static void initialize(void){++inits;}
static void frame(void){++frames;sapp_quit();}
static void cleanup(void){++cleanups;}
int main(int argc,char** argv)
{
    assert(argc==1 || (argc==2 && !strcmp(argv[1],"recover")));
    const char* current=getenv("DISPLAY");char* saved=current?strdup(current):NULL;
    sapp_desc desc={0};desc.width=128;desc.height=96;
    desc.window_title="Sokol X11 startup lifetime";
    desc.enable_clipboard=true;desc.enable_dragndrop=true;
    desc.allocator.alloc_fn=allocate;desc.allocator.free_fn=release;
    desc.logger.func=log_message;desc.init_cb=initialize;
    desc.frame_cb=frame;desc.cleanup_cb=cleanup;
    for(unsigned i=0;i<12;++i){
        /* No server connection for unset/empty or syntactically invalid names. */
        if(i%3==0)assert(unsetenv("DISPLAY")==0);
        else assert(setenv("DISPLAY",i%3==1?"":"not-a-valid-display-name",1)==0);
        unsigned before=allocations;
        sapp_run(&desc);
        assert(!sapp_isvalid() && !inits && !frames && !cleanups);
        assert(panics==i+1 && allocations==before+2 && allocations==frees);
    }
    if(saved)assert(setenv("DISPLAY",saved,1)==0);else assert(unsetenv("DISPLAY")==0);
    free(saved);
    if(argc==2){
        assert(getenv("DISPLAY") && *getenv("DISPLAY"));
        sapp_run(&desc);
        assert(inits==1 && frames==1 && cleanups==1 && panics==12);
        assert(allocations==frees && !sapp_isvalid());
    }
    printf("Sokol X11: 12 startup failures, %u allocations/%u frees, recovery=%u\n",allocations,frees,frames);
    return 0;
}
