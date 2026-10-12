#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REF_CHECK(x) do { if (!(x)) { fprintf(stderr,"HTML reference lifecycle line%d: %s\n",__LINE__,#x);exit(1); } } while(0)
typedef struct reference_case { const char *name,*html,*text,*info; } reference_case;
static const reference_case cases[]={
    {"semicolon-copy-control","<pre>&copy;</pre>","\302\251",""},
    {"legacy-copy-end","<pre>&copy</pre>","\302\251",""},
    {"legacy-copy-letter","<pre>&copycat</pre>","\302\251cat",""},
    {"legacy-copy-digit","<pre>&copy7</pre>","\302\2517",""},
    {"legacy-copy-equals","<pre>&copy=X</pre>","\302\251=X",""},
    {"nonlegacy-apos","<pre>&apos</pre>","&apos",""},
    {"semicolon-apos-control","<pre>&apos;</pre>","'",""},
    {"longest-notin","<pre>&notin;</pre>","\342\210\211",""},
    {"longest-legacy-not","<pre>&notin</pre>","\302\254in",""},
    {"legacy-unknown-suffix","<pre>&notit;</pre>","\302\254it;",""},
    {"unknown-name","<pre>&unknownName;</pre>","&unknownName;",""},
    {"uppercase-amp-control","<pre>&AMP;</pre>","&",""},
    {"uppercase-legacy-amp","<pre>&AMP</pre>","&",""},
    {"case-sensitive","<pre>&amP;</pre>","&amP;",""},
    {"two-codepoints","<pre>&NotEqualTilde;</pre>","\342\211\202\314\270",""},
    {"ligature-two-characters","<pre>&fjlig;</pre>","fj",""},
    {"longest-table-name","<pre>&CounterClockwiseContourIntegral;</pre>","\342\210\263",""},
    {"semicolon-followed-letter","<pre>&amp;x</pre>","&x",""},
    {"single-decoding","<pre>&amp;amp;</pre>","&amp;",""},
    {"lone-ampersand","<pre>&</pre>","&",""},
    {"adjacent-references","<pre>&copy&not;</pre>","\302\251\302\254",""},
    {"attribute-legacy-end","<pre><code data-language='&copy'>Z</code></pre>","Z","\302\251"},
    {"attribute-legacy-letter","<pre><code data-language='&copycat'>Z</code></pre>","Z","&copycat"},
    {"attribute-legacy-equals","<pre><code data-language='&copy=X'>Z</code></pre>","Z","&copy=X"},
    {"attribute-legacy-digit","<pre><code data-language='&copy7'>Z</code></pre>","Z","&copy7"},
    {"attribute-semicolon-letter","<pre><code data-language='&copy;cat'>Z</code></pre>","Z","\302\251cat"},
    {"attribute-legacy-punctuation","<pre><code data-language='&copy-'>Z</code></pre>","Z","\302\251-"},
    {"attribute-quote-control","<pre><code data-language='&quot;'>Z</code></pre>","Z","\""},
    {"attribute-longest-notin","<pre><code data-language='&notin;'>Z</code></pre>","Z","\342\210\211"},
    {"attribute-no-semicolon-notin","<pre><code data-language='&notin'>Z</code></pre>","Z","&notin"},
    {"attribute-unknown-suffix","<pre><code data-language='&notit;'>Z</code></pre>","Z","&notit;"}
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

#include "xui_document_html_named_reference_samples.h"
static void reference_table_cases(void)
{
    size_t i;
    for (i = 0; i < sizeof(named_table_cases) / sizeof(named_table_cases[0]); i++) {
        const named_table_case* row = named_table_cases + i;
        char html[256], expected_text[256];
        reference_case fixture = {"standard-table", html, expected_text, ""};
        xui_document_fragment fragment = NULL;
        xui_document document = NULL;
        xui_document_transaction txn = NULL;
        uint64_t first, count;
        if (row->attribute) {
            snprintf(html, sizeof(html), "<pre><code data-language='%s'>Z</code></pre>", row->input);
            strcpy(expected_text, "Z"); fixture.info = row->expected;
        } else {
            snprintf(html, sizeof(html), "<pre>X%s</pre>", row->input);
            snprintf(expected_text, sizeof(expected_text), "X%s", row->expected);
        }
        REF_CHECK(xuiDocumentFragmentImportHtml(html, strlen(html), &fragment) == XUI_OK);
        REF_CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        REF_CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        REF_CHECK(xuiDocumentTxnInsertFragment(txn, fragment, XUI_DOCUMENT_ROOT, 0, &first, &count) == XUI_OK && count == 1);
        REF_CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        if (!content(document, &fixture)) {
            fprintf(stderr, "Named table case%zu context%d input%s\n", i, row->attribute, row->input);
            REF_CHECK(0);
        }
        xuiDocumentRelease(document); xuiDocumentFragmentRelease(fragment);
    }
    puts("HTML named references:22310 standard-table/suffix/text-attribute cases passed");
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
        "<pre>&copycat&amp;&notit;&NotEqualTilde;&CounterClockwiseContourIntegral;</pre>",
        "<pre><code data-language='&copy-&AMP'>&copy&notin;&fjlig;</code></pre>"
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
    printf("HTML named reference importer buffers: %u allocation failures/null output/retry/zero leaks passed\n", total);
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
    printf("HTML named references:31 independent literal cases,%u failures\n",failures);
    if(!failures) puts("HTML named references: fragment/native/HTML roundtrip, old snapshots and single Undo/Redo passed");
    if(!failures) reference_table_cases();
#ifndef XGE_DLL
    if(!failures) reference_buffer_failures();
#endif
    return failures?1:0;
}
