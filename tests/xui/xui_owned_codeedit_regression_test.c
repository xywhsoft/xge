/* Existing CPU/offscreen CodeEdit functional regression on the same real TU. */
#define main unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#include "../../src/xui/core/xui_text_internal.h"
#include "../../src/xui/code/xui_code_edit.c"
#define main native_codeedit_original_main
#include "xui_code_edit_test.c"
#undef main
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    return native_codeedit_original_main();
}
