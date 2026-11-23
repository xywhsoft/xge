/* Included by xge_graphics_context.c on Windows. The hidden drawable and its
 * driver stay owned until the renderer and all of its GPU children retire. */
typedef HGLRC (WINAPI *xge_wgl_create_proc)(HDC);
typedef BOOL (WINAPI *xge_wgl_delete_proc)(HGLRC);
typedef BOOL (WINAPI *xge_wgl_current_proc)(HDC,HGLRC);
typedef HGLRC (WINAPI *xge_wgl_get_context_proc)(void);
typedef HDC (WINAPI *xge_wgl_get_dc_proc)(void);
typedef HGLRC (WINAPI *xge_wgl_create_attribs_proc)(HDC,HGLRC,const int*);

typedef struct xge_wgl_offscreen_t {
    xge_gl_library library;
    HINSTANCE instance;
    ATOM windowClass;
    HWND window;
    HDC dc;
    HGLRC initialContext;
    HGLRC coreContext;
    xge_wgl_delete_proc deleteContext;
    xge_wgl_current_proc makeCurrent;
    xge_wgl_get_context_proc getContext;
    xge_wgl_get_dc_proc getDC;
} xge_wgl_offscreen_t;

static int __xgeWglOffscreenFail(const char* operation)
{
    xge_error_info_t error={sizeof(error),XGE_ERROR_BACKEND_FAILED,1,
        "wgl",operation,"Windows offscreen OpenGL operation failed"};
    (void)xgeReportError(&error);
    return XGE_ERROR_BACKEND_FAILED;
}

static bool __xgeWglOffscreenIsCurrent(const xge_wgl_offscreen_t* state)
{
    return state->coreContext && state->getContext()==state->coreContext && state->getDC()==state->dc;
}

static int __xgeWglOffscreenMakeCurrent(xge_wgl_offscreen_t* state)
{
    if(!state->coreContext)return XGE_ERROR_NOT_INITIALIZED;
    if(__xgeWglOffscreenIsCurrent(state))return XGE_OK;
    return state->makeCurrent(state->dc,state->coreContext) ? XGE_OK:__xgeWglOffscreenFail("wglMakeCurrent");
}

static int __xgeWglOffscreenUnit(xge_wgl_offscreen_t* state)
{
    if(state->coreContext || state->initialContext){
        HGLRC current=state->getContext();
        if(current && (current==state->coreContext || current==state->initialContext) &&
           !state->makeCurrent(NULL,NULL))return __xgeWglOffscreenFail("wglMakeCurrent(NULL)");
    }
    if(state->coreContext){
        if(!state->deleteContext(state->coreContext))return __xgeWglOffscreenFail("wglDeleteContext(core)");
        state->coreContext=NULL;
    }
    if(state->initialContext){
        if(!state->deleteContext(state->initialContext))return __xgeWglOffscreenFail("wglDeleteContext(initial)");
        state->initialContext=NULL;
    }
    if(state->dc){
        if(!ReleaseDC(state->window,state->dc))return __xgeWglOffscreenFail("ReleaseDC");
        state->dc=NULL;
    }
    if(state->window){
        if(!DestroyWindow(state->window))return __xgeWglOffscreenFail("DestroyWindow");
        state->window=NULL;
    }
    if(state->windowClass){
        if(!UnregisterClassA(MAKEINTRESOURCEA(state->windowClass),state->instance))
            return __xgeWglOffscreenFail("UnregisterClass");
        state->windowClass=0;
    }
    if(state->library.handle){
        if(__xgeGLSystemClose(&state->library)!=XGE_OK)return __xgeWglOffscreenFail("FreeLibrary");
        state->library=(xge_gl_library){0};
    }
    return XGE_OK;
}

