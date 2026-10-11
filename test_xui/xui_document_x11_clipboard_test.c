#define _POSIX_C_SOURCE 200809L
#include "xui_document_ui.h"
#include "xge.h"
#include "test_xui/xui_test_proxy.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { PUBLISH=1, READ, REFUSE, CLEAR, QUIT };
static int transfer(int fd, void* data, size_t bytes, int writing)
{
    unsigned char* p=data;
    while(bytes) {
        ssize_t n=writing?write(fd,p,bytes):read(fd,p,bytes);
        if(n<0 && errno==EINTR)continue;
        if(n<=0)return 0;
        p+=n;bytes-=(size_t)n;
    }
    return 1;
}
static int peer(int fd)
{
    Display* display=XOpenDisplay(NULL); Window window; Atom clipboard,utf8,targets,property;
    char* payload=NULL; char* expected=NULL; unsigned payload_bytes=0,expected_bytes=0; int refuse=0,running=1;
    if(!display)return 2;
    window=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0,1,1,0,0,0);
    clipboard=XInternAtom(display,"CLIPBOARD",False);utf8=XInternAtom(display,"UTF8_STRING",False);
    targets=XInternAtom(display,"TARGETS",False);property=XInternAtom(display,"XUI_CLIPBOARD_PROBE",False);
    while(running) {
        fd_set set;int xfd=ConnectionNumber(display);FD_ZERO(&set);FD_SET(fd,&set);FD_SET(xfd,&set);
        if(XPending(display)) FD_CLR(fd,&set);
        else if(select((fd>xfd?fd:xfd)+1,&set,NULL,NULL,NULL)<0) {if(errno==EINTR)continue;break;}
        if(FD_ISSET(fd,&set)) {
            unsigned header[2],ack=0;
            if(!transfer(fd,header,sizeof(header),0))break;
            char* text=malloc((size_t)header[1]+1u);if(!text)break;
            if(header[1] && !transfer(fd,text,header[1],0)){free(text);break;}text[header[1]]=0;
            switch(header[0]) {
            case PUBLISH: case REFUSE:
                free(payload);payload=text;payload_bytes=header[1];text=NULL;refuse=header[0]==REFUSE;
                XSetSelectionOwner(display,clipboard,window,CurrentTime);XSync(display,False);
                ack=XGetSelectionOwner(display,clipboard)==window?0u:1u;transfer(fd,&ack,sizeof(ack),1);break;
            case READ:
                free(expected);expected=text;expected_bytes=header[1];text=NULL;
                XConvertSelection(display,clipboard,utf8,property,window,CurrentTime);XFlush(display);break;
            case CLEAR:
                XSetSelectionOwner(display,clipboard,None,CurrentTime);XSync(display,False);transfer(fd,&ack,sizeof(ack),1);break;
            case QUIT: running=0;transfer(fd,&ack,sizeof(ack),1);break;
            default:running=0;break;
            }
            free(text);
        }
        while(XPending(display)) {
            XEvent event;XNextEvent(display,&event);
            if(event.type==SelectionRequest) {
                XSelectionRequestEvent* req=&event.xselectionrequest;XEvent reply={0};
                reply.xselection.type=SelectionNotify;reply.xselection.display=display;reply.xselection.requestor=req->requestor;
                reply.xselection.selection=req->selection;reply.xselection.target=req->target;reply.xselection.time=req->time;reply.xselection.property=None;
                Atom destination=req->property==None?req->target:req->property;
                if(!refuse && req->target==utf8) {
                    XChangeProperty(display,req->requestor,destination,utf8,8,PropModeReplace,(unsigned char*)payload,(int)payload_bytes);reply.xselection.property=destination;
                } else if(!refuse && req->target==targets) {
                    XChangeProperty(display,req->requestor,destination,XA_ATOM,32,PropModeReplace,(unsigned char*)&utf8,1);reply.xselection.property=destination;
                }
                XSendEvent(display,req->requestor,False,0,&reply);XFlush(display);
            } else if(event.type==SelectionNotify && expected) {
                unsigned reply[2]={1,0};unsigned char* data=NULL;Atom type;int format;unsigned long bytes,after;
                if(event.xselection.property!=None && XGetWindowProperty(display,window,property,0,LONG_MAX,True,AnyPropertyType,&type,&format,&bytes,&after,&data)==Success) {
                    reply[1]=(unsigned)bytes;
                    if(type==utf8 && format==8 && after==0 && bytes==expected_bytes && (!bytes || (data && !memcmp(data,expected,bytes))))reply[0]=0;
                }
                if(data)XFree(data);
                transfer(fd,reply,sizeof(reply),1);free(expected);expected=NULL;
            }
        }
    }
    free(payload);free(expected);XDestroyWindow(display,window);XCloseDisplay(display);close(fd);return 0;
}
static int socket_fd;static unsigned checks,failures,test_case;static int waiting;static unsigned expected_peer_bytes;
static xui_context context;static xui_widget editor;static xui_document document;static xui_test_proxy_state_t proxy;
static const char initial[]="before keep 中文 😀 tail\n",selected[]="keep 中文 😀",paste[]="pasted 中文 😀";
#define CHECK(e) do {checks++;if(!(e)){failures++;fprintf(stderr,"X11 clipboard case %u line %d: %s\n",test_case,__LINE__,#e);}}while(0)
static void send_command(unsigned op,const char* text,unsigned length,int ack)
{
    unsigned header[2]={op,length},result=1;
    CHECK(transfer(socket_fd,header,sizeof(header),1));if(length)CHECK(transfer(socket_fd,(void*)text,length,1));
    if(ack){CHECK(transfer(socket_fd,&result,sizeof(result),0));CHECK(result==0);}
}
static int source_is(const char* expected)
{
    xui_document_snapshot snapshot=NULL;uint64_t bytes=0;size_t length=strlen(expected);char* actual=malloc(length+1u);
    if(!actual)return 0;
    int result=xuiDocumentAcquireSnapshot(document,&snapshot);
    if(result==XUI_OK)result=xuiDocumentSnapshotCopySource(snapshot,actual,length+1u,&bytes);
    xuiDocumentSnapshotRelease(snapshot);int same=result==XUI_OK && bytes==length && !strcmp(actual,expected);free(actual);return same;
}
static void dispose_document(void)
{
    if(editor)xuiWidgetDestroy(editor);
    editor=NULL;
    if(context)xuiDestroy(context);
    context=NULL;xuiDocumentRelease(document);document=NULL;
}
static int setup(unsigned mode,const char* text,unsigned length,int whole_source)
{
    xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=mode?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH,.iMarkdownDialect=XUI_MD_GFM};
    xui_doc_editor_desc_t edit={0};xui_proxy_t native=xuiProxyXge();xui_doc_range_t range={0};xui_document_snapshot snapshot=NULL;uint64_t matches;int result;
    xuiTestProxyInit(&proxy);proxy.tProxy.clipboardSetText=native.clipboardSetText;proxy.tProxy.clipboardSetItems=native.clipboardSetItems;
    proxy.tProxy.clipboardGetText=native.clipboardGetText;proxy.tProxy.clipboardGetData=native.clipboardGetData;
    result=xuiCreate(&context);if(result==XUI_OK)result=xuiSetProxy(context,&proxy.tProxy);
    if(result==XUI_OK)result=xuiDocumentCreate(&desc,&document);
    if(result==XUI_OK && mode)result=xuiDocumentLoadMarkdown(document,text,length);
    edit.iSize=sizeof(edit);edit.tView.iSize=sizeof(edit.tView);edit.tView.pDocument=document;edit.iMode=mode==2?XUI_DOC_SOURCE_TEXT:mode==3?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
    edit.iAsyncSourceThresholdBytes=UINT64_MAX;
    if(result==XUI_OK)result=xuiDocumentEditorCreate(context,&edit,&editor);
    if(result==XUI_OK)result=xuiSetRootWidget(context,editor);
    if(result==XUI_OK && !mode)result=xuiDocumentEditorInsertText(editor,text,length);
    if(result==XUI_OK)result=xuiDocumentClearHistory(document);
    if(result==XUI_OK)result=xuiDocumentAcquireSnapshot(document,&snapshot);
    if(result==XUI_OK)result=xuiDocumentMarkSaved(document,snapshot);
    xuiDocumentSnapshotRelease(snapshot);snapshot=NULL;
    if(result==XUI_OK && (mode==2 || mode==3)) {
        result=xuiDocumentViewGetSelection(editor,&range);
        range.tAnchor.iOffset=whole_source?0:7;range.tCaret.iOffset=whole_source?length:7+strlen(selected);
    } else if(result==XUI_OK && whole_source) {
        return xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_SELECT_ALL);
    } else if(result==XUI_OK) {
        result=xuiDocumentAcquireSnapshot(document,&snapshot);
        if(result==XUI_OK)result=xuiDocumentSnapshotFind(snapshot,XUI_DOC_SEMANTIC,selected,strlen(selected),NULL,&range,1,&matches);
        if(result==XUI_OK && matches!=1)result=XUI_ERROR;
        xuiDocumentSnapshotRelease(snapshot);
    }
    if(result==XUI_OK)result=xuiDocumentViewSetSelection(editor,&range);
    return result;
}
static int frame(void* user)
{
    (void)user;
    if(waiting) {
        fd_set set;struct timeval timeout={0};FD_ZERO(&set);FD_SET(socket_fd,&set);
        if(select(socket_fd+1,&set,NULL,NULL,&timeout)<=0)return XGE_OK;
        unsigned result[2];CHECK(transfer(socket_fd,result,sizeof(result),0));CHECK(result[0]==0);CHECK(result[1]==expected_peer_bytes);
        printf("X11 clipboard case %u external read: result=%u bytes=%u expected=%u\n",test_case,result[0],result[1],expected_peer_bytes);
        waiting=0;dispose_document();test_case++;
    }
    if(test_case>=34){xgeQuit();return XGE_OK;}
    int result;unsigned mode=test_case/4,op=test_case%4;const char* expected=selected;unsigned length=(unsigned)strlen(selected);
    send_command(PUBLISH,"external seed",13,1);
    if(test_case<16) {
        CHECK(setup(mode,initial,strlen(initial),0)==XUI_OK);uint64_t revision=xuiDocumentGetRevision(document);
        if(op==2 || op==3)send_command(PUBLISH,op==2?paste:"",op==2?(unsigned)strlen(paste):0,1);
        result=xuiDocumentEditorExecute(editor,op==0?XUI_DOC_EDIT_COPY:op==1?XUI_DOC_EDIT_CUT:XUI_DOC_EDIT_PASTE);
        CHECK(result==XUI_OK);
        CHECK(xuiDocumentGetRevision(document)==revision+((op==1 || op==2)?1:0));
        if(mode)CHECK(source_is(op==1?"before  tail\n":op==2?"before pasted 中文 😀 tail\n":initial));
        if(op==1 || op==2){CHECK(xuiDocumentUndo(document,NULL)==XUI_OK);CHECK(!xuiDocumentIsDirty(document));if(mode)CHECK(source_is(initial));CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);}
        printf("X11 clipboard case %u mode=%u operation=%u result=%d\n",test_case,mode,op,result);
        if(op>=2){dispose_document();test_case++;return XGE_OK;}
    } else if(test_case==16) {
        xge_clipboard_item_t items[2]={{XGE_CLIPBOARD_FORMAT_TEXT_UTF8,selected,strlen(selected)},{XGE_CLIPBOARD_FORMAT_HTML,"<b>selected</b>",15}};
        result=xgeClipboardSetItems(items,2);CHECK(result==XGE_OK);printf("X11 multi-format first-text result=%d\n",result);
    } else if(test_case==17) {
        unsigned bytes=2u*1024u*1024u+17u;char* text=malloc((size_t)bytes+1u);char* combined=malloc((size_t)bytes+15u);char* actual=malloc((size_t)bytes+1u);
        CHECK(text && combined && actual);if(!text || !combined || !actual){free(text);free(combined);free(actual);xgeQuit();return XGE_OK;}
        memset(text,'p',bytes);text[bytes]=0;
        CHECK(setup(2,initial,strlen(initial),0)==XUI_OK);uint64_t revision=xuiDocumentGetRevision(document);
        send_command(PUBLISH,text,bytes,1);
        CHECK(xgeClipboardGetData(XGE_CLIPBOARD_FORMAT_TEXT_UTF8,NULL,0)==(int)bytes);
        CHECK(xgeClipboardGetData(XGE_CLIPBOARD_FORMAT_TEXT_UTF8,actual,bytes)==(int)bytes);CHECK(!memcmp(actual,text,bytes));
        result=xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_PASTE);CHECK(result==XUI_OK);CHECK(xuiDocumentGetRevision(document)==revision+1);
        memcpy(combined,"before ",7);memcpy(combined+7,text,bytes);memcpy(combined+7+bytes," tail\n",7);
        CHECK(source_is(combined));CHECK(xuiDocumentUndo(document,NULL)==XUI_OK);CHECK(source_is(initial));CHECK(!xuiDocumentIsDirty(document));
        CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);CHECK(source_is(combined));
        printf("X11 long external Paste: result=%d bytes=%u\n",result,bytes);
        free(text);free(combined);free(actual);dispose_document();test_case++;return XGE_OK;
    } else if(test_case==18 || test_case==19) {
        length=2u*1024u*1024u+17u;char* text=malloc((size_t)length+1u);CHECK(text!=NULL);if(!text){xgeQuit();return XGE_OK;}
        memset(text,'z',length);text[length]=0;
        if(test_case==18){xge_clipboard_item_t item={XGE_CLIPBOARD_FORMAT_TEXT_UTF8,text,length};result=xgeClipboardSetItems(&item,1);}
        else {CHECK(setup(2,text,length,1)==XUI_OK);result=xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_CUT);CHECK(source_is(""));CHECK(xuiDocumentUndo(document,NULL)==XUI_OK);CHECK(source_is(text));CHECK(!xuiDocumentIsDirty(document));CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);CHECK(source_is(""));}
        CHECK(result==XUI_OK);printf("X11 long text case %u result=%d bytes=%u\n",test_case,result,length);
        send_command(READ,text,length,0);free(text);expected_peer_bytes=length;waiting=1;return XGE_OK;
    } else if(test_case==20 || test_case==21) {
        CHECK(setup(2,initial,strlen(initial),0)==XUI_OK);uint64_t revision=xuiDocumentGetRevision(document);
        send_command(test_case==20?REFUSE:CLEAR,"",0,1);
        result=xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_PASTE);
        CHECK(result==(test_case==20?XGE_ERROR_BACKEND_FAILED:XUI_OK));CHECK(xuiDocumentGetRevision(document)==revision);CHECK(source_is(initial));CHECK(!xuiDocumentCanUndo(document));
        CHECK(!strcmp(xgeClipboardGetText(),""));
        printf("X11 clipboard unavailable/empty case %u result=%d\n",test_case,result);dispose_document();test_case++;return XGE_OK;
    } else if(test_case==27 || test_case==28 || test_case>=30) {
        const char* image=test_case>=32?"---\n":test_case>=30?"caption ![](missing.png)\n":"![](missing.png)\n";CHECK(setup(1,image,strlen(image),1)==XUI_OK);
        uint64_t revision=xuiDocumentGetRevision(document);xui_doc_range_t before,after;
        CHECK(xuiDocumentViewGetSelection(editor,&before)==XUI_OK);
        result=xuiDocumentEditorExecute(editor,(test_case==27 || test_case==30 || test_case==32)?XUI_DOC_EDIT_COPY:XUI_DOC_EDIT_CUT);CHECK(result==XGE_ERROR_UNSUPPORTED);
        CHECK(xuiDocumentGetRevision(document)==revision);CHECK(source_is(image));CHECK(!xuiDocumentCanUndo(document));CHECK(!xuiDocumentIsDirty(document));
        CHECK(xuiDocumentViewGetSelection(editor,&after)==XUI_OK);
        CHECK(before.tAnchor.iNodeId==after.tAnchor.iNodeId && before.tAnchor.iOffset==after.tAnchor.iOffset && before.tCaret.iNodeId==after.tCaret.iNodeId && before.tCaret.iOffset==after.tCaret.iOffset);
        printf("X11 text-only object clipboard case %u result=%d\n",test_case,result);
        if(result!=XGE_ERROR_UNSUPPORTED){dispose_document();test_case++;return XGE_OK;}
        expected="external seed";length=13;
    } else if(test_case==29) {
        xge_clipboard_item_t item={XGE_CLIPBOARD_FORMAT_TEXT_UTF8,"",0};result=xgeClipboardSetItems(&item,1);CHECK(result==XGE_OK);
        expected="";length=0;printf("X11 valid single empty text write result=%d\n",result);
    } else if(test_case>=25) {
        Display* display=XOpenDisplay(NULL);CHECK(display!=NULL);if(!display){xgeQuit();return XGE_OK;}
        long words=XExtendedMaxRequestSize(display);if(!words)words=XMaxRequestSize(display);XCloseDisplay(display);
        CHECK(words>8 && words<32L*1024L*1024L);if(words<=8 || words>=32L*1024L*1024L){xgeQuit();return XGE_OK;}
        /* Exceed the whole server request, independent of the backend header allowance. */
        unsigned bytes=(unsigned)words*4u+1u;char* text=malloc((size_t)bytes+1u);CHECK(text!=NULL);if(!text){xgeQuit();return XGE_OK;}
        memset(text,'q',bytes);text[bytes]=0;
        if(test_case==25){xge_clipboard_item_t item={XGE_CLIPBOARD_FORMAT_TEXT_UTF8,text,bytes};result=xgeClipboardSetItems(&item,1);}
        else {
            CHECK(setup(2,text,bytes,1)==XUI_OK);uint64_t revision=xuiDocumentGetRevision(document);xui_doc_range_t before,after;
            CHECK(xuiDocumentViewGetSelection(editor,&before)==XUI_OK);
            result=xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_CUT);
            CHECK(xuiDocumentGetRevision(document)==revision);CHECK(source_is(text));CHECK(!xuiDocumentCanUndo(document));CHECK(!xuiDocumentIsDirty(document));
            CHECK(xuiDocumentViewGetSelection(editor,&after)==XUI_OK);
            CHECK(before.tAnchor.iNodeId==after.tAnchor.iNodeId && before.tAnchor.iOffset==after.tAnchor.iOffset && before.tCaret.iNodeId==after.tCaret.iNodeId && before.tCaret.iOffset==after.tCaret.iOffset);
        }
        CHECK(result==XGE_ERROR_BUFFER_TOO_SMALL);printf("X11 over-request-limit case %u result=%d bytes=%u\n",test_case,result,bytes);free(text);
        /* A negative old-library run may publish data the protocol cannot send.
         * Record the wrong success and source mutation without requesting that oversized property. */
        if(result!=XGE_ERROR_BUFFER_TOO_SMALL){dispose_document();test_case++;return XGE_OK;}
        expected="external seed";length=13;
    } else {
        const char embedded[]={'a',0,'b'};
        xge_clipboard_item_t items[2]={{XGE_CLIPBOARD_FORMAT_HTML,"<b>selected</b>",15},{XGE_CLIPBOARD_FORMAT_TEXT_UTF8,selected,strlen(selected)}};
        if(test_case==24){items[1].pData=embedded;items[1].iDataSize=sizeof(embedded);}
        result=xgeClipboardSetItems(items,test_case==23?1:2);
        CHECK(result==(test_case==22?XGE_OK:test_case==23?XGE_ERROR_UNSUPPORTED:XGE_ERROR_INVALID_ARGUMENT));
        if(test_case!=22){expected="external seed";length=13;}
        printf("X11 first-text/rejected write case %u result=%d\n",test_case,result);
    }
    send_command(READ,expected,length,0);expected_peer_bytes=length;waiting=1;return XGE_OK;
}
int main(int argc,char** argv)
{
    if(argc!=2 || strcmp(argv[1],"--isolated-display") || !getenv("XUI_TEST_PRIVATE_DISPLAY")) {fprintf(stderr,"Run with the private-display test driver.\n");return 2;}
    setvbuf(stdout,NULL,_IOLBF,0);int sockets[2];if(socketpair(AF_UNIX,SOCK_STREAM,0,sockets))return 2;
    pid_t child=fork();if(child<0)return 2;if(!child){close(sockets[0]);_exit(peer(sockets[1]));}
    close(sockets[1]);socket_fd=sockets[0];xge_desc_t desc={.iRunMode=XGE_RUN_GAME_LOOP,.iWidth=32,.iHeight=32,.sTitle="Private X11 clipboard test",.iFlags=XGE_INIT_OFFSCREEN};
    int result=xgeInit(&desc);if(result==XGE_OK)result=xgeRun(frame,NULL);dispose_document();xgeUnit();send_command(QUIT,"",0,1);close(socket_fd);
    int status;waitpid(child,&status,0);CHECK(WIFEXITED(status) && WEXITSTATUS(status)==0);
    printf("Native X11 Document clipboard: %u completed cases, %u failures, %u checks.\n",test_case,failures,checks);
    return result==XGE_OK && !failures && test_case==34?0:1;
}
