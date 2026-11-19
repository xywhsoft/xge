#define XUI_OWNED_FONTS_HELPERS
#include "xui_owned_fonts_test.c"

static void assertTokenColor(xui_context context,const char* name,uint32_t color)
{
    xui_style_value_t value={0};
    assert(xuiStyleGetToken(context,name,&value)==XUI_OK);
    assert(value.iType==XUI_STYLE_VALUE_COLOR && value.iColor==color);
}

static void assertOriginalToken(xui_context context,const char* name)
{
    xui_style_value_t value={0};
    assert(xuiStyleGetToken(context,name,&value)==XUI_OK);
    assert(value.iType==XUI_STYLE_VALUE_STRING && !strcmp(value.sText,"old token"));
}

int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;
    xrtMemDebugSnapshot(&before);

    xui_test_proxy_state_t state;
    Owner owner={.refs=1};
    xui_proxy proxy=proxyCreate(&state,&owner);
    xui_context context=NULL;
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    assert(xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);
    unsigned finalized=0;
    xui_font first=fontCreate(proxy,&finalized,XUI_OK);
    xui_font second=fontCreate(proxy,&finalized,XUI_OK);
    assert(xuiSetDefaultFont(context,first)==XUI_OK);

    const xui_style_value_t old={.iSize=sizeof(old),.iType=XUI_STYLE_VALUE_STRING,.sText="old token"};
    assert(xuiStyleSetToken(context,"theme.text",&old)==XUI_OK);
    assert(xuiStyleSetToken(context,"chrome.backdrop",&old)==XUI_OK);
    const uint32_t generation=xuiStyleGetGeneration(context);
    const uint32_t tokens=xuiStyleGetTokenGeneration(context);

    xui_theme_t original,theme,checked;
    assert(xuiGetTheme(context,&original)==XUI_OK);
    theme=original;
    theme.pFont=second;
    theme.iTextColor=0x010203ff;
    theme.fBorderWidth=3;
    for(int budget=0;budget<26;++budget){
        xui_style_value_t absent={0};
        assert(xrtMemDebugFailAfter(budget));
        assert(xuiSetTheme(context,&theme)==XUI_ERROR_OUT_OF_MEMORY);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        assert(xuiGetTheme(context,&checked)==XUI_OK && checked.pFont==first);
        assert(checked.iTextColor==original.iTextColor && checked.fBorderWidth==original.fBorderWidth);
        assert(xuiGetDefaultFont(context)==first && refs(xuiFontOwnership(first))==2);
        assert(refs(xuiFontOwnership(second))==1 && !context->pActiveResourceUpdates);
        assertOriginalToken(context,"theme.text");
        assert(xuiStyleGetToken(context,"theme.border_width",&absent)==XUI_ERROR_FILE_NOT_FOUND);
        assert(xuiStyleGetGeneration(context)==generation && xuiStyleGetTokenGeneration(context)==tokens);
    }
    assert(xuiSetTheme(context,&theme)==XUI_OK);
    assert(xuiGetDefaultFont(context)==second && refs(xuiFontOwnership(first))==1 && refs(xuiFontOwnership(second))==2);
    assertTokenColor(context,"theme.text",theme.iTextColor);
    assert(xuiStyleGetGeneration(context)==generation+1 && xuiStyleGetTokenGeneration(context)==tokens+14);

    xui_chrome_style_t originalChrome,chrome,checkedChrome;
    assert(xuiGetChromeStyle(context,&originalChrome)==XUI_OK);
    chrome=originalChrome;
    chrome.iBackdropColor=0x01020340;
    chrome.fShadowSize=19;
    for(int budget=0;budget<14;++budget){
        xui_style_value_t absent={0};
        assert(xrtMemDebugFailAfter(budget));
        assert(xuiSetChromeStyle(context,&chrome)==XUI_ERROR_OUT_OF_MEMORY);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        assert(xuiGetChromeStyle(context,&checkedChrome)==XUI_OK);
        assert(checkedChrome.iBackdropColor==originalChrome.iBackdropColor && checkedChrome.fShadowSize==originalChrome.fShadowSize);
        assertOriginalToken(context,"chrome.backdrop");
        assert(xuiStyleGetToken(context,"chrome.shadow_size",&absent)==XUI_ERROR_FILE_NOT_FOUND);
        assert(xuiStyleGetGeneration(context)==generation+1 && xuiStyleGetTokenGeneration(context)==tokens+14);
    }
    assert(xuiSetChromeStyle(context,&chrome)==XUI_OK);
    assertTokenColor(context,"chrome.backdrop",chrome.iBackdropColor);
    assert(xuiStyleGetGeneration(context)==generation+2 && xuiStyleGetTokenGeneration(context)==tokens+22);

    xrtSetErrorKind(XERR_STATE);
    const xerror* ambient=xrtGetError();
    assert(ambient && xuiSetTheme(context,&theme)==XUI_OK && xrtGetError()==ambient);
    assert(xuiSetChromeStyle(context,&chrome)==XUI_OK && xrtGetError()==ambient);
    xrtClearError();

    xmemdebugsnapshot warmBefore,warmAfter;
    xrtMemDebugSnapshot(&warmBefore);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){
        assert(xuiSetTheme(context,&theme)==XUI_OK && xuiSetChromeStyle(context,&chrome)==XUI_OK);
    }
    assert(xrtMemDebugFailAfter(-1));
    xrtMemDebugSnapshot(&warmAfter);
    assert(warmBefore.AllocCount==warmAfter.AllocCount && warmBefore.FreeCount==warmAfter.FreeCount &&
        warmBefore.ReallocCount==warmAfter.ReallocCount);

    xui_font retiring=fontCreate(proxy,&finalized,XUI_ERROR_CALLBACK_FAILED);
    theme.pFont=retiring;
    assert(xuiSetTheme(context,&theme)==XUI_OK && xuiFontReleaseChecked(retiring)==XUI_OK);
    theme.pFont=second;
    theme.iTextColor=0xa0b0c0ff;
    assert(xuiSetTheme(context,&theme)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(finalized==1 && xuiGetTheme(context,&checked)==XUI_OK && checked.pFont==second);
    assert(checked.iTextColor==theme.iTextColor);
    assertTokenColor(context,"theme.text",theme.iTextColor);

    assert(xuiDestroyChecked(context)==XUI_OK && xuiFontReleaseChecked(first)==XUI_OK && xuiFontReleaseChecked(second)==XUI_OK);
    assert(finalized==3 && xuiProxyReleaseChecked(proxy)==XUI_OK && owner.refs==1 && drop(&owner));
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes &&
        before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Theme and Chrome: 40 allocation failures, coherent late failure, and 2000 warm updates passed");
    return 0;
}
