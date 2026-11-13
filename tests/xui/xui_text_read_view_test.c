/* Local CPU status/extent reads. Views borrow their model; no view is read
 * after mutation or physical owner release. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    static const char text[]={'A',0,(char)0xe4,(char)0xbd,(char)0xa0,'B'};
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xstrview view={(const char*)1,SIZE_MAX};
    assert(xuiCodeDocumentGetTextView(NULL,&view)==XUI_ERROR_INVALID_ARGUMENT && !view.Data && !view.Size);
    assert(xuiCodeDocumentGetTextView(NULL,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xuiCodeDocumentGetTextView(document,&view)==XUI_OK && view.Data && !view.Size);
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(xuiCodeDocumentSetTextLength(document,text,sizeof(text))==XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    int status=xuiCodeDocumentGetTextView(document,&view);
    bool failed=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
    assert(failed && status==XUI_ERROR_OUT_OF_MEMORY && !view.Data && !view.Size);
    assert(xuiCodeDocumentGetTextView(document,&view)==XUI_OK && view.Size==sizeof(text) && !memcmp(view.Data,text,sizeof(text)));
    const char* cached=view.Data;
    xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i){
        assert(xuiCodeDocumentGetTextView(document,&view)==XUI_OK);
        assert(view.Data==cached && view.Size==sizeof(text));
    }
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);

    xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);xui_context context=NULL;
    assert(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&backend.tProxy)==XUI_OK);
    xui_widget editor=NULL,input=NULL,ordinary=NULL;
    assert(xuiCodeEditCreate(context,&editor,NULL)==XUI_OK);
    assert(xuiCodeEditSetTextLength(editor,text,sizeof(text))==XUI_OK);
    assert(xuiCodeEditGetTextView(editor,&view)==XUI_OK && view.Size==sizeof(text) && !memcmp(view.Data,text,sizeof(text)));
    assert(xuiEditGetTextView(editor,&view)==XUI_OK && view.Size==sizeof(text) && !memcmp(view.Data,text,sizeof(text)));
    assert(xuiCodeEditGetTextView(editor,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeEditGetTextView(NULL,&view)==XUI_ERROR_INVALID_ARGUMENT && !view.Data && !view.Size);
    assert(xuiInputCreate(context,&input,NULL)==XUI_OK);
    assert(xuiInputSetText(input,"field")==XUI_OK);
    assert(xuiEditGetTextView(input,&view)==XUI_OK && view.Size==5 && !memcmp(view.Data,"field",5));
    assert(xuiWidgetCreate(context,&ordinary)==XUI_OK);
    assert(xuiEditGetTextView(ordinary,&view)==XUI_OK && !view.Data && !view.Size);
    assert(xuiEditGetTextView(NULL,&view)==XUI_OK && !view.Data && !view.Size);
    assert(xuiEditGetTextView(editor,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    xuiDestroy(context);xrtClearError();
    puts("Text read views: NUL/UTF-8 extents, native status, empty/absent views, cold snapshot OOM and 1000 zero-heap cached reads passed");
    return 0;
}
