/* Private GL function-table transaction. Fake addresses are never invoked. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define XGE_GL_IMPL
#include "xge_gl.h"
typedef struct Source {const char* missing;unsigned calls;int alternate,allOptional;} Source;
static volatile int marker;
static void first(void){marker=1;}
static void second(void){marker=2;}
static void* resolve(const char* name,void* raw)
{
    Source* source=raw;++source->calls;
    if(source->missing && (!strcmp(name,source->missing) ||
        (!strcmp(source->missing,"depth-pair") &&
         (!strcmp(name,"glClearDepth") || !strcmp(name,"glClearDepthf")))))return NULL;
    if(source->allOptional && (!strcmp(name,"glGenQueries") || !strcmp(name,"glDeleteQueries") ||
        !strcmp(name,"glQueryCounter") || !strcmp(name,"glGetQueryObjectiv") || !strcmp(name,"glGetQueryObjectui64v")))return NULL;
    return source->alternate ? (void*)second:(void*)first;
}
static void assertFirst(void)
{
#define XGE_GL_SLOT(Type,Name,Required) assert(xge_##Name==(Type)first);
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
}
static void* legacy(const char* name){(void)name;return (void*)first;}
int main(void)
{
    Source source={0};unsigned rejected=0;
    assert(xge_gl_load_ex(resolve,&source)==1 && source.calls>50);assertFirst();
    assert(xge_gl_load_ex(NULL,&source)==0);assertFirst();
    /* Every required slot must reject its omission without partial publication. */
#define XGE_GL_SLOT(Type,Name,Required) if(Required){source=(Source){#Name,0,1,0};assert(!xge_gl_load_ex(resolve,&source));assertFirst();++rejected;}
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
#if XGE_ENABLE_3D
    source=(Source){"depth-pair",0,1,0};assert(!xge_gl_load_ex(resolve,&source));assertFirst();
    source=(Source){"glClearDepth",0,0,0};assert(xge_gl_load_ex(resolve,&source));
    source=(Source){"glClearDepthf",0,0,0};assert(xge_gl_load_ex(resolve,&source));
    source=(Source){NULL,0,0,1};assert(xge_gl_load_ex(resolve,&source));
    assert(!xge_glQueryCounter && !xge_glGenQueries);
#endif
    assert(xge_gl_load(legacy));assertFirst();assert(!xge_gl_load(NULL));assertFirst();
    xge_gl_unload();
#define XGE_GL_SLOT(Type,Name,Required) assert(!xge_##Name);
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
    assert(marker==0);printf("Typed GL transaction: %u required slots rejected atomically; unload and legacy loader passed\n",rejected);return 0;
}
