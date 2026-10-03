/* Public, window-free reproduction: a Latin parenthesis retains its script
 * when an ASCII paragraph is divided into different font items. */
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(e) do {if(!(e)){fprintf(stderr,"ASCII script public line %d: %s\n",__LINE__,#e);exit(2);}}while(0)
int main(void)
{
    xui_proxy_t p=xuiProxyXge();xui_context c;xui_font normal,bold;
    unsigned profile;int errors=0;xge_font_t native={0};xge_glyph_run_t run={0};
    CHECK(xgeFontLoad(&native,"test/data/xge_ascii_script_fixture.ttf",40)==XGE_OK);
    const struct {const char* context;int offset,bytes,glyph;float width;uint32_t script;} inputs[]={
        {"(a)",0,1,7,12,0},{"(a)",2,1,8,12,0},{"(a)\n()",4,1,5,8,0},
        {"()",0,2,5,16,0},{"(a)",0,1,5,8,UINT32_C(0x5a797979)},
        {"()",0,2,7,24,UINT32_C(0x4c61746e)}
    };
    for(unsigned i=0;i<sizeof(inputs)/sizeof(*inputs);i++)for(unsigned rtl=0;rtl<2;rtl++){
        xge_text_shape_desc_t input={.iSize=sizeof(input),.pFont=&native,.sText=inputs[i].context+inputs[i].offset,
            .iTextSize=inputs[i].bytes,.sContext=inputs[i].context,.iContextSize=-1,.iContextOffset=inputs[i].offset,
            .iScript=inputs[i].script,.iFlags=XGE_TEXT_SHAPE_DEFAULT|(rtl?XGE_TEXT_SHAPE_RTL:0)};
        int wanted=inputs[i].glyph;
        if(rtl)wanted=wanted%2?wanted+1:wanted-1;
        CHECK(xgeTextShape(&input,&run)==XGE_OK && run.iGlyphCount==inputs[i].bytes);
        printf("Public XGE ASCII case %u RTL%u: glyph=%d advance=%g; expected %d/%g\n",i,rtl,run.pGlyphs[0].iGlyph,run.fWidth,wanted,(double)inputs[i].width);
        errors+=run.pGlyphs[0].iGlyph!=wanted || fabs(run.fWidth-inputs[i].width)>.001;
        for(int j=0;j<run.iGlyphCount;j++){
            int glyph=inputs[i].glyph+j;
            if(rtl)glyph=glyph%2?glyph+1:glyph-1;
            errors+=run.pGlyphs[j].iGlyph!=glyph;
            CHECK(run.pGlyphs[j].pFont==&native && run.pGlyphs[j].iCluster==(unsigned)j && run.pGlyphs[j].iClusterEnd==(unsigned)(j+1));
        }
        xgeGlyphRunFree(&run);
    }
    xgeFontFree(&native);
    CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK &&
        p.fontLoadFile(&p,&normal,"test/data/xge_ascii_script_fixture.ttf",40,0)==XUI_OK &&
        p.fontLoadFile(&p,&bold,"test/data/xge_ascii_script_fixture.ttf",40,0)==XUI_OK && xuiSetDefaultFont(c,normal)==XUI_OK);
    for(profile=0;profile<2;profile++){
        xui_document d;xui_document_snapshot s;xui_document_renderer r;
        uint64_t paragraph,first,last;xui_doc_rect_t a,z;
        xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=profile?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH};
        CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);
        if(profile){
            CHECK(xuiDocumentLoadMarkdown(d,"(**a**)",7)==XUI_OK && xuiDocumentAcquireSnapshot(d,&s)==XUI_OK &&
                xuiDocumentSnapshotGetChild(s,1,0,&paragraph)==XUI_OK && xuiDocumentSnapshotGetChild(s,paragraph,0,&first)==XUI_OK &&
                xuiDocumentSnapshotGetChild(s,paragraph,2,&last)==XUI_OK);xuiDocumentSnapshotRelease(s);
        }else{
            xui_document_transaction t;xui_doc_node_desc_t n={.iSize=sizeof(n),.iKind=XUI_DOC_PARAGRAPH};
            CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnInsertNode(t,1,XUI_DOCUMENT_APPEND,&n,&paragraph)==XUI_OK);
            n.iKind=XUI_DOC_TEXT;n.sText="(";n.iTextBytes=1;
            CHECK(xuiDocumentTxnInsertNode(t,paragraph,XUI_DOCUMENT_APPEND,&n,&first)==XUI_OK);
            n.sText="a";n.iTextBytes=1;n.tAttributes.iMarks=XUI_DOC_BOLD;
            CHECK(xuiDocumentTxnInsertNode(t,paragraph,XUI_DOCUMENT_APPEND,&n,&last)==XUI_OK);
            n.sText=")";n.tAttributes.iMarks=0;
            CHECK(xuiDocumentTxnInsertNode(t,paragraph,XUI_DOCUMENT_APPEND,&n,&last)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);
            xuiDocumentTxnRelease(t);
        }
        xui_doc_renderer_desc_t layout={.iSize=sizeof(layout),.tFonts={normal,bold,normal,bold,normal}};
        CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(c,&layout,&r)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK && xuiDocumentRendererLayout(r,180,0,180)==XUI_OK);
        xui_doc_position_t at={.iSize=sizeof(at),.iKind=XUI_DOC_POSITION_TEXT,.iDocumentId=xuiDocumentGetIdentity(d),
            .iRevision=xuiDocumentGetRevision(d),.iNodeId=first,.iOffset=1,.iAffinity=XUI_DOC_BEFORE};
        CHECK(xuiDocumentRendererGetCaretRect(r,&at,&a)==XUI_OK);at.iNodeId=last;at.iOffset=1;
        CHECK(xuiDocumentRendererGetCaretRect(r,&at,&z)==XUI_OK);
        printf("Public %s ASCII fonts: parenthesis=%g total=%g; expected 12/48\n",profile?"Markdown":"Rich",a.x,z.x);
        errors+=fabs(a.x-12)>.001 || fabs(z.x-48)>.001;
        xuiDocumentSnapshotRelease(s);xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
    }
    xuiDestroy(c);p.fontDestroy(&p,normal);p.fontDestroy(&p,bold);return errors?1:0;
}
