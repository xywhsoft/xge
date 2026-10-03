/* Global fallback is copied by the real XGE proxy into each font instance.
 * Cmap subsets retain the same outlines/layout tables as the complete font. */
static void document_font_fallback_cases(xui_context context,xui_proxy proxy,
    const xui_doc_renderer_desc_t* desc,const xui_surface* targets,float size)
{
    const char* paths[]={"test/data/xge_fallback_no_i.ttf","test/data/xge_fallback_no_mark.ttf","test/data/xge_fallback_no_base.ttf"};unsigned i;
    CHECK(xgeFontFallbackSet("test/data/xge_opentype_fixture.ttf",size)==XGE_OK);
    for(i=0;i<3;i++){
        xui_font font;xui_doc_renderer_desc_t fallback=*desc;
        CHECK(proxy->fontLoadFile(proxy,&font,paths[i],size,0)==XUI_OK);
        fallback.tFonts=(xui_doc_font_set_t){font,font,font,font,font};
        document_ligature_marks(context,proxy,&fallback,targets,size,0);
        document_ligature_marks(context,proxy,&fallback,targets,size,1);
        if(i==0){
            xui_font sized;float enlarged=size*1.5f;
            CHECK(proxy->fontCreateSized(proxy,&sized,font,enlarged)==XUI_OK);
            fallback.tFonts=(xui_doc_font_set_t){sized,sized,sized,sized,sized};
            document_ligature_marks(context,proxy,&fallback,targets,enlarged,0);
            document_ligature_marks(context,proxy,&fallback,targets,enlarged,1);
            proxy->fontDestroy(proxy,sized);
        }
        proxy->fontDestroy(proxy,font);
    }
    xgeFontFallbackClear();
    puts("Native Document SFNT fallback: base/mark/ligature cmap gaps, retained/sized fonts, Rich/Markdown/Source/Live and both GPU text backends passed");
}
