#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"cross-profile line %d: %s\n",__LINE__,#x);exit(2); } } while (0)
typedef struct reference_case { const char *name,*html,*text,*info; } reference_case;
#include "xui_document_html_markdown_reference_samples.h"
static const reference_case boundaries[] = {
    {"plain-lf", "<pre>A&#10;</pre>", "A\n", ""},
    {"plain-no-lf", "<pre>A</pre>", "A", ""},
    {"body-cr-lf", "<pre>A&#13;&#10;B&#10;</pre>", "A\r\nB\n", ""},
    {"body-tab-lf", "<pre>A&#9;B&#10;</pre>", "A\tB\n", ""},
    {"info-amp-lf", "<pre><code data-language='A&amp;B'>Z&#10;</code></pre>", "Z\n", "A&B"},
    {"info-entity-literal-lf", "<pre><code data-language='&amp;copy;'>Z&#10;</code></pre>", "Z\n", "&copy;"},
    {"info-backslash-lf", "<pre><code data-language='a\\*b'>Z&#10;</code></pre>", "Z\n", "a\\*b"},
    {"info-leading-space-lf", "<pre><code data-language='&#32;c'>Z&#10;</code></pre>", "Z\n", " c"},
    {"info-trailing-space-lf", "<pre><code data-language='c&#32;'>Z&#10;</code></pre>", "Z\n", "c "},
    {"info-lf-lf", "<pre><code data-language='A&#10;B'>Z&#10;</code></pre>", "Z\n", "A\nB"},
    {"info-cr-lf", "<pre><code data-language='A&#13;B'>Z&#10;</code></pre>", "Z\n", "A\rB"},
    {"info-tab-lf", "<pre><code data-language='A&#9;B'>Z&#10;</code></pre>", "Z\n", "A\tB"},
    {"info-tilde-lf", "<pre><code data-language='~~~'>Z&#10;</code></pre>", "Z\n", "~~~"},
    {"info-backtick-lf", "<pre><code data-language='`c`'>Z&#10;</code></pre>", "Z\n", "`c`"}
};
static void code_content(xui_document_snapshot snapshot, uint64_t id,const reference_case* c)
{
    xui_doc_node_info_t node={0};char actual[512];uint64_t bytes=0;
    node.iSize=sizeof(node);CHECK(xuiDocumentSnapshotGetNode(snapshot,id,&node)==XUI_OK);
    CHECK(node.iKind==XUI_DOC_CODE_BLOCK);
    CHECK(xuiDocumentSnapshotCopyText(snapshot,id,actual,sizeof(actual),&bytes)==XUI_OK);
    CHECK(bytes==strlen(c->text) && !memcmp(actual,c->text,(size_t)bytes));
    CHECK(!strcmp(node.sInfo?node.sInfo:"",c->info));
}
static uint64_t source_copy(xui_document document,char *data,uint64_t capacity)
{
    xui_document_snapshot snapshot=NULL;uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot,data,capacity,&bytes)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);return bytes;
}
static void run_case(const reference_case* c,uint32_t dialect,int representable)
{
    const char original[]="tail [r]\r\n\r\n[r]: /keep \"unchanged\"\r\n";
    xui_document rich=NULL,converted=NULL,markdown=NULL,reopened=NULL;
    xui_document_fragment fragment=NULL; xui_document_transaction txn=NULL;
    xui_document_snapshot snapshot=NULL,old=NULL,saved=NULL;
    xui_doc_desc_t desc={0};xui_doc_markdown_loss_t losses[8]={0};
    uint64_t first=0,count=0,total=0,bytes=0,revision;int result,conversion,insertion;
    char before[2048],after[2048],restored[2048],*native=NULL;
    CHECK(xuiDocumentFragmentImportHtml(c->html,strlen(c->html),&fragment)==XUI_OK);
    CHECK(xuiDocumentCreate(NULL,&rich)==XUI_OK);
    CHECK(xuiDocumentBeginTransaction(rich,NULL,&txn)==XUI_OK);
    CHECK(xuiDocumentTxnInsertFragment(txn,fragment,XUI_DOCUMENT_ROOT,0,&first,&count)==XUI_OK && count==1);
    CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);txn=NULL;
    CHECK(xuiDocumentAcquireSnapshot(rich,&snapshot)==XUI_OK);code_content(snapshot,first,c);
    CHECK(xuiDocumentSnapshotAnalyzeMarkdownConversion(snapshot,dialect,losses,8,&total)==XUI_OK);
    conversion=xuiDocumentSnapshotConvertToMarkdown(snapshot,dialect,&converted);
    CHECK(conversion==(representable?XUI_OK:XUI_DOC_ERROR_UNREPRESENTABLE));
    if(conversion==XUI_OK){
        CHECK(!total && converted && xuiDocumentGetProfile(converted)==XUI_DOCUMENT_MARKDOWN);
        CHECK(!xuiDocumentIsDirty(converted) && !xuiDocumentCanUndo(converted));
        CHECK(xuiDocumentAcquireSnapshot(converted,&saved)==XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(saved,XUI_DOCUMENT_ROOT,0,&first)==XUI_OK);
        code_content(saved,first,c);
        xuiDocumentSnapshotRelease(saved);saved=NULL;
    }else CHECK(conversion==XUI_DOC_ERROR_UNREPRESENTABLE && !converted && total>0);
    code_content(snapshot,first,c);
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=dialect;
    CHECK(xuiDocumentCreate(&desc,&markdown)==XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(markdown,original,strlen(original))==XUI_OK);
    CHECK(xuiDocumentClearHistory(markdown)==XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(markdown,&old)==XUI_OK);
    CHECK(xuiDocumentMarkSaved(markdown,old)==XUI_OK);
    revision=xuiDocumentSnapshotGetRevision(old);bytes=source_copy(markdown,before,sizeof(before));
    CHECK(bytes==strlen(original) && !memcmp(before,original,(size_t)bytes));
    CHECK(xuiDocumentBeginTransaction(markdown,NULL,&txn)==XUI_OK);
    first=count=UINT64_MAX;
    insertion=xuiDocumentTxnInsertFragment(txn,fragment,XUI_DOCUMENT_ROOT,0,&first,&count);
    CHECK(insertion==(representable?XUI_OK:XUI_DOC_ERROR_UNREPRESENTABLE));
    result=xuiDocumentTxnCommit(txn,NULL);xuiDocumentTxnRelease(txn);txn=NULL;
    if(insertion==XUI_OK){
        uint64_t after_bytes;
        CHECK(result==XUI_OK && first && count==1);
        CHECK(xuiDocumentAcquireSnapshot(markdown,&saved)==XUI_OK);code_content(saved,first,c);
        CHECK(xuiDocumentSerialize(saved,&native,&bytes)==XUI_OK);
        CHECK(xuiDocumentDeserialize(NULL,native,bytes,&reopened)==XUI_OK);
        xuiDocumentFreeBuffer(native);native=NULL;
        xuiDocumentSnapshotRelease(saved);saved=NULL;
        after_bytes=source_copy(markdown,after,sizeof(after));
        CHECK(after_bytes>=strlen(original) && !memcmp(after+after_bytes-strlen(original),original,strlen(original)));
        CHECK(source_copy(reopened,restored,sizeof(restored))==after_bytes && !memcmp(after,restored,(size_t)after_bytes));
        CHECK(xuiDocumentUndo(markdown,NULL)==XUI_OK && !xuiDocumentIsDirty(markdown) && !xuiDocumentCanUndo(markdown));
        CHECK(source_copy(markdown,restored,sizeof(restored))==strlen(original) && !strcmp(restored,original));
        CHECK(xuiDocumentRedo(markdown,NULL)==XUI_OK);
        CHECK(source_copy(markdown,restored,sizeof(restored))==after_bytes && !memcmp(after,restored,(size_t)after_bytes));
    }else{
        CHECK(result==insertion && !first && !count);
        CHECK(!xuiDocumentIsDirty(markdown) && !xuiDocumentCanUndo(markdown) && !xuiDocumentCanRedo(markdown));
        CHECK(xuiDocumentAcquireSnapshot(markdown,&saved)==XUI_OK);
        CHECK(xuiDocumentSnapshotGetRevision(saved)==revision);
        xuiDocumentSnapshotRelease(saved);saved=NULL;
        CHECK(source_copy(markdown,after,sizeof(after))==strlen(original) && !strcmp(after,original));
    }
    CHECK(xuiDocumentSnapshotCopySource(old,before,sizeof(before),&bytes)==XUI_OK && !strcmp(before,original));
    printf("%s|dialect=%u|convert=%d|losses=%llu|reasons=%u|insert=%d|atomic=1\n",c->name,dialect,conversion,(unsigned long long)total,total?losses[0].iReasons:0,insertion);
    xuiDocumentSnapshotRelease(old);xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(reopened);xuiDocumentRelease(markdown);xuiDocumentRelease(converted);
    xuiDocumentRelease(rich);xuiDocumentFragmentRelease(fragment);
}

