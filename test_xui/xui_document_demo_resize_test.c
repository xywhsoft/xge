/* Resize only the window created by this test. Linux requires an owned display. */
#define main document_demo_original_main
#include "../examples/xui_document/main.c"
#undef main
#include <math.h>
#ifndef _WIN32
#include <X11/Xlib.h>
#endif

static demo app;
static const char* destination;
static const char* mode_name;
static unsigned phase, stage, checks, native_frames;
static uint64_t rich_revision, md_revision;
static char* rich_bytes;
static char* md_bytes;
static uint64_t rich_length, md_length;
static int succeeded, with_find;
#ifndef _WIN32
static Display* sender;
#endif
static const int sizes[][2] = {{640,760},{960,640},{1440,900},{W,H}};
#define REQUIRE(e) do { checks++; if (!(e)) { fprintf(stderr,"native resize line %d phase %u stage %u: %s\n",__LINE__,phase,stage,#e); app.result=XUI_ERROR_INVALID_STATE; xgeQuit(); return app.result; } } while (0)

static int serialized(xui_document document,char** text,uint64_t* length)
{
    xui_document_snapshot snapshot=NULL; int r=xuiDocumentAcquireSnapshot(document,&snapshot);
    if(r==XUI_OK) r=xuiDocumentSerialize(snapshot,text,length);
    xuiDocumentSnapshotRelease(snapshot); return r;
}
static int content_kept(xui_document document,const char* before,uint64_t bytes)
{
    char* after=NULL; uint64_t length=0; int r=serialized(document,&after,&length);
    int same=r==XUI_OK && length==bytes && !memcmp(before,after,(size_t)bytes);
    xuiDocumentFreeBuffer(after); return same;
}
static int resize_native(int width,int height)
{
#ifdef _WIN32
    HWND window=(HWND)xgePlatformNativeHandle(); RECT outer,client;
    if(!window || !GetWindowRect(window,&outer) || !GetClientRect(window,&client)) return 0;
    return SetWindowPos(window,NULL,0,0,width+(outer.right-outer.left)-(client.right-client.left),
        height+(outer.bottom-outer.top)-(client.bottom-client.top),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE)!=0;
#else
    Window window=(Window)(uintptr_t)xgePlatformNativeHandle();
    if(!window || !sender) return 0;
    XResizeWindow(sender,window,(unsigned)width,(unsigned)height); XSync(sender,False); return 1;
#endif
}
static int contained(xui_rect_t r,int width,int height)
{
    return r.fX>=0 && r.fY>=0 && r.fW>0 && r.fH>0 && r.fX+r.fW<=width && r.fY+r.fH<=height;
}
static int click_native(xui_widget widget)
{
    xui_rect_t r=xuiWidgetGetWorldRect(widget); int x=(int)r.fX+12,y=(int)r.fY+14;
#ifdef _WIN32
    HWND window=(HWND)xgePlatformNativeHandle(); LPARAM point=MAKELPARAM(x,y);
    return PostMessageW(window,WM_MOUSEMOVE,0,point) && PostMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,point) &&
        PostMessageW(window,WM_LBUTTONUP,0,point);
#else
    Window window=(Window)(uintptr_t)xgePlatformNativeHandle(); XEvent event={0};
    event.xmotion.type=MotionNotify; event.xmotion.display=sender; event.xmotion.window=window;
    event.xmotion.root=DefaultRootWindow(sender); event.xmotion.same_screen=True; event.xmotion.x=x; event.xmotion.y=y;
    if(!XSendEvent(sender,window,False,PointerMotionMask,&event)) return 0;
    memset(&event,0,sizeof(event)); event.xbutton.display=sender; event.xbutton.window=window;
    event.xbutton.root=DefaultRootWindow(sender); event.xbutton.same_screen=True;
    event.xbutton.x=x; event.xbutton.y=y; event.xbutton.button=Button1; event.xbutton.type=ButtonPress;
    if(!XSendEvent(sender,window,False,ButtonPressMask,&event)) return 0;
    event.xbutton.type=ButtonRelease; event.xbutton.state=Button1Mask;
    if(!XSendEvent(sender,window,False,ButtonReleaseMask,&event)) return 0;
    XSync(sender,False); return 1;
