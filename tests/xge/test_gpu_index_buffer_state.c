#include "single/xge.c"

#include <stdio.h>

#if defined(_WIN32) || defined(XGE_HAS_EGL)
enum { COPY_READ_BUFFER=0x8F36, MAP_READ_BIT=0x0001, ELEMENT_ARRAY_BUFFER_BINDING=0x8895 };
typedef void* (APIENTRYP MapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield);
typedef GLboolean (APIENTRYP UnmapBuffer)(GLenum);

#define CHECK(condition) do { if(!(condition)) { \
    fprintf(stderr,"GPU index buffer line %d: %s\n",__LINE__,#condition); \
    return 1; \
} } while(0)

int main(void)
{
    const xge_desc_t desc={.iRunMode=XGE_RUN_MANUAL,.iWidth=4,.iHeight=4};
    xge_graphics_context context=NULL;
    CHECK(xgeInit(&desc)==XGE_OK);
    CHECK(xgeGraphicsContextCreateOffscreenOwned(&context,4,4,NULL)==XGE_OK);

    xge_glBindVertexArray(0);
    CHECK(xge_glGetError()==GL_NO_ERROR);
    xge_buffer index=NULL;
    CHECK(xgeBufferCreateOwned(&index,XGE_BUFFER_INDEX,XGE_BUFFER_DYNAMIC,NULL,16)==XGE_OK);
    GLenum error=xge_glGetError();
    if(error!=GL_NO_ERROR)fprintf(stderr,"GL error after owned index creation: 0x%x\n",error);
    CHECK(error==GL_NO_ERROR && index->iBackendId!=0);

    GLuint vao=0,marker=0;
    xge_glGenVertexArrays(1,&vao);
    xge_glGenBuffers(1,&marker);
    xge_glBindVertexArray(vao);
    xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,marker);
    CHECK(xge_glGetError()==GL_NO_ERROR);
    CHECK(xgeBufferUpload(index)==XGE_OK);
    GLint bound=0;
    xge_glGetIntegerv(ELEMENT_ARRAY_BUFFER_BINDING,&bound);
    CHECK((GLuint)bound==marker && xge_glGetError()==GL_NO_ERROR);
    unsigned char patch=0x6c;
    CHECK(xgeBufferUpdate(index,15,&patch,1)==XGE_OK);
    xge_glGetIntegerv(ELEMENT_ARRAY_BUFFER_BINDING,&bound);
    CHECK((GLuint)bound==marker && xge_glGetError()==GL_NO_ERROR);

#if defined(_WIN32)
    MapBufferRange map=(MapBufferRange)__xgeGLSystemProc("glMapBufferRange",&context->wgl.library);
    UnmapBuffer unmap=(UnmapBuffer)__xgeGLSystemProc("glUnmapBuffer",&context->wgl.library);
#elif defined(XGE_HAS_EGL)
    MapBufferRange map=(MapBufferRange)__xgeContextEglLoad("glMapBufferRange",NULL);
    UnmapBuffer unmap=(UnmapBuffer)__xgeContextEglLoad("glUnmapBuffer",NULL);
#endif
    CHECK(map && unmap);
    xge_glBindBuffer(COPY_READ_BUFFER,index->iBackendId);
    const unsigned char* bytes=map(COPY_READ_BUFFER,0,16,MAP_READ_BIT);
    CHECK(bytes);
    for(int i=0;i<15;++i)CHECK(bytes[i]==0);
    CHECK(bytes[15]==patch);
    CHECK(unmap(COPY_READ_BUFFER)==GL_TRUE);
    xge_glBindBuffer(COPY_READ_BUFFER,0);

    xge_glBindVertexArray(0);
    xge_glDeleteVertexArrays(1,&vao);
    xge_glDeleteBuffers(1,&marker);
    CHECK(xgeUnitChecked()==XGE_ERROR_BUSY);
    xgeBufferFree(index);
    CHECK(xgeGraphicsContextReleaseChecked(context)==XGE_OK);
    CHECK(xgeUnitChecked()==XGE_OK && xge_glGetError==NULL);
    puts("XGE_GPU_INDEX_STATE_OK");
    return 0;
}
#else
int main(void)
{
    puts("XGE_GPU_INDEX_CONTEXT_REQUIRED");
    return 0;
}
#endif
