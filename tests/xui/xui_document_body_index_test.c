#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BODY_CHECK(x) do{if(!(x)){fprintf(stderr,"body source line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct body_allocator{long remaining;unsigned live;}body_allocator;
static void* body_alloc(void* user,size_t size)
{body_allocator* a=user;void* p;if(!a->remaining)return NULL;if(a->remaining>0)a->remaining--;p=malloc(size);if(p)a->live++;return p;}
static void body_free(void* user,void* p)
{body_allocator* a=user;BODY_CHECK(a->live);a->live--;free(p);}
static xui_doc_source_binding_t body_role(xui_document_source_index index,uint32_t syntax,
    uint32_t token,uint32_t detail,uint32_t flags,uint64_t start,uint64_t end)
{
    xui_doc_source_index_info_t info={0};xui_doc_source_binding_t found={0};uint64_t i,matches=0;
    info.iSize=sizeof(info);BODY_CHECK(xuiDocumentSourceIndexGetInfo(index,&info)==XUI_OK);
    for(i=0;i<info.iBindingCount;i++){
        xui_doc_source_binding_t b={0};xui_doc_source_record_t r={0};b.iSize=sizeof(b);r.iSize=sizeof(r);
        BODY_CHECK(xuiDocumentSourceIndexGetBinding(index,i,&b)==XUI_OK);
        if(b.iOriginKind!=XUI_DOC_SOURCE_BINDING_DOCUMENT_SYNTAX || b.tRange.iKind!=token || b.tRange.iDetail!=detail ||
            b.tRange.iSourceStart!=start || b.tRange.iSourceEnd!=end)continue;
        BODY_CHECK(xuiDocumentSourceIndexGetRecord(index,b.tRange.iRecordIndex,&r)==XUI_OK);
        if(r.iKind!=XUI_DOC_SOURCE_RECORD_SYNTAX || r.iDetail!=syntax || (r.iFlags&flags)!=flags)continue;
        BODY_CHECK(r.iFlags&XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT);
        BODY_CHECK(!r.iNodeId && !b.tRange.iNodeId && (b.iMetadata&XUI_DOC_SOURCE_BINDING_SOURCE_ONLY));
        BODY_CHECK(b.tRange.iTextStart==UINT64_MAX && b.tRange.iTextEnd==UINT64_MAX);
        BODY_CHECK(b.tRange.iOriginalSourceStart<=start && end<=b.tRange.iOriginalSourceEnd);
        found=b;matches++;
    }
    if(matches!=1)fprintf(stderr,"body role syntax=%u token=%u detail=%u flags=%u range=%llu..%llu matches=%llu\n",
        syntax,token,detail,flags,(unsigned long long)start,(unsigned long long)end,(unsigned long long)matches);
    BODY_CHECK(matches==1);return found;
}
static void body_partition(xui_document_source_index index,const char* source)
{
    xui_doc_source_index_info_t info={0};uint64_t i,end=0;char bytes[256];info.iSize=sizeof(info);
    BODY_CHECK(xuiDocumentSourceIndexGetInfo(index,&info)==XUI_OK && info.iSourceBytes==strlen(source));
    for(i=0;i<info.iRecordCount;i++){
        xui_doc_source_record_t r={0},p={0};r.iSize=p.iSize=sizeof(r);
        BODY_CHECK(xuiDocumentSourceIndexGetRecord(index,i,&r)==XUI_OK);
        if(!i)BODY_CHECK(r.iParentIndex==UINT64_MAX);
        else{
            BODY_CHECK(r.iParentIndex<i && xuiDocumentSourceIndexGetRecord(index,r.iParentIndex,&p)==XUI_OK);
            if(!(r.iFlags&XUI_DOC_SOURCE_RECORD_UNMAPPED))BODY_CHECK(p.iSourceStart<=r.iSourceStart && r.iSourceEnd<=p.iSourceEnd);
        }
    }
    for(i=0;i<info.iTokenCount;i++){
        xui_doc_source_token_t t={0};uint64_t ordinal;t.iSize=sizeof(t);
        BODY_CHECK(xuiDocumentSourceIndexGetToken(index,i,&t)==XUI_OK && t.iSourceStart==end && t.iSourceEnd>end);
        BODY_CHECK(t.iSourceEnd-t.iSourceStart<=sizeof(bytes));
        BODY_CHECK(xuiDocumentSourceIndexReadSource(index,t.iSourceStart,bytes,t.iSourceEnd-t.iSourceStart)==XUI_OK);
        BODY_CHECK(!memcmp(bytes,source+t.iSourceStart,(size_t)(t.iSourceEnd-t.iSourceStart)));
        BODY_CHECK(xuiDocumentSourceIndexFindToken(index,t.iSourceStart,&ordinal)==XUI_OK && ordinal==i);end=t.iSourceEnd;
    }
    BODY_CHECK(end==strlen(source));
}
static void body_goldens(void)
{
    const char* fixtures[]={"**bold**","# Heading #\r\n","[**label**]( <uri> \t 'title' )\n",
        "x <!-- raw --> y\n","> \t- [x] *item*\r\n> \t  next\r\n","a | b\n:-- | --:\nx | y\n",
        "---\r\n<!-- yaml -->\r\n...\r\n","[a][b][c][d][e][f]"};
    unsigned fixture;
    for(fixture=0;fixture<sizeof(fixtures)/sizeof(*fixtures);fixture++){
        const char* source=fixtures[fixture];const char* p;body_allocator a={-1,0};
        xui_doc_desc_t desc={0};xui_document d;xui_document_snapshot snapshot;xui_document_source_index index;
        xui_doc_stats_t before={0},after={0};xui_doc_source_index_info_t info={0};uint64_t revision;
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
        desc.onAlloc=body_alloc;desc.onFree=body_free;desc.pAllocatorUser=&a;
        if(!fixture || fixture==7)desc.iMaxNodes=3;
        BODY_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,source,strlen(source))==XUI_OK);
        BODY_CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentMarkSaved(d,snapshot)==XUI_OK);
        before.iSize=sizeof(before);BODY_CHECK(xuiDocumentGetStats(d,&before)==XUI_OK);revision=xuiDocumentGetRevision(d);
        BODY_CHECK(xuiDocumentSnapshotCreateSourceIndex(snapshot,NULL,&index)==XUI_OK);
        after.iSize=sizeof(after);BODY_CHECK(xuiDocumentGetStats(d,&after)==XUI_OK);
        BODY_CHECK(before.iNodes==after.iNodes && before.iUndoCount==after.iUndoCount && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d));
        /* Retained source projections are readable after the live document and
         * caller snapshot disappear, with all further allocations forbidden. */
        xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(d);a.remaining=0;
        body_partition(index,source);
        if(!fixture){
            uint64_t i,body_records=0;info.iSize=sizeof(info);BODY_CHECK(xuiDocumentSourceIndexGetInfo(index,&info)==XUI_OK);
            BODY_CHECK(info.iRecordCount==10);
            for(i=0;i<info.iRecordCount;i++){
                xui_doc_source_record_t r={0};r.iSize=sizeof(r);BODY_CHECK(xuiDocumentSourceIndexGetRecord(index,i,&r)==XUI_OK);
                if(r.iFlags&XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT)body_records++;
            }
            BODY_CHECK(body_records==6);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_SCOPE,XUI_DOC_SOURCE_TOKEN_SCOPE,0,0,0,8);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_BLOCK,XUI_DOC_SOURCE_TOKEN_SCOPE,XUI_DOC_PARAGRAPH,0,0,8);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_INLINE,XUI_DOC_SOURCE_TOKEN_SCOPE,XUI_DOC_SYNTAX_STRONG,0,0,8);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_MARKER,XUI_DOC_SOURCE_TOKEN_INLINE_MARKER,XUI_DOC_SYNTAX_STRONG,XUI_DOC_SOURCE_RECORD_SYNTAX_OPEN,0,2);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_MARKER,XUI_DOC_SOURCE_TOKEN_INLINE_MARKER,XUI_DOC_SYNTAX_STRONG,XUI_DOC_SOURCE_RECORD_SYNTAX_CLOSE,6,8);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_TEXT,XUI_DOC_SOURCE_TOKEN_CONTENT,XUI_DOC_TEXT,0,2,6);
        }else if(fixture==1){
            body_role(index,XUI_DOC_SOURCE_SYNTAX_MARKER,XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER,XUI_DOC_BLOCK_SYNTAX_ATX_HEADING,XUI_DOC_SOURCE_RECORD_SYNTAX_OPEN,0,1);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_MARKER,XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER,XUI_DOC_BLOCK_SYNTAX_ATX_HEADING,XUI_DOC_SOURCE_RECORD_SYNTAX_CLOSE,10,11);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_FIELD,XUI_DOC_SOURCE_TOKEN_CONTENT,XUI_DOC_HEADING,0,2,9);
        }else if(fixture==2){
            p=strstr(source,"<uri>");BODY_CHECK(p);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_FIELD,XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT,XUI_DOC_SYNTAX_DESTINATION,0,p-source+1,p-source+4);
            p=strstr(source,"'title'");BODY_CHECK(p);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_FIELD,XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT,XUI_DOC_SYNTAX_TITLE,0,p-source+1,p-source+6);
        }else if(fixture==3){
            body_role(index,XUI_DOC_SOURCE_SYNTAX_OPAQUE,XUI_DOC_SOURCE_TOKEN_HTML_COMMENT,XUI_DOC_HTML,XUI_DOC_SOURCE_RECORD_OPAQUE,2,14);
        }else if(fixture==4){
            xui_doc_source_binding_t b=body_role(index,XUI_DOC_SOURCE_SYNTAX_INDENT,XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT,0,XUI_DOC_SOURCE_RECORD_SYNTAX_QUOTE_INDENT,0,3);
            BODY_CHECK(b.iMetadata&XUI_DOC_SOURCE_BINDING_COLUMNS);
            /* "> \t" occupies three bytes and columns 0..4: the content column
             * after "> " is 2, and Tab then advances to column 4. */
            BODY_CHECK(b.iStartColumn==0 && b.iMarkerColumn==0 && b.iContentColumn==2 && b.iEndColumn==4);
            b=body_role(index,XUI_DOC_SOURCE_SYNTAX_INDENT,XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT,0,XUI_DOC_SOURCE_RECORD_SYNTAX_LIST_FIRST_INDENT,3,9);
            BODY_CHECK(b.iStartColumn==4 && b.iMarkerColumn==UINT64_MAX && b.iContentColumn==10 && b.iEndColumn==10);
            b=body_role(index,XUI_DOC_SOURCE_SYNTAX_INDENT,XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT,0,XUI_DOC_SOURCE_RECORD_SYNTAX_LIST_CONTINUATION_INDENT,18,22);
            BODY_CHECK(b.iStartColumn==2 && b.iMarkerColumn==UINT64_MAX && b.iContentColumn==4 && b.iEndColumn==6);
        }else if(fixture==5){
            xui_doc_source_binding_t b;
            p=strstr(source,":--");BODY_CHECK(p);
            b=body_role(index,XUI_DOC_SOURCE_SYNTAX_MARKER,XUI_DOC_SOURCE_TOKEN_TABLE,XUI_DOC_TABLE_TOKEN_UNDERLINE,XUI_DOC_SOURCE_RECORD_SYNTAX_TABLE,p-source,p-source+3);
            BODY_CHECK((b.iMetadata&XUI_DOC_SOURCE_BINDING_TABLE) && b.iRow==1 && b.iOrdinal==0 && b.iMarkerFlags==XUI_DOC_TABLE_TOKEN_LEFT_COLON);
        }else if(fixture==6){
            xui_doc_source_binding_t b;
            b=body_role(index,XUI_DOC_SOURCE_SYNTAX_FIELD,XUI_DOC_SOURCE_TOKEN_OPAQUE,XUI_DOC_FRONT_MATTER,0,5,20);
            BODY_CHECK(b.tRange.iFlags&XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_OPAQUE,XUI_DOC_SOURCE_TOKEN_OPAQUE,XUI_DOC_FRONT_MATTER,XUI_DOC_SOURCE_RECORD_OPAQUE,5,20);
            body_role(index,XUI_DOC_SOURCE_SYNTAX_MARKER,XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER,XUI_DOC_BLOCK_SYNTAX_FRONT_MATTER,XUI_DOC_SOURCE_RECORD_SYNTAX_CLOSE,20,23);
        }else{
            xui_doc_source_token_t t={0};uint64_t ordinal;
            const uint64_t labels[]={4,7,10,13,16};unsigned candidate;
            for(candidate=0;candidate<5;candidate++){
                body_role(index,XUI_DOC_SOURCE_SYNTAX_INLINE,XUI_DOC_SOURCE_TOKEN_SCOPE,XUI_DOC_SYNTAX_CANDIDATE_LINK,
                    XUI_DOC_SOURCE_RECORD_SYNTAX_CANDIDATE,labels[candidate]-1,labels[candidate]+2);
                body_role(index,XUI_DOC_SOURCE_SYNTAX_FIELD,XUI_DOC_SOURCE_TOKEN_REFERENCE_LABEL,XUI_DOC_SYNTAX_CANDIDATE_LINK,0,
                    labels[candidate],labels[candidate]+1);
            }
            BODY_CHECK(xuiDocumentSourceIndexFindToken(index,4,&ordinal)==XUI_OK);t.iSize=sizeof(t);
            BODY_CHECK(xuiDocumentSourceIndexGetToken(index,ordinal,&t)==XUI_OK && t.iKind==XUI_DOC_SOURCE_TOKEN_CONTENT && t.iNodeId);
            BODY_CHECK(t.iSourceStart==0 && t.iSourceEnd==18 && t.iTextStart==0 && t.iTextEnd==18);
        }
        xuiDocumentSourceIndexRelease(index);BODY_CHECK(!a.live);
        printf("Independent public body syntax fixture=%u passed\n",fixture);
    }
    puts("Public ordinary source: eight independent grammar goldens, semantic provenance retained, exact raw partitions, unchanged nodes/history, no-allocation queries and retained lifetime passed.");
}

