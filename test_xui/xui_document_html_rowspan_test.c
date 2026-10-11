#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPAN_CHECK(x) do { if (!(x)) { fprintf(stderr, "HTML rowspan line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
typedef struct span_case {
    const char *name, *html;
    unsigned cells, tables, rows;
    unsigned spans[16], columns[16], headers;
} span_case;
static const span_case cases[] = {
    {"body", "<table><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr></tbody></table>", 3,1,2,{2,1,1},{1,1,1},0},
    {"implicit", "<table><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr><tr><td>D</td></tr></table>", 4,1,3,{3,1,1,1},{1,1,1,1},0},
    {"single", "<table><tbody><tr><td rowspan='0'>A</td></tr></tbody></table>", 1,1,1,{1},{1},0},
    {"late", "<table><tbody><tr><td>P</td><td>Q</td></tr><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr></tbody></table>", 5,1,3,{1,1,2,1,1},{1,1,1,1,1},0},
    {"head-body-foot", "<table><thead><tr><th rowspan='0'>H</th><th>I</th></tr><tr><th>J</th></tr></thead><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr></tbody><tfoot><tr><td rowspan='0'>F</td><td>G</td></tr><tr><td>K</td></tr></tfoot></table>", 9,1,6,{2,1,1,2,1,1,2,1,1},{1,1,1,1,1,1,1,1,1},7},
    {"two-bodies", "<table><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr></tbody><tbody><tr><td rowspan='0'>D</td><td>E</td></tr><tr><td>F</td></tr><tr><td>G</td></tr></tbody></table>", 7,1,5,{2,1,1,3,1,1,1},{1,1,1,1,1,1,1},0},
    {"omitted-group-end", "<table><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td></tr><tbody><tr><td rowspan='0'>D</td><td>E</td></tr><tr><td>F</td></tr><tr><td>G</td></tr></table>", 7,1,5,{2,1,1,3,1,1,1},{1,1,1,1,1,1,1},0},
    {"covered-rows", "<table><tbody><tr><td rowspan='0'>A</td></tr><tr></tr><tr></tr></tbody></table>", 1,1,3,{3},{1},0},
    {"column-span", "<table><tbody><tr><td rowspan='0' colspan='2'>A</td><td>B</td></tr><tr><td>C</td></tr><tr><td>D</td></tr></tbody></table>", 4,1,3,{3,1,1,1},{2,1,1,1},0},
    {"staggered", "<table><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td rowspan='0'>C</td></tr><tr></tr></tbody></table>", 3,1,3,{3,1,2},{1,1,1},0},
    {"nested", "<table><tbody><tr><td rowspan='0'><table><tbody><tr><td rowspan='0'>N</td><td>O</td></tr><tr><td>P</td></tr><tr><td>Q</td></tr></tbody></table></td><td>B</td></tr><tr><td>C</td></tr></tbody></table>", 7,2,5,{2,3,1,1,1,1,1},{1,1,1,1,1,1,1},0},
    {"sibling", "<table><tr><td rowspan='0'>A</td></tr><tr></tr></table><table><tr><td rowspan='0'>B</td></tr><tr></tr><tr></tr></table>", 2,2,5,{2,3},{1,1},0},
    {"entity-uppercase", "<TABLE><TBODY><TR><TD ROWSPAN=&#48;>A</TD><TD>B</TD></TR><TR><TD>C</TD></TR></TBODY></TABLE>", 3,1,2,{2,1,1},{1,1,1},0},
    {"leading-space-plus", "<table><tr><td rowspan=' +00'>A</td><td>B</td></tr><tr><td>C</td></tr></table>", 3,1,2,{2,1,1},{1,1,1},0},
    {"negative-zero-grows", "<table><tr><td rowspan='-0'>A</td><td>B</td></tr><tr><td>C</td></tr></table>", 3,1,2,{2,1,1},{1,1,1},0},
    {"invalid-fallback", "<table><tr><td rowspan='invalid'>A</td><td>B</td></tr><tr><td>C</td><td>D</td></tr></table>", 4,1,2,{1,1,1,1},{1,1,1,1},0},
    {"explicit-control", "<table><tbody><tr><td rowspan='2'>A</td><td>B</td></tr><tr><td>C</td></tr></tbody></table>", 3,1,2,{2,1,1},{1,1,1},0},
    {"pending-growth", "<table><tbody><tr><td rowspan='0'>0</td><td rowspan='0'>1</td><td rowspan='0'>2</td><td rowspan='0'>3</td><td rowspan='0'>4</td><td rowspan='0'>5</td><td rowspan='0'>6</td><td rowspan='0'>7</td><td rowspan='0'>8</td><td rowspan='0'>9</td></tr><tr></tr></tbody></table>", 10,1,2,{2,2,2,2,2,2,2,2,2,2},{1,1,1,1,1,1,1,1,1,1},0}
};

