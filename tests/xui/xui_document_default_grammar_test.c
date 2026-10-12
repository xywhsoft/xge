#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"default source-only line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct allocator{unsigned live;}allocator;
static void* allocate(void* user,size_t size){allocator* a=user;void* p=malloc(size);if(p)a->live++;return p;}
static void release(void* user,void* p){allocator* a=user;CHECK(a->live);a->live--;free(p);}
int main(void)
{
    const unsigned clauses=300000;size_t bytes=11+(size_t)clauses*6+1;char* source=malloc(bytes+1);
    unsigned mode,i;CHECK(source);memcpy(source,"[^unused]: ",11);
    for(i=0;i<clauses;i++)memcpy(source+11+(size_t)i*6,"**x** ",6);
    source[bytes-1]='\n';source[bytes]=0;
    for(mode=0;mode<2;mode++){
        allocator a={0};xui_doc_desc_t desc={0};xui_document d;xui_document_snapshot before,after;
        xui_doc_stats_t stats={0};int result;unsigned live;uint64_t revision;
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMaxNodes=1;desc.bDisableHistory=1;
        desc.iMaxSyntaxRecords=mode?1000000:0;desc.onAlloc=allocate;desc.onFree=release;desc.pAllocatorUser=&a;
        CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&before)==XUI_OK);
        CHECK(xuiDocumentMarkSaved(d,before)==XUI_OK);live=a.live;revision=xuiDocumentGetRevision(d);
        result=xuiDocumentLoadMarkdown(d,source,bytes);stats.iSize=sizeof(stats);CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK);
        CHECK(stats.iNodes==1 && !stats.iUndoCount && !stats.iRedoCount);
        if(mode){
            CHECK(result==XUI_DOC_ERROR_LIMIT && a.live==live && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d));
        }else{
            char chunk[4096];uint64_t copied=0,at=0,total;
            CHECK(result==XUI_OK && xuiDocumentAcquireSnapshot(d,&after)==XUI_OK);
            CHECK(xuiDocumentSnapshotCopySource(after,NULL,0,&total)==XUI_OK && total==bytes);
            while(at<total){uint64_t count=total-at>sizeof(chunk)?sizeof(chunk):total-at;
                CHECK(xuiDocumentSnapshotReadSource(after,at,chunk,count)==XUI_OK && !memcmp(chunk,source+at,(size_t)count));at+=count;copied+=count;}
            CHECK(copied==bytes);xuiDocumentSnapshotRelease(after);
        }
        printf("Default grammar independence mode=%u source=%zu clauses=%u semantic-nodes=%llu result=%d current=%llu\n",
            mode,bytes,clauses,(unsigned long long)stats.iNodes,result,(unsigned long long)stats.iCurrentBytes);
        xuiDocumentRelease(d);{char empty[2];uint64_t size;CHECK(xuiDocumentSnapshotCopySource(before,empty,sizeof(empty),&size)==XUI_OK && !size && !empty[0]);}
        xuiDocumentSnapshotRelease(before);CHECK(!a.live);
    }
    free(source);puts("Default source-only grammar admits >1m records with semantic cap1; explicit cap1m rejects atomically, saved snapshot survives and zero leaks.");return 0;
}
