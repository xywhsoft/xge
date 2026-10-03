/* Portable ownership and optional-backend contract; no GPU pixel claim. */
#include "../src/xui_document_layout_internal.h"
typedef struct retained_paint_test_data {char text[4];} retained_paint_test_data;
static xui_text_shape_proc retained_paint_base_shape;
static xui_draw_text_proc retained_paint_base_draw;
static unsigned retained_paint_created,retained_paint_freed,retained_paint_drawn,retained_paint_raw;
static int retained_paint_reject,retained_paint_fail;
static void retained_paint_free(void* owned)
{retained_paint_freed++;free(owned);}
static int retained_paint_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* shape)
{
    int result=retained_paint_base_shape(p,item,shape);
    if(result==XUI_OK && (item->iFlags & XUI_TEXT_SHAPE_RETAIN_PAINT)){
        retained_paint_test_data* data;
        CHECK(item->iTextSize==3);
        if(retained_paint_fail){retained_paint_fail=0;xuiTextShapeFree(shape);return XUI_ERROR_OUT_OF_MEMORY;}
        data=malloc(sizeof(*data));CHECK(data);memcpy(data->text,item->sText,3);data->text[3]=0;
        shape->pPaint=data;shape->paintFree=retained_paint_free;shape->iPaintBytes=sizeof(*data);
        retained_paint_created++;
    }
    return result;
}
static int retained_paint_range(xui_proxy p,xui_draw_context draw,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    retained_paint_test_data* data=shape->pPaint;
    (void)p;(void)draw;(void)rect;(void)color;(void)flags;
    CHECK(data && shape->paintFree==retained_paint_free && start>=0 && end<=3 && end>start &&
        strcmp(data->text,"a b")==0);
    if(retained_paint_reject)return XUI_ERROR_UNSUPPORTED;
    retained_paint_drawn++;return XUI_OK;
}
static int retained_paint_draw(xui_proxy p,xui_draw_context draw,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags)
{retained_paint_raw++;return retained_paint_base_draw(p,draw,item,rect,color,flags);}
static void document_retained_paint_contract(xui_test_proxy_state_t* state)
{
    xui_proxy_t proxy=state->tProxy;xui_context context;xui_font font;xui_surface surface;
    xui_document document;xui_document_snapshot snapshot;xui_document_renderer renderer;
    xui_doc_desc_t profile={0};unsigned pass;
    retained_paint_base_shape=proxy.textShape;retained_paint_base_draw=proxy.drawText;
    proxy.textShape=retained_paint_shape;proxy.drawText=retained_paint_draw;proxy.drawTextShapeRange=retained_paint_range;
    retained_paint_created=retained_paint_freed=retained_paint_drawn=retained_paint_raw=0;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&font,"retained-paint.ttf",20,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK &&
        xuiTestSurfaceCreate(state,&surface,160,160,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    profile.iSize=sizeof(profile);profile.iProfile=XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&profile,&document)==XUI_OK && xuiDocumentLoadMarkdown(document,"a b",3)==XUI_OK &&
        xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,NULL,&renderer)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
    retained_paint_fail=1;
    CHECK(xuiDocumentRendererLayout(renderer,40,0,160)==XUI_ERROR_OUT_OF_MEMORY &&
        retained_paint_created==retained_paint_freed);
    CHECK(xuiDocumentRendererLayout(renderer,40,0,160)==XUI_OK && retained_paint_created==1 &&
        renderer->blocks[0].runs[0].shape.iPaintBytes==sizeof(retained_paint_test_data) &&
        renderer->blocks[0].cache_bytes>=sizeof(retained_paint_test_data));
    xuiDocumentRelease(document);
    for(pass=0;pass<3;pass++){
        xui_draw_context draw;unsigned drawn=retained_paint_drawn,raw=retained_paint_raw;
        retained_paint_reject=pass==1;
        CHECK(xuiDocumentRendererLayout(renderer,pass==2?45:40,0,160)==XUI_OK && retained_paint_created==1 &&
            proxy.drawBegin(&proxy,&draw,surface)==XUI_OK &&
            xuiDocumentRendererDraw(renderer,draw,0,0,(xui_rect_t){0,0,160,160},NULL,0)==XUI_OK &&
            proxy.drawEnd(&proxy,draw)==XUI_OK);
        if(retained_paint_reject)CHECK(retained_paint_drawn==drawn && retained_paint_raw>raw);
        else CHECK(retained_paint_drawn>drawn && retained_paint_raw==raw);
    }
    xuiDocumentRendererRelease(renderer);CHECK(retained_paint_created==retained_paint_freed);
    proxy.surfaceDestroy(&proxy,surface);xuiDestroy(context);proxy.fontDestroy(&proxy,font);
    puts("Document retained paint contract: ownership, OOM/retry, cache accounting, width reuse, released snapshot and unsupported-before-paint fallback passed");
}