typedef struct span_seen { unsigned cells,tables,rows; } span_seen;
static void check_tree(xui_document_snapshot snapshot, uint64_t id,
    const span_case* fixture, span_seen* seen)
{
    xui_doc_node_info_t info = {0};
    uint64_t i,child;
    info.iSize = sizeof(info);
    SPAN_CHECK(xuiDocumentSnapshotGetNode(snapshot,id,&info)==XUI_OK);
    if (info.iKind==XUI_DOC_TABLE) seen->tables++;
    if (info.iKind==XUI_DOC_ROW) seen->rows++;
    if (info.iKind==XUI_DOC_CELL) {
        unsigned index=seen->cells++;
        SPAN_CHECK(index<fixture->cells && index<16);
        SPAN_CHECK(info.tAttributes.iRowSpan==fixture->spans[index]);
        SPAN_CHECK(info.tAttributes.iColumnSpan==fixture->columns[index]);
        SPAN_CHECK(!!(info.tAttributes.iFlags & XUI_DOC_HEADER)==!!(fixture->headers & (1u<<index)));
    }
    for (i=0;i<info.iChildCount;i++) {
        SPAN_CHECK(xuiDocumentSnapshotGetChild(snapshot,id,i,&child)==XUI_OK);
        check_tree(snapshot,child,fixture,seen);
    }
}
static void check_document(xui_document document,const span_case* fixture)
{
    xui_document_snapshot snapshot=NULL;
    span_seen seen={0};
    SPAN_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    check_tree(snapshot,XUI_DOCUMENT_ROOT,fixture,&seen);
    SPAN_CHECK(seen.cells==fixture->cells && seen.tables==fixture->tables && seen.rows==fixture->rows);
    xuiDocumentSnapshotRelease(snapshot);
}
static char* native(xui_document document,uint64_t* bytes)
{
    xui_document_snapshot snapshot=NULL;
    char* buffer=NULL;
    SPAN_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    SPAN_CHECK(xuiDocumentSerialize(snapshot,&buffer,bytes)==XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    return buffer;
}
static void check_fixture(const span_case* fixture)
{
    xui_document_fragment fragment=NULL,decoded=NULL,again=NULL;
    xui_document document=NULL,reopened=NULL,md=NULL;
    xui_document_transaction txn=NULL;
    xui_document_snapshot old=NULL,snapshot=NULL;
    xui_doc_node_info_t root={0};
    xui_doc_desc_t desc={0};
    char *wire=NULL,*html=NULL,*canonical=NULL,*before=NULL,*after=NULL,*restored=NULL;
    uint64_t bytes=0,html_bytes=0,canonical_bytes=0,before_bytes=0,after_bytes=0,restored_bytes=0,first=0,count=0;
    uint64_t revision;
    SPAN_CHECK(xuiDocumentFragmentImportHtml(fixture->html,strlen(fixture->html),&fragment)==XUI_OK);
    SPAN_CHECK(xuiDocumentFragmentSerialize(fragment,&wire,&bytes)==XUI_OK);
    SPAN_CHECK(xuiDocumentFragmentDeserialize(wire,bytes,&decoded)==XUI_OK);
    xuiDocumentFreeBuffer(wire);wire=NULL;
    SPAN_CHECK(xuiDocumentFragmentExportHtml(decoded,&html,&html_bytes)==XUI_OK);
    SPAN_CHECK(strstr(html,"rowspan=\"0\"")==NULL);
    SPAN_CHECK(xuiDocumentFragmentImportHtml(html,html_bytes,&again)==XUI_OK);
    SPAN_CHECK(xuiDocumentFragmentExportHtml(again,&canonical,&canonical_bytes)==XUI_OK);
    SPAN_CHECK(html_bytes==canonical_bytes && !memcmp(html,canonical,(size_t)html_bytes));
    xuiDocumentFreeBuffer(canonical);
    if (!strcmp(fixture->name,"body") || !strcmp(fixture->name,"explicit-control")) {
        static const char expected[]="<table><tr><td rowspan=\"2\"><p><span>A</span></p>\n</td>\n<td><p><span>B</span></p>\n</td>\n</tr>\n<tr><td><p><span>C</span></p>\n</td>\n</tr>\n</table>\n";
        SPAN_CHECK(html_bytes==sizeof(expected)-1 && !memcmp(html,expected,sizeof(expected)-1));
    }
    SPAN_CHECK(xuiDocumentCreate(NULL,&document)==XUI_OK);
    before=native(document,&before_bytes);
    SPAN_CHECK(xuiDocumentAcquireSnapshot(document,&old)==XUI_OK);
    SPAN_CHECK(xuiDocumentMarkSaved(document,old)==XUI_OK);
    SPAN_CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
    SPAN_CHECK(xuiDocumentTxnInsertFragment(txn,decoded,XUI_DOCUMENT_ROOT,0,&first,&count)==XUI_OK);
    SPAN_CHECK(first && count==fixture->tables-(fixture->tables==2 && !strcmp(fixture->name,"nested")));
    SPAN_CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);
    xuiDocumentTxnRelease(txn);txn=NULL;
    check_document(document,fixture);
    root.iSize=sizeof(root);
    SPAN_CHECK(xuiDocumentSnapshotGetNode(old,XUI_DOCUMENT_ROOT,&root)==XUI_OK && root.iChildCount==0);
    after=native(document,&after_bytes);
    SPAN_CHECK(xuiDocumentDeserialize(NULL,after,after_bytes,&reopened)==XUI_OK);
    check_document(reopened,fixture);xuiDocumentRelease(reopened);
    SPAN_CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
    restored=native(document,&restored_bytes);
    SPAN_CHECK(restored_bytes==before_bytes && !memcmp(restored,before,(size_t)before_bytes));
    xuiDocumentFreeBuffer(restored);
    SPAN_CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);
    check_document(document,fixture);
    restored=native(document,&restored_bytes);
    SPAN_CHECK(restored_bytes==after_bytes && !memcmp(restored,after,(size_t)after_bytes));
    xuiDocumentFreeBuffer(restored);
    if (fixture->spans[0]>1) {
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;
        SPAN_CHECK(xuiDocumentCreate(&desc,&md)==XUI_OK);
        SPAN_CHECK(xuiDocumentLoadMarkdown(md,"anchor\n",7)==XUI_OK);
        revision=xuiDocumentGetRevision(md);
        SPAN_CHECK(xuiDocumentBeginTransaction(md,NULL,&txn)==XUI_OK);
        {
            int result=xuiDocumentTxnInsertFragment(txn,decoded,XUI_DOCUMENT_ROOT,1,&first,&count);
            if(result!=XUI_DOC_ERROR_UNREPRESENTABLE) fprintf(stderr,"Markdown merged fragment %s returned %d\n",fixture->name,result);
            SPAN_CHECK(result==XUI_DOC_ERROR_UNREPRESENTABLE);
        }
        SPAN_CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_DOC_ERROR_UNREPRESENTABLE);
        xuiDocumentTxnRelease(txn);txn=NULL;
        SPAN_CHECK(xuiDocumentGetRevision(md)==revision);
        SPAN_CHECK(xuiDocumentAcquireSnapshot(md,&snapshot)==XUI_OK);
        {
            char source[8]={0};
            SPAN_CHECK(xuiDocumentSnapshotReadSource(snapshot,0,source,7)==XUI_OK && !memcmp(source,"anchor\n",7));
        }
        xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(md);
    }
    xuiDocumentFreeBuffer(before);xuiDocumentFreeBuffer(after);xuiDocumentFreeBuffer(html);
    xuiDocumentSnapshotRelease(old);xuiDocumentRelease(document);
    xuiDocumentFragmentRelease(again);xuiDocumentFragmentRelease(decoded);xuiDocumentFragmentRelease(fragment);
    printf("HTML rowspan fixture %s: independent spans, native/HTML roundtrip, old snapshot and single Undo/Redo passed\n",fixture->name);
}
static void generated(unsigned width,unsigned rows,int expected)
{
    char html[30000];size_t at=0;
    unsigned i;
    xui_document_fragment fragment=(xui_document_fragment)(uintptr_t)1;
    xui_document document=NULL;
    xui_document_transaction txn=NULL;
    xui_document_snapshot snapshot=NULL;
    xui_doc_node_info_t info={0};uint64_t table,row,cell,count;
    at+=(size_t)sprintf(html+at,"<table><tbody><tr>");
    for(i=0;i<width;i++) at+=(size_t)sprintf(html+at,"<td rowspan='0'>%u</td>",i);
    at+=(size_t)sprintf(html+at,"</tr>");
    for(i=1;i<rows;i++) at+=(size_t)sprintf(html+at,"<tr></tr>");
    at+=(size_t)sprintf(html+at,"</tbody></table>");
    SPAN_CHECK(at<sizeof(html));
    SPAN_CHECK(xuiDocumentFragmentImportHtml(html,at,&fragment)==expected);
    if(expected!=XUI_OK) {SPAN_CHECK(fragment==NULL);return;}
    SPAN_CHECK(xuiDocumentCreate(NULL,&document)==XUI_OK);
    SPAN_CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
    SPAN_CHECK(xuiDocumentTxnInsertFragment(txn,fragment,XUI_DOCUMENT_ROOT,0,&table,&count)==XUI_OK && count==1);
    SPAN_CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    SPAN_CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    SPAN_CHECK(xuiDocumentSnapshotGetChild(snapshot,table,0,&row)==XUI_OK);
    info.iSize=sizeof(info);
    SPAN_CHECK(xuiDocumentSnapshotGetNode(snapshot,table,&info)==XUI_OK && info.iChildCount==rows);
    for(i=0;i<width;i++) {
        SPAN_CHECK(xuiDocumentSnapshotGetChild(snapshot,row,i,&cell)==XUI_OK);
        SPAN_CHECK(xuiDocumentSnapshotGetNode(snapshot,cell,&info)==XUI_OK && info.tAttributes.iRowSpan==rows);
    }
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);xuiDocumentFragmentRelease(fragment);
}

