/* Native X11 keyboard/mouse/WM_DELETE_WINDOW queue, on the driver's private
 * display only. This does not exercise a physical keyboard or an IME server. */
#define main document_demo_original_main
#include "../examples/xui_document/main.c"
#undef main
#include <X11/Xlib.h>
#include <X11/keysym.h>

static demo app;
static Display* sender;
static KeySym* original_keys;
static int first_keycode, original_width;
static const char* destination;
static const char* mode_name;
static const char seed[] = "seed text\n";
static const char typed[] = "中文seed text\n";
static unsigned phase, checks, native_frames;
static uint64_t before_revision;
static xge_platform_runtime_t before_events, final_events;
static int succeeded;
#define REQUIRE(e) do { checks++; if (!(e)) { fprintf(stderr,"native X11 input line %d phase %u: %s\n",__LINE__,phase,#e); app.result=XUI_ERROR_INVALID_STATE; xgeQuit(); return app.result; } } while(0)

static int source_is(const char* expected)
{
    char actual[128]; uint64_t bytes=0; xui_document_snapshot snapshot=NULL;
    int r=xuiDocumentAcquireSnapshot(app.md_doc,&snapshot);
    if(r==XUI_OK) r=xuiDocumentSnapshotCopySource(snapshot,actual,sizeof(actual),&bytes);
    xuiDocumentSnapshotRelease(snapshot);
    return r==XUI_OK && bytes==strlen(expected) && !strcmp(actual,expected);
}
static int mark_saved(xui_document document)
{
    xui_document_snapshot snapshot=NULL; int r=xuiDocumentAcquireSnapshot(document,&snapshot);
    if(r==XUI_OK) r=xuiDocumentMarkSaved(document,snapshot);
    xuiDocumentSnapshotRelease(snapshot); return r;
}
static int window_exists(Window window)
{
    XWindowAttributes attributes;
    return XGetWindowAttributes(sender,window,&attributes) && attributes.width==W && attributes.height==H;
}
static int send_key(Window window,KeyCode code,unsigned modifiers)
{
    XEvent event={0};
    event.xkey.display=sender; event.xkey.window=window;
    event.xkey.root=DefaultRootWindow(sender); event.xkey.same_screen=True;
    event.xkey.keycode=code; event.xkey.state=modifiers; event.xkey.time=CurrentTime;
    event.xkey.type=KeyPress;
    if(!code || !XSendEvent(sender,window,False,KeyPressMask,&event)) return 0;
    event.xkey.type=KeyRelease;
    return XSendEvent(sender,window,False,KeyReleaseMask,&event)!=0;
}
static int send_close(Window window)
{
    XEvent event={0};
    event.xclient.type=ClientMessage; event.xclient.display=sender;
    event.xclient.window=window; event.xclient.message_type=XInternAtom(sender,"WM_PROTOCOLS",False);
    event.xclient.format=32; event.xclient.data.l[0]=(long)XInternAtom(sender,"WM_DELETE_WINDOW",False);
    event.xclient.data.l[1]=CurrentTime;
    return XSendEvent(sender,window,False,NoEventMask,&event)!=0;
}
static int send_button(Window window,xui_widget button)
{
    XEvent event={0}; xui_rect_t rect; int x,y;
    if(!button || xuiLayout(app.context)!=XUI_OK) return 0;
    rect=xuiWidgetGetWorldRect(button); if(rect.fW<=0 || rect.fH<=0) return 0;
    x=(int)(rect.fX+rect.fW/2); y=(int)(rect.fY+rect.fH/2);
    event.xmotion.type=MotionNotify; event.xmotion.display=sender; event.xmotion.window=window;
    event.xmotion.root=DefaultRootWindow(sender); event.xmotion.same_screen=True;
    event.xmotion.x=x; event.xmotion.y=y; event.xmotion.time=CurrentTime;
    if(!XSendEvent(sender,window,False,PointerMotionMask,&event)) return 0;
    memset(&event,0,sizeof(event)); event.xbutton.display=sender; event.xbutton.window=window;
    event.xbutton.root=DefaultRootWindow(sender); event.xbutton.same_screen=True;
    event.xbutton.x=x; event.xbutton.y=y; event.xbutton.button=Button1; event.xbutton.time=CurrentTime;
    event.xbutton.type=ButtonPress;
    if(!XSendEvent(sender,window,False,ButtonPressMask,&event)) return 0;
    event.xbutton.type=ButtonRelease; event.xbutton.state=Button1Mask;
    return XSendEvent(sender,window,False,ButtonReleaseMask,&event)!=0;
}
static int capture(const char* name)
{
    char path[1024]; uint64_t source,preview;
    snprintf(path,sizeof(path),"%s/%s",destination,name);
    return capture_async(&app,path,&source,&preview);
}
static int native_frame(void* unused)
{
    Window window; int r; (void)unused;
    REQUIRE(++native_frames<300);
    r=frame(&app); REQUIRE(r==XGE_OK);
    window=(Window)(uintptr_t)xgePlatformNativeHandle(); REQUIRE(window && window_exists(window));
    if(!phase) {
        xui_doc_range_t range={0}; xui_document_snapshot snapshot=NULL; uint64_t matches=0;
        uint32_t mode=!strcmp(mode_name,"source")?XUI_DOC_SOURCE_TEXT:!strcmp(mode_name,"visual")?XUI_DOC_VISUAL:XUI_DOC_LIVE_MARKDOWN;
        REQUIRE(xuiDocumentLoadMarkdown(app.md_doc,seed,strlen(seed))==XUI_OK);
        REQUIRE(xuiDocumentViewSetMode(app.markdown,mode)==XUI_OK);
        if(mode==XUI_DOC_VISUAL) {
            REQUIRE(xuiDocumentAcquireSnapshot(app.md_doc,&snapshot)==XUI_OK);
            r=xuiDocumentSnapshotFind(snapshot,XUI_DOC_SEMANTIC,"seed",4,NULL,&range,1,&matches);
            xuiDocumentSnapshotRelease(snapshot); REQUIRE(r==XUI_OK && matches==1);
        } else {
            REQUIRE(xuiDocumentViewGetSelection(app.markdown,&range)==XUI_OK);
            REQUIRE(range.tAnchor.iKind==XUI_DOC_POSITION_SOURCE); range.tAnchor.iOffset=0;
        }
        range.tCaret=range.tAnchor; REQUIRE(xuiDocumentViewSetSelection(app.markdown,&range)==XUI_OK);
        REQUIRE(xuiDocumentClearHistory(app.md_doc)==XUI_OK);
        REQUIRE(mark_saved(app.rich_doc)==XUI_OK && mark_saved(app.md_doc)==XUI_OK);
        REQUIRE(xuiSetFocusWidget(app.context,app.markdown)==XUI_OK);
        REQUIRE(xgePlatformRuntimeGet(&before_events)==XGE_OK);
        before_revision=xuiDocumentGetRevision(app.md_doc);
        REQUIRE(send_key(window,(KeyCode)first_keycode,0));
        REQUIRE(send_key(window,(KeyCode)(first_keycode+1),0));
        REQUIRE(send_close(window)); XSync(sender,False);
        REQUIRE(xuiDocumentGetRevision(app.md_doc)==before_revision && !xuiDocumentIsDirty(app.md_doc));
        phase=1;
    } else if(phase==1 && app.close_box && xuiMsgBoxIsOpen(app.close_box)) {
        REQUIRE(xgePlatformRuntimeGet(&final_events)==XGE_OK);
        REQUIRE(final_events.iTextEventCount==before_events.iTextEventCount+2);
        REQUIRE(final_events.iQuitEventCount==before_events.iQuitEventCount+1);
        REQUIRE(xuiDocumentGetRevision(app.md_doc)==before_revision+2 && xuiDocumentIsDirty(app.md_doc));
        REQUIRE(source_is(typed) && xuiDocumentViewGetDocument(app.preview)==app.md_doc);
        REQUIRE(capture("native-x11-close-prompt.png")==XUI_OK);
        REQUIRE(send_button(window,xuiMsgBoxGetButtonWidget(app.close_box,2))); XSync(sender,False); phase=2;
    } else if(phase==2 && !xuiMsgBoxIsOpen(app.close_box)) {
        REQUIRE(!app.close_requested && !app.close_saving && xuiDocumentIsDirty(app.md_doc));
        fprintf(stderr,"native cancel: source_kept=%d editor_focus=%d\n",source_is(typed),xuiGetFocusWidget(app.context)==app.markdown);
        REQUIRE(source_is(typed));
        REQUIRE(xuiGetFocusWidget(app.context)==app.markdown);
        REQUIRE(capture("native-x11-cancel-kept.png")==XUI_OK);
        REQUIRE(send_key(window,XKeysymToKeycode(sender,XK_z),ControlMask)); XSync(sender,False); phase=3;
    } else if(phase==3) {
        REQUIRE(source_is(seed) && !xuiDocumentIsDirty(app.md_doc));
        REQUIRE(xuiDocumentCanRedo(app.md_doc) && !xuiDocumentCanUndo(app.md_doc));
        REQUIRE(send_key(window,XKeysymToKeycode(sender,XK_y),ControlMask)); XSync(sender,False); phase=4;
    } else if(phase==4) {
        REQUIRE(source_is(typed) && xuiDocumentIsDirty(app.md_doc));
        REQUIRE(xuiDocumentCanUndo(app.md_doc) && !xuiDocumentCanRedo(app.md_doc));
        REQUIRE(send_close(window)); XSync(sender,False); phase=5;
    } else if(phase==5 && xuiMsgBoxIsOpen(app.close_box)) {
        REQUIRE(xgePlatformRuntimeGet(&final_events)==XGE_OK);
        REQUIRE(final_events.iQuitEventCount==before_events.iQuitEventCount+2);
        REQUIRE(source_is(typed) && xuiDocumentViewGetDocument(app.preview)==app.md_doc);
        REQUIRE(send_button(window,xuiMsgBoxGetButtonWidget(app.close_box,1))); XSync(sender,False); phase=6;
    } else if(phase==6 && !xuiMsgBoxIsOpen(app.close_box)) {
        REQUIRE(!app.close_requested && !app.close_saving && xuiDocumentIsDirty(app.md_doc));
        REQUIRE(source_is(typed)); REQUIRE(xgePlatformRuntimeGet(&final_events)==XGE_OK);
        REQUIRE(final_events.iMouseEventCount>=before_events.iMouseEventCount+6);
        succeeded=1;
    }
    return XGE_OK;
}
int main(int argc,char** argv)
{
    xge_desc_t desc={0}; int r,min_key,max_key; KeySym keys[2]={0x01004e2d,0x01006587};
    if(argc!=4 || strcmp(argv[3],"--isolated-display") || !getenv("XUI_TEST_PRIVATE_DISPLAY") ||
        (strcmp(argv[2],"source") && strcmp(argv[2],"visual") && strcmp(argv[2],"live"))) {
        fprintf(stderr,"Run with the authenticated private-display driver.\n"); return 2;
    }
    destination=argv[1]; mode_name=argv[2]; sender=XOpenDisplay(NULL); if(!sender) return 2;
    XDisplayKeycodes(sender,&min_key,&max_key); if(max_key-min_key<2) {XCloseDisplay(sender);return 2;}
    first_keycode=max_key-1; original_keys=XGetKeyboardMapping(sender,(KeyCode)first_keycode,2,&original_width);
    if(!original_keys) {XCloseDisplay(sender);return 2;}
    /* Before XGE opens its Xlib connection, so its initial key map includes
     * the two test Unicode keysyms. Only the private server is modified. */
    XChangeKeyboardMapping(sender,first_keycode,1,keys,2); XSync(sender,False);
    desc.iWidth=W;desc.iHeight=H;desc.sTitle="Native X11 Document input";
    desc.iRunMode=XGE_RUN_GAME_LOOP;desc.iTargetFPS=60;desc.iFlags=XGE_INIT_VSYNC;
    r=xgeInit(&desc); if(r==XGE_OK) r=xgeRun(native_frame,NULL);
    fprintf(stderr,"native X11 result mode=%s run=%d demo=%d phase=%u frames=%u checks=%u succeeded=%d text=%d keys=%d mouse=%d quit=%d\n",
        mode_name,r,app.result,phase,native_frames,checks,succeeded,
        final_events.iTextEventCount-before_events.iTextEventCount,final_events.iKeyEventCount-before_events.iKeyEventCount,
        final_events.iMouseEventCount-before_events.iMouseEventCount,final_events.iQuitEventCount-before_events.iQuitEventCount);
    xgeSetQuitRequestCallback(NULL,NULL);
    if(app.close_box)xuiMsgBoxDestroy(app.close_box);
    if(app.file_dialog)xuiFileDialogDestroy(app.file_dialog);
    if(app.discard_box)xuiMsgBoxDestroy(app.discard_box);
    xuiDocumentRelease(app.pending_open);xuiDestroy(app.context);xuiDocumentRelease(app.rich_doc);xuiDocumentRelease(app.md_doc);
    if(app.proxy.surfaceDestroy)app.proxy.surfaceDestroy(&app.proxy,app.target);
    if(app.proxy.fontDestroy) {app.proxy.fontDestroy(&app.proxy,app.mono);app.proxy.fontDestroy(&app.proxy,app.italic);app.proxy.fontDestroy(&app.proxy,app.bold);app.proxy.fontDestroy(&app.proxy,app.font);}
    xgeFontFallbackClear();xgeUnit();
    XChangeKeyboardMapping(sender,first_keycode,original_width,original_keys,2);XSync(sender,False);
    XFree(original_keys);XCloseDisplay(sender);
    if(r==XGE_OK && succeeded && app.result==XUI_OK) {
        printf("Native X11 Document %s: queued Unicode before close, mouse cancel/discard, keyboard undo/redo, shared preview and window lifetime passed (%u checks).\n",mode_name,checks);return 0;
    }
    return 1;
}
