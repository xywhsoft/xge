/* Standalone browser example: public HTML/URL APIs, no message bridge. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xge.h"
#include "xui.h"

typedef struct browser_demo browser_demo;
typedef struct browser_action { browser_demo* app; int command; } browser_action;
struct browser_demo {
    xui_context context; xui_proxy_t proxy; xui_surface target; xui_font font;
    xui_widget root, address, status, view, buttons[5]; browser_action actions[5];
    int width, height, verify, result, phase, frames, navigations;
    char profile[1024];
};
static const char home[] =
    "<!doctype html><html lang=zh-CN><meta charset=utf-8><title>XUI WebView</title>"
    "<meta name=viewport content='width=device-width,initial-scale=1'>"
    "<style>body{font:16px system-ui;margin:32px;color:#243247;background:#f4f7fb}"
    "main{max-width:720px;margin:auto;background:white;padding:24px;border-radius:12px}"
    "input,textarea{box-sizing:border-box;width:100%;padding:10px;margin:8px 0;font:inherit}"
    "details{margin:20px 0}a{color:#236dcc}</style><main>"
    "<h1>独立网页示例</h1><p>本页由系统浏览器内核显示，可输入、选择文字和打开链接。</p>"
    "<label>输入文字<input placeholder='试试输入中文'></label>"
    "<label>多行文本<textarea rows=4 placeholder='输入或粘贴内容'></textarea></label>"
    "<details><summary>展开更多内容</summary><p>HTML 与 URL 导航共享此网页控件。</p></details>"
    "<a href='about:blank'>打开空白页</a></main></html>";

static void status(browser_demo* d, const char* text) { xuiLabelSetText(d->status, text); }
static void operation_error(browser_demo* d, int error)
{
    char text[160]; snprintf(text, sizeof(text), "Browser operation returned %d (HRESULT %08x).", error,
        d->view ? (unsigned)xuiWebViewGetNativeError(d->view) : 0);
    status(d, text); fprintf(stderr, "%s\n", text);
    if (d->verify) { d->result = 1; xgeQuit(); }
}
static void browser_event(xui_widget widget, int event, int32_t error, void* user)
{
    browser_demo* d = user; int r;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        status(d, "Browser ready. Loading local page...");
        r = xuiWebViewLoadHtml(widget, home, sizeof(home)-1);
        if (r != XUI_OK) operation_error(d, r);
    } else if (event == XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE) {
        d->navigations++; status(d, "Page loaded. Click the page to type, or enter a URL above.");
    } else if (event == XUI_WEBVIEW_EVENT_FAILED || event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED) {
        char text[160]; snprintf(text, sizeof(text), "Browser failed (HRESULT %08x). Check WebView2 Runtime and URL.", (unsigned)error);
        status(d, text); fprintf(stderr, "%s\n", text);
        if (d->verify) { d->result=1; xgeQuit(); }
    } else if (event == XUI_WEBVIEW_EVENT_CLOSED) status(d, "Browser closed.");
}
static void click(xui_widget widget, void* user)
{
    browser_action* a = user; browser_demo* d = a->app; int r = XUI_OK; double zoom;
    (void)widget;
    if (a->command == 0) r = xuiWebViewNavigate(d->view, xuiInputGetText(d->address));
    else if (a->command == 1) r = xuiWebViewLoadHtml(d->view, home, sizeof(home)-1);
    else if (a->command == 2) r = xuiWebViewFocus(d->view);
    else {
        r = xuiWebViewGetZoomFactor(d->view, &zoom);
        if (r == XUI_OK) {
            zoom += a->command == 3 ? -.25 : .25;
            if (zoom < .25) zoom = .25;
            if (zoom > 5) zoom = 5;
            r = xuiWebViewSetZoomFactor(d->view, zoom);
        }
    }
    if (r != XUI_OK) operation_error(d, r);
}
static int layout(browser_demo* d, int width, int height)
{
    static const int offsets[]={382,320,220,154,80}, widths[]={56,94,60,68,70};
    xui_surface_desc_t desc={0}; xui_surface candidate=NULL; int i,r;
    if (width < 1 || height < 1) return XUI_OK;
    if (d->width == width && d->height == height) return XUI_OK;
    desc.iKind=XUI_SURFACE_KIND_TEXTURE; desc.iFormat=XUI_SURFACE_FORMAT_RGBA8;
    desc.iWidth=width; desc.iHeight=height; desc.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    r=d->proxy.surfaceCreate(&d->proxy,&candidate,&desc); if(r!=XUI_OK)return r;
    if(d->target)d->proxy.surfaceDestroy(&d->proxy,d->target);
    d->target=candidate; d->width=width; d->height=height;
    xuiInputViewport(d->context,width,height); xuiWidgetSetRect(d->root,(xui_rect_t){0,0,width,height});
    xuiWidgetSetRect(d->address,(xui_rect_t){10,10,width>432?width-402:30,28});
    for(i=0;i<5;i++)xuiWidgetSetRect(d->buttons[i],(xui_rect_t){width-offsets[i],10,widths[i],28});
    xuiWidgetSetRect(d->view,(xui_rect_t){10,50,width>20?width-20:1,height>88?height-88:1});
    xuiWidgetSetRect(d->status,(xui_rect_t){10,height-30,width>20?width-20:1,24});
    return XUI_OK;
}
static int setup(browser_demo* d)
{
    xui_webview_desc_t web={0}; xui_input_desc_t input={0}; xui_label_desc_t label={0};
    const char* names[] = {"Go","Local HTML","Focus","Zoom -","Zoom +"};
    const char* font=getenv("XUI_WEBVIEW_FONT"); int i,r;
    d->proxy=xuiProxyXge();
#define OK(e) do {r=(e);if(r!=XUI_OK)return r;} while(0)
    OK(xuiCreate(&d->context)); OK(xuiSetProxy(d->context,&d->proxy));
    OK(d->proxy.fontLoadFile(&d->proxy,&d->font,font&&*font?font:"C:\\Windows\\Fonts\\segoeui.ttf",16,XUI_FONT_FORMAT_TTF));
    OK(xuiSetDefaultFont(d->context,d->font)); OK(xuiWidgetCreate(d->context,&d->root)); OK(xuiSetRootWidget(d->context,d->root));
    input.iSize=sizeof(input);input.sText="about:blank";input.sPlaceholder="Enter a URL";input.pFont=d->font;
    OK(xuiInputCreate(d->context,&d->address,&input)); OK(xuiWidgetAddChild(d->root,d->address));
    label.iSize=sizeof(label);label.sText="Starting browser...";label.pFont=d->font;
    OK(xuiLabelCreate(d->context,&d->status,&label)); OK(xuiWidgetAddChild(d->root,d->status));
    for(i=0;i<5;i++) {
        xui_button_desc_t button={0};button.iSize=sizeof(button);button.sText=names[i];button.pFont=d->font;
        OK(xuiButtonCreate(d->context,&d->buttons[i],&button)); OK(xuiWidgetAddChild(d->root,d->buttons[i]));
        d->actions[i]=(browser_action){d,i}; OK(xuiButtonSetClick(d->buttons[i],click,&d->actions[i]));
    }
    web.iSize=sizeof(web);web.sUserDataFolder=d->profile;web.onEvent=browser_event;web.pUser=d;
    OK(xuiWebViewCreate(d->context,&d->view,&web)); OK(xuiWidgetAddChild(d->root,d->view));
    OK(layout(d,1024,720));
    r=xuiWebViewInitializeAsync(d->view);
    if(r!=XUI_OK)operation_error(d,r);
    return XUI_OK;
#undef OK
}
static int ui_click(browser_demo* d, int index)
{
    xui_rect_t rect=xuiWidgetGetWorldRect(d->buttons[index]);int r;
    r=xuiInputPointerMove(d->context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,0);
    if(r==XUI_OK)r=xuiInputPointerDown(d->context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT);
    if(r==XUI_OK)r=xuiInputPointerUp(d->context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,XUI_POINTER_BUTTON_LEFT,0);
    if(r==XUI_OK)r=xuiDispatchPendingEvents(d->context);
    return r;
}
static int capture_toolbar(browser_demo* d)
{
    unsigned char* pixels=malloc((size_t)d->width*d->height*4);int r;
    if(!pixels)return XUI_ERROR_OUT_OF_MEMORY;
    r=d->proxy.surfaceReadRGBA(&d->proxy,d->target,pixels,d->width*4);
    /* The native browser is a separate child surface. This captures only the
     * XGE toolbar, never claiming to capture the browser compositor. */
    if(r==XUI_OK)r=xgeImageSavePNG("webview-toolbar.png",d->width,d->height<48?d->height:48,pixels,d->width*4);
    free(pixels);return r;
}
static int frame(void* user)
{
    browser_demo* d=user; xui_rect_i_t damage; int r; double zoom;
    if(!d->context && (r=setup(d))!=XUI_OK)goto failed;
    if(d->verify && d->result) {xgeQuit();return XGE_OK;}
    if((r=layout(d,xgeGetWidth(),xgeGetHeight()))!=XUI_OK)goto failed;
    if((r=xuiProxyXgePumpInputRect(d->context,(xui_rect_t){0,0,d->width,d->height}))!=XUI_OK)goto failed;
    if((r=xuiDispatchPendingEvents(d->context))!=XUI_OK)goto failed;
    if((r=xuiLayout(d->context))!=XUI_OK || (r=xuiUpdate(d->context,.016f))!=XUI_OK)goto failed;
    if(d->verify) {
        if(++d->frames>1800) {r=XUI_DOC_ERROR_BUSY;goto failed;}
        if(!d->phase && d->navigations==1) {r=ui_click(d,0);d->phase=1;}
        else if(d->phase==1 && d->navigations==2) {r=ui_click(d,1);d->phase=2;}
        else if(d->phase==2 && d->navigations==3) {
            if((r=capture_toolbar(d))!=XUI_OK)goto failed;
            if((r=ui_click(d,4))!=XUI_OK || (r=xuiWebViewGetZoomFactor(d->view,&zoom))!=XUI_OK)goto failed;
            if(zoom!=1.25) {r=XUI_ERROR_INVALID_STATE;goto failed;}
            if((r=ui_click(d,2))!=XUI_OK)goto failed;
            if(xuiGetFocusWidget(d->context)!=d->view) {r=XUI_ERROR_INVALID_STATE;goto failed;}
            if((r=xuiWebViewClose(d->view))!=XUI_OK)goto failed;
            if(xuiWebViewGetState(d->view)!=XUI_WEBVIEW_CLOSED) {r=XUI_ERROR_INVALID_STATE;goto failed;}
            puts("Standalone WebView: local HTML, toolbar URL/local navigation, zoom, focus and close passed.");
            d->phase=3;xgeQuit();
        }
        if(r!=XUI_OK)goto failed;
    }
    if((r=xgeBegin())!=XGE_OK)goto failed;
    damage=(xui_rect_i_t){0,0,d->width,d->height};
    d->proxy.surfaceClear(&d->proxy,d->target,XUI_COLOR_RGBA(239,242,247,255));
    r=xuiRender(d->context,d->target,&damage,1);
    if(r==XUI_OK) {xgeClear(XUI_COLOR_WHITE);r=d->proxy.surfaceDraw(&d->proxy,d->target,(xui_rect_t){0,0,d->width,d->height},(xui_rect_t){0,0,d->width,d->height},XUI_COLOR_WHITE,XUI_SURFACE_DRAW_SCREEN_SPACE);}
    xgeEnd();if(r!=XUI_OK)goto failed;return XGE_OK;
failed:
    fprintf(stderr,"Standalone WebView failed: %d\n",r);d->result=1;xgeQuit();return XGE_OK;
}
int main(int argc,char** argv)
{
    browser_demo d={0};xge_desc_t desc={0};wchar_t base[MAX_PATH];DWORD length;int run;
    if(argc>2 || (argc==2 && strcmp(argv[1],"--verify"))) {fprintf(stderr,"Usage: xui_webview [--verify]\n");return 2;}
    d.verify=argc==2;
    if(d.verify) {
        length=GetTempPathW(MAX_PATH,base);
        if(!length || length>=MAX_PATH)return 2;
        if(!WideCharToMultiByte(CP_UTF8,0,base,-1,d.profile,sizeof(d.profile)-64,NULL,NULL))return 2;
        snprintf(d.profile+strlen(d.profile),64,"xui-webview-demo-%lu",(unsigned long)GetCurrentProcessId());
    } else {
        length=GetEnvironmentVariableW(L"LOCALAPPDATA",base,MAX_PATH);
        if(!length || length>=MAX_PATH)return 2;
        if(!WideCharToMultiByte(CP_UTF8,0,base,-1,d.profile,sizeof(d.profile)-64,NULL,NULL))return 2;
        strcat(d.profile,"\\XGE\\WebViewDemo");
    }
    desc.iWidth=1024;desc.iHeight=720;desc.sTitle="XUI WebView";desc.iRunMode=XGE_RUN_GAME_LOOP;desc.iTargetFPS=60;desc.iFlags=XGE_INIT_VSYNC;
    if(xgeInit(&desc)!=XGE_OK)return 2;
    run=xgeRun(frame,&d);
    if(d.context)xuiDestroy(d.context);
    if(d.target)d.proxy.surfaceDestroy(&d.proxy,d.target);
    if(d.font)d.proxy.fontDestroy(&d.proxy,d.font);
    xgeUnit();return run==XGE_OK && !d.result && (!d.verify || d.phase==3)?0:1;
}