static int __xgeWglOffscreenInit(xge_wgl_offscreen_t* state,int width,int height)
{
    state->library.backend=XGE_GPU_BACKEND_OPENGL33;
    if(__xgeGLSystemOpen(&state->library)!=XGE_OK)return __xgeWglOffscreenFail("LoadLibrary(opengl32)");
    HMODULE driver=(HMODULE)state->library.handle;
    xge_wgl_create_proc create=(xge_wgl_create_proc)GetProcAddress(driver,"wglCreateContext");
    state->deleteContext=(xge_wgl_delete_proc)GetProcAddress(driver,"wglDeleteContext");
    state->makeCurrent=(xge_wgl_current_proc)GetProcAddress(driver,"wglMakeCurrent");
    state->getContext=(xge_wgl_get_context_proc)GetProcAddress(driver,"wglGetCurrentContext");
    state->getDC=(xge_wgl_get_dc_proc)GetProcAddress(driver,"wglGetCurrentDC");
    if(!create || !state->deleteContext || !state->makeCurrent || !state->getContext || !state->getDC)
        return XGE_ERROR_UNSUPPORTED;
    if(state->getContext())return XGE_ERROR_BUSY;

    char className[64];
    int length=snprintf(className,sizeof(className),"XGE3Offscreen-%p",(void*)state);
    if(length<0 || (size_t)length>=sizeof(className))return XGE_ERROR_INVALID_STATE;
    state->instance=GetModuleHandleA(NULL);
    WNDCLASSA windowClass={0};
    windowClass.style=CS_OWNDC;
    windowClass.lpfnWndProc=DefWindowProcA;
    windowClass.hInstance=state->instance;
    windowClass.lpszClassName=className;
    state->windowClass=RegisterClassA(&windowClass);
    if(!state->windowClass)return __xgeWglOffscreenFail("RegisterClass");
    state->window=CreateWindowExA(0,MAKEINTRESOURCEA(state->windowClass),"",
        WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,width,height,NULL,NULL,state->instance,NULL);
    if(!state->window)return __xgeWglOffscreenFail("CreateWindowEx");
    RECT client;
    if(!GetClientRect(state->window,&client) || client.right!=width || client.bottom!=height)
        return __xgeWglOffscreenFail("GetClientRect");
    state->dc=GetDC(state->window);
    if(!state->dc)return __xgeWglOffscreenFail("GetDC");
    PIXELFORMATDESCRIPTOR pixels={0};
    pixels.nSize=sizeof(pixels);
    pixels.nVersion=1;
    pixels.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pixels.iPixelType=PFD_TYPE_RGBA;
    pixels.cColorBits=32;
    pixels.cAlphaBits=8;
    pixels.cDepthBits=24;
    pixels.cStencilBits=8;
    int format=ChoosePixelFormat(state->dc,&pixels);
    if(!format || !SetPixelFormat(state->dc,format,&pixels))return __xgeWglOffscreenFail("SetPixelFormat");

    state->initialContext=create(state->dc);
    if(!state->initialContext)return __xgeWglOffscreenFail("wglCreateContext");
    if(!state->makeCurrent(state->dc,state->initialContext))return __xgeWglOffscreenFail("wglMakeCurrent(initial)");
    xge_wgl_create_attribs_proc createCore=(xge_wgl_create_attribs_proc)
        __xgeGLSystemProc("wglCreateContextAttribsARB",&state->library);
    if(!createCore)return XGE_ERROR_UNSUPPORTED;
    enum { majorVersion=0x2091,minorVersion=0x2092,profileMask=0x9126,coreProfile=1 };
    const int attributes[]={majorVersion,3,minorVersion,3,profileMask,coreProfile,0};
    state->coreContext=createCore(state->dc,NULL,attributes);
    if(!state->coreContext)return XGE_ERROR_UNSUPPORTED;
    int status=__xgeWglOffscreenMakeCurrent(state);
    if(status!=XGE_OK)return status;
    if(!state->deleteContext(state->initialContext))return __xgeWglOffscreenFail("wglDeleteContext(initial)");
    state->initialContext=NULL;

    PFNGLGETINTEGERVPROC get=(PFNGLGETINTEGERVPROC)__xgeGLSystemProc("glGetIntegerv",&state->library);
    if(!get)return XGE_ERROR_UNSUPPORTED;
    GLint major=0,minor=0,limits[2]={0};
    get(GL_MAJOR_VERSION,&major);
    get(GL_MINOR_VERSION,&minor);
    get(GL_MAX_VIEWPORT_DIMS,limits);
    if(major<3 || (major==3 && minor<3) || width>limits[0] || height>limits[1])return XGE_ERROR_UNSUPPORTED;
    return xge_gl_load_ex(__xgeGLSystemProc,&state->library) ? XGE_OK:__xgeWglOffscreenFail("load GL 3.3");
}
