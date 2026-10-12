#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REF_CHECK(x) do { if (!(x)) { fprintf(stderr,"HTML reference lifecycle line%d: %s\n",__LINE__,#x);exit(1); } } while(0)
typedef struct reference_case { const char *name,*html,*text,*info; } reference_case;
static const reference_case cases[]={
    {"decimal-control","<pre>&#65;</pre>","A",""},
    {"hexadecimal-control","<pre>&#x41;&#X42;</pre>","AB",""},
    {"zero-replacement","<pre>&#0;</pre>","\xef\xbf\xbd",""},
    {"surrogate-replacement","<pre>&#xD800;&#57343;</pre>","\xef\xbf\xbd\xef\xbf\xbd",""},
    {"unicode-last-scalar","<pre>&#x10FFFF;</pre>","\xf4\x8f\xbf\xbf",""},
    {"outside-range-decimal","<pre>&#1114112;</pre>","\xef\xbf\xbd",""},
    {"outside-range-hexadecimal","<pre>&#x110000;</pre>","\xef\xbf\xbd",""},
    {"integer-overflow","<pre>&#184467440737095516160000;</pre>","\xef\xbf\xbd",""},
    {"long-leading-zeroes","<pre>&#00000000000000000000000000000000000000000000000000000000000065;</pre>","A",""},
    {"decimal-no-semicolon","<pre>&#65</pre>","A",""},
    {"hexadecimal-no-semicolon","<pre>&#x41z</pre>","Az",""},
    {"decimal-digit-prefix","<pre>&#65foo;</pre>","Afoo;",""},
    {"single-decoding","<pre>&#38;#65;</pre>","&#65;",""},
    {"invalid-sign","<pre>&#-1;</pre>","&#-1;",""},
    {"missing-hex-digits","<pre>&#x;</pre>","&#x;",""},
    {"missing-decimal-digits","<pre>&#;</pre>","&#;",""},
    {"invalid-whitespace","<pre>&# 65;</pre>","&# 65;",""},
    {"c1-euro","<pre>&#128;</pre>","\xe2\x82\xac",""},
    {"c1-smart-quotes","<pre>&#147;&#148;</pre>","\xe2\x80\x9c\xe2\x80\x9d",""},
    {"c1-unmapped-control","<pre>&#129;</pre>","\xc2\x81",""},
    {"reference-whitespace","<pre>A&#13;&#9;&#10;B</pre>","A\r\t\nB",""},
    {"attribute-numeric-reference","<pre><code data-language='&#99&#x2b;&#43;'>Z</code></pre>","Z","c++"}
};
static int content(xui_document document,const reference_case* fixture)
{
    xui_document_snapshot snapshot=NULL;
    xui_doc_node_info_t node={0};uint64_t id,bytes=0;char text[512]={0};int okay;
    REF_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    REF_CHECK(xuiDocumentSnapshotGetChild(snapshot,XUI_DOCUMENT_ROOT,0,&id)==XUI_OK);
    node.iSize=sizeof(node);
    REF_CHECK(xuiDocumentSnapshotGetNode(snapshot,id,&node)==XUI_OK);
    REF_CHECK(node.iKind==XUI_DOC_CODE_BLOCK);
    REF_CHECK(xuiDocumentSnapshotCopyText(snapshot,id,text,sizeof(text),&bytes)==XUI_OK);
    okay=bytes==strlen(fixture->text) && !memcmp(text,fixture->text,(size_t)bytes) &&
        !strcmp(node.sInfo?node.sInfo:"",fixture->info);
    xuiDocumentSnapshotRelease(snapshot);
    return okay;
}
static void lifecycle(xui_document document,xui_document_snapshot old,
    xui_document_fragment fragment,const reference_case* fixture)
{
    xui_document_snapshot snapshot=NULL;
    xui_document reopened=NULL,copy=NULL;
    xui_document_transaction txn=NULL;
    xui_document_fragment decoded=NULL,html_fragment=NULL;
    xui_doc_node_info_t root={0};uint64_t bytes,first,count;
    char *wire=NULL,*html=NULL;
    REF_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    REF_CHECK(xuiDocumentSerialize(snapshot,&wire,&bytes)==XUI_OK);
    REF_CHECK(xuiDocumentDeserialize(NULL,wire,bytes,&reopened)==XUI_OK);
    REF_CHECK(content(reopened,fixture));xuiDocumentFreeBuffer(wire);wire=NULL;
    xuiDocumentRelease(reopened);xuiDocumentSnapshotRelease(snapshot);
    REF_CHECK(xuiDocumentFragmentSerialize(fragment,&wire,&bytes)==XUI_OK);
    REF_CHECK(xuiDocumentFragmentDeserialize(wire,bytes,&decoded)==XUI_OK);
    xuiDocumentFreeBuffer(wire);
    REF_CHECK(xuiDocumentFragmentExportHtml(decoded,&html,&bytes)==XUI_OK);
    REF_CHECK(xuiDocumentFragmentImportHtml(html,bytes,&html_fragment)==XUI_OK);
    xuiDocumentFreeBuffer(html);
    REF_CHECK(xuiDocumentCreate(NULL,&copy)==XUI_OK);
    REF_CHECK(xuiDocumentBeginTransaction(copy,NULL,&txn)==XUI_OK);
    REF_CHECK(xuiDocumentTxnInsertFragment(txn,html_fragment,XUI_DOCUMENT_ROOT,0,&first,&count)==XUI_OK && count==1);
    REF_CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    REF_CHECK(content(copy,fixture));xuiDocumentRelease(copy);
    xuiDocumentFragmentRelease(decoded);xuiDocumentFragmentRelease(html_fragment);
    root.iSize=sizeof(root);
    REF_CHECK(xuiDocumentSnapshotGetNode(old,XUI_DOCUMENT_ROOT,&root)==XUI_OK && !root.iChildCount);
    REF_CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document));
    REF_CHECK(xuiDocumentRedo(document,NULL)==XUI_OK && content(document,fixture));
}