#endif
}
static int overlap(xui_rect_t a,xui_rect_t b)
{
    return a.fX<b.fX+b.fW && b.fX<a.fX+a.fW && a.fY<b.fY+b.fH && b.fY<a.fY+a.fH;
}
static xui_widget button_named(const char* name)
{
    xui_widget child=xuiWidgetGetFirstChild(app.root);
    while(child) {
        const char* text=xuiButtonGetText(child);
        if(text && !strcmp(text,name)) return child;
        child=xuiWidgetGetNextSibling(child);
    }
    return NULL;
}
static int capture_stage(int width,int height)
{
    char path[1024]; size_t stride=(size_t)width*4; unsigned char* pixels=malloc(stride*(size_t)height);
    int r; if(!pixels) return XUI_ERROR_OUT_OF_MEMORY;
    r=app.proxy.surfaceReadRGBA(&app.proxy,app.target,pixels,(int)stride);
    snprintf(path,sizeof(path),"%s/resize-%ux%u.png",destination,(unsigned)width,(unsigned)height);
    if(r==XUI_OK) r=xgeImageSavePNG(path,width,height,pixels,(int)stride);
    free(pixels); return r;
}
static xui_widget find_control(xui_widget window,xui_widget_type type,int ordinal)
{
    xui_widget child;
    for(child=xuiWidgetGetFirstChild(xuiWindowGetClientWidget(window));child;child=xuiWidgetGetNextSibling(child))
        if(xuiWidgetIsType(child,type) && ordinal--==0) return child;
    return NULL;
}
static int close_find(void)
{
    int r=xuiInputKeyDown(app.context,XUI_KEY_ESCAPE,0);
    if(r==XUI_OK) r=xuiInputKeyUp(app.context,XUI_KEY_ESCAPE,0);
    if(r==XUI_OK) r=xuiDispatchPendingEvents(app.context);
    return r;
}
static int find_fits(xui_widget editor)
{
    xui_widget window=xuiDocumentEditorGetFindWindow(editor),child; xui_rect_t client;
    if(!window || !xuiWindowIsOpen(window) || !contained(xuiWidgetGetWorldRect(window),app.width,app.height)) return 0;
    client=xuiWidgetGetWorldRect(xuiWindowGetClientWidget(window));
    for(child=xuiWidgetGetFirstChild(xuiWindowGetClientWidget(window));child;child=xuiWidgetGetNextSibling(child)) {
        xui_rect_t r=xuiWidgetGetWorldRect(child);
        if(xuiWidgetGetVisible(child) && (r.fW<=0 || r.fH<=0 || r.fX<client.fX || r.fY<client.fY || r.fX+r.fW>client.fX+client.fW || r.fY+r.fH>client.fY+client.fH)) return 0;
    }
    return 1;
}
static int open_find(xui_widget editor)
{
    xui_widget window,input,button; xui_rect_t r; int result; uint64_t count=0;
    result=stage%2?xuiDocumentEditorOpenReplace(editor):xuiDocumentEditorOpenFind(editor);
    if(result!=XUI_OK) return result;
    result=xuiLayout(app.context); if(result!=XUI_OK) return result;
    REQUIRE(find_fits(editor));
    window=xuiDocumentEditorGetFindWindow(editor); input=find_control(window,xuiInputGetType(app.context),0);
    REQUIRE(input && xuiInputSetText(input,"a")==XUI_OK);
    for(button=xuiWidgetGetFirstChild(xuiWindowGetClientWidget(window));button;button=xuiWidgetGetNextSibling(button)) {
        const char* text=xuiButtonGetText(button);
        if(text && !strcmp(text,xuiTranslate(app.context,XUI_TR_FIND_ALL))) break;
    }
    REQUIRE(button); r=xuiWidgetGetWorldRect(button);
    REQUIRE(xuiInputPointerDown(app.context,r.fX+r.fW/2,r.fY+r.fH/2,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT)==XUI_OK);
    REQUIRE(xuiInputPointerUp(app.context,r.fX+r.fW/2,r.fY+r.fH/2,XUI_POINTER_BUTTON_LEFT,0)==XUI_OK);
    REQUIRE(xuiDispatchPendingEvents(app.context)==XUI_OK);
    REQUIRE(xuiDocumentViewGetFindResultCount(editor,&count)==XUI_OK && count>0);
    return xuiSetFocusWidget(app.context,input);
}
static int native_frame(void* unused)
{
    int r; (void)unused;
    if(++native_frames>=300) { fprintf(stderr,"native resize timed out\n"); app.result=XUI_ERROR_INVALID_STATE; xgeQuit(); return app.result; }
    r=frame(&app); REQUIRE(r==XGE_OK);
    if(!phase) {
#ifdef _WIN32
        ShowWindow((HWND)xgePlatformNativeHandle(),SW_HIDE);
#endif
        REQUIRE(xuiDocumentViewSetMode(app.markdown,!strcmp(mode_name,"visual")?XUI_DOC_VISUAL:
            !strcmp(mode_name,"live")?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_SOURCE_TEXT)==XUI_OK);
        REQUIRE(serialized(app.rich_doc,&rich_bytes,&rich_length)==XUI_OK);
        REQUIRE(serialized(app.md_doc,&md_bytes,&md_length)==XUI_OK);
        rich_revision=xuiDocumentGetRevision(app.rich_doc); md_revision=xuiDocumentGetRevision(app.md_doc);
        REQUIRE(xuiSetFocusWidget(app.context,!strcmp(mode_name,"rich")?app.rich:app.markdown)==XUI_OK);
        REQUIRE(resize_native(sizes[stage][0],sizes[stage][1])); phase=1;
    } else if(phase==1 && xgeGetWidth()==sizes[stage][0] && xgeGetHeight()==sizes[stage][1]) {
        const char* names[]={"Undo","Redo","Bold","Italic","Source","Visual","Live MD","Open","Save as","Find","Replace","Format"};
        xui_surface_desc_t surface={0}; xui_rect_t root=xuiWidgetGetRect(app.root);
        xui_widget editors[]={app.rich,app.markdown,app.preview}; xui_widget active=!strcmp(mode_name,"rich")?app.rich:app.markdown;
        int width=sizes[stage][0],height=sizes[stage][1],i,j;
        REQUIRE(root.fW==width && root.fH==height);
        REQUIRE(app.proxy.surfaceGetDesc(&app.proxy,app.target,&surface)==XUI_OK);
        REQUIRE(surface.iWidth==width && surface.iHeight==height);
        REQUIRE(xuiGetViewportSize(app.context).iW==width && xuiGetViewportSize(app.context).iH==height);
        REQUIRE(xuiGetFocusWidget(app.context)==(with_find && stage ?
            find_control(xuiDocumentEditorGetFindWindow(active),xuiInputGetType(app.context),0):active));
        if(with_find && stage) {
            xui_widget input=find_control(xuiDocumentEditorGetFindWindow(active),xuiInputGetType(app.context),0);
            REQUIRE(find_fits(active) && input && !strcmp(xuiInputGetText(input),"a"));
            REQUIRE(close_find()==XUI_OK && !xuiWindowIsOpen(xuiDocumentEditorGetFindWindow(active)));
            REQUIRE(xuiGetFocusWidget(app.context)==active);
        }
        REQUIRE(xuiDocumentGetRevision(app.rich_doc)==rich_revision && xuiDocumentGetRevision(app.md_doc)==md_revision);
        REQUIRE(content_kept(app.rich_doc,rich_bytes,rich_length) && content_kept(app.md_doc,md_bytes,md_length));
        REQUIRE(xuiDocumentViewGetDocument(app.preview)==app.md_doc);
        for(i=0;i<3;i++) {
            xui_rect_t rect=xuiWidgetGetWorldRect(editors[i]); REQUIRE(contained(rect,width,height));
            for(j=i+1;j<3;j++) REQUIRE(!overlap(rect,xuiWidgetGetWorldRect(editors[j])));
        }
        for(i=0;i<DEMO_BUTTON_COUNT;i++) {
            xui_widget button=button_named(names[i]); xui_rect_t rect;
            REQUIRE(button); rect=xuiWidgetGetWorldRect(button); REQUIRE(contained(rect,width,height));
            for(j=0;j<3;j++) REQUIRE(!overlap(rect,xuiWidgetGetWorldRect(editors[j])));
            for(j=i+1;j<DEMO_BUTTON_COUNT;j++) REQUIRE(!overlap(rect,xuiWidgetGetWorldRect(button_named(names[j]))));
        }
        REQUIRE(contained(xuiWidgetGetWorldRect(app.status),width,height));
        REQUIRE(capture_stage(width,height)==XUI_OK);
        REQUIRE(xuiSetFocusWidget(app.context,active==app.rich?app.markdown:app.rich)==XUI_OK);
        REQUIRE(click_native(active)); phase=2;
    } else if(phase==2 && xuiGetFocusWidget(app.context)==(!strcmp(mode_name,"rich")?app.rich:app.markdown)) {
        xui_widget active=!strcmp(mode_name,"rich")?app.rich:app.markdown;
        REQUIRE(capture_stage(sizes[stage][0],sizes[stage][1])==XUI_OK);
        REQUIRE(xuiDocumentEditorInsertText(active,"中文",strlen("中文"))==XUI_OK);
        REQUIRE(xuiDocumentEditorFlush(active)==XUI_OK);
        REQUIRE(xuiDocumentEditorExecute(active,XUI_DOC_EDIT_UNDO)==XUI_OK);
        REQUIRE(content_kept(app.rich_doc,rich_bytes,rich_length) && content_kept(app.md_doc,md_bytes,md_length));
        REQUIRE(xuiDocumentEditorExecute(active,XUI_DOC_EDIT_REDO)==XUI_OK);
        REQUIRE(xuiDocumentEditorExecute(active,XUI_DOC_EDIT_UNDO)==XUI_OK);
        rich_revision=xuiDocumentGetRevision(app.rich_doc); md_revision=xuiDocumentGetRevision(app.md_doc);
        if(with_find) {
            char path[1024];
            REQUIRE(open_find(active)==XUI_OK);
            REQUIRE(xuiUpdate(app.context,.016f)==XUI_OK && xuiLayout(app.context)==XUI_OK);
            REQUIRE(app.proxy.surfaceClear(&app.proxy,app.target,XUI_COLOR_WHITE)==XUI_OK);
            { xui_rect_i_t damage={0,0,app.width,app.height}; REQUIRE(xuiRender(app.context,app.target,&damage,1)==XUI_OK); }
            snprintf(path,sizeof(path),"%s/find-resize-%dx%d.png",destination,app.width,app.height);
            { xui_surface_desc_t sd={0}; unsigned char* pixels=malloc((size_t)app.width*app.height*4);
              REQUIRE(pixels && app.proxy.surfaceGetDesc(&app.proxy,app.target,&sd)==XUI_OK);
              r=app.proxy.surfaceReadRGBA(&app.proxy,app.target,pixels,app.width*4);
              if(r==XUI_OK) r=xgeImageSavePNG(path,app.width,app.height,pixels,app.width*4);
              free(pixels); REQUIRE(r==XUI_OK); }
        }
        if(++stage==4) { succeeded=1; xgeQuit(); }
        else { REQUIRE(resize_native(sizes[stage][0],sizes[stage][1])); phase=1; }
    }
    return XGE_OK;
}
int main(int argc,char** argv)
{
    xge_desc_t desc={0}; int r;
    if(argc!=3 && argc!=4) return 2;
    destination=argv[1]; mode_name=argv[2];
    with_find=argc==4 && !strcmp(argv[3],"find");
#ifndef _WIN32
    if(!getenv("XUI_TEST_PRIVATE_DISPLAY") || strcmp(getenv("XUI_TEST_PRIVATE_DISPLAY"),"1")) return 2;
    sender=XOpenDisplay(NULL); if(!sender) return 2;
#endif
    desc.iWidth=W; desc.iHeight=H; desc.sTitle="Native Document resize";
    desc.iRunMode=XGE_RUN_GAME_LOOP; desc.iTargetFPS=60; desc.iFlags=XGE_INIT_VSYNC;
    if(xgeInit(&desc)!=XGE_OK) return 2;
    r=xgeRun(native_frame,NULL);
    fprintf(stderr,"native resize result: run=%d demo=%d stage=%u frames=%u checks=%u succeeded=%d\n",r,app.result,stage,native_frames,checks,succeeded);
    xgeSetQuitRequestCallback(NULL,NULL);
    if(app.close_box) xuiMsgBoxDestroy(app.close_box);
    if(app.file_dialog) xuiFileDialogDestroy(app.file_dialog);
    if(app.discard_box) xuiMsgBoxDestroy(app.discard_box);
    xuiDocumentRelease(app.pending_open); xuiDestroy(app.context);
    xuiDocumentRelease(app.rich_doc); xuiDocumentRelease(app.md_doc);
    if(app.proxy.surfaceDestroy && app.target) app.proxy.surfaceDestroy(&app.proxy,app.target);
    if(app.proxy.fontDestroy) { app.proxy.fontDestroy(&app.proxy,app.mono); app.proxy.fontDestroy(&app.proxy,app.italic); app.proxy.fontDestroy(&app.proxy,app.bold); app.proxy.fontDestroy(&app.proxy,app.font); }
    xuiDocumentFreeBuffer(rich_bytes); xuiDocumentFreeBuffer(md_bytes);
    xgeFontFallbackClear(); xgeUnit();
#ifndef _WIN32
    XCloseDisplay(sender);
#endif
    if(r==XGE_OK && succeeded && app.result==XUI_OK) {
        printf("Native Document resize %s: %u checks passed.\n",mode_name,checks); return 0;
    }
    return 1;
}
