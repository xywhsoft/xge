#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct number_case {
    const char* name;
    const char* html;
    int status;
    unsigned rows,columns;
} number_case;
static const number_case cases[]={
    {"column-control","<table><tr><td colspan='2'>A</td></tr></table>",XUI_OK,1,2},
    {"column-zero","<table><tr><td colspan='0'>A</td></tr></table>",XUI_OK,1,1},
    {"column1000","<table><tr><td colspan='1000'>A</td></tr></table>",XUI_OK,1,1000},
    {"column1001","<table><tr><td colspan='1001'>A</td></tr></table>",XUI_OK,1,1000},
    {"column1024","<table><tr><td colspan='1024'>A</td></tr></table>",XUI_OK,1,1000},
    {"column1025","<table><tr><td colspan='1025'>A</td></tr></table>",XUI_OK,1,1000},
    {"column-overflow","<table><tr><td colspan='184467440737095516160000'>A</td></tr></table>",XUI_OK,1,1000},
    {"column-prefix","<table><tr><td colspan='2x'>A</td></tr></table>",XUI_OK,1,2},
    {"column-trailing-space","<table><tr><td colspan='2 '>A</td></tr></table>",XUI_OK,1,2},
    {"column-vertical-tab","<table><tr><td colspan='\v2'>A</td></tr></table>",XUI_OK,1,1},
    {"row-control","<table><tr><td rowspan='2'>A</td><td>B</td></tr><tr><td>C</td></tr></table>",XUI_OK,2,1},
    {"row-prefix","<table><tr><td rowspan='2x'>A</td><td>B</td></tr><tr><td>C</td></tr></table>",XUI_OK,2,1},
    {"row-trailing-space","<table><tr><td rowspan='2 '>A</td><td>B</td></tr><tr><td>C</td></tr></table>",XUI_OK,2,1},
    {"row-negative-zero","<table><tr><td rowspan='-0'>A</td><td>B</td></tr><tr><td>C</td></tr></table>",XUI_OK,2,1},
    {"row-prefix-zero","<table><tr><td rowspan='0x'>A</td><td>B</td></tr><tr><td>C</td></tr></table>",XUI_OK,2,1},
    {"row-negative-fallback","<table><tr><td rowspan='-1'>A</td><td>B</td></tr><tr><td>C</td><td>D</td></tr></table>",XUI_OK,1,1},
    {"row-vertical-tab","<table><tr><td rowspan='\v2'>A</td><td>B</td></tr><tr><td>C</td><td>D</td></tr></table>",XUI_OK,1,1},
    {"row-limit1025","<table><tr><td rowspan='1025'>A</td><td>B</td></tr><tr><td>C</td></tr></table>",XUI_DOC_ERROR_LIMIT,0,0},
    {"row-overflow-limit","<table><tr><td rowspan='184467440737095516160000'>A</td><td>B</td></tr><tr><td>C</td></tr></table>",XUI_DOC_ERROR_LIMIT,0,0},
    {"column-group1001","<table><colgroup><col span='1001' width='20'></colgroup><tr><td colspan='1000'>A</td></tr></table>",XUI_OK,1,1000},
    {"column-group-prefix","<table><colgroup><col span='2x' width='20'></colgroup><tr><td colspan='2'>A</td></tr></table>",XUI_OK,1,2},
    {"column-group-overflow","<table><colgroup><col span='184467440737095516160000' width='20'></colgroup><tr><td colspan='1000'>A</td></tr></table>",XUI_OK,1,1000}
};