#if defined(XUI_HTML_ROWSPAN_IMPORT_FAULT_TEST)
#include "../src/xui_document_internal.h"
static long fault_budget=-1;
static unsigned fault_attempts,fault_live;
static void* fault_owned[8192];
static void* fault_alloc(void* user,size_t bytes)
{
    unsigned i;void* pointer;(void)user;
    fault_attempts++;
    if(!fault_budget) return NULL;
    if(fault_budget>0) fault_budget--;
    pointer=malloc(bytes);SPAN_CHECK(pointer);
    for(i=0;i<8192 && fault_owned[i];i++) {}
    SPAN_CHECK(i<8192);fault_owned[i]=pointer;fault_live++;
    return pointer;
}
static void fault_free(void* user,void* pointer)
{
    unsigned i;(void)user;
    if(!pointer) return;
    for(i=0;i<8192 && fault_owned[i]!=pointer;i++) {}
    SPAN_CHECK(i<8192 && fault_live);fault_owned[i]=NULL;fault_live--;free(pointer);
}
static void* fault_malloc(size_t bytes) {return fault_alloc(NULL,bytes);}
static void fault_buffer_free(void* pointer) {fault_free(NULL,pointer);}
static void* fault_realloc(void* pointer,size_t bytes)
{
    unsigned i;void* next;
    if(!pointer) return fault_malloc(bytes);
    for(i=0;i<8192 && fault_owned[i]!=pointer;i++) {}
    SPAN_CHECK(i<8192);fault_attempts++;
    if(!fault_budget) return NULL;
    if(fault_budget>0) fault_budget--;
    next=realloc(pointer,bytes);SPAN_CHECK(next);fault_owned[i]=next;
    return next;
}
static int fault_create(const xui_doc_desc_t* desc,xui_document* out)
{
    xui_doc_desc_t adjusted=*desc;
    adjusted.onAlloc=fault_alloc;adjusted.onFree=fault_free;
    return xuiDocumentCreate(&adjusted,out);
}
/* Exercise every production importer buffer and Document allocation. The
 * copied entry is compiled from the unchanged production file, with only
 * allocator boundaries and its external symbol renamed. */
