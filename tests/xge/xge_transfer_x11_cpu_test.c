/* Exact production X11 selection responder, with all publication/responding
 * Xlib calls replaced by typed CPU fakes. Never opens a display or a window. */
#if defined(__linux__)
#define _POSIX_C_SOURCE 200809L
#define XRT_MODULE_ALL
#include "xge.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <assert.h>
#include <string.h>
static Window cpuOwner;
static int cpuSetCalls,cpuPropertyCalls,cpuSendCalls,cpuPropertyFormat,cpuPropertyCount;
static Atom cpuLastType,cpuLastProperty;
static unsigned char cpuBytes[2048];
static XSelectionEvent cpuReply;
static char cpuNames[128][128];static int cpuNameCount;
static Atom cpuInternAtom(Display* display,const char* name,Bool only)
{(void)display;(void)only;for(int i=0;i<cpuNameCount;++i)if(!strcmp(name,cpuNames[i]))return (Atom)i+100;assert(cpuNameCount<128 && strlen(name)<128);strcpy(cpuNames[cpuNameCount],name);return (Atom)cpuNameCount+++100;}
static int cpuSetSelectionOwner(Display* display,Atom selection,Window owner,Time time)
{(void)display;(void)selection;(void)time;cpuSetCalls++;cpuOwner=owner;return 0;}
static Window cpuGetSelectionOwner(Display* display,Atom selection){(void)display;(void)selection;return cpuOwner;}
static long cpuMaxRequestSize(Display* display){(void)display;return 128;}
static int cpuChangeProperty(Display* display,Window window,Atom property,Atom type,int format,int mode,const unsigned char* bytes,int count)
{(void)display;(void)window;(void)mode;cpuPropertyCalls++;cpuLastProperty=property;cpuLastType=type;cpuPropertyFormat=format;cpuPropertyCount=count;
 size_t size=(size_t)count*(format==32 ? sizeof(Atom):1u);assert(size<=sizeof(cpuBytes));memcpy(cpuBytes,bytes,size);return 0;}
static Status cpuSendEvent(Display* display,Window window,Bool propagate,long mask,XEvent* event)
{(void)display;(void)window;(void)propagate;(void)mask;cpuSendCalls++;memcpy(&cpuReply,(const void*)event,sizeof(cpuReply));return 1;}
static int cpuFlush(Display* display){(void)display;return 0;}
#define XInternAtom cpuInternAtom
#define XSetSelectionOwner cpuSetSelectionOwner
#define XGetSelectionOwner cpuGetSelectionOwner
#define XExtendedMaxRequestSize cpuMaxRequestSize
#define XMaxRequestSize cpuMaxRequestSize
#define XChangeProperty cpuChangeProperty
#define XSendEvent cpuSendEvent
#define XFlush cpuFlush
#include "../../single/xge.c"
#undef NDEBUG
#include <assert.h>
static void cpuRequest(Atom target,Atom property)
{
    XEvent event={0};event.xselectionrequest.type=SelectionRequest;event.xselectionrequest.display=_sapp.x11.display;
    event.xselectionrequest.owner=_sapp.x11.window;event.xselectionrequest.requestor=19;
    event.xselectionrequest.selection=_sapp.x11.CLIPBOARD;event.xselectionrequest.target=target;event.xselectionrequest.property=property;
    int sent=cpuSendCalls;_sapp_x11_on_selectionrequest(&event);assert(cpuSendCalls==sent+1);
}
int main(void)
{
    _sapp.x11.display=(Display*)1;_sapp.x11.window=7;
    _sapp.x11.CLIPBOARD=cpuInternAtom(NULL,"CLIPBOARD",False);
    _sapp.x11.TARGETS=cpuInternAtom(NULL,"TARGETS",False);
    _sapp.x11.UTF8_STRING=cpuInternAtom(NULL,"UTF8_STRING",False);
    _sapp.clipboard.enabled=true;_sapp.clipboard.buf_size=2;_sapp.clipboard.buffer=xrtMalloc(2);_sapp.clipboard.buffer[0]=0;
    _sapp.desc.allocator.alloc_fn=__xgeSokolAlloc;_sapp.desc.allocator.free_fn=__xgeSokolFree;
    g_xge.bSokolRunning=1;
    char text[100];memset(text,'a',sizeof(text));
    xge_clipboard_item_t items[]={{XGE_CLIPBOARD_FORMAT_TEXT_UTF8,text,sizeof(text)},{"application/x-native-binary","a\0b",3}};
    assert(xgeClipboardSetItems(items,2)==XGE_OK && cpuSetCalls==1);
    text[0]='x';
    cpuRequest(_sapp.x11.TARGETS,42);assert(cpuReply.property==42 && cpuPropertyFormat==32 && cpuPropertyCount==4 && cpuLastType==XA_ATOM);
    Atom binary=cpuInternAtom(NULL,items[1].sFormat,False);cpuRequest(binary,None);
    assert(cpuReply.property==binary && cpuLastProperty==binary && cpuLastType==binary && cpuPropertyFormat==8 && cpuPropertyCount==3 && !memcmp(cpuBytes,"a\0b",3));
    cpuRequest(_sapp.x11.UTF8_STRING,42);assert(cpuPropertyCount==100 && cpuBytes[0]=='a');
    char copied[3]={0};assert(xgeClipboardGetData(items[1].sFormat,copied,3)==3 && !memcmp(copied,"a\0b",3));
    xge_x11_clipboard_snapshot_t* old=g_xgeX11Clipboard;
    assert(xrtMemDebugFailAfter(0));assert(xgeClipboardSetItems(items,2)==XGE_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();
    assert(g_xgeX11Clipboard==old && cpuSetCalls==1);
    items[1].iDataSize=1000;assert(xgeClipboardSetItems(items,2)==XGE_ERROR_BUFFER_TOO_SMALL && cpuSetCalls==1);items[1].iDataSize=3;
    int properties=cpuPropertyCalls;cpuRequest(cpuInternAtom(NULL,"unknown",False),42);assert(cpuReply.property==None && cpuPropertyCalls==properties);
    cpuOwner=8;cpuRequest(binary,42);assert(cpuReply.property==None && cpuPropertyCalls==properties);
    __xgeClipboardPlatformCleanup();assert(!g_xgeX11Clipboard);
    xrtFree(_sapp.clipboard.buffer);_sapp.clipboard.buffer=NULL;return 0;
}
#else
int main(void){return 0;}
#endif
