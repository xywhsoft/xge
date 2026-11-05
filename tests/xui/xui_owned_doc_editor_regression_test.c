/* Existing comprehensive offscreen native editor behavior, host-XRT bound. */
#define main xui_unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#define main xui_original_doc_editor_main
#include "xui_document_editor_test.c"
#undef main
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    return xui_original_doc_editor_main();
}
