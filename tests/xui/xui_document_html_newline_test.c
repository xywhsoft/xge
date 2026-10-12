#define main reference_original_main
#include "xui_document_html_markdown_reference_test.c"
#undef main
#include "xui_document_html_newline_samples.h"

static void newline_lifecycle(const reference_case* c)
{
    xui_document_fragment fragment=NULL,decoded=NULL;
    xui_document document=NULL,reopened=NULL,copy=NULL;
    xui_document_transaction transaction=NULL;
    xui_document_snapshot old=NULL,snapshot=NULL,other=NULL;
    xui_doc_node_info_t root={0};uint64_t first,count,bytes;
    char *wire=NULL,*html=NULL;
    CHECK(xuiDocumentFragmentImportHtml(c->html,strlen(c->html),&fragment)==XUI_OK);
    CHECK(xuiDocumentCreate(NULL,&document)==XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document,&old)==XUI_OK && xuiDocumentMarkSaved(document,old)==XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
    CHECK(xuiDocumentTxnInsertFragment(transaction,fragment,XUI_DOCUMENT_ROOT,0,&first,&count)==XUI_OK && count==1);
    CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);code_content(snapshot,first,c);
    CHECK(xuiDocumentSerialize(snapshot,&wire,&bytes)==XUI_OK);
    CHECK(xuiDocumentDeserialize(NULL,wire,bytes,&reopened)==XUI_OK);xuiDocumentFreeBuffer(wire);wire=NULL;
    CHECK(xuiDocumentAcquireSnapshot(reopened,&other)==XUI_OK);
    code_content(other,code_id(other,XUI_DOCUMENT_ROOT),c);xuiDocumentSnapshotRelease(other);other=NULL;
    CHECK(xuiDocumentFragmentSerialize(fragment,&wire,&bytes)==XUI_OK);
    CHECK(xuiDocumentFragmentDeserialize(wire,bytes,&decoded)==XUI_OK);xuiDocumentFreeBuffer(wire);wire=NULL;
    CHECK(xuiDocumentFragmentExportHtml(decoded,&html,&bytes)==XUI_OK);
    CHECK(!memchr(html,'\r',(size_t)bytes));
    xuiDocumentFragmentRelease(decoded);decoded=NULL;
    CHECK(xuiDocumentFragmentImportHtml(html,bytes,&decoded)==XUI_OK);xuiDocumentFreeBuffer(html);html=NULL;
    CHECK(xuiDocumentCreate(NULL,&copy)==XUI_OK);
    CHECK(xuiDocumentBeginTransaction(copy,NULL,&transaction)==XUI_OK);
    CHECK(xuiDocumentTxnInsertFragment(transaction,decoded,XUI_DOCUMENT_ROOT,0,&first,&count)==XUI_OK && count==1);
    CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
    CHECK(xuiDocumentAcquireSnapshot(copy,&other)==XUI_OK);
    code_content(other,code_id(other,XUI_DOCUMENT_ROOT),c);xuiDocumentSnapshotRelease(other);other=NULL;
    root.iSize=sizeof(root);
    CHECK(xuiDocumentSnapshotGetNode(old,XUI_DOCUMENT_ROOT,&root)==XUI_OK && !root.iChildCount);
    CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document));
    CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document,&other)==XUI_OK);
    code_content(other,code_id(other,XUI_DOCUMENT_ROOT),c);xuiDocumentSnapshotRelease(other);
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentSnapshotRelease(old);
    xuiDocumentFragmentRelease(decoded);xuiDocumentFragmentRelease(fragment);
    xuiDocumentRelease(copy);xuiDocumentRelease(reopened);xuiDocumentRelease(document);
}

