/* Existing CPU/offscreen MessageList behavior on the actual owning-cache TU. */
#define main unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#include "../../src/xui/core/xui_text_internal.h"
#include "../../src/xui/document/xui_document_layout_internal.h"
#include "../../src/xui/controls/xui_message_list.c"
#define main native_message_original_main
#include "xui_message_list_test.c"
#undef main
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    return native_message_original_main();
}
