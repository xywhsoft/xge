#include "xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EDIT_CHECK(x) do { if (!(x)) { fprintf(stderr,"Rowspan Editor line %d: %s\n",__LINE__,#x);exit(1); } } while(0)
static void expect_table(xui_document document,unsigned rows,unsigned first_span,unsigned late_row,unsigned late_span)
{
    xui_document_snapshot snapshot=NULL;
    xui_doc_node_info_t info={0};uint64_t table,row,cell;
    EDIT_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,XUI_DOCUMENT_ROOT,0,&table)==XUI_OK);
    info.iSize=sizeof(info);
    EDIT_CHECK(xuiDocumentSnapshotGetNode(snapshot,table,&info)==XUI_OK);
    if(info.iKind!=XUI_DOC_TABLE) fprintf(stderr,"Pasted first node kind=%u; expected Table\n",info.iKind);
    EDIT_CHECK(info.iKind==XUI_DOC_TABLE && info.iChildCount==rows);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,table,0,&row)==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,row,0,&cell)==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetNode(snapshot,cell,&info)==XUI_OK && info.tAttributes.iRowSpan==first_span);
    if(late_span) {
        EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,table,late_row,&row)==XUI_OK);
        EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,row,0,&cell)==XUI_OK);
        EDIT_CHECK(xuiDocumentSnapshotGetNode(snapshot,cell,&info)==XUI_OK && info.tAttributes.iRowSpan==late_span);
    }
    xuiDocumentSnapshotRelease(snapshot);
}
static xui_widget editor_create(xui_context context,xui_document document)
{
    xui_widget editor=NULL;
    xui_doc_editor_desc_t desc={0};
    desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);desc.tView.pDocument=document;desc.iMode=XUI_DOC_VISUAL;
    EDIT_CHECK(xuiDocumentEditorCreate(context,&desc,&editor)==XUI_OK);
    EDIT_CHECK(xuiSetRootWidget(context,editor)==XUI_OK);
    EDIT_CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,640,400})==XUI_OK);
    return editor;
}
static int clipboard_cases(void)
{
    static const char* html[]={
        "<table><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr></tbody></table>",
        "<table><thead><tr><th rowspan='0'>H</th><th>I</th></tr><tr><th>J</th></tr></thead><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr><tr><td>D</td></tr></tbody></table>"
    };
    xui_test_proxy_state_t proxy; xui_context context=NULL; xui_font font=NULL;
    unsigned i;
    setvbuf(stdout,NULL,_IONBF,0);xuiTestProxyInit(&proxy);
    EDIT_CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    EDIT_CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    EDIT_CHECK(xuiSetDefaultFont(context,font)==XUI_OK);
    for(i=0;i<2;i++) {
        xui_document document=NULL,pasted=NULL;
        xui_document_snapshot before=NULL,after=NULL;
        xui_doc_node_info_t root={0};xui_doc_range_t range={0};
        xui_widget editor;uint64_t version;
        EDIT_CHECK(xuiDocumentCreate(NULL,&document)==XUI_OK);
        EDIT_CHECK(xuiDocumentAcquireSnapshot(document,&before)==XUI_OK);
        EDIT_CHECK(xuiDocumentMarkSaved(document,before)==XUI_OK);
        editor=editor_create(context,document);
        EDIT_CHECK(xuiTestProxySetClipboardText(&proxy,"fallback")==XUI_OK);
        proxy.iClipboardDocumentSize=0;proxy.iClipboardHtmlSize=strlen(html[i]);
        memcpy(proxy.arrClipboardHtml,html[i],proxy.iClipboardHtmlSize);
        EDIT_CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_PASTE)==XUI_OK);
        expect_table(document,i?5:2,2,2,i?3:0);
        EDIT_CHECK(xuiDocumentAcquireSnapshot(document,&after)==XUI_OK);
        root.iSize=sizeof(root);
        EDIT_CHECK(xuiDocumentSnapshotGetNode(before,XUI_DOCUMENT_ROOT,&root)==XUI_OK && root.iChildCount==0);
        EDIT_CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document));
        EDIT_CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO)==XUI_OK);
        expect_table(document,i?5:2,2,2,i?3:0);
        range.tAnchor.iSize=range.tCaret.iSize=sizeof(range.tAnchor);
        range.tAnchor.iDocumentId=range.tCaret.iDocumentId=xuiDocumentGetIdentity(document);
        range.tAnchor.iRevision=range.tCaret.iRevision=xuiDocumentGetRevision(document);
        range.tAnchor.iNodeId=range.tCaret.iNodeId=XUI_DOCUMENT_ROOT;
        range.tAnchor.iKind=range.tCaret.iKind=XUI_DOC_POSITION_GAP;
        range.tAnchor.iAffinity=XUI_DOC_BEFORE;range.tCaret.iAffinity=XUI_DOC_AFTER;range.tCaret.iOffset=1;
        EDIT_CHECK(xuiDocumentViewSetSelection(editor,&range)==XUI_OK);
        version=xuiDocumentGetRevision(document);
        EDIT_CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_COPY)==XUI_OK && xuiDocumentGetRevision(document)==version);
        EDIT_CHECK(proxy.iClipboardDocumentSize>16 && proxy.iClipboardHtmlSize>0);
        xuiWidgetDestroy(editor);
        EDIT_CHECK(xuiDocumentCreate(NULL,&pasted)==XUI_OK);editor=editor_create(context,pasted);
        EDIT_CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_PASTE)==XUI_OK);
        expect_table(pasted,i?5:2,2,2,i?3:0);
        EDIT_CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK && !xuiDocumentCanUndo(pasted));
        xuiWidgetDestroy(editor);xuiDocumentRelease(pasted);
        xuiDocumentSnapshotRelease(after);xuiDocumentSnapshotRelease(before);xuiDocumentRelease(document);
        printf("HTML rowspan Editor case%u: HTML-only Paste, row-group spans, Copy/native Paste and one-step Undo/Redo passed; proxy backend\n",i);
    }
    xuiDestroy(context);proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    puts("HTML rowspan Editor: two clipboard/history cases passed; proxy backend");
    return 0;
}