#ifndef XGE_DLL
#include "../src/xui/document/xui_document_internal.h"
static long reference_budget = -1;
static unsigned reference_live, reference_attempts;
static void* reference_owned[64];
static void* reference_realloc(void* pointer, size_t bytes)
{
    unsigned slot;
    int was_null = pointer == NULL;
    void* result;
    reference_attempts++;
    if (!reference_budget) return NULL;
    if (reference_budget > 0) reference_budget--;
    for (slot = 0; slot < 64 && reference_owned[slot] != pointer; slot++) {}
    REF_CHECK(slot < 64);
    result = realloc(pointer, bytes);
    REF_CHECK(result);
    if (was_null) reference_live++;
    reference_owned[slot] = result; return result;
}
static void* reference_malloc(size_t bytes)
{
    return reference_realloc(NULL, bytes);
}
static void reference_free(void* pointer)
{
    unsigned slot;
    if (!pointer) return;
    for (slot = 0; slot < 64 && reference_owned[slot] != pointer; slot++) {}
    REF_CHECK(slot < 64 && reference_live);
    reference_owned[slot] = NULL; reference_live--; free(pointer);
}
#define malloc reference_malloc
#define realloc reference_realloc
#define free reference_free
#define doc_html_import_document reference_fault_import_document
#include "../src/xui/document/xui_document_html_import.c"
#undef doc_html_import_document
#undef free
#undef realloc
#undef malloc
static void reference_buffer_failures(void)
{
    const char* fixtures[] = {
        "<pre>&#184467440737095516160000;&#128;&#65&#x41z</pre>",
        "<pre><code data-language='&#99&#x2b;&#43;'>"
        "&#00000000000000000000000000000000000000000000000000000000000065;"
        "&#xD800;&#x110000;&#147;&#148;</code></pre>"
    };
    unsigned fixture, total = 0;
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(fixtures[0]); fixture++) {
        unsigned budget;
        for (budget = 0; budget < 64; budget++) {
            xui_document document = (xui_document)(uintptr_t)1;
            unsigned depth = 99;
            int result;
            reference_budget = budget;
            result = reference_fault_import_document(fixtures[fixture], strlen(fixtures[fixture]), &document, &depth);
            reference_budget = -1;
            REF_CHECK(reference_live == 0);
            if (result == XUI_OK) { xuiDocumentRelease(document); break; }
            REF_CHECK(result == XUI_ERROR_OUT_OF_MEMORY && !document && depth == 0);
            REF_CHECK(reference_fault_import_document(fixtures[fixture], strlen(fixtures[fixture]), &document, &depth) == XUI_OK);
            REF_CHECK(document && !reference_live); xuiDocumentRelease(document); total++;
        }
        REF_CHECK(budget > 0 && budget < 64);
    }
    printf("HTML character reference importer buffers: %u allocation failures/null output/retry/zero leaks passed\n", total);
}
#endif

int main(void)
{
    unsigned i,failures=0;
    setvbuf(stdout,NULL,_IONBF,0);
    for(i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        const reference_case* fixture=cases+i;
        xui_document_fragment fragment=NULL;
        xui_document document=NULL;
        xui_document_transaction txn=NULL;
        xui_document_snapshot old=NULL;uint64_t first,count;int okay;
        REF_CHECK(xuiDocumentFragmentImportHtml(fixture->html,strlen(fixture->html),&fragment)==XUI_OK);
        REF_CHECK(xuiDocumentCreate(NULL,&document)==XUI_OK && xuiDocumentAcquireSnapshot(document,&old)==XUI_OK);
        REF_CHECK(xuiDocumentMarkSaved(document,old)==XUI_OK);
        REF_CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
        REF_CHECK(xuiDocumentTxnInsertFragment(txn,fragment,XUI_DOCUMENT_ROOT,0,&first,&count)==XUI_OK && count==1);
        REF_CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
        okay=content(document,fixture);
        if(okay) lifecycle(document,old,fragment,fixture);else failures++;
        printf("HTML character reference %s: %s\n",fixture->name,okay?"passed":"FAILED");
        xuiDocumentSnapshotRelease(old);xuiDocumentRelease(document);xuiDocumentFragmentRelease(fragment);
    }
    printf("HTML character references:22 independent literal cases,%u failures\n",failures);
    if(!failures) puts("HTML character references: fragment/native/HTML roundtrip, old snapshots and single Undo/Redo passed");
#ifndef XGE_DLL
    if(!failures) reference_buffer_failures();
#endif
    return failures?1:0;
}