#define malloc fault_malloc
#define realloc fault_realloc
#define free fault_buffer_free
#define xuiDocumentCreate fault_create
#define doc_html_import_document fault_import_document
#include "../src/xui_document_html_import.c"
#undef doc_html_import_document
#undef xuiDocumentCreate
#undef free
#undef realloc
#undef malloc
static void faults(void)
{
    static const unsigned selected[]={0,5,10,17};
    unsigned f,total=0;
    for(f=0;f<sizeof(selected)/sizeof(selected[0]);f++) {
        const span_case* fixture=&cases[selected[f]];
        xui_document document=NULL;unsigned depth=0,points,i;
        fault_attempts=0;fault_budget=-1;
        SPAN_CHECK(fault_import_document(fixture->html,strlen(fixture->html),&document,&depth)==XUI_OK);
        points=fault_attempts;check_document(document,fixture);xuiDocumentRelease(document);
        SPAN_CHECK(fault_live==0 && points>0);
        for(i=0;i<points;i++) {
            int result;
            document=(xui_document)(uintptr_t)1;depth=99;fault_attempts=0;fault_budget=i;
            result=fault_import_document(fixture->html,strlen(fixture->html),&document,&depth);
            fault_budget=-1;
            SPAN_CHECK(result==XUI_ERROR_OUT_OF_MEMORY && document==NULL && depth==0);
            SPAN_CHECK(fault_live==0);
        }
        SPAN_CHECK(fault_import_document(fixture->html,strlen(fixture->html),&document,&depth)==XUI_OK);
        check_document(document,fixture);xuiDocumentRelease(document);SPAN_CHECK(fault_live==0);
        total+=points;
        printf("HTML rowspan importer OOM %s: %u points, null output, retry and zero leaks passed\n",fixture->name,points);
    }
    printf("HTML rowspan importer OOM total: %u points in 4 fixtures passed\n",total);
}
#endif
int main(void)
{
    unsigned i;
    setvbuf(stdout,NULL,_IONBF,0);
    for(i=0;i<sizeof(cases)/sizeof(cases[0]);i++) check_fixture(&cases[i]);
    generated(10,2,XUI_OK);generated(1,1024,XUI_OK);generated(1,1025,XUI_DOC_ERROR_LIMIT);
    {
        const char bad[]="<table><tbody><tr><td rowspan='0'>A</td><td>B</td></tr><tr><td>C</td><td>D</td></tr></tbody></table>";
        xui_document_fragment fragment=(xui_document_fragment)(uintptr_t)1;
        SPAN_CHECK(xuiDocumentFragmentImportHtml(bad,sizeof(bad)-1,&fragment)==XUI_DOC_ERROR_SCHEMA && fragment==NULL);
    }
    puts("HTML rowspan: 18 literal fixtures, pending-array growth,1024/1025 limits and invalid grid passed");
#if defined(XUI_HTML_ROWSPAN_IMPORT_FAULT_TEST)
    faults();
#endif
    return 0;
}
