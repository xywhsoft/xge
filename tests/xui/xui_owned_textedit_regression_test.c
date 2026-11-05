/* Existing CPU/offscreen TextEdit behavior against the same owning-cache build. */
#define main unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#define main native_text_edit_original_main
#include "xui_text_edit_test.c"
#undef main
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    return native_text_edit_original_main();
}
