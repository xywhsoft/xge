/* Reuse the original raw-provider contract tests with matched hosted XRT. */
#include "xui_code_margin_host.h"
#define main providerOriginalMain
#include "xui_code_provider_test.c"
#undef main
int main(void){marginBind();return providerOriginalMain();}
