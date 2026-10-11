#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#if !defined(XUI_DLL) && !defined(XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH)
#include "../src/xui_document_internal.h"
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"Input mapping line %d: %s\n",__LINE__,#c); exit(1); } } while (0)
static xui_document open_document(const char* source)
{
    xui_doc_desc_t desc={0}; xui_document document;
    desc.iSize=sizeof(desc); desc.iProfile=XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document,source,strlen(source))==XUI_OK); return document;
}
static xui_doc_source_patch_t patch(uint64_t first,uint64_t last,const char* source)
{
    xui_doc_source_patch_t p={0}; p.iSize=sizeof(p); p.iStart=first;p.iEnd=last;p.sText=source;p.iTextBytes=strlen(source);return p;
}
static xui_doc_prepare_info_t info(xui_document_prepare prepare)
{ xui_doc_prepare_info_t result={0};result.iSize=sizeof(result);CHECK(xuiDocumentPrepareGetInfo(prepare,&result)==XUI_OK);return result; }
static xui_doc_position_t base_position(xui_document document,uint64_t offset,unsigned affinity)
{
    xui_doc_position_t p={0};p.iSize=sizeof(p);p.iKind=XUI_DOC_POSITION_SOURCE;p.iNodeId=XUI_DOCUMENT_ROOT;
    p.iDocumentId=xuiDocumentGetIdentity(document);p.iRevision=xuiDocumentGetRevision(document);p.iOffset=offset;p.iAffinity=affinity;return p;
}
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
typedef struct publication_notice {
    xui_document_prepare prepare;
    xui_doc_position_t candidate;
    unsigned calls;
} publication_notice;
static void publication_changed(xui_document document,xui_document_change_set change,void* user)
{
    publication_notice* notice=user;xui_doc_position_t mapped;xui_doc_change_info_t ci={0};int quality;
    CHECK(info(notice->prepare).iState==XUI_DOC_PREPARE_PUBLISHING);
    ci.iSize=sizeof(ci);CHECK(xuiDocumentChangeSetGetInfo(change,&ci)==XUI_OK);
    CHECK(ci.iInputGeneration==notice->candidate.iInputGeneration);
    CHECK(xuiDocumentMapPosition(change,&notice->candidate,&mapped,&quality)==XUI_OK);
    CHECK(quality==XUI_DOC_MAP_EXACT && mapped.iOffset==4 && !mapped.iInputGeneration);
    CHECK(mapped.iRevision==xuiDocumentGetRevision(document));notice->calls++;
}
#endif
static void publication(void)
{
    xui_document document=open_document("ab\n"); xui_document_prepare prepare; xui_document_change_set change;
    xui_doc_source_patch_t p=patch(0,0,"PRE ");xui_doc_position_t at,mapped; int result,quality;
    uint64_t revision=xuiDocumentGetRevision(document);
    CHECK(xuiDocumentPrepareSource(document,NULL,&p,1,&prepare)==XUI_OK);
    CHECK(xuiDocumentPrepareSourcePosition(prepare,4,XUI_DOC_AFTER,&at)==XUI_OK);
    CHECK(xuiDocumentPrepareRun(prepare)==XUI_OK);
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
    publication_notice notice={prepare,at,0};uint64_t token;
    CHECK(xuiDocumentSubscribe(document,publication_changed,&notice,&token)==XUI_OK);
#endif
    CHECK(xuiDocumentPreparePublish(document,prepare,&change)==XUI_OK && change);
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
    CHECK(notice.calls==1);xuiDocumentUnsubscribe(document,token);
#endif
    result=xuiDocumentMapPosition(change,&at,&mapped,&quality);
    if(result!=XUI_OK)fprintf(stderr,"Input publication mapping returned %d\n",result);
    CHECK(result==XUI_OK && quality==XUI_DOC_MAP_EXACT);
    CHECK(mapped.iOffset==4 && !mapped.iInputGeneration && mapped.iRevision==revision+1);
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
    {
        xui_doc_change_info_t ci={0};xui_doc_position_t sentinel,bad;int unchanged=123;
        ci.iSize=sizeof(ci);CHECK(xuiDocumentChangeSetGetInfo(change,&ci)==XUI_OK && ci.iInputGeneration==info(prepare).iGeneration);
        memset(&sentinel,0xa5,sizeof(sentinel));mapped=sentinel;bad=at;bad.iInputGeneration++;
        CHECK(xuiDocumentMapPosition(change,&bad,&mapped,&unchanged)==XUI_DOC_ERROR_STALE);
        CHECK(!memcmp(&mapped,&sentinel,sizeof(mapped)) && unchanged==123);
        CHECK(xuiDocumentMapPosition(change,&at,&at,&quality)==XUI_OK && at.iOffset==4 && !at.iInputGeneration);
    }
#endif
    xuiDocumentChangeSetRelease(change);
    CHECK(xuiDocumentUndo(document,&change)==XUI_OK);
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
    { xui_doc_change_info_t ci={0};ci.iSize=sizeof(ci);CHECK(xuiDocumentChangeSetGetInfo(change,&ci)==XUI_OK && !ci.iInputGeneration); }
#endif
    xuiDocumentChangeSetRelease(change);CHECK(xuiDocumentRedo(document,&change)==XUI_OK);
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
    { xui_doc_change_info_t ci={0};ci.iSize=sizeof(ci);CHECK(xuiDocumentChangeSetGetInfo(change,&ci)==XUI_OK && !ci.iInputGeneration); }
#endif
    xuiDocumentChangeSetRelease(change);xuiDocumentPrepareRelease(prepare);xuiDocumentRelease(document);
    puts("Input publication: exact generation during callback, no double shift, alias, atomic stale and shared Undo/Redo passed");
}
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
static void source_is(xui_document_prepare prepare,const char* expected)
{ char text[128];uint64_t bytes;CHECK(xuiDocumentPrepareCopySource(prepare,text,sizeof(text),&bytes)==XUI_OK && bytes==strlen(expected) && !strcmp(text,expected)); }
static void expect_map(xui_document_prepare from,xui_document_prepare to,xui_doc_position_t at,uint64_t offset,int quality)
{
    xui_doc_position_t mapped;int actual;
    CHECK(xuiDocumentPrepareMapSourcePosition(from,to,&at,&mapped,&actual)==XUI_OK);
    CHECK(mapped.iOffset==offset && actual==quality && mapped.iInputGeneration==(to?info(to).iGeneration:0));
    CHECK(mapped.iRevision==at.iRevision && mapped.iDocumentId==at.iDocumentId && mapped.iAffinity==at.iAffinity);
    CHECK(xuiDocumentPrepareMapSourcePosition(from,to,&at,&at,&actual)==XUI_OK && at.iOffset==offset && actual==quality);
}
static void goldens(void)
{
    static const uint64_t positions[]={0,1,2,4,5,6,7};
    static const uint64_t first[][2]={{0,2},{3,3},{4,7},{7,7},{8,8},{9,9},{10,10}};
    static const uint64_t second[][2]={{0,2},{3,3},{4,4},{4,4},{5,5},{6,6},{7,11}};
    xui_document document=open_document("ab\xc3\xa9" "cd\n"),other;
    xui_document_prepare a,b,c,restarted,same,buffered,complete; xui_doc_source_patch_t edits[2];
    xui_doc_position_t at,sentinel,out; int unchanged=42; size_t i;unsigned affinity;
    edits[0]=patch(2,4,"XYZ");edits[1]=patch(0,0,"Q ");
    CHECK(xuiDocumentPrepareSource(document,NULL,edits,2,&a)==XUI_OK);source_is(a,"Q abXYZcd\n");
    edits[0]=patch(4,7,"");edits[1]=patch(7,7,"\xf0\x9f\x98\x80");
    CHECK(xuiDocumentPrepareContinueSource(document,a,edits,2,&b)==XUI_OK);source_is(b,"Q abcd\n\xf0\x9f\x98\x80");
    CHECK(xuiDocumentPrepareContinueSource(document,b,NULL,0,&c)==XUI_OK);
    for(i=0;i<sizeof(positions)/sizeof(*positions);i++)for(affinity=0;affinity<2;affinity++) {
        at=base_position(document,positions[i],affinity);
        expect_map(NULL,a,at,first[i][affinity],XUI_DOC_MAP_EXACT);
        expect_map(NULL,c,at,second[i][affinity],XUI_DOC_MAP_EXACT);
    }
    for(affinity=0;affinity<2;affinity++) {
        CHECK(xuiDocumentPrepareSourcePosition(a,5,affinity,&at)==XUI_OK);expect_map(a,c,at,4,XUI_DOC_MAP_DELETED);
        CHECK(xuiDocumentPrepareSourcePosition(c,4,affinity,&at)==XUI_OK);
        expect_map(c,a,at,affinity?7:4,XUI_DOC_MAP_EXACT);expect_map(c,NULL,at,affinity?4:2,XUI_DOC_MAP_EXACT);
        CHECK(xuiDocumentPrepareSourcePosition(c,11,affinity,&at)==XUI_OK);expect_map(c,a,at,10,XUI_DOC_MAP_EXACT);
        expect_map(c,c,at,11,XUI_DOC_MAP_EXACT);
    }
    memset(&sentinel,0xa5,sizeof(sentinel));out=sentinel;at=base_position(document,3,XUI_DOC_AFTER);
    CHECK(xuiDocumentPrepareMapSourcePosition(NULL,c,&at,&out,&unchanged)==XUI_DOC_ERROR_UTF8);
    CHECK(!memcmp(&out,&sentinel,sizeof(out)) && unchanged==42);
    at=base_position(document,2,XUI_DOC_AFTER);at.iInputGeneration=info(a).iGeneration;
    CHECK(xuiDocumentPrepareMapSourcePosition(NULL,c,&at,&out,&unchanged)==XUI_DOC_ERROR_STALE);
    CHECK(!memcmp(&out,&sentinel,sizeof(out)) && unchanged==42);
    other=open_document("ab\xc3\xa9" "cd\n");edits[0]=patch(0,0,"Q ");
    CHECK(xuiDocumentPrepareSource(other,NULL,edits,1,&same)==XUI_OK);
    CHECK(xuiDocumentPrepareSourcePosition(c,0,XUI_DOC_AFTER,&at)==XUI_OK);
    CHECK(xuiDocumentPrepareMapSourcePosition(c,same,&at,&out,&unchanged)==XUI_DOC_ERROR_STALE);
    xuiDocumentPrepareRelease(same);xuiDocumentRelease(other);
    CHECK(xuiDocumentCancelPrepare(document)==XUI_OK);
    CHECK(xuiDocumentPrepareSourcePosition(a,5,XUI_DOC_AFTER,&at)==XUI_OK);expect_map(a,c,at,4,XUI_DOC_MAP_DELETED);
    edits[0]=patch(2,4,"XYZ");edits[1]=patch(0,0,"Q ");
    CHECK(xuiDocumentPrepareSource(document,NULL,edits,2,&restarted)==XUI_OK);
    CHECK(xuiDocumentPrepareMapSourcePosition(a,restarted,&at,&out,&unchanged)==XUI_DOC_ERROR_STALE);
    CHECK(!memcmp(&out,&sentinel,sizeof(out)) && unchanged==42);
    CHECK(xuiDocumentPrepareSourcePosition(restarted,5,XUI_DOC_AFTER,&at)==XUI_OK);
    CHECK(xuiDocumentPrepareMapSourcePosition(restarted,a,&at,&out,&unchanged)==XUI_DOC_ERROR_STALE);
    CHECK(xuiDocumentCancelPrepare(document)==XUI_OK);
    CHECK(xuiDocumentPrepareStreamSource(document,NULL,NULL,"\xf0\x9f",2,0,&buffered)==XUI_OK);
    CHECK(info(buffered).iBufferedUtf8Bytes==2 && info(buffered).iSourceBytes==7);
    CHECK(xuiDocumentPrepareStreamSource(document,buffered,NULL,"\x98\x80",2,1,&complete)==XUI_OK);
    CHECK(xuiDocumentPrepareSourcePosition(buffered,7,XUI_DOC_AFTER,&at)==XUI_OK);expect_map(buffered,complete,at,11,XUI_DOC_MAP_EXACT);
    at.iAffinity=XUI_DOC_BEFORE;expect_map(buffered,complete,at,7,XUI_DOC_MAP_EXACT);
    CHECK(xuiDocumentCancelPrepare(document)==XUI_OK);
    xuiDocumentPrepareRelease(a);xuiDocumentPrepareRelease(b);xuiDocumentPrepareRelease(c);xuiDocumentPrepareRelease(restarted);
    xuiDocumentPrepareRelease(buffered);xuiDocumentPrepareRelease(complete);xuiDocumentRelease(document);
    puts("Input lineage: literal batch/UTF8/affinity/skipped/reverse/deleted/alias/stale/restart/cancel/stream goldens passed");
}
#ifndef XUI_DLL
static void preview_forks(void)
{
    xui_document document=open_document("xyz\n");xui_document_prepare root,left,right,child;
    xui_doc_source_patch_t p=patch(0,0,"A");xui_doc_position_t at,out,sentinel;int quality=91;
    CHECK(xuiDocumentPrepareSource(document,NULL,NULL,0,&root)==XUI_OK);
    CHECK(doc_prepare_preview(document,root,&p,&left)==XUI_OK);
    CHECK(doc_prepare_preview(document,root,&p,&right)==XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(document,root,&p,1,&child)==XUI_OK);
    source_is(left,"Axyz\n");source_is(right,"Axyz\n");source_is(child,"Axyz\n");
    memset(&sentinel,0xa5,sizeof(sentinel));out=sentinel;
    CHECK(xuiDocumentPrepareSourcePosition(left,2,XUI_DOC_AFTER,&at)==XUI_OK);
    CHECK(xuiDocumentPrepareMapSourcePosition(left,right,&at,&out,&quality)==XUI_DOC_ERROR_STALE);
    CHECK(xuiDocumentPrepareMapSourcePosition(left,child,&at,&out,&quality)==XUI_DOC_ERROR_STALE);
    CHECK(!memcmp(&out,&sentinel,sizeof(out)) && quality==91);
    CHECK(xuiDocumentPrepareSourcePosition(root,1,XUI_DOC_AFTER,&at)==XUI_OK);
    expect_map(root,left,at,2,XUI_DOC_MAP_EXACT);expect_map(root,right,at,2,XUI_DOC_MAP_EXACT);
    CHECK(xuiDocumentPrepareSourcePosition(right,2,XUI_DOC_AFTER,&at)==XUI_OK);
    expect_map(right,root,at,1,XUI_DOC_MAP_EXACT);
    CHECK(xuiDocumentCancelPrepare(document)==XUI_OK);
    xuiDocumentRelease(document); /* Retained immutable handles outlive the owner. */
    expect_map(right,root,at,1,XUI_DOC_MAP_EXACT);
    xuiDocumentPrepareRelease(root);xuiDocumentPrepareRelease(left);xuiDocumentPrepareRelease(right);xuiDocumentPrepareRelease(child);
    puts("Input lineage: identical-byte detached siblings rejected, ancestor/reverse after owner release passed");
}
#endif
typedef struct allocator_state { atomic_uint_fast64_t calls,live,fail_at;atomic_int arm,entered,release; } allocator_state;
static void pause_worker(void)
{
#ifdef _WIN32
    Sleep(1);
#else
    sched_yield();
#endif
}
static void* allocate(void* user,size_t bytes)
{
    allocator_state* a=user;uint64_t call=atomic_fetch_add(&a->calls,1)+1;void* memory;
    if(atomic_exchange(&a->arm,0)) { atomic_store(&a->entered,1);while(!atomic_load(&a->release))pause_worker(); }
    if(call==atomic_load(&a->fail_at))return NULL;
    memory=malloc(bytes);if(memory)atomic_fetch_add(&a->live,1);return memory;
}
static void deallocate(void* user,void* memory)
{ allocator_state* a=user;if(memory){CHECK(atomic_fetch_sub(&a->live,1)>0);free(memory);} }
static xui_document allocated_document(allocator_state* allocator)
{
    xui_doc_desc_t desc={0};xui_document document;
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.onAlloc=allocate;desc.onFree=deallocate;desc.pAllocatorUser=allocator;
    CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);CHECK(xuiDocumentLoadMarkdown(document,"xyz\n",4)==XUI_OK);return document;
}
static void faults(void)
{
    unsigned continued;
    for(continued=0;continued<2;continued++) {
      uint64_t count=0,point;
      for(point=0;!point || point<=count;point++) {
        allocator_state allocator={0};xui_document document=allocated_document(&allocator);xui_document_prepare previous,next;
        xui_doc_source_patch_t p=patch(0,0,"A");uint64_t calls,live,revision;int result;
        CHECK(xuiDocumentPrepareSource(document,NULL,NULL,0,&previous)==XUI_OK);
        calls=atomic_load(&allocator.calls);live=atomic_load(&allocator.live);revision=xuiDocumentGetRevision(document);
        if(point)atomic_store(&allocator.fail_at,calls+point);
        result=continued?xuiDocumentPrepareContinueSource(document,previous,&p,1,&next):xuiDocumentPrepareSource(document,NULL,&p,1,&next);
        if(!point){CHECK(result==XUI_OK);count=atomic_load(&allocator.calls)-calls;}
        else {
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !next);
            CHECK(atomic_load(&allocator.live)==live && xuiDocumentGetRevision(document)==revision);
            CHECK(!info(previous).bCancellationRequested && xuiDocumentHasPrepare(document));source_is(previous,"xyz\n");
            atomic_store(&allocator.fail_at,0);
            result=continued?xuiDocumentPrepareContinueSource(document,previous,&p,1,&next):xuiDocumentPrepareSource(document,NULL,&p,1,&next);
            CHECK(result==XUI_OK);
        }
        source_is(next,"Axyz\n");calls=atomic_load(&allocator.calls);
        { xui_doc_position_t at;CHECK(xuiDocumentPrepareSourcePosition(previous,1,XUI_DOC_AFTER,&at)==XUI_OK);
          if(continued)expect_map(previous,next,at,2,XUI_DOC_MAP_EXACT);
          else { at.iInputGeneration=0;expect_map(NULL,next,at,2,XUI_DOC_MAP_EXACT); }
          CHECK(xuiDocumentPrepareSourcePosition(next,0,XUI_DOC_BEFORE,&at)==XUI_OK);expect_map(next,NULL,at,0,XUI_DOC_MAP_EXACT); }
        CHECK(atomic_load(&allocator.calls)==calls);
        CHECK(xuiDocumentCancelPrepare(document)==XUI_OK);xuiDocumentPrepareRelease(previous);xuiDocumentPrepareRelease(next);xuiDocumentRelease(document);
        CHECK(!atomic_load(&allocator.live));
      }
      printf("Input lineage creation faults continued=%u: %llu points, pending/revision/live atomic, retry/no-query-alloc/zero-leaks passed\n",continued,(unsigned long long)count);
    }
}
static void long_chain(void)
{
    const uint64_t size=UINT64_C(10)*1024*1024;char* source=malloc((size_t)size);
    allocator_state allocator={0};xui_document document=allocated_document(&allocator);
    xui_document_prepare first,current,next,middle=NULL;xui_doc_source_patch_t p;
    xui_doc_stats_t before={0},after={0};xui_doc_position_t at;uint64_t live,calls;unsigned i;
    CHECK(source);memset(source,'a',(size_t)size);p=patch(0,4,"");p.sText=source;p.iTextBytes=size;
    CHECK(xuiDocumentPrepareSource(document,NULL,&p,1,&first)==XUI_OK);free(source);
    current=first;xuiDocumentPrepareRetain(current);before.iSize=after.iSize=sizeof(before);
    CHECK(xuiDocumentGetStats(document,&before)==XUI_OK);live=before.iLiveBytes;
    for(i=0;i<2048;i++) {
        p=patch(size+i,size+i,"b");CHECK(xuiDocumentPrepareContinueSource(document,current,&p,1,&next)==XUI_OK);
        xuiDocumentPrepareRelease(current);current=next;
        if(i==1023) {middle=current;xuiDocumentPrepareRetain(middle);}
        CHECK(xuiDocumentGetStats(document,&after)==XUI_OK && after.iLiveBytes<live+65536);live=after.iLiveBytes;
    }
    CHECK(after.iMarkdownParses==before.iMarkdownParses && after.iLiveBytes<before.iLiveBytes+2048*4096);
    CHECK(info(current).iPatchCount==2049 && info(current).iSourceBytes==size+2048);
    calls=atomic_load(&allocator.calls);
    for(i=0;i<32;i++) {
        CHECK(xuiDocumentPrepareSourcePosition(first,size,XUI_DOC_AFTER,&at)==XUI_OK);
        expect_map(first,current,at,size+2048,XUI_DOC_MAP_EXACT);
        at.iAffinity=XUI_DOC_BEFORE;expect_map(first,current,at,size,XUI_DOC_MAP_EXACT);
        CHECK(xuiDocumentPrepareSourcePosition(middle,size+1024,XUI_DOC_AFTER,&at)==XUI_OK);
        expect_map(middle,current,at,size+2048,XUI_DOC_MAP_EXACT);
        CHECK(xuiDocumentPrepareSourcePosition(current,size+2048,XUI_DOC_AFTER,&at)==XUI_OK);
        expect_map(current,first,at,size,XUI_DOC_MAP_EXACT);
    }
    CHECK(atomic_load(&allocator.calls)==calls);
    CHECK(xuiDocumentCancelPrepare(document)==XUI_OK);xuiDocumentRelease(document);
    expect_map(current,first,at,size,XUI_DOC_MAP_EXACT);
    xuiDocumentPrepareRelease(first);xuiDocumentPrepareRelease(middle);xuiDocumentPrepareRelease(current);
    CHECK(!atomic_load(&allocator.live));
    printf("Input lineage: 2048 continuations on 10 MiB, bounded retained growth %llu, skipped/reverse/no-query-alloc/owner-release/zero-leaks passed\n",
        (unsigned long long)(after.iLiveBytes-before.iLiveBytes));
}
typedef struct worker_state { xui_document_prepare prepare;int result; } worker_state;
#ifdef _WIN32
static DWORD WINAPI run_worker(void* data)
#else
static void* run_worker(void* data)
#endif
{ worker_state* worker=data;worker->result=xuiDocumentPrepareRun(worker->prepare);return 0; }
static void concurrent(void)
{
    allocator_state allocator={0};xui_document document=allocated_document(&allocator);xui_document_prepare prepare;
    xui_doc_source_patch_t p=patch(0,0,"PRE ");xui_doc_position_t at=base_position(document,2,XUI_DOC_AFTER);worker_state worker;
    uint64_t calls;int i;
    CHECK(xuiDocumentPrepareSource(document,NULL,&p,1,&prepare)==XUI_OK);worker.prepare=prepare;worker.result=99;
    atomic_store(&allocator.arm,1);
#ifdef _WIN32
    HANDLE thread=CreateThread(NULL,0,run_worker,&worker,0,NULL);CHECK(thread);
#else
    pthread_t thread;CHECK(!pthread_create(&thread,NULL,run_worker,&worker));
#endif
    while(!atomic_load(&allocator.entered))pause_worker();
    CHECK(info(prepare).iState==XUI_DOC_PREPARE_RUNNING);
    calls=atomic_load(&allocator.calls);
    for(i=0;i<5000;i++)expect_map(NULL,prepare,at,6,XUI_DOC_MAP_EXACT);
    CHECK(atomic_load(&allocator.calls)==calls);atomic_store(&allocator.release,1);
#ifdef _WIN32
    CHECK(WaitForSingleObject(thread,10000)==WAIT_OBJECT_0);CloseHandle(thread);
#else
    CHECK(!pthread_join(thread,NULL));
#endif
    CHECK(worker.result==XUI_OK);CHECK(xuiDocumentPreparePublish(document,prepare,NULL)==XUI_OK);
    xuiDocumentPrepareRelease(prepare);xuiDocumentRelease(document);CHECK(!atomic_load(&allocator.live));
    puts("Input lineage: 5000 concurrent queries during actual blocked RUNNING, no allocation and zero leaks passed");
}
#endif
int main(void)
{
    publication();
#ifndef XUI_DOC_INPUT_MAP_NEGATIVE_PUBLISH
    goldens();
#ifndef XUI_DLL
    preview_forks();
#endif
    faults();long_chain();concurrent();
#endif
    puts("Unified Document input mapping tests passed");return 0;
}