static void body_same(xui_document_source_index a,xui_document_source_index b)
{
    xui_doc_source_index_info_t x={0},y={0};uint64_t i;x.iSize=y.iSize=sizeof(x);
    BODY_CHECK(xuiDocumentSourceIndexGetInfo(a,&x)==XUI_OK && xuiDocumentSourceIndexGetInfo(b,&y)==XUI_OK);
    BODY_CHECK(x.iSourceBytes==y.iSourceBytes && x.iRecordCount==y.iRecordCount && x.iBindingCount==y.iBindingCount &&
        x.iTokenCount==y.iTokenCount && x.iUnclassifiedBytes==y.iUnclassifiedBytes && x.iOpaqueBytes==y.iOpaqueBytes);
    for(i=0;i<x.iRecordCount;i++){
        xui_doc_source_record_t p={0},q={0};p.iSize=q.iSize=sizeof(p);
        BODY_CHECK(xuiDocumentSourceIndexGetRecord(a,i,&p)==XUI_OK && xuiDocumentSourceIndexGetRecord(b,i,&q)==XUI_OK);
        BODY_CHECK(!!p.iNodeId==!!q.iNodeId);p.iNodeId=q.iNodeId=0;BODY_CHECK(!memcmp(&p,&q,sizeof(p)));
    }
    for(i=0;i<x.iTokenCount;i++){
        xui_doc_source_token_t p={0},q={0};p.iSize=q.iSize=sizeof(p);
        BODY_CHECK(xuiDocumentSourceIndexGetToken(a,i,&p)==XUI_OK && xuiDocumentSourceIndexGetToken(b,i,&q)==XUI_OK);
        BODY_CHECK(!!p.iNodeId==!!q.iNodeId);p.iNodeId=q.iNodeId=0;BODY_CHECK(!memcmp(&p,&q,sizeof(p)));
    }
    for(i=0;i<x.iBindingCount;i++){
        xui_doc_source_binding_t p={0},q={0};p.iSize=q.iSize=sizeof(p);
        BODY_CHECK(xuiDocumentSourceIndexGetBinding(a,i,&p)==XUI_OK && xuiDocumentSourceIndexGetBinding(b,i,&q)==XUI_OK);
        BODY_CHECK(!!p.tRange.iNodeId==!!q.tRange.iNodeId);p.tRange.iNodeId=q.tRange.iNodeId=0;BODY_CHECK(!memcmp(&p,&q,sizeof(p)));
    }
}
static void body_query_edits(void)
{
    const char* source[]={"# Heading #\r\n","[**label**]( <uri> \t 'title' )\n","---\r\n<!-- yaml -->\r\n...\r\n"};
    const char* replacement[]={"New **bold**","new-target","key: **literal**\r\n"};
    const char* golden[]={"# New **bold** #\r\n","[**label**]( <new-target> \t 'title' )\n","---\r\nkey: **literal**\r\n...\r\n"};
    const uint64_t start[]={2,14,5},end[]={9,17,20};unsigned fixture,mode;
    for(fixture=0;fixture<3;fixture++)for(mode=0;mode<3;mode++){
        body_allocator allocator={-1,0};xui_doc_desc_t desc={0};xui_document d,oracle;
        xui_document_snapshot before,after,full,history;xui_document_source_index old,current,expected,restored;
        xui_doc_source_binding_t field;char bytes[256];
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
        desc.onAlloc=body_alloc;desc.onFree=body_free;desc.pAllocatorUser=&allocator;
        BODY_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,source[fixture],strlen(source[fixture]))==XUI_OK);
        BODY_CHECK(xuiDocumentAcquireSnapshot(d,&before)==XUI_OK && xuiDocumentMarkSaved(d,before)==XUI_OK &&
            xuiDocumentSnapshotCreateSourceIndex(before,NULL,&old)==XUI_OK);
        field=body_role(old,XUI_DOC_SOURCE_SYNTAX_FIELD,fixture==0?XUI_DOC_SOURCE_TOKEN_CONTENT:
            fixture==1?XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT:XUI_DOC_SOURCE_TOKEN_OPAQUE,
            fixture==0?XUI_DOC_HEADING:fixture==1?XUI_DOC_SYNTAX_DESTINATION:XUI_DOC_FRONT_MATTER,0,start[fixture],end[fixture]);
        if(!mode){
            xui_document_transaction txn;xui_doc_txn_desc_t td={0};td.iSize=sizeof(td);td.iDomain=XUI_DOC_SOURCE;
            BODY_CHECK(xuiDocumentBeginTransaction(d,&td,&txn)==XUI_OK &&
                xuiDocumentTxnReplaceSource(txn,field.tRange.iSourceStart,field.tRange.iSourceEnd,
                    replacement[fixture],strlen(replacement[fixture]))==XUI_OK && xuiDocumentTxnCommit(txn,NULL)==XUI_OK);
            xuiDocumentTxnRelease(txn);
        }else{
            xui_document_prepare prepare;xui_doc_source_patch_t patch={0};patch.iSize=sizeof(patch);
            patch.iStart=field.tRange.iSourceStart;patch.iEnd=field.tRange.iSourceEnd;patch.sText=replacement[fixture];patch.iTextBytes=strlen(replacement[fixture]);
            BODY_CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&prepare)==XUI_OK && xuiDocumentPrepareRun(prepare)==XUI_OK);
            if(mode==2){
                xui_document_prepare next;xui_doc_source_patch_t extra={0};extra.iSize=sizeof(extra);
                extra.iStart=extra.iEnd=strlen(golden[fixture]);extra.sText="\r\nTAIL\r\n";extra.iTextBytes=8;
                BODY_CHECK(xuiDocumentPrepareContinueSource(d,prepare,&extra,1,&next)==XUI_OK);
                xuiDocumentPrepareRelease(prepare);prepare=next;BODY_CHECK(xuiDocumentPrepareRun(prepare)==XUI_OK);
            }
            BODY_CHECK(xuiDocumentPreparePublish(d,prepare,NULL)==XUI_OK);xuiDocumentPrepareRelease(prepare);
        }
        BODY_CHECK(strlen(golden[fixture])+(mode==2?8:0)<sizeof(bytes));strcpy(bytes,golden[fixture]);if(mode==2)strcat(bytes,"\r\nTAIL\r\n");
        BODY_CHECK(xuiDocumentAcquireSnapshot(d,&after)==XUI_OK && xuiDocumentSnapshotCreateSourceIndex(after,NULL,&current)==XUI_OK);
        body_partition(current,bytes);body_partition(old,source[fixture]);
        BODY_CHECK(xuiDocumentCreate(&desc,&oracle)==XUI_OK && xuiDocumentLoadMarkdown(oracle,bytes,strlen(bytes))==XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK && xuiDocumentSnapshotCreateSourceIndex(full,NULL,&expected)==XUI_OK);
        body_same(current,expected);
        BODY_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&history)==XUI_OK &&
            xuiDocumentSnapshotCreateSourceIndex(history,NULL,&restored)==XUI_OK);
        body_same(old,restored);xuiDocumentSourceIndexRelease(restored);xuiDocumentSnapshotRelease(history);
        BODY_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&history)==XUI_OK &&
            xuiDocumentSnapshotCreateSourceIndex(history,NULL,&restored)==XUI_OK);
        body_same(current,restored);xuiDocumentSourceIndexRelease(restored);xuiDocumentSnapshotRelease(history);
        xuiDocumentSourceIndexRelease(old);xuiDocumentSourceIndexRelease(current);xuiDocumentSourceIndexRelease(expected);
        xuiDocumentSnapshotRelease(before);xuiDocumentSnapshotRelease(after);xuiDocumentSnapshotRelease(full);
        xuiDocumentRelease(d);xuiDocumentRelease(oracle);BODY_CHECK(!allocator.live);
    }
    puts("Public body fields: nine query-driven heading/link/front-matter SOURCE/Prepare/Continue edits, exact untouched delimiters, all public records/tokens/bindings full-reparse equality and Undo/Redo passed.");
}
int main(void){body_goldens();body_query_edits();return 0;}
