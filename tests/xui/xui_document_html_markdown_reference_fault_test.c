#define main reference_driver_main
#include "xui_document_html_markdown_reference_test.c"
#undef main
#ifndef XUI_DLL
#include "../src/xui/document/xui_document_internal.h"
#include <stdatomic.h>
#endif

typedef struct fault_allocator {
    long remaining;
    uint64_t live;
#ifndef XUI_DLL
    long cancel_after;
    atomic_int cancellation;
#endif
} fault_allocator;
static void* fault_alloc(void* user,size_t bytes)
{
    fault_allocator* a=user;void* result;
#ifndef XUI_DLL
    if(!a->cancel_after)atomic_store(&a->cancellation,XUI_DOC_ERROR_CANCELLED);
    else if(a->cancel_after>0)a->cancel_after--;
#endif
    if(!a->remaining)return NULL;
    if(a->remaining>0)a->remaining--;
    result=malloc(bytes);if(result)a->live++;return result;
}
static void fault_free(void* user,void* pointer)
{
    fault_allocator* a=user;if(pointer){CHECK(a->live);a->live--;free(pointer);}
}
static int command(xui_document document,xui_document_fragment fragment,uint64_t id,
    const char* info,unsigned insert,fault_allocator* a,long point,unsigned cancel,
    xui_document_change_set* changes)
{
    xui_document_transaction txn=NULL;uint64_t first=UINT64_MAX,count=UINT64_MAX;int result;
    CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
#ifndef XUI_DLL
    if(cancel){txn->cancellation=&a->cancellation;a->cancel_after=point;}
    else
#else
    CHECK(!cancel);
#endif
        a->remaining=point;
    if(insert){
        result=xuiDocumentTxnInsertFragment(txn,fragment,XUI_DOCUMENT_ROOT,0,&first,&count);
        if(result!=XUI_OK)CHECK(!first && !count);
    }else result=xuiDocumentTxnSetResource(txn,id,NULL,info,NULL);
    if(result==XUI_OK)result=xuiDocumentTxnCommit(txn,changes);
    a->remaining=-1;
#ifndef XUI_DLL
    a->cancel_after=-1;
#endif
    if(result!=XUI_OK)CHECK(xuiDocumentTxnCommit(txn,NULL)==result);
    xuiDocumentTxnRelease(txn);
#ifndef XUI_DLL
    atomic_store(&a->cancellation,0);
#endif
    return result;
}
int main(void)
{
    const char* originals[]={
        "lead &amp;\r\n\r\n> ```c keep-meta &copy; \r\n> Z\r\n> ```\r\n\r\ntail [r]\r\n\r\n[r]: /keep \"unchanged\"\r\n",
        "tail [r]\r\n\r\n[r]: /keep \"unchanged\"\r\n"
    };
    const char pattern[]="A\r\n\t ~`\\&";
    char language[4097],html[32768];size_t i,at=0;
    xui_document_fragment fragment=NULL;unsigned insert,cancel,cancellation_modes=1;
#ifndef XUI_DLL
    cancellation_modes=2;
#endif
    for(i=0;i<sizeof(language)-1;i++) language[i]=pattern[i%(sizeof(pattern)-1)];
    language[i]=0;
    at=(size_t)snprintf(html,sizeof(html),"<pre><code data-language='");
    for(i=0;language[i];i++){
        const char* entity=language[i]=='\r'?"&#13;":language[i]=='\n'?"&#10;":language[i]=='\t'?"&#9;":language[i]=='&'?"&amp;":NULL;
        if(entity){size_t n=strlen(entity);CHECK(at+n<sizeof(html));memcpy(html+at,entity,n);at+=n;}
        else{CHECK(at+1<sizeof(html));html[at++]=language[i];}
    }
    CHECK(at+sizeof("'>Z&#10;</code></pre>")<sizeof(html));strcpy(html+at,"'>Z&#10;</code></pre>");
    CHECK(xuiDocumentFragmentImportHtml(html,strlen(html),&fragment)==XUI_OK);
    for(insert=0;insert<2;insert++)for(cancel=0;cancel<cancellation_modes;cancel++){
        long point;unsigned failures=0;
        for(point=0;point<8192;point++){
            fault_allocator a={0};xui_doc_desc_t desc={0};xui_document document=NULL;
            xui_document_snapshot old=NULL,current=NULL; xui_document_change_set changes=NULL;
            char after[65536],restored[65536];uint64_t revision,id=0,bytes,cut=0;int result;
            reference_case fixture={"long-info",html,"Z\n",language};const char* original=originals[insert];
            a.remaining=-1;
#ifndef XUI_DLL
            a.cancel_after=-1;atomic_init(&a.cancellation,0);
#endif
            desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_GFM;
            desc.onAlloc=fault_alloc;desc.onFree=fault_free;desc.pAllocatorUser=&a;
            CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
            CHECK(xuiDocumentLoadMarkdown(document,original,strlen(original))==XUI_OK);
            CHECK(xuiDocumentClearHistory(document)==XUI_OK && xuiDocumentAcquireSnapshot(document,&old)==XUI_OK);
            CHECK(xuiDocumentMarkSaved(document,old)==XUI_OK);revision=xuiDocumentGetRevision(document);
            if(!insert){id=code_id(old,XUI_DOCUMENT_ROOT);CHECK(id);cut=(uint64_t)(strstr(original,"c keep-meta")-original);}
            result=command(document,fragment,id,language,insert,&a,point,cancel,&changes);
            CHECK(result==XUI_OK || result==(cancel?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
            if(result!=XUI_OK){
                failures++;CHECK(!changes && xuiDocumentGetRevision(document)==revision);
                CHECK(!xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
                CHECK(source_copy(document,restored,sizeof(restored))==strlen(original) && !strcmp(restored,original));
                CHECK(command(document,fragment,id,language,insert,&a,-1,0,&changes)==XUI_OK);
            }
            CHECK(xuiDocumentGetRevision(document)==revision+1 && xuiDocumentIsDirty(document) && xuiDocumentCanUndo(document));
            CHECK(xuiDocumentAcquireSnapshot(document,&current)==XUI_OK);
            code_content(current,code_id(current,XUI_DOCUMENT_ROOT),&fixture);
            bytes=source_copy(document,after,sizeof(after));
            if(insert)CHECK(bytes>=strlen(original) && !memcmp(after+bytes-strlen(original),original,strlen(original)));
            else CHECK(bytes>=cut+strlen(original+cut+1) && !memcmp(after,original,(size_t)cut) &&
                !memcmp(after+bytes-strlen(original+cut+1),original+cut+1,strlen(original+cut+1)));
            CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document));
            CHECK(source_copy(document,restored,sizeof(restored))==strlen(original) && !strcmp(restored,original));
            CHECK(xuiDocumentRedo(document,NULL)==XUI_OK);
            CHECK(source_copy(document,restored,sizeof(restored))==bytes && !memcmp(after,restored,(size_t)bytes));
            CHECK(xuiDocumentSnapshotCopySource(old,restored,sizeof(restored),&bytes)==XUI_OK && !strcmp(restored,original));
            xuiDocumentChangeSetRelease(changes);xuiDocumentSnapshotRelease(current);xuiDocumentSnapshotRelease(old);
            xuiDocumentRelease(document);CHECK(a.live==0);
            if(result==XUI_OK)break;
        }
        CHECK(point<8192 && failures==(unsigned)point);
        printf("Fence info fault insert=%u cancel=%u: %ld points; atomic/retry/old-snapshot/protected-source/single-history/zero-leaks passed\n",insert,cancel,point);
    }
    xuiDocumentFragmentRelease(fragment);
    printf("Fence info faults:%u exhaustive4096-byte language OOM/cancel scans passed\n",2*cancellation_modes);return 0;
}
