/* Select only the existing image groups. The original renderer main contains
 * GPU tests and is never invoked. The test-only release adapter waits at UI
 * checked boundaries because the old group assumes the former blocking
 * Release; the owning fixture separately proves the new nonblocking contract. */
#define main xui_unused_image_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#include "../../include/xui_document_ui.h"
static void imageRegressionRelease(xui_doc_image_request);
#define xuiDocumentImageResourceRelease imageRegressionRelease
#define main xui_unused_original_document_renderer_main
#include "xui_document_renderer_test.c"
#undef main
#undef xuiDocumentImageResourceRelease
#include "../../src/xui/document/xui_document_image_resource.c"
#include "../../src/xui/document/xui_document_image_async.c"

static void imageRegressionRelease(xui_doc_image_request job)
{
    xui_context context=xuiDocumentImageResourceGetContext(job);
    xuiDocumentImageResourceRelease(job);
    if(!context)return;
    for(unsigned i=0;i<10000;++i){
        bool pending=false;doc_image_index_enter();
        for(xui_doc_image_request scan=doc_image_requests;scan;scan=scan->next)
            if(scan->context==context && scan->owned.pending){pending=true;break;}
        doc_image_index_leave();if(!pending)return;
        xui_call_scope_t boundary={0};assert(xuiCallScopeBegin(context,&boundary)==XUI_OK && xuiCallScopeEnd(&boundary)==XUI_OK);
        xrtThreadYield();
    }
    assert(0 && "released image worker did not retire at checked UI boundaries");
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    xui_test_proxy_state_t proxy;xuiTestProxyInit(&proxy);xui_context context=NULL;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    image_resource_load(context,&proxy);image_resource_async(context);
    CHECK(xuiDestroyChecked(context)==XUI_OK);xrtClearError();
    puts("Original synchronous/async image assertions preserved; only blocking-release timing is awaited by the test harness");return 0;
}