#ifndef XUI_DLL
#include "../src/xui/document/xui_document_internal.h"
static long newline_budget=-1;
static void* newline_owned[64];
static unsigned newline_live;
static void* newline_realloc(void* pointer,size_t bytes)
{
    unsigned slot;void* result;
    if(!newline_budget)return NULL;
    if(newline_budget>0)newline_budget--;
    for(slot=0;slot<64 && newline_owned[slot]!=pointer;slot++) {}
    CHECK(slot<64);result=realloc(pointer,bytes);CHECK(result);
    if(!pointer)newline_live++;
    newline_owned[slot]=result;return result;
}
static void* newline_malloc(size_t bytes){return newline_realloc(NULL,bytes);}
static void newline_free(void* pointer)
{
    unsigned slot;if(!pointer)return;
    for(slot=0;slot<64 && newline_owned[slot]!=pointer;slot++) {}
    CHECK(slot<64 && newline_live);newline_owned[slot]=NULL;newline_live--;free(pointer);
}
#define malloc newline_malloc
#define realloc newline_realloc
#define free newline_free
#define doc_html_import_document newline_fault_import_document
#include "../src/xui/document/xui_document_html_import.c"
#undef doc_html_import_document
#define doc_html_safe_url newline_fault_safe_url
#define doc_html_export_children newline_fault_export_children
#define xuiDocumentExportHtml newline_fault_export
#include "../src/xui/document/xui_document_html.c"
#undef xuiDocumentExportHtml
#undef doc_html_export_children
#undef doc_html_safe_url
#undef free
#undef realloc
#undef malloc
static void newline_buffer_failures(void)
{
    unsigned failures=0;
    for(size_t i=0;i<sizeof(newline_cases)/sizeof(*newline_cases);i++){
        const reference_case* c=newline_cases+i;unsigned point;
        for(point=0;point<64;point++){
            xui_document document=(xui_document)(uintptr_t)1;unsigned depth=99;int result;
            newline_budget=point;
            result=newline_fault_import_document(c->html,strlen(c->html),&document,&depth);
            newline_budget=-1;CHECK(!newline_live);
            if(result==XUI_OK){
                xui_document_snapshot snapshot=NULL;CHECK(document);
                CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
                code_content(snapshot,code_id(snapshot,XUI_DOCUMENT_ROOT),c);
                xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);break;
            }
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !document && !depth);
            CHECK(newline_fault_import_document(c->html,strlen(c->html),&document,&depth)==XUI_OK && document && !newline_live);
            xuiDocumentRelease(document);failures++;
        }
        CHECK(point>0 && point<64);
    }
    {
        const reference_case* c=newline_cases+23;
        const char expected[]="<pre><code data-language=\"A&#13;B\">Z&#13;\n</code></pre>\n";
        xui_document document=NULL;xui_document_snapshot snapshot=NULL;unsigned point,depth;
        CHECK(newline_fault_import_document(c->html,strlen(c->html),&document,&depth)==XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
        for(point=0;point<64;point++){
            char* html=(char*)(uintptr_t)1;uint64_t bytes=99;int result;
            newline_budget=point;result=newline_fault_export(snapshot,&html,&bytes);newline_budget=-1;
            if(result==XUI_OK){
                CHECK(newline_live==1 && bytes==strlen(expected) && !strcmp(html,expected));
                newline_free(html);CHECK(!newline_live);break;
            }
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !html && !bytes && !newline_live);
            CHECK(newline_fault_export(snapshot,&html,&bytes)==XUI_OK && newline_live==1);
            CHECK(bytes==strlen(expected) && !strcmp(html,expected));
            newline_free(html);CHECK(!newline_live);failures++;
            code_content(snapshot,code_id(snapshot,XUI_DOCUMENT_ROOT),c);
        }
        CHECK(point>0 && point<64);xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);
    }
    printf("HTML newline buffers:%u importer/exporter allocation failures, null outputs, retry and zero leaks passed\n",failures);
}
#endif
int main(void)
{
    static const unsigned lossless_indices[]={3,4,5,22,24,25};
    for(size_t i=0;i<sizeof(newline_cases)/sizeof(*newline_cases);i++)newline_lifecycle(newline_cases+i);
    puts("HTML newlines:26 independent literal goldens, fragment/native/HTML roundtrip, old snapshot and one Undo/Redo passed");
    for(uint32_t dialect=XUI_MD_COMMONMARK;dialect<=XUI_MD_EXTENDED;dialect++)
    for(size_t i=0;i<sizeof(newline_cases)/sizeof(*newline_cases);i++){
        int representable=0;
        for(size_t j=0;j<sizeof(lossless_indices)/sizeof(*lossless_indices);j++)if(i==lossless_indices[j])representable=1;
        run_case(newline_cases+i,dialect,representable);
    }
    puts("HTML newlines:78 cross-profile cases,18 strict lossless conversions/insertions,60 atomic refusals passed");
#ifndef XUI_DLL
    newline_buffer_failures();
#endif
    return 0;
}
