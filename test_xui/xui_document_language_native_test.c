/* locl oracle uses independent literal glyphs, not a second language request. */
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W = 320, H = 128 };
static unsigned form, pass, cases, shapes, painted;
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s (form=%u pass=%u)\n",__FILE__,__LINE__,#e,form,pass); exit(1); } } while(0)
static xui_proxy_t original;
typedef struct language_paint {
    xui_font font; xui_rect_t rect; uint32_t color, flags, shape_flags;
    unsigned glyph, count; float offset;
} language_paint;
static language_paint paints[16];
/* Cache the shape input independently of Document attributes. Reflow and
 * colour-only edits must use the same locl glyphs after old attrs are freed. */
typedef struct language_retained_input {
    void* key; xui_font font; uint32_t flags;
    char text[16], language[256]; int bytes;
} language_retained_input;
static language_retained_input retained_inputs[256];
static unsigned language_glyph(const char* tag)
{
    CHECK(tag);
    if (!strncmp(tag, "tr", 2)) return 5;
    if (!strncmp(tag, "en", 2)) return 4;
    CHECK(!strcmp(tag, "und")); return 3;
}
static unsigned language_letters(const xui_text_item_t* item)
{
    int bytes = item->iTextSize < 0 ? (int)strlen(item->sText) : item->iTextSize;
    unsigned count = 0;
    for (int i = 0; i < bytes; i++) count += item->sText[i] == 'i';
    if (count) { CHECK(item->iSize == sizeof(*item)); (void)language_glyph(item->sLanguage); }
    return count;
}
static int language_shape(xui_proxy proxy, const xui_text_item_t* item, xui_text_shape_t* out)
{
    unsigned letters=language_letters(item);int result;
    if (letters) shapes++;
    result=original.textShape(proxy,item,out);
    if(result==XUI_OK && letters && out->pPaint){
        unsigned i;
        for(i=0;i<256;i++)if(!retained_inputs[i].key || retained_inputs[i].key==out->pPaint)break;
        CHECK(i<256 && item->iTextSize>=0 && item->iTextSize<(int)sizeof(retained_inputs[i].text));
        retained_inputs[i].key=out->pPaint;retained_inputs[i].font=item->pFont;
        retained_inputs[i].flags=item->iFlags;retained_inputs[i].bytes=item->iTextSize;
        memcpy(retained_inputs[i].text,item->sText,(size_t)item->iTextSize);retained_inputs[i].text[item->iTextSize]=0;
        CHECK(item->sLanguage && strlen(item->sLanguage)<sizeof(retained_inputs[i].language));
        strcpy(retained_inputs[i].language,item->sLanguage);
    }
    return result;
}
static void language_capture(const xui_text_item_t* item, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    unsigned count = language_letters(item);
    if (!count) return;
    CHECK(painted < 16);
    paints[painted++] = (language_paint){item->pFont, rect, color, flags, item->iFlags,
        language_glyph(item->sLanguage), count, item->fDrawOffsetX};
}
static int language_draw(xui_proxy proxy, xui_draw_context dc, const xui_text_item_t* item,
    xui_rect_t rect, uint32_t color, uint32_t flags)
{
    language_capture(item, rect, color, flags);
    return original.drawText(proxy, dc, item, rect, color, flags);
}
static int language_spans(xui_proxy proxy, xui_draw_context dc, const xui_text_item_t* item,
    xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    if (count) color = spans[0].iColor;
    language_capture(item, rect, color, flags);
    return original.drawTextSpans(proxy, dc, item, rect, color, flags, spans, count);
}
static int language_range(xui_proxy proxy,xui_draw_context dc,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    int result=original.drawTextShapeRange(proxy,dc,shape,start,end,rect,color,flags);
    if(result==XUI_OK){
        unsigned i;
        for(i=0;i<256;i++)if(retained_inputs[i].key==shape->pPaint)break;
        CHECK(i<256 && start>=0 && end<=retained_inputs[i].bytes);
        language_retained_input* input=&retained_inputs[i];
        language_capture(&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=input->font,
            .sText=input->text+start,.iTextSize=end-start,.sLanguage=input->language,.iFlags=input->flags},rect,color,flags);
    }
    return result;
}
static int language_range_spans(xui_proxy proxy,xui_draw_context dc,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,
    const xui_text_paint_span_t* spans,int count)
{
    int result=original.drawTextShapeRangeSpans(proxy,dc,shape,start,end,rect,color,flags,offset,spans,count);
    if(result==XUI_OK){
        unsigned i;
        for(i=0;i<256;i++)if(retained_inputs[i].key==shape->pPaint)break;
        CHECK(i<256 && start>=0 && end<=retained_inputs[i].bytes);
        language_retained_input* input=&retained_inputs[i];
        if(count)color=spans[0].iColor;
        language_capture(&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=input->font,
            .sText=input->text+start,.iTextSize=end-start,.sLanguage=input->language,
            .iFlags=input->flags,.fDrawOffsetX=offset},rect,color,flags);
    }
    return result;
}
static xui_doc_position_t language_position(xui_document document, uint64_t node,
    uint64_t offset, unsigned mode)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p);
    p.iDocumentId = xuiDocumentGetIdentity(document); p.iRevision = xuiDocumentGetRevision(document);
    p.iKind = mode == XUI_DOC_VISUAL ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_SOURCE;
    p.iNodeId = mode == XUI_DOC_VISUAL ? node : 1; p.iOffset = offset; p.iAffinity = XUI_DOC_AFTER;
    return p;
}
static void language_update(xui_document document, uint64_t node, const char* language,
    int color_only, xui_document_renderer renderer)
{
    xui_document_snapshot snapshot; xui_document_transaction transaction;
    xui_document_change_set change; xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, node, &info) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    if (color_only) info.tAttributes.iTextColor = XUI_COLOR_RGBA(160, 70, 240, 255);
    else info.tAttributes.sLanguage = language;
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &info.tAttributes) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, &change) == XUI_OK);
    xuiDocumentTxnRelease(transaction); xuiDocumentSnapshotRelease(snapshot);
    if (renderer) {
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, change) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    xuiDocumentChangeSetRelease(change);
}
static int language_frame(void* user)
{
    float size = *(float*)user; xui_proxy_t proxy = original = xuiProxyXge();
    xui_context context; xui_font font; xui_surface targets[2];
    xui_surface_desc_t surface = {0}; unsigned char actual[W*H*4], expected[W*H*4];
    proxy.textShape = language_shape; proxy.drawText = language_draw; proxy.drawTextSpans = language_spans;
    proxy.drawTextShapeRange=language_range;
    proxy.drawTextShapeRangeSpans=language_range_spans;
    memset(retained_inputs,0,sizeof(retained_inputs));
#ifdef TEST_DRAW_TEXT_ONLY
    proxy.drawTextSpans = NULL;
#endif
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy) == XUI_OK &&
        proxy.fontLoadFile(&proxy, &font, "test/data/xge_context_fixture.ttf", size, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    surface.iWidth = W; surface.iHeight = H; surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy, &targets[0], &surface) == XUI_OK &&
        proxy.surfaceCreate(&proxy, &targets[1], &surface) == XUI_OK);
    for (form = 0; form < 7; form++) {
        xui_document document; xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_desc_t profile = {0}; xui_doc_renderer_desc_t desc = {0};
        uint64_t nodes[3] = {0}, paragraph = 0, first_offset = 0, last_offset = 3;
        unsigned mode = form == 4 ? XUI_DOC_SOURCE_TEXT : form == 5 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        int split = form == 1 || form == 2;
        profile.iSize = sizeof(profile); profile.iProfile = form >= 3 && form <= 5 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        profile.bDisableHistory = 1;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (profile.iProfile == XUI_DOCUMENT_MARKDOWN) {
            CHECK(xuiDocumentLoadMarkdown(document, "iii", 3) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &nodes[0]) == XUI_OK);
            nodes[2] = nodes[0]; xuiDocumentSnapshotRelease(snapshot);
        } else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0};
            node.iSize = sizeof(node); node.iKind = form == 6 ? XUI_DOC_CODE_BLOCK : XUI_DOC_PARAGRAPH;
            if (form == 6) { node.sText = "iii"; node.iTextBytes = 3; node.sInfo = "c"; node.tAttributes.sLanguage = "en"; }
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
                xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            if (form == 6) nodes[0] = nodes[2] = paragraph;
            else for (unsigned i = 0; i < (split ? 3u : 1u); i++) {
                node = (xui_doc_node_desc_t){0}; node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT;
                node.sText = split ? (form == 2 && !i ? "i\xe2\x80\x8d" : "i") : "iii";
                node.iTextBytes = strlen(node.sText); node.tAttributes.sLanguage = i == 1 ? "tr" : i == 2 ? "und" : NULL;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &nodes[i]) == XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            if (!split) nodes[2] = nodes[0]; else last_offset = 1;
        }
        language_update(document, 1, split ? "en" : "tr", 0, NULL);
        desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        desc.iTextColor = XUI_COLOR_RGBA(255, 255, 255, 255); desc.iCodeBackground = XUI_COLOR_RGBA(1, 0, 0, 0);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererSetMode(renderer, mode) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        xui_doc_position_t start = language_position(document, nodes[0], first_offset, mode);
        xui_doc_position_t end = language_position(document, nodes[2], last_offset, mode);
        if (mode == XUI_DOC_LIVE_MARKDOWN) CHECK(xuiDocumentRendererSetActivePosition(renderer, &start) == XUI_OK);
        for (pass = 0; pass < 6; pass++) {
            xui_doc_rect_t a, b; xui_draw_context dc; unsigned ink = 0, letters = 0;
            if (pass == 2) CHECK(xuiDocumentRendererInvalidateFonts(renderer) == XUI_OK);
            if (pass == 3) language_update(document, 1, split ? "tr" : "en", 0, renderer);
            if (pass == 4 && profile.iProfile == XUI_DOCUMENT_RICH) language_update(document, split ? nodes[2] : nodes[0], NULL, 1, renderer);
            if (pass == 5) { xuiDocumentRelease(document); document = NULL; }
            if (document) { start.iRevision = end.iRevision = xuiDocumentGetRevision(document); }
            shapes = painted = 0;
            CHECK(xuiDocumentRendererLayout(renderer, pass == 1 ? 180 : W, 0, H) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &start, &a) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &end, &b) == XUI_OK);
            if (!pass || pass == 2) CHECK(shapes);
            double em = split ? (pass >= 3 ? 1.3 : 1.2) : form == 6 ? 1.2 : pass >= 3 ? 1.2 : 1.5;
            CHECK(fabs((b.x - a.x) - em * size) < .003);
            CHECK(proxy.surfaceClear(&proxy, targets[0], 0) == XUI_OK && proxy.drawBegin(&proxy, &dc, targets[0]) == XUI_OK &&
                xuiDocumentRendererDraw(renderer, dc, 0, 0, (xui_rect_t){0, 0, W, H}, NULL, 0) == XUI_OK &&
                proxy.drawEnd(&proxy, dc) == XUI_OK && painted);
            CHECK(original.surfaceClear(&original, targets[1], 0) == XUI_OK && original.drawBegin(&original, &dc, targets[1]) == XUI_OK);
            for (unsigned i = 0; i < painted; i++) {
                language_paint* paint = &paints[i]; char pua[64]; CHECK(paint->count <= 16);
                for (unsigned j = 0; j < paint->count; j++) {
                    unsigned glyph = split ? (letters == 0 ? (pass >= 3 ? 5u : 4u) : letters == 1 ? 5u : 3u) :
                        form == 6 || pass >= 3 ? 4u : 5u;
                    CHECK(paint->glyph == glyph);
                    pua[j*3] = (char)0xee; pua[j*3+1] = (char)0x84; pua[j*3+2] = (char)(0x80 + glyph); letters++;
                }
                CHECK(original.drawText(&original, dc, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t),
                    .pFont=paint->font, .sText=pua, .iTextSize=(int)paint->count*3, .iFlags=paint->shape_flags,
                    .fDrawOffsetX=paint->offset}, paint->rect, paint->color, paint->flags) == XUI_OK);
            }
            CHECK(letters == 3 && original.drawEnd(&original, dc) == XUI_OK &&
                original.surfaceReadRGBA(&original, targets[0], actual, W*4) == XUI_OK &&
                original.surfaceReadRGBA(&original, targets[1], expected, W*4) == XUI_OK);
            for (unsigned i = 0; i < W*H; i++) { CHECK(actual[i*4+3] == expected[i*4+3]); ink += actual[i*4+3] > 0; }
            CHECK(ink > 20); cases++;
        }
        xuiDocumentRendererRelease(renderer);
    }
    proxy.surfaceDestroy(&proxy, targets[0]); proxy.surfaceDestroy(&proxy, targets[1]);
    xuiDestroy(context); proxy.fontDestroy(&proxy, font);
    printf("Native Document language: %u cumulative cases at %g; en/tr/und locl glyphs, same-font boundaries, ZWJ projection, Rich/Markdown Visual/Source/Live/code, reflow, font/language/color invalidation and released Document; exact whole-frame GPU alpha passed\n", cases, (double)size);
    xgeQuit(); return XGE_OK;
}
int main(void)
{
    xge_desc_t engine = {0}; float sizes[] = {40, 37};
    engine.iWidth = W; engine.iHeight = H; engine.sTitle = "Document language";
    engine.iFlags = XGE_INIT_OFFSCREEN; engine.iRunMode = XGE_RUN_GAME_LOOP;
    for (unsigned i = 0; i < 2; i++) { CHECK(xgeInit(&engine) == XGE_OK && xgeRun(language_frame, &sizes[i]) == XGE_OK); xgeUnit(); }
    return 0;
}
