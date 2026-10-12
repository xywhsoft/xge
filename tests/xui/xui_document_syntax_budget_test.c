#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LIMIT_CHECK(x) do{if(!(x)){fprintf(stderr,"syntax limit line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct limit_allocator{unsigned live;}limit_allocator;
static void* limit_alloc(void* user,size_t bytes)
{limit_allocator* a=user;void* p=malloc(bytes);if(p)a->live++;return p;}
static void limit_free(void* user,void* p)
{limit_allocator* a=user;LIMIT_CHECK(a->live);a->live--;free(p);}
static void exact_source(xui_document_snapshot s,const char* expected)
{
    char actual[256];uint64_t size=0;
    LIMIT_CHECK(xuiDocumentSnapshotCopySource(s,actual,sizeof(actual),&size)==XUI_OK);
    LIMIT_CHECK(size==strlen(expected) && !memcmp(actual,expected,(size_t)size+1));
}
int main(void)
{
    const char* source[]={"","abc","**abc**","[unused]: /uri\n","---\nkey: value\n---\n",
        "********abc********","[a][b][c][d][e][f]","[^unused]: ********body********\n"};
    uint64_t nodes[]={1,3,3,1,2,3,3,1};unsigned i,minimum[8]={0};
    for(i=0;i<sizeof(source)/sizeof(*source);i++){
        uint64_t cap;int reached=0;
        for(cap=1;cap<=128;cap++){
            limit_allocator a={0};xui_doc_desc_t desc={0};xui_document d;xui_document_snapshot before,after;
            xui_doc_stats_t stats={0};uint64_t revision;unsigned live;int result;
            desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
            desc.iMaxNodes=nodes[i];desc.iMaxSyntaxRecords=cap;desc.onAlloc=limit_alloc;desc.onFree=limit_free;desc.pAllocatorUser=&a;
            LIMIT_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);
            LIMIT_CHECK(xuiDocumentAcquireSnapshot(d,&before)==XUI_OK && xuiDocumentMarkSaved(d,before)==XUI_OK);
            revision=xuiDocumentGetRevision(d);live=a.live;
            result=xuiDocumentLoadMarkdown(d,source[i],strlen(source[i]));stats.iSize=sizeof(stats);
            LIMIT_CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK);
            if(result==XUI_OK){
                LIMIT_CHECK(stats.iNodes<=nodes[i]);minimum[i]=(unsigned)cap;reached=1;
                LIMIT_CHECK(xuiDocumentAcquireSnapshot(d,&after)==XUI_OK);exact_source(after,source[i]);
                xuiDocumentSnapshotRelease(after);
            }else{
                if(result!=XUI_DOC_ERROR_LIMIT)fprintf(stderr,"fixture=%u cap=%llu actual error=%d\n",i,(unsigned long long)cap,result);
                LIMIT_CHECK(result==XUI_DOC_ERROR_LIMIT && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d));
                LIMIT_CHECK(!stats.iUndoCount && !stats.iRedoCount && stats.iNodes==1 && a.live==live);
                LIMIT_CHECK(xuiDocumentAcquireSnapshot(d,&after)==XUI_OK);exact_source(after,"");
                xuiDocumentSnapshotRelease(after);
            }
            xuiDocumentRelease(d);exact_source(before,"");xuiDocumentSnapshotRelease(before);LIMIT_CHECK(!a.live);
            if(reached)break;
        }
        if(!reached)fprintf(stderr,"fixture=%u never succeeded with semantic cap %llu\n",i,(unsigned long long)nodes[i]);
        LIMIT_CHECK(reached);
        printf("Independent syntax limit fixture=%u semantic-cap=%llu minimum-explicit-cap=%u\n",i,(unsigned long long)nodes[i],minimum[i]);
    }
    /* A syntax budget must not bypass the actual semantic node cap. */
    {
        xui_doc_desc_t desc={0};xui_document d;xui_doc_stats_t stats={0};
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMaxNodes=2;desc.iMaxSyntaxRecords=UINT64_MAX;
        LIMIT_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);
        LIMIT_CHECK(xuiDocumentLoadMarkdown(d,"abc",3)==XUI_DOC_ERROR_LIMIT);
        stats.iSize=sizeof(stats);LIMIT_CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK && stats.iNodes==1 && !stats.iUndoCount);
        xuiDocumentRelease(d);
    }
    /* Failed synchronous and worker preparations preserve a nonempty saved
     * document and its existing history, then a valid edit remains possible. */
    {
        limit_allocator a={0};xui_doc_desc_t desc={0};xui_document d;xui_document_snapshot saved,current;
        xui_doc_stats_t before={0},after={0};xui_document_prepare pending;xui_doc_prepare_info_t info={0};
        xui_doc_source_patch_t patch={0};unsigned live;uint64_t revision;
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMaxNodes=3;desc.iMaxSyntaxRecords=8;
        desc.onAlloc=limit_alloc;desc.onFree=limit_free;desc.pAllocatorUser=&a;
        LIMIT_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);
        LIMIT_CHECK(xuiDocumentLoadMarkdown(d,"seed",4)==XUI_OK && xuiDocumentLoadMarkdown(d,"saved",5)==XUI_OK);
        LIMIT_CHECK(xuiDocumentAcquireSnapshot(d,&saved)==XUI_OK && xuiDocumentMarkSaved(d,saved)==XUI_OK);
        before.iSize=sizeof(before);LIMIT_CHECK(xuiDocumentGetStats(d,&before)==XUI_OK);
        live=a.live;revision=xuiDocumentGetRevision(d);
        LIMIT_CHECK(xuiDocumentLoadMarkdown(d,"********abc********",19)==XUI_DOC_ERROR_LIMIT);
        LIMIT_CHECK(a.live==live && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d));
        patch.iSize=sizeof(patch);patch.iEnd=5;patch.sText="********abc********";patch.iTextBytes=19;
        LIMIT_CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&pending)==XUI_OK);
        LIMIT_CHECK(xuiDocumentPrepareRun(pending)==XUI_DOC_ERROR_LIMIT);
        info.iSize=sizeof(info);LIMIT_CHECK(xuiDocumentPrepareGetInfo(pending,&info)==XUI_OK);
        LIMIT_CHECK(info.iState==XUI_DOC_PREPARE_FAILED && info.iResult==XUI_DOC_ERROR_LIMIT);
        LIMIT_CHECK(xuiDocumentPreparePublish(d,pending,NULL)==XUI_DOC_ERROR_LIMIT);
        LIMIT_CHECK(xuiDocumentCancelPrepare(d)==XUI_OK);xuiDocumentPrepareRelease(pending);
        after.iSize=sizeof(after);LIMIT_CHECK(xuiDocumentGetStats(d,&after)==XUI_OK);
        LIMIT_CHECK(a.live==live && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d));
        LIMIT_CHECK(after.iNodes==before.iNodes && after.iUndoCount==before.iUndoCount && after.iRedoCount==before.iRedoCount);
        LIMIT_CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);exact_source(current,"saved");xuiDocumentSnapshotRelease(current);
        LIMIT_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && xuiDocumentRedo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d));
        LIMIT_CHECK(xuiDocumentLoadMarkdown(d,"**ok**",6)==XUI_OK);
        LIMIT_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d));
        xuiDocumentRelease(d);exact_source(saved,"saved");xuiDocumentSnapshotRelease(saved);LIMIT_CHECK(!a.live);
    }
    puts("Separate semantic/grammar limits: eight fixtures and all lower caps; exact error, atomic source/history/savepoint, worker failure, recovery and retained lifetime passed.");
    return 0;
}
