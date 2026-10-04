/* Reuse the established independent full-load/source/history test helpers. */
#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main
#include "xui_document_reference_sharing_cases.h"
int main(void)
{
    markdown_reference_range_sharing();
    markdown_reference_storage_shrink();
    markdown_reference_range_sharing_failures();
    return 0;
}