#include <math.h>

int main(void)
{
    static const char html[]="<table><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr></tbody></table>";
    xui_test_proxy_state_t proxy;xui_context context=NULL;xui_font font=NULL;xui_surface target=NULL;
    xui_document document=NULL;xui_document_fragment fragment=NULL;xui_document_transaction txn=NULL;
    xui_document_snapshot snapshot=NULL;xui_widget editor;
    uint64_t table,count,row0,row1,cells[3];unsigned width;
    xui_rect_i_t damage={0,0,640,400};
    clipboard_cases();
    xuiTestProxyInit(&proxy);EDIT_CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    EDIT_CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    EDIT_CHECK(xuiSetDefaultFont(context,font)==XUI_OK);
    EDIT_CHECK(xuiTestSurfaceCreate(&proxy,&target,640,400,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    EDIT_CHECK(xuiDocumentCreate(NULL,&document)==XUI_OK);
    EDIT_CHECK(xuiDocumentFragmentImportHtml(html,sizeof(html)-1,&fragment)==XUI_OK);
    EDIT_CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
    EDIT_CHECK(xuiDocumentTxnInsertFragment(txn,fragment,XUI_DOCUMENT_ROOT,0,&table,&count)==XUI_OK && count==1);
    EDIT_CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);xuiDocumentFragmentRelease(fragment);
    EDIT_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,table,0,&row0)==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,table,1,&row1)==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,row0,0,&cells[0])==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,row0,1,&cells[1])==XUI_OK);
    EDIT_CHECK(xuiDocumentSnapshotGetChild(snapshot,row1,0,&cells[2])==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);editor=editor_create(context,document);
    for(width=0;width<2;width++) {
        xui_doc_cell_hit_t bounds[3]={{0}},hit={0};unsigned i;
        EDIT_CHECK(xuiSetViewportSize(context,width?640:240,400)==XUI_OK);
        EDIT_CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,width?640:240,400})==XUI_OK);
        for(i=0;i<3;i++) {
            bounds[i].iSize=sizeof(bounds[i]);
            EDIT_CHECK(xuiDocumentViewGetCellRect(editor,cells[i],&bounds[i])==XUI_OK);
            EDIT_CHECK(bounds[i].iTableId==table && bounds[i].iCellId==cells[i]);
            EDIT_CHECK(bounds[i].iColumn==(i?1:0) && bounds[i].iRow==(i==2?1:0));
            EDIT_CHECK(bounds[i].iRowSpan==(i?1:2) && bounds[i].iColumnSpan==1);
            EDIT_CHECK(bounds[i].tBounds.width>0 && bounds[i].tBounds.height>0);
        }
        EDIT_CHECK(fabs(bounds[0].tBounds.y-bounds[1].tBounds.y)<.01);
        EDIT_CHECK(fabs(bounds[0].tBounds.y+bounds[0].tBounds.height-bounds[2].tBounds.y-bounds[2].tBounds.height)<.01);
        EDIT_CHECK(bounds[0].tBounds.height>bounds[1].tBounds.height);
        EDIT_CHECK(bounds[1].tBounds.x>=bounds[0].tBounds.x+bounds[0].tBounds.width-.01);
        hit.iSize=sizeof(hit);
        EDIT_CHECK(xuiDocumentViewHitTestCell(editor,bounds[0].tBounds.x+bounds[0].tBounds.width/2,
            bounds[2].tBounds.y+bounds[2].tBounds.height/2,&hit)==XUI_OK && hit.iCellId==cells[0]);
        EDIT_CHECK(xuiDocumentViewHitTestCell(editor,bounds[2].tBounds.x+bounds[2].tBounds.width/2,
            bounds[2].tBounds.y+bounds[2].tBounds.height/2,&hit)==XUI_OK && hit.iCellId==cells[2]);
        xuiTestSurfaceReset(target);
        EDIT_CHECK(xuiUpdate(context,.016f)==XUI_OK && xuiRender(context,target,&damage,1)==XUI_OK);
        EDIT_CHECK(xuiTestSurfaceGetDrawCount(target)>0);
        {
            xui_document_renderer renderer=NULL;
            xui_doc_renderer_desc_t desc={0};xui_draw_context draw=NULL;
            EDIT_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
            desc.iSize=sizeof(desc);desc.tFonts.normal=font;
            EDIT_CHECK(xuiDocumentRendererCreate(context,&desc,&renderer)==XUI_OK);
            EDIT_CHECK(xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
            EDIT_CHECK(xuiDocumentRendererLayout(renderer,width?640:240,0,400)==XUI_OK);
            xuiTestSurfaceReset(target);
            EDIT_CHECK(proxy.tProxy.drawBegin(&proxy.tProxy,&draw,target)==XUI_OK);
            EDIT_CHECK(xuiDocumentRendererDraw(renderer,draw,0,0,(xui_rect_t){0,0,width?640:240,400},NULL,0)==XUI_OK);
            EDIT_CHECK(proxy.tProxy.drawEnd(&proxy.tProxy,draw)==XUI_OK);
            EDIT_CHECK(xuiTestSurfaceGetTextDrawCount(target)>=3);
            xuiDocumentRendererRelease(renderer);xuiDocumentSnapshotRelease(snapshot);
        }
    }
    xuiWidgetDestroy(editor);xuiDocumentRelease(document);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy,target);xuiDestroy(context);proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    puts("HTML rowspan Editor geometry:240/640 widths, merged cell bounds, lower-row hit and actual draw requests passed; proxy backend");
    return 0;
}
