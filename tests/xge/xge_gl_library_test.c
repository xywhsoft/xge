/* Private library/table transaction. Fake function addresses are NEVER called;
 * only the real typed table loader's publication/rollback is inspected. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define XGE_OK 0
#define XGE_ERROR_BACKEND_FAILED -8
#define XGE_GL_IMPL
#include "xge_gl.h"
#include "xge_gl_library.h"
typedef struct Library {int references,backend;} Library;
static Library libraries[4];
static unsigned opens,closes,lookups;
static int failOpen,failClose;
static const char* missing;
static volatile int invoked;
static void first(void){++invoked;}
static void second(void){++invoked;}
static int __xgeGLSystemOpen(xge_gl_library* library)
{
    if(failOpen)return XGE_ERROR_BACKEND_FAILED;
    for(unsigned i=0;i<4;++i)if(!libraries[i].references){
        libraries[i].references=1;libraries[i].backend=library->backend;
        library->handle=&libraries[i];++opens;return XGE_OK;
    }
    assert(!"unbounded library ownership");return XGE_ERROR_BACKEND_FAILED;
}
static int __xgeGLSystemClose(xge_gl_library* library)
{
    Library* real=(Library*)library->handle;
    assert(real && real->references==1 && real->backend==library->backend);
    if(failClose)return XGE_ERROR_BACKEND_FAILED;
    --real->references;++closes;return XGE_OK;
}
static void* __xgeGLSystemProc(const char* name,void* raw)
{
    xge_gl_library* library=(xge_gl_library*)raw;
    Library* real=(Library*)library->handle;
    assert(real && real->references==1 && real->backend==library->backend);++lookups;
    if(missing && (!strcmp(name,missing) ||
       (!strcmp(missing,"depth-pair") && (!strcmp(name,"glClearDepth") || !strcmp(name,"glClearDepthf")))))return NULL;
    return (void*)(library->backend==1 ? first:second);
}
static void assertFirst(void)
{
#define XGE_GL_SLOT(Type,Name,Required) assert(xge_##Name==(Type)first);
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
}
static void assertSecond(void)
{
#define XGE_GL_SLOT(Type,Name,Required) assert(xge_##Name==(Type)second);
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
}
static void assertEmpty(void)
{
#define XGE_GL_SLOT(Type,Name,Required) assert(!xge_##Name);
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
}
int main(void)
{
    assert(__xgeGraphicsLoadGL(1)==XGE_OK);assertFirst();assert(opens==1 && closes==0);
    assert(__xgeGraphicsLoadGL(1)==XGE_OK);assertFirst();assert(opens==1);
    failOpen=1;assert(__xgeGraphicsLoadGL(2)==XGE_ERROR_BACKEND_FAILED);assertFirst();failOpen=0;
    unsigned required=0;
#define XGE_GL_SLOT(Type,Name,Required) if(Required){missing=#Name;assert(__xgeGraphicsLoadGL(2)==XGE_ERROR_BACKEND_FAILED);assertFirst();assert(opens==closes+1);++required;}
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
#if XGE_ENABLE_3D
    missing="depth-pair";assert(__xgeGraphicsLoadGL(2)==XGE_ERROR_BACKEND_FAILED);assertFirst();
#endif
    missing="glBindFramebuffer";failClose=1;
    assert(__xgeGraphicsLoadGL(2)==XGE_ERROR_BACKEND_FAILED);assertFirst();assert(g_xgeGLRetiredLibrary.handle);
    unsigned retainedOpens=opens,retainedLookups=lookups;
    for(int i=0;i<3;++i)assert(__xgeGraphicsLoadGL(2)==XGE_ERROR_BACKEND_FAILED);
    assert(opens==retainedOpens && lookups==retainedLookups);assertFirst();
    /* Retire also tries the independent active owner. Both failing closes
     * retain their exact references, while every published slot is cleared. */
    assert(__xgeGraphicsLibraryRetire()==XGE_ERROR_BACKEND_FAILED);assertEmpty();
    assert(g_xgeGLLibrary.handle && g_xgeGLRetiredLibrary.handle && opens==closes+2);
    failClose=0;assert(__xgeGraphicsLibraryRetire()==XGE_OK);assert(opens==closes);
    missing=NULL;assert(__xgeGraphicsLoadGL(1)==XGE_OK);assertFirst();
    failClose=1;assert(__xgeGraphicsLoadGL(2)==XGE_ERROR_BACKEND_FAILED);assertSecond();
    assert(g_xgeGLLibrary.backend==2 && g_xgeGLRetiredLibrary.backend==1 && opens==closes+2);
    failClose=0;assert(__xgeGraphicsLoadGL(2)==XGE_OK);assertSecond();assert(opens==closes+1);
    for(int i=0;i<20;++i){
        assert(__xgeGraphicsLoadGL(i%2+1)==XGE_OK);assert(opens==closes+1);
    }
    assert(__xgeGraphicsLibraryRetire()==XGE_OK);assertEmpty();assert(opens==closes);
    assert(__xgeGraphicsLibraryRetire()==XGE_OK && !invoked);
    printf("GL libraries: %u required omissions, bounded failed-close owners, transactional backend switches and exact retirement passed\n",required);
    return 0;
}