static const struct {size_t boundary;const char* source;} info_goldens[]={
    {5,"~~~&#38;copy;\nZ\n~~~\n"}, {6,"~~~a&#92;*b\nZ\n~~~\n"},
    {7,"~~~&#32;c\nZ\n~~~\n"}, {8,"~~~c&#32;\nZ\n~~~\n"},
    {9,"~~~A&#10;B\nZ\n~~~\n"}, {10,"~~~A&#13;B\nZ\n~~~\n"},
    {11,"~~~A&#9;B\nZ\n~~~\n"}, {12,"~~~&#126;~~\nZ\n~~~\n"}
};
static uint64_t code_id(xui_document_snapshot snapshot,uint64_t parent)
{
    xui_doc_node_info_t node={0};uint64_t i,id,found;
    node.iSize=sizeof(node);CHECK(xuiDocumentSnapshotGetNode(snapshot,parent,&node)==XUI_OK);
    if(node.iKind==XUI_DOC_CODE_BLOCK)return parent;
    for(i=0;i<node.iChildCount;i++){
        CHECK(xuiDocumentSnapshotGetChild(snapshot,parent,i,&id)==XUI_OK);
        found=code_id(snapshot,id);if(found)return found;
    }
    return 0;
}
static void info_representation_and_commands(void)
{
    uint32_t dialect;size_t i;unsigned quote,fence;
    for(dialect=XUI_MD_COMMONMARK;dialect<=XUI_MD_EXTENDED;dialect++)
      for(i=0;i<sizeof(info_goldens)/sizeof(*info_goldens);i++){
        const reference_case* c=boundaries+info_goldens[i].boundary;
        xui_doc_desc_t desc={0};xui_document document=NULL;
        xui_document_snapshot snapshot=NULL,old=NULL;uint64_t id;
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=dialect;
        CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document,info_goldens[i].source,strlen(info_goldens[i].source))==XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
        id=code_id(snapshot,XUI_DOCUMENT_ROOT);CHECK(id);code_content(snapshot,id,c);
        xuiDocumentSnapshotRelease(snapshot);snapshot=NULL;xuiDocumentRelease(document);document=NULL;
        for(quote=0;quote<2;quote++)for(fence=0;fence<2;fence++){
            const char* prefix=quote?"> ":"";const char* marker=fence?"~~~":"```";
            char original[512],after[2048],restored[2048];uint64_t bytes,cut;
            xui_document_transaction txn=NULL;
            snprintf(original,sizeof(original),"lead &amp;\r\n\r\n%s%sc keep-meta &copy; \r\n%sZ\r\n%s%s\r\n\r\ntail [r]\r\n\r\n[r]: /keep \"unchanged\"\r\n",prefix,marker,prefix,prefix,marker);
            cut=(uint64_t)(strstr(original,"c keep-meta")-original);
            CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
            CHECK(xuiDocumentLoadMarkdown(document,original,strlen(original))==XUI_OK);
            CHECK(xuiDocumentClearHistory(document)==XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document,&old)==XUI_OK);
            CHECK(xuiDocumentMarkSaved(document,old)==XUI_OK);id=code_id(old,XUI_DOCUMENT_ROOT);CHECK(id);
            CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
            CHECK(xuiDocumentTxnSetResource(txn,id,NULL,c->info,NULL)==XUI_OK);
            CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
            CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
            code_content(snapshot,id,c);xuiDocumentSnapshotRelease(snapshot);snapshot=NULL;
            bytes=source_copy(document,after,sizeof(after));
            CHECK(bytes>=cut+strlen(original+cut+1) && !memcmp(after,original,(size_t)cut));
            CHECK(!memcmp(after+bytes-strlen(original+cut+1),original+cut+1,strlen(original+cut+1)));
            CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
            CHECK(source_copy(document,restored,sizeof(restored))==strlen(original) && !strcmp(restored,original));
            CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);
            CHECK(source_copy(document,restored,sizeof(restored))==bytes && !memcmp(after,restored,(size_t)bytes));
            CHECK(xuiDocumentSnapshotCopySource(old,restored,sizeof(restored),&bytes)==XUI_OK && !strcmp(restored,original));
            xuiDocumentSnapshotRelease(old);old=NULL;xuiDocumentRelease(document);document=NULL;
        }
      }
    puts("HTML Markdown info:24 independent raw-source goldens and96 metadata-preserving root/quote backtick/tilde resource commands passed");
}

int main(void)
{
    size_t i;uint32_t dialect;
    info_representation_and_commands();
    for(dialect=XUI_MD_COMMONMARK;dialect<=XUI_MD_EXTENDED;dialect++){
        for(i=0;i<sizeof(numeric_cases)/sizeof(*numeric_cases);i++)run_case(numeric_cases+i,dialect,0);
        for(i=0;i<sizeof(named_cases)/sizeof(*named_cases);i++)run_case(named_cases+i,dialect,0);
        for(i=0;i<sizeof(boundaries)/sizeof(*boundaries);i++)run_case(boundaries+i,dialect,i!=1 && i!=2);
    }
    puts("Cross-profile references:201 code-block cases;36 exact accepted conversions/insertions and165 atomic refusals, native/source/shared history checked");
    return 0;
}
