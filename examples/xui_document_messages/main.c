#include "xui_document_ui.h"
#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct demo demo;
typedef struct action { demo* app; int command; } action;
struct demo {
    xui_context context; xui_proxy_t proxy; xui_surface target;
    xui_font font, bold, italic, mono;
    xui_widget root, list, editor, status, buttons[4];
    xui_document rich, reply, draft; action actions[4];
    int width, height, verify, result, phase, frames, checks;
    float reply_height, draft_y;
    uint64_t pixels_before;
};
static const char reply_text[] =
    "## Markdown reply\n\n**One kernel**, rendered inside a message.\n\n"
    "| Format | Renderer |\n| --- | --- |\n| Rich | Document |\n| Markdown | Document |\n\n"
    "```c\nxuiMessageListSetNodeDocument(list, id, &binding);\n```\n\n"
    "> 中文与 Markdown 在同一消息列表中显示。\n";
static const char draft_text[] = "Edit this **shared draft**.\n";
static const char typed_text[] = "中文";
static const char extra_text[] =
    "\n### Local update\n\nThe host appended this text through one Document transaction.\n\n"
    "- The message keeps its binding.\n- Its height and the following row update.\n"
    "- Scroll to read the result.\n\nThis example does not contact an agent service.\n";

#define TRY(expr) do { int result_ = (expr); if (result_ != XUI_OK) { \
    fprintf(stderr, "line %d: %s returned %d\n", __LINE__, #expr, result_); return result_; } } while (0)
#define VERIFY(expr) do { d->checks++; if (!(expr)) { \
    fprintf(stderr, "verification line %d: %s\n", __LINE__, #expr); return XUI_ERROR_INVALID_STATE; } } while (0)

static int copy_source(xui_document document, char* text, size_t capacity)
{
    xui_document_snapshot snapshot = NULL; uint64_t bytes; int result;
    TRY(xuiDocumentAcquireSnapshot(document, &snapshot));
    result = xuiDocumentSnapshotCopySource(snapshot, text, capacity, &bytes);
    xuiDocumentSnapshotRelease(snapshot); return result;
}
static int append_reply(demo* d)
{
    xui_document_snapshot snapshot = NULL; xui_document_transaction transaction = NULL;
    xui_doc_txn_desc_t desc = {0};
    uint64_t bytes; int result;
    TRY(xuiDocumentAcquireSnapshot(d->reply, &snapshot));
    result = xuiDocumentSnapshotCopySource(snapshot, NULL, 0, &bytes);
    xuiDocumentSnapshotRelease(snapshot); if (result != XUI_OK) return result;
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
    TRY(xuiDocumentBeginTransaction(d->reply, &desc, &transaction));
    result = xuiDocumentTxnReplaceSource(transaction, bytes, bytes, extra_text, sizeof(extra_text)-1);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
    xuiDocumentTxnRelease(transaction); return result;
}
static void click(xui_widget widget, void* user)
{
    action* a = user; demo* d = a->app; int result;
    (void)widget;
    if (a->command == 3) result = append_reply(d);
    else result = xuiDocumentViewSetMode(d->editor, a->command == 0 ? XUI_DOC_SOURCE_TEXT :
        a->command == 1 ? XUI_DOC_VISUAL : XUI_DOC_LIVE_MARKDOWN);
    if (result == XUI_OK) result = xuiSetFocusWidget(d->context, d->editor);
    if (result != XUI_OK) {
        char text[120]; snprintf(text, sizeof(text), "Operation returned %d. Current content retained.", result);
        xuiLabelSetText(d->status, text);
        fprintf(stderr,"%s (command %d)\n",text,a->command);
        if (d->verify) { d->result = 1; xgeQuit(); }
    }
}
static int layout(demo* d, int width, int height)
{
    xui_surface_desc_t surface = {0}; xui_surface candidate = NULL; int i;
    if (width < 1 || height < 1 || (width == d->width && height == d->height)) return XUI_OK;
    surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
    surface.iWidth = width; surface.iHeight = height;
    surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    TRY(d->proxy.surfaceCreate(&d->proxy, &candidate, &surface));
    if (d->target) d->proxy.surfaceDestroy(&d->proxy, d->target);
    d->target = candidate; d->width = width; d->height = height;
    TRY(xuiInputViewport(d->context, width, height));
    TRY(xuiWidgetSetRect(d->root, (xui_rect_t){0,0,width,height}));
    TRY(xuiWidgetSetRect(d->list, (xui_rect_t){16,52,width-32,height-278}));
    TRY(xuiWidgetSetRect(d->editor, (xui_rect_t){16,height-180,width-32,128}));
    for (i = 0; i < 4; i++) TRY(xuiWidgetSetRect(d->buttons[i], (xui_rect_t){16+i*116,height-216,108,28}));
    return xuiWidgetSetRect(d->status, (xui_rect_t){16,height-40,width-32,28});
}
static int make_rich(demo* d)
{
    xui_document_transaction transaction = NULL; xui_doc_node_desc_t node = {0};
    xui_doc_node_id paragraph, text_id; const char* text[] = {
        "Native rich text message", "这是富文本。消息列表与独立文档视图共用渲染器。"};
    int i, result = XUI_OK;
    TRY(xuiDocumentCreate(NULL, &d->rich));
    TRY(xuiDocumentBeginTransaction(d->rich, NULL, &transaction));
    for (i = 0; i < 2 && result == XUI_OK; i++) {
        memset(&node, 0, sizeof(node)); node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
        result = xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT, XUI_DOCUMENT_APPEND, &node, &paragraph);
        if (result != XUI_OK) break;
        node.iKind = XUI_DOC_TEXT; node.sText = text[i]; node.iTextBytes = strlen(text[i]);
        node.tAttributes.iMarks = i == 0 ? XUI_DOC_BOLD : 0;
        result = xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &text_id);
    }
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
    xuiDocumentTxnRelease(transaction); return result;
}
static int setup(demo* d)
{
    xui_doc_desc_t document = {0}; xui_message_list_desc_t list = {0};
    xui_message_document_desc_t binding = {0}; xui_doc_editor_desc_t editor = {0};
    xui_message_node_t nodes[3] = {{0}}; xui_label_desc_t label = {0}; xui_widget caption;
    const char* names[] = {"Source", "Visual", "Live MD", "Append reply"};
    const char* ids[] = {"rich", "reply", "draft"}; xui_document docs[3]; int i;
#ifdef _WIN32
    const char* normal = "C:\\Windows\\Fonts\\segoeui.ttf";
    const char* bold = "C:\\Windows\\Fonts\\segoeuib.ttf";
    const char* italic = "C:\\Windows\\Fonts\\segoeuii.ttf";
    const char* mono = "C:\\Windows\\Fonts\\consola.ttf";
#else
    const char* normal = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    const char* bold = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
    const char* italic = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Oblique.ttf";
    const char* mono = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
#endif
    const char* fallback = getenv("XUI_DOCUMENT_CJK_FONT");
    if (fallback && *fallback) TRY(xgeFontFallbackSet(fallback, 16));
#ifdef _WIN32
    else if (xgeFontFallbackSet("C:\\Windows\\Fonts\\msyh.ttc", 16) != XGE_OK &&
             xgeFontFallbackSet("C:\\Windows\\Fonts\\simsun.ttc", 16) != XGE_OK)
        fputs("Chinese fallback unavailable; set XUI_DOCUMENT_CJK_FONT.\n", stderr);
#endif
    d->proxy = xuiProxyXge(); TRY(xuiCreate(&d->context)); TRY(xuiSetProxy(d->context, &d->proxy));
    TRY(d->proxy.fontLoadFile(&d->proxy, &d->font, normal, 16, XUI_FONT_FORMAT_TTF));
    TRY(d->proxy.fontLoadFile(&d->proxy, &d->bold, bold, 16, XUI_FONT_FORMAT_TTF));
    TRY(d->proxy.fontLoadFile(&d->proxy, &d->italic, italic, 16, XUI_FONT_FORMAT_TTF));
    TRY(d->proxy.fontLoadFile(&d->proxy, &d->mono, mono, 15, XUI_FONT_FORMAT_TTF));
    TRY(xuiSetDefaultFont(d->context, d->font));
    TRY(xuiWidgetCreate(d->context, &d->root)); TRY(xuiSetRootWidget(d->context, d->root));
    label.iSize = sizeof(label); label.pFont = d->font;
    label.sText = "Document messages / native Rich + Markdown / shared editable draft";
    TRY(xuiLabelCreate(d->context, &caption, &label)); TRY(xuiWidgetAddChild(d->root, caption));
    TRY(xuiWidgetSetRect(caption, (xui_rect_t){16,12,1000,28}));
    label.sText = "Select message text | Ctrl+C copies | Draft changes appear in the You message";
    TRY(xuiLabelCreate(d->context, &d->status, &label)); TRY(xuiWidgetAddChild(d->root, d->status));
    TRY(make_rich(d)); document.iSize = sizeof(document); document.iProfile = XUI_DOCUMENT_MARKDOWN;
    document.iMarkdownDialect = XUI_MD_GFM;
    TRY(xuiDocumentCreate(&document, &d->reply)); TRY(xuiDocumentCreate(&document, &d->draft));
    TRY(xuiDocumentLoadMarkdown(d->reply, reply_text, sizeof(reply_text)-1));
    TRY(xuiDocumentLoadMarkdown(d->draft, draft_text, sizeof(draft_text)-1));
    docs[0] = d->rich; docs[1] = d->reply; docs[2] = d->draft;
    for (i = 0; i < 3; i++) {
        nodes[i].iSize = sizeof(nodes[i]); nodes[i].sId = ids[i];
        nodes[i].sSender = i == 2 ? "You / shared draft" : "Document example";
        nodes[i].sText = "Document content";
        nodes[i].iType = i == 2 ? XUI_MESSAGE_NODE_SELF : XUI_MESSAGE_NODE_OTHER;
    }
    list.iSize = sizeof(list); list.arrNodes = nodes; list.iNodeCount = 3; list.pFont = d->font;
    TRY(xuiMessageListCreate(d->context, &d->list, &list)); TRY(xuiWidgetAddChild(d->root, d->list));
    binding.iSize = sizeof(binding); binding.tRenderer.iSize = sizeof(binding.tRenderer);
    binding.tRenderer.tFonts = (xui_doc_font_set_t){d->font,d->bold,d->italic,d->bold,d->mono};
    for (i = 0; i < 3; i++) { binding.pDocument = docs[i]; TRY(xuiMessageListSetNodeDocument(d->list, ids[i], &binding)); }
    editor.iSize = sizeof(editor); editor.tView.iSize = sizeof(editor.tView);
    editor.tView.pDocument = d->draft; editor.tView.tRenderer = binding.tRenderer;
    editor.iMode = XUI_DOC_SOURCE_TEXT;
    TRY(xuiDocumentEditorCreate(d->context, &editor, &d->editor)); TRY(xuiWidgetAddChild(d->root, d->editor));
    for (i = 0; i < 4; i++) {
        xui_button_desc_t button = {0}; button.iSize = sizeof(button); button.sText = names[i]; button.pFont = d->font;
        TRY(xuiButtonCreate(d->context, &d->buttons[i], &button)); d->actions[i] = (action){d,i};
        TRY(xuiButtonSetClick(d->buttons[i], click, &d->actions[i]));
        TRY(xuiWidgetSetFocusable(d->buttons[i], 0)); TRY(xuiWidgetAddChild(d->root, d->buttons[i]));
    }
    TRY(layout(d,1080,760)); return xuiSetFocusWidget(d->context, d->editor);
}
static int pointer(demo* d, int type, float x, float y, uint32_t buttons)
{
    int result;
    if (type == 0) result = xuiInputPointerDown(d->context,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
    else if (type == 1) result = xuiInputPointerMove(d->context,x,y,buttons);
    else result = xuiInputPointerUp(d->context,x,y,XUI_POINTER_BUTTON_LEFT,buttons);
    if (result == XUI_OK) result = xuiDispatchPendingEvents(d->context);
    return result;
}
static int button_click(demo* d, int index)
{
    xui_rect_t rect = xuiWidgetGetWorldRect(d->buttons[index]); float x = rect.fX+rect.fW/2, y = rect.fY+rect.fH/2;
    TRY(pointer(d,0,x,y,XUI_POINTER_BUTTON_LEFT)); return pointer(d,2,x,y,0);
}
static int capture(demo* d, const char* path, uint64_t* hash)
{
    size_t bytes = (size_t)d->width*d->height*4, i;
    unsigned char* pixels = malloc(bytes); uint64_t value = 1469598103934665603ULL; int result;
    if (!pixels) return XUI_ERROR_OUT_OF_MEMORY;
    result = d->proxy.surfaceReadRGBA(&d->proxy,d->target,pixels,d->width*4);
    if (result == XUI_OK) {
        for (i=0;i<bytes;i++) value = (value ^ pixels[i])*1099511628211ULL;
        result = xgeImageSavePNG(path,d->width,d->height,pixels,d->width*4);
    }
    if (hash) *hash=value;
    free(pixels); return result;
}
static int verify(demo* d)
{
    char source[1024], selected[256]; uint64_t revision, pixels;
    xui_doc_position_t hit; xui_doc_range_t selection;
    xui_doc_renderer_stats_t stats = {0}; xui_message_list_metrics_t metrics = {0};
    xui_rect_t world, content, bubble; float x,y;
    if (++d->frames > 180) return XUI_DOC_ERROR_BUSY;
    if (d->frames < 3) return XUI_OK;
    if (d->phase == 0) {
        world=xuiWidgetGetWorldRect(d->list); bubble=xuiWidgetGetWorldRect(d->buttons[0]);
        VERIFY(world.fY+world.fH <= bubble.fY);
        content=xuiWidgetGetWorldRect(d->editor); VERIFY(bubble.fY+bubble.fH <= content.fY);
        VERIFY(xuiMessageListGetNodeCount(d->list) == 3);
        VERIFY(xuiMessageListGetNodeDocument(d->list,"rich") == d->rich);
        VERIFY(xuiMessageListGetNodeDocument(d->list,"reply") == d->reply);
        VERIFY(xuiMessageListGetNodeDocument(d->list,"draft") == d->draft);
        stats.iSize = sizeof(stats); TRY(xuiMessageListGetNodeDocumentRenderStats(d->list,"reply",&stats));
        VERIFY(stats.iBlocks >= 3 && stats.iShapedBytes > 0);
        TRY(capture(d,"document-messages-initial.png",&d->pixels_before));
        TRY(xuiMessageListSetScroll(d->list,0)); metrics.iSize = sizeof(metrics);
        TRY(xuiMessageListGetMetrics(d->list,&metrics));
        world=xuiWidgetGetWorldRect(d->list); content=xuiWidgetGetContentRect(d->list);
        bubble=xuiMessageListGetBubbleRect(d->list,0);
        x=world.fX+content.fX+bubble.fX+metrics.fBubblePaddingX+2;
        y=world.fY+content.fY+bubble.fY+metrics.fBubblePaddingY+8;
        TRY(xuiMessageListHitNodeDocument(d->list,"rich",x,y,&hit)); VERIFY(hit.iNodeId != 0);
        TRY(pointer(d,0,x,y,XUI_POINTER_BUTTON_LEFT));
        TRY(pointer(d,1,x+150,y,XUI_POINTER_BUTTON_LEFT)); TRY(pointer(d,2,x+150,y,0));
        VERIFY(xuiMessageListGetSelectedText(d->list,selected,sizeof(selected)) > 4);
        VERIFY(strstr(selected,"Native") != NULL);
        TRY(xuiMessageListGetNodeDocumentSelection(d->list,"rich",&selection));
        VERIFY(selection.tAnchor.iNodeId && selection.tCaret.iNodeId);
        d->phase=1;
    } else if (d->phase == 1) {
        TRY(capture(d,"document-messages-selection.png",NULL));
        revision=xuiDocumentGetRevision(d->draft);
        TRY(button_click(d,1)); VERIFY(xuiDocumentViewGetMode(d->editor)==XUI_DOC_VISUAL);
        TRY(button_click(d,2)); VERIFY(xuiDocumentViewGetMode(d->editor)==XUI_DOC_LIVE_MARKDOWN);
        TRY(button_click(d,0)); VERIFY(xuiDocumentViewGetMode(d->editor)==XUI_DOC_SOURCE_TEXT);
        VERIFY(xuiDocumentGetRevision(d->draft)==revision);
        TRY(copy_source(d->draft,source,sizeof(source))); VERIFY(!strcmp(source,draft_text));
        TRY(xuiDocumentEditorExecute(d->editor,XUI_DOC_EDIT_SELECT_ALL));
        TRY(xuiInputText(d->context,0x4e2d)); TRY(xuiInputText(d->context,0x6587));
        TRY(xuiDispatchPendingEvents(d->context));
        TRY(xuiDocumentEditorFlush(d->editor));
        TRY(copy_source(d->draft,source,sizeof(source))); VERIFY(!strcmp(source,typed_text));
        TRY(xuiMessageListClearTextSelection(d->list)); d->phase=2;
    } else if (d->phase == 2) {
        TRY(capture(d,"document-messages-draft.png",&pixels)); VERIFY(pixels != d->pixels_before);
        metrics.iSize=sizeof(metrics); TRY(xuiMessageListGetMetrics(d->list,&metrics));
        world=xuiWidgetGetWorldRect(d->list); content=xuiWidgetGetContentRect(d->list);
        bubble=xuiMessageListGetBubbleRect(d->list,2);
        x=world.fX+content.fX+bubble.fX+metrics.fBubblePaddingX+2;
        y=world.fY+content.fY+bubble.fY+metrics.fBubblePaddingY+8;
        TRY(pointer(d,0,x,y,XUI_POINTER_BUTTON_LEFT));
        TRY(pointer(d,1,x+150,y,XUI_POINTER_BUTTON_LEFT)); TRY(pointer(d,2,x+150,y,0));
        VERIFY(xuiMessageListGetSelectedText(d->list,selected,sizeof(selected))==sizeof(typed_text) && !strcmp(selected,typed_text));
        TRY(xuiMessageListClearTextSelection(d->list));
        TRY(xuiDocumentEditorExecute(d->editor,XUI_DOC_EDIT_UNDO));
        TRY(copy_source(d->draft,source,sizeof(source))); VERIFY(!strcmp(source,draft_text));
        TRY(xuiDocumentEditorExecute(d->editor,XUI_DOC_EDIT_REDO));
        TRY(copy_source(d->draft,source,sizeof(source))); VERIFY(!strcmp(source,typed_text));
        d->reply_height=xuiMessageListGetNodeRect(d->list,1).fH;
        d->draft_y=xuiMessageListGetNodeRect(d->list,2).fY;
        TRY(button_click(d,3)); d->phase=3;
    } else if (d->phase == 3) {
        VERIFY(xuiMessageListGetNodeRect(d->list,1).fH > d->reply_height+30);
        VERIFY(xuiMessageListGetNodeRect(d->list,2).fY > d->draft_y+30);
        VERIFY(xuiMessageListGetNodeDocument(d->list,"reply") == d->reply);
        TRY(copy_source(d->reply,source,sizeof(source))); VERIFY(strstr(source,"### Local update"));
        TRY(capture(d,"document-messages-update.png",NULL));
        TRY(xuiMessageListScrollToEnd(d->list)); VERIFY(xuiMessageListGetScroll(d->list)>0);
        d->phase=4;
    } else if (d->phase == 4) {
        TRY(capture(d,"document-messages-tail.png",NULL));
        TRY(layout(d,760,760)); TRY(xuiMessageListSetScroll(d->list,0)); d->phase=5;
    } else if (d->phase == 5) {
        VERIFY(d->width == 760);
        VERIFY(xuiMessageListGetNodeRect(d->list,1).fH >= d->reply_height);
        TRY(capture(d,"document-messages-narrow.png",NULL));
        printf("Native Document messages: %d checks; rich/MD bindings, hit/selection, shared input, modes, history, row update and scroll passed.\n",d->checks);
        d->phase=6; xgeQuit();
    }
    return XUI_OK;
}
static int frame(void* user)
{
    demo* d=user; xui_rect_i_t damage; int result;
    if (!d->context && (result=setup(d))!=XUI_OK) goto fail;
    result=layout(d,d->verify && d->phase==5 ? 760 : xgeGetWidth(),xgeGetHeight()); if(result!=XUI_OK)goto fail;
    if((result=xuiProxyXgePumpInputRect(d->context,(xui_rect_t){0,0,d->width,d->height}))!=XUI_OK)goto fail;
    if((result=xuiDispatchPendingEvents(d->context))!=XUI_OK)goto fail;
    if((result=xuiLayout(d->context))!=XUI_OK || (result=xuiUpdate(d->context,.016f))!=XUI_OK)goto fail;
    if((result=xgeBegin())!=XGE_OK)goto fail;
    damage=(xui_rect_i_t){0,0,d->width,d->height};
    result=d->proxy.surfaceClear(&d->proxy,d->target,XUI_COLOR_RGBA(241,244,249,255));
    if(result==XUI_OK)result=xuiRender(d->context,d->target,&damage,1);
    if(result==XUI_OK) {xgeClear(XUI_COLOR_WHITE); result=d->proxy.surfaceDraw(&d->proxy,d->target,
        (xui_rect_t){0,0,d->width,d->height},(xui_rect_t){0,0,d->width,d->height},XUI_COLOR_WHITE,XUI_SURFACE_DRAW_SCREEN_SPACE);}
    xgeEnd(); if(result!=XUI_OK)goto fail;
    if(d->verify && (result=verify(d))!=XUI_OK)goto fail;
    if(d->result)xgeQuit();
    return XGE_OK;
fail:
    fprintf(stderr,"Native Document messages failed: %d\n",result); d->result=1; xgeQuit(); return XGE_OK;
}
int main(int argc,char** argv)
{
    demo d={0}; xge_desc_t window={0}; int result;
    if(argc>2 || (argc==2 && strcmp(argv[1],"--verify"))) {
        fputs("Usage: xui_document_messages [--verify]\n",stderr);return 2;
    }
    d.verify=argc==2; window.iWidth=1080; window.iHeight=760;
    window.sTitle="Document messages"; window.iRunMode=XGE_RUN_GAME_LOOP;
    window.iTargetFPS=60; window.iFlags=XGE_INIT_VSYNC;
    if(xgeInit(&window)!=XGE_OK)return 2;
    result=xgeRun(frame,&d);
    if(d.verify)printf("Message demo final: run=%d result=%d phase=%d frames=%d checks=%d\n",result,d.result,d.phase,d.frames,d.checks);
    if(d.context)xuiDestroy(d.context);
    xuiDocumentRelease(d.rich); xuiDocumentRelease(d.reply); xuiDocumentRelease(d.draft);
    if(d.target)d.proxy.surfaceDestroy(&d.proxy,d.target);
    if(d.mono)d.proxy.fontDestroy(&d.proxy,d.mono);
    if(d.italic)d.proxy.fontDestroy(&d.proxy,d.italic);
    if(d.bold)d.proxy.fontDestroy(&d.proxy,d.bold);
    if(d.font)d.proxy.fontDestroy(&d.proxy,d.font);
    xgeUnit(); return result==XGE_OK && !d.result && (!d.verify || d.phase==6)?0:1;
}
