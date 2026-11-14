#ifndef XGE_GL_LIBRARY_H
#define XGE_GL_LIBRARY_H
/* Private cold-path owner of the committed GL function table. The engine
 * serializes table replacement against drawing and joins render workers before
 * retirement. A failed OS close keeps its exact reference for checked retry.
 * No allocation, cache lookup or owner operation is added to a draw call. */
typedef struct xge_gl_library {
    void* handle;
    void* entry;
    int backend;
} xge_gl_library;
static int __xgeGLSystemOpen(xge_gl_library* library);
static int __xgeGLSystemClose(xge_gl_library* library);
static void* __xgeGLSystemProc(const char* name,void* library);
static xge_gl_library g_xgeGLLibrary, g_xgeGLRetiredLibrary;

static int __xgeGLLibraryClose(xge_gl_library* library)
{
    if(library->handle && __xgeGLSystemClose(library)!=XGE_OK)
        return XGE_ERROR_BACKEND_FAILED;
    *library=(xge_gl_library){NULL,NULL,0};return XGE_OK;
}
static int __xgeGraphicsLoadGL(int backend)
{
    /* At most one failed rollback/retirement can be outstanding. Settle it
     * before acquiring another reference, even when reloading the same backend. */
    if(__xgeGLLibraryClose(&g_xgeGLRetiredLibrary)!=XGE_OK)
        return XGE_ERROR_BACKEND_FAILED;
    xge_gl_library candidate=g_xgeGLLibrary;
    int acquired=!candidate.handle || candidate.backend!=backend;
    if(acquired){
        candidate=(xge_gl_library){NULL,NULL,0};candidate.backend=backend;
        if(__xgeGLSystemOpen(&candidate)!=XGE_OK)return XGE_ERROR_BACKEND_FAILED;
    }
    /* Typed slots are committed atomically by xge_gl_load_ex. Until that
     * succeeds, the old library must continue to back the old complete table. */
    if(!xge_gl_load_ex(__xgeGLSystemProc,&candidate)){
        if(acquired && __xgeGLLibraryClose(&candidate)!=XGE_OK)
            g_xgeGLRetiredLibrary=candidate;
        return XGE_ERROR_BACKEND_FAILED;
    }
    if(acquired){
        g_xgeGLRetiredLibrary=g_xgeGLLibrary;
        g_xgeGLLibrary=candidate;
        /* A failure here follows a committed table replacement. Keep both
         * physical references; the new table still has its own active owner. */
        if(__xgeGLLibraryClose(&g_xgeGLRetiredLibrary)!=XGE_OK)
            return XGE_ERROR_BACKEND_FAILED;
    }
    return XGE_OK;
}
static int __xgeGraphicsLibraryRetire(void)
{
    /* No slot may outlive the last supporting library reference. Unit's
     * resource/worker admission has already succeeded at this point. */
    xge_gl_unload();
    int first=__xgeGLLibraryClose(&g_xgeGLRetiredLibrary);
    int active=__xgeGLLibraryClose(&g_xgeGLLibrary);
    return first!=XGE_OK ? first:active;
}
#endif
