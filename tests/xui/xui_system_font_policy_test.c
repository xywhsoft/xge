#include "xui.h"
#include "xui_test_proxy.h"

#include <stddef.h>
#include <stdio.h>

#define CHECK(expr,message) do{if(!(expr)){printf("xui_system_font_policy_test failed: %s\n",message);return 1;}}while(0)

static int test_auto_lazy(void)
{
	xui_test_proxy_state_t state;
	xui_context context=NULL;
	xui_font font=NULL;
	xuiTestProxyInit(&state);
	CHECK(xuiCreate(&context)==XUI_OK,"create AUTO context");
	CHECK(xuiSetProxy(context,&state.tProxy)==XUI_OK,"bind AUTO proxy");
	CHECK(state.iSystemFontLoadCount==0,"AUTO must not load while binding");
	CHECK(xuiGetDefaultFont(context)!=NULL,"AUTO first use must load");
	CHECK(state.iSystemFontLoadCount==1,"AUTO must load exactly once");
	CHECK(xuiGetDefaultFont(context)!=NULL && state.iSystemFontLoadCount==1,"AUTO cache must be stable");
	CHECK(state.tProxy.fontLoadFile(&state.tProxy,&font,NULL,19.0f,0)==XUI_OK,"create custom font");
	CHECK(xuiSetDefaultFont(context,font)==XUI_OK,"replace automatic default");
	xuiDestroy(context);
	CHECK(state.iFontDestroyCount==1,"context must destroy its automatic font");
	state.tProxy.fontDestroy(&state.tProxy,font);
	CHECK(state.iFontDestroyCount==2,"custom raw font stays caller-owned");
	return 0;
}

static int test_auto_custom_first(void)
{
	xui_test_proxy_state_t state;
	xui_context context=NULL;
	xui_font font=NULL;
	xuiTestProxyInit(&state);
	CHECK(xuiCreate(&context)==XUI_OK,"create custom-first context");
	CHECK(xuiSetProxy(context,&state.tProxy)==XUI_OK,"bind custom-first proxy");
	CHECK(state.tProxy.fontLoadFile(&state.tProxy,&font,NULL,17.0f,0)==XUI_OK,"create custom-first font");
	CHECK(xuiSetDefaultFont(context,font)==XUI_OK,"set custom-first font");
	CHECK(xuiGetDefaultFont(context)==font,"custom font must remain default");
	CHECK(state.iSystemFontLoadCount==0,"custom-first AUTO must allocate no system font");
	xuiDestroy(context);
	CHECK(state.iFontDestroyCount==0,"raw custom font must not be context-owned");
	state.tProxy.fontDestroy(&state.tProxy,font);
	return 0;
}

static int test_disabled(void)
{
	xui_test_proxy_state_t state;
	xui_context_desc_t desc;
	xui_context context=NULL;
	xuiTestProxyInit(&state);
	xuiContextDescDefault(&desc);
	desc.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
	CHECK(xuiCreateEx(&desc,&context)==XUI_OK,"create DISABLED context");
	CHECK(xuiSetProxy(context,&state.tProxy)==XUI_OK,"bind DISABLED proxy");
	CHECK(xuiEnsureDefaultFont(context)==XUI_ERROR_UNSUPPORTED,"DISABLED ensure result");
	CHECK(xuiGetDefaultFont(context)==NULL,"DISABLED getter");
	CHECK(state.iSystemFontLoadCount==0,"DISABLED must never call backend");
	xuiDestroy(context);
	return 0;
}

static int test_eager_and_rollback(void)
{
	xui_test_proxy_state_t state;
	xui_context_desc_t desc;
	xui_context context=NULL;
	xui_proxy_t proxy;
	xuiTestProxyInit(&state);
	xuiContextDescDefault(&desc);
	desc.iSystemFontPolicy=XUI_SYSTEM_FONT_EAGER;
	desc.fSystemFontSize=18.0f;
	CHECK(xuiCreateEx(&desc,&context)==XUI_OK,"create EAGER context");
	xuiTestProxyFailSystemFontLoad(&state,XUI_ERROR_RESOURCE_FAILED);
	CHECK(xuiSetProxy(context,&state.tProxy)==XUI_ERROR_RESOURCE_FAILED,"EAGER failure must fail binding");
	CHECK(xuiGetProxy(context,&proxy)==XUI_ERROR_NOT_INITIALIZED,"failed EAGER binding must roll back");
	CHECK(xuiSetProxy(context,&state.tProxy)==XUI_OK,"EAGER retry binding");
	CHECK(state.iSystemFontLoadCount==2,"EAGER must load during each binding attempt");
	CHECK(xuiGetDefaultFont(context)!=NULL,"EAGER default font");
	xuiDestroy(context);
	CHECK(state.iFontDestroyCount==1,"EAGER automatic font cleanup");
	return 0;
}

static int test_previous_proxy_prefix(void)
{
	xui_test_proxy_state_t state;
	xui_context context=NULL;
	xuiTestProxyInit(&state);
	state.tProxy.iSize=(uint32_t)offsetof(xui_proxy_t,fontLoadSystem);
	CHECK(xuiCreate(&context)==XUI_OK,"create prefix context");
	CHECK(xuiSetProxy(context,&state.tProxy)==XUI_OK,"bind previous proxy prefix");
	CHECK(xuiGetDefaultFont(context)==NULL,"old proxy has no system resolver");
	CHECK(state.iSystemFontLoadCount==0,"old proxy prefix must not read appended callback");
	xuiDestroy(context);
	return 0;
}

int main(void)
{
	if(test_auto_lazy()!=0)return 1;
	if(test_auto_custom_first()!=0)return 1;
	if(test_disabled()!=0)return 1;
	if(test_eager_and_rollback()!=0)return 1;
	if(test_previous_proxy_prefix()!=0)return 1;
	printf("xui_system_font_policy_test passed\n");
	return 0;
}
