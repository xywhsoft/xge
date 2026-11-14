/* Actual owned Font/paint Layout; no fake layout implementation or window. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"

static void queryCase(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    const char text[]={'a',0,'b'};
    xui_text_layout_desc_t desc=layoutDesc(font,text);desc.iTextSize=sizeof(text);
    assert(xuiTextLayoutCreate(p.context,&p.layout,&desc)==XUI_OK);
    /* Historical API cannot describe these complete source bytes. */
    assert(strlen(xuiTextLayoutGetText(p.layout))==1);
#ifdef XUI_TEST_LAYOUT_QUERIES_FIXED
    xstrview view={0};xui_vec2_t size={-1,-1};int lines=-1,truncated=-1;
    assert(xuiTextLayoutGetTextView(p.layout,&view)==XUI_OK && view.Size==sizeof(text));
    assert(!memcmp(view.Data,text,sizeof(text)));
    assert(xuiTextLayoutGetState(p.layout,&size,&lines,&truncated)==XUI_OK);
    xui_vec2_t legacy=xuiTextLayoutGetSize(p.layout);
    assert(size.fX==legacy.fX && size.fY==legacy.fY);
    assert(lines==xuiTextLayoutGetLineCount(p.layout) && truncated==xuiTextLayoutGetTruncated(p.layout));
    assert(xuiTextLayoutGetState(p.layout,NULL,NULL,NULL)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){
        assert(xuiTextLayoutGetState(p.layout,&size,&lines,&truncated)==XUI_OK);
        assert(xuiTextLayoutGetTextView(p.layout,&view)==XUI_OK && view.Size==sizeof(text));
    }
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(xuiTextLayoutGetTextView(p.layout,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    size=(xui_vec2_t){101,102};lines=103;truncated=104;
    assert(xuiTextLayoutGetState(NULL,&size,&lines,&truncated)==XUI_ERROR_INVALID_ARGUMENT);
    assert(size.fX==101 && size.fY==102 && lines==103 && truncated==104);
    view=(xstrview){text,sizeof(text)};
    assert(xuiTextLayoutGetTextView(NULL,&view)==XUI_ERROR_INVALID_ARGUMENT && !view.Data && !view.Size);
    assert(xuiSetVirtualDpi(p.context,1.5f)==XUI_OK);
    assert(xrtMemDebugFailAfter(0));int status=xuiTextLayoutGetState(p.layout,&size,&lines,&truncated);
    xrtMemDebugFailClear();xrtClearError();assert(status==XUI_ERROR_OUT_OF_MEMORY);
    assert(size.fX==101 && size.fY==102 && lines==103 && truncated==104);
    assert(xuiTextLayoutGetTextView(p.layout,&view)==XUI_OK && view.Size==sizeof(text) && !memcmp(view.Data,text,sizeof(text)));
    assert(xuiSetVirtualDpi(p.context,2)==XUI_OK);view=(xstrview){text,sizeof(text)};
    assert(xrtMemDebugFailAfter(0));status=xuiTextLayoutGetTextView(p.layout,&view);
    xrtMemDebugFailClear();xrtClearError();assert(status==XUI_ERROR_OUT_OF_MEMORY && !view.Data && !view.Size);
    assert(xuiTextLayoutGetState(p.layout,&size,&lines,&truncated)==XUI_OK);
    desc.sText="";desc.iTextSize=0;assert(xuiTextLayoutReset(p.layout,&desc)==XUI_OK);
    assert(xuiTextLayoutGetTextView(p.layout,&view)==XUI_OK && view.Data && !view.Size);
    size=(xui_vec2_t){101,102};lines=103;truncated=104;
#endif
    assert(xuiDestroyChecked(p.context)==XUI_OK && !xuiTextLayoutGetContext(p.layout));
    /* Old scalar ABI silently reports zero for an expired Context. */
    assert(!xuiTextLayoutGetSize(p.layout).fX && !xuiTextLayoutGetLineCount(p.layout) && !xrtGetError());
#ifdef XUI_TEST_LAYOUT_QUERIES_FIXED
    assert(xuiTextLayoutGetState(p.layout,&size,&lines,&truncated)==XUI_ERROR_INVALID_STATE);
    assert(size.fX==101 && size.fY==102 && lines==103 && truncated==104);
    assert(xuiTextLayoutGetTextView(p.layout,&view)==XUI_ERROR_INVALID_STATE && !view.Data && !view.Size);
#endif
    assert(xuiTextLayoutDestroyChecked(p.layout)==XUI_OK && p.paintDrops==p.shapes);
    assert(refs(xuiFontOwnership(font))==1);
    assert(xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}

int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    queryCase();layoutBasicAndReset();layoutDrawReentry();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
#ifdef XUI_TEST_LAYOUT_QUERIES_FIXED
    puts("Layout queries: sized NUL/empty source, checked state/expired Context, DPI OOM rollback, 1000 zero-heap reads and unique cleanup passed");
#else
    puts("Legacy layout limits reproduced: NUL text truncates, expired scalar reads silently return zero");
#endif
    return 0;
}
