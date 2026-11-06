/* No window or graphics device: the Context must preserve resource identity. */
#include "xui_internal.h"
#include <assert.h>

int main(void)
{
    xui_proxy owned = NULL;
    xui_context context = NULL;
    assert(xuiProxyXgeCreate(&owned) == XUI_OK);
    assert(xuiProxyIsOwned(owned));
    assert(xuiCreateOwned(owned, &context) == XUI_OK);
    assert(xuiGetProxyOwned(context) == owned);
    assert(xuiInternalContextGetProxy(context) == owned);
    assert(xuiProxyIsOwned(xuiInternalContextGetProxy(context)));
    assert(xuiDestroyChecked(context) == XUI_OK);
    assert(xuiProxyIsOwned(owned));
    assert(xuiProxyReleaseChecked(owned) == XUI_OK);

    /* Borrowed C backends still use the Context's stored snapshot. */
    xui_proxy_t borrowed = xuiProxyXge();
    context = NULL;
    assert(xuiCreate(&context) == XUI_OK);
    assert(xuiSetProxy(context, &borrowed) == XUI_OK);
    assert(xuiGetProxyOwned(context) == NULL);
    assert(xuiInternalContextGetProxy(context) != &borrowed);
    assert(!xuiProxyIsOwned(xuiInternalContextGetProxy(context)));
    assert(xuiInternalContextGetProxy(context)->surfaceCreate == borrowed.surfaceCreate);
    assert(xuiDestroyChecked(context) == XUI_OK);
    return 0;
}