#define NUM_CHECK(x) do { if (!(x)) { fprintf(stderr,"HTML numbers lifecycle line%d: %s\n",__LINE__,#x);exit(1); } } while(0)
static void first_cell(xui_document document,const number_case* fixture)
{
    xui_document_snapshot snapshot=NULL;
    xui_doc_node_info_t info={0};uint64_t table,row,cell;
    NUM_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    NUM_CHECK(xuiDocumentSnapshotGetChild(snapshot,XUI_DOCUMENT_ROOT,0,&table)==XUI_OK);
    NUM_CHECK(xuiDocumentSnapshotGetChild(snapshot,table,0,&row)==XUI_OK);
    NUM_CHECK(xuiDocumentSnapshotGetChild(snapshot,row,0,&cell)==XUI_OK);
    info.iSize=sizeof(info);
    NUM_CHECK(xuiDocumentSnapshotGetNode(snapshot,cell,&info)==XUI_OK);
    NUM_CHECK(info.tAttributes.iRowSpan==fixture->rows && info.tAttributes.iColumnSpan==fixture->columns);
    xuiDocumentSnapshotRelease(snapshot);
}
static void number_lifecycle(xui_document_fragment fragment,const number_case* fixture)
{
    xui_document_fragment decoded=NULL,again=NULL;
    xui_document document=NULL,reopened=NULL;
    xui_document_transaction txn=NULL;
    xui_document_snapshot old=NULL,snapshot=NULL;
    xui_doc_node_info_t root={0};uint64_t first,count,bytes,html_bytes,canonical_bytes;
    char *wire=NULL,*html=NULL,*canonical=NULL;
    NUM_CHECK(xuiDocumentFragmentSerialize(fragment,&wire,&bytes)==XUI_OK);
    NUM_CHECK(xuiDocumentFragmentDeserialize(wire,bytes,&decoded)==XUI_OK);
    xuiDocumentFreeBuffer(wire);wire=NULL;
    NUM_CHECK(xuiDocumentFragmentExportHtml(decoded,&html,&html_bytes)==XUI_OK);
    NUM_CHECK(xuiDocumentFragmentImportHtml(html,html_bytes,&again)==XUI_OK);
    NUM_CHECK(xuiDocumentFragmentExportHtml(again,&canonical,&canonical_bytes)==XUI_OK);
    NUM_CHECK(html_bytes==canonical_bytes && !memcmp(html,canonical,(size_t)html_bytes));
    xuiDocumentFreeBuffer(html);xuiDocumentFreeBuffer(canonical);
    NUM_CHECK(xuiDocumentCreate(NULL,&document)==XUI_OK);
    NUM_CHECK(xuiDocumentAcquireSnapshot(document,&old)==XUI_OK);
    NUM_CHECK(xuiDocumentMarkSaved(document,old)==XUI_OK);
    NUM_CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
    NUM_CHECK(xuiDocumentTxnInsertFragment(txn,again,XUI_DOCUMENT_ROOT,0,&first,&count)==XUI_OK && count==1);
    NUM_CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);txn=NULL;
    first_cell(document,fixture);
    root.iSize=sizeof(root);
    NUM_CHECK(xuiDocumentSnapshotGetNode(old,XUI_DOCUMENT_ROOT,&root)==XUI_OK && root.iChildCount==0);
    NUM_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    NUM_CHECK(xuiDocumentSerialize(snapshot,&wire,&bytes)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    NUM_CHECK(xuiDocumentDeserialize(NULL,wire,bytes,&reopened)==XUI_OK);
    first_cell(reopened,fixture);xuiDocumentFreeBuffer(wire);xuiDocumentRelease(reopened);
    NUM_CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document));
    NUM_CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);first_cell(document,fixture);
    xuiDocumentSnapshotRelease(old);xuiDocumentRelease(document);
    xuiDocumentFragmentRelease(again);xuiDocumentFragmentRelease(decoded);
}

int main(void)
{
    unsigned i,failures=0;
    setvbuf(stdout,NULL,_IONBF,0);
    for(i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        const number_case* fixture=&cases[i];
        xui_document_fragment fragment=(xui_document_fragment)(uintptr_t)1;
        xui_document document=NULL;xui_document_transaction txn=NULL;
        xui_document_snapshot snapshot=NULL;
        xui_doc_node_info_t info={0};uint64_t table,row,cell,count;
        unsigned observed_rows=0,observed_columns=0;
        int actual=xuiDocumentFragmentImportHtml(fixture->html,strlen(fixture->html),&fragment);
        int ok=actual==fixture->status;
        if(actual==XUI_OK) {
            ok=ok && fragment && xuiDocumentCreate(NULL,&document)==XUI_OK;
            ok=ok && xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK;
            ok=ok && xuiDocumentTxnInsertFragment(txn,fragment,XUI_DOCUMENT_ROOT,0,&table,&count)==XUI_OK;
            ok=ok && xuiDocumentTxnCommit(txn,NULL)==XUI_OK;
            ok=ok && xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK;
            ok=ok && xuiDocumentSnapshotGetChild(snapshot,table,0,&row)==XUI_OK;
            ok=ok && xuiDocumentSnapshotGetChild(snapshot,row,0,&cell)==XUI_OK;
            info.iSize=sizeof(info);
            ok=ok && xuiDocumentSnapshotGetNode(snapshot,cell,&info)==XUI_OK;
            if(ok) {observed_rows=info.tAttributes.iRowSpan;observed_columns=info.tAttributes.iColumnSpan;}
            ok=ok && observed_rows==fixture->rows && observed_columns==fixture->columns;
        } else ok=ok && !fragment;
        if(ok && actual==XUI_OK) number_lifecycle(fragment,fixture);
        xuiDocumentSnapshotRelease(snapshot);xuiDocumentTxnRelease(txn);
        xuiDocumentRelease(document);
        if(actual==XUI_OK) xuiDocumentFragmentRelease(fragment);
        if(!ok) failures++;
        printf("HTML table numbers %s:actual=%d expected=%d row=%u/%u column=%u/%u %s\n",fixture->name,
            actual,fixture->status,observed_rows,fixture->rows,observed_columns,fixture->columns,ok?"passed":"FAILED");
    }
    printf("HTML table numbers:22 independent literal cases,%u failures\n",failures);
    if(!failures) puts("HTML table numbers: fragment/native/HTML roundtrip, old snapshots and single Undo/Redo passed");
    return failures?1:0;
}
