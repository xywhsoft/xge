#define DOC_CST_TRANSITION_NO_MAIN
#include "xui_document_whole_cst_test.c"
#undef DOC_CST_TRANSITION_NO_MAIN
typedef struct trivia_golden { uint32_t kind;uint64_t start,end; } trivia_golden;
static uint64_t trivia_hits(doc_state* state,const trivia_golden* expected)
{
    uint64_t i,j,hits=0;
    for(i=0;i<doc_source_forest_count(state);i++){
        uint64_t base;doc_source_cst_header h;CHECK(doc_source_forest_info(state,i,&base,&h)==XUI_OK);
        for(j=0;j<h.count;j++){
            doc_source_cst_record r;CHECK(doc_source_forest_read(state,i,j,&r)==XUI_OK);
            if(r.kind==expected->kind && base+r.start==expected->start && base+r.end==expected->end &&
                ((r.kind==DOC_CST_FIELD && r.detail==DOC_CST_FIELD_WHITESPACE) ||
                 (r.kind==DOC_CST_BREAK && r.flags==DOC_CST_BREAK_ENDING)))hits++;
        }
    }
    return hits;
}
static uint64_t transformed_offset(const char* original,uint64_t offset,unsigned eol,unsigned bom)
{
    uint64_t i,out=bom?3:0;for(i=0;i<offset;i++)out+=original[i]=='\n' && eol==1?2:1;return out;
}
static void trivia_goldens(void)
{
    const char* source[]={" \t\n\n", "#  heading  #  \n\n", "a b \t\n"};
    const trivia_golden first[]={{DOC_CST_FIELD,0,2},{DOC_CST_BREAK,2,3},{DOC_CST_BREAK,3,4}};
    const trivia_golden heading[]={{DOC_CST_FIELD,1,3},{DOC_CST_FIELD,10,12},{DOC_CST_FIELD,13,15},{DOC_CST_BREAK,15,16},{DOC_CST_BREAK,16,17}};
    const trivia_golden paragraph[]={{DOC_CST_FIELD,3,5},{DOC_CST_BREAK,5,6}};
    const trivia_golden* cases[]={first,heading,paragraph};const unsigned counts[]={3,5,2};
    unsigned fixture,eol,bom;uint64_t total=0;
    for(fixture=0;fixture<3;fixture++)for(eol=0;eol<3;eol++)for(bom=0;bom<2;bom++){
        uint64_t n,i;char* bytes=source_variant(source[fixture],eol,bom,&n);
        fault_allocator a={-1,0};xui_document d=open_doc(bytes,n,3,&a);xui_document_snapshot snapshot;
        xui_document_source_index index;xui_doc_source_index_info_t info={0};
        CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentSnapshotCreateSourceIndex(snapshot,NULL,&index)==XUI_OK);
        for(i=0;i<counts[fixture];i++){
            trivia_golden expected=cases[fixture][i];uint64_t hits;
            expected.start=transformed_offset(source[fixture],expected.start,eol,bom);
            expected.end=transformed_offset(source[fixture],expected.end,eol,bom);hits=trivia_hits(snapshot->state,&expected);
            if(hits!=1)fprintf(stderr,"trivia golden fixture=%u eol=%u bom=%u kind=%u range=%llu..%llu hits=%llu\n",
                fixture,eol,bom,expected.kind,(unsigned long long)expected.start,(unsigned long long)expected.end,(unsigned long long)hits);
            CHECK(hits==1);total++;
        }
        info.iSize=sizeof(info);CHECK(xuiDocumentSourceIndexGetInfo(index,&info)==XUI_OK);
        for(i=0;i<info.iTokenCount;i++){
            xui_doc_source_token_t token={0};token.iSize=sizeof(token);CHECK(xuiDocumentSourceIndexGetToken(index,i,&token)==XUI_OK);
            if(!fixture && token.iSourceStart>=(bom?3u:0u))CHECK(token.iFlags&XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED);
            if(fixture==2 && token.iSourceStart<=1+(bom?3u:0u) && token.iSourceEnd>1+(bom?3u:0u))
                CHECK(token.iKind==XUI_DOC_SOURCE_TOKEN_CONTENT);
        }
        if(!fixture){xui_doc_stats_t stats={0};stats.iSize=sizeof(stats);CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK && stats.iNodes==1);}
        body_projection_complete(snapshot,index);memory_oracle_check(d);
        xuiDocumentSourceIndexRelease(index);xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(d);CHECK(!a.live);free(bytes);
    }
    printf("Trivia CST independent goldens: %llu exact ranges, LF/CRLF/CR/BOM, public confirmed tokens, semantic content preserved and no semantic trivia nodes\n",(unsigned long long)total);
}
int main(void)
{
    const char* source[]={" \t\n\n", "a\n\nb \t\n", "#  heading  #  \n\n", "> a\n>\n> b\n\n", "- a\n\n- b\n\n",
        "[r]: uri\n\n[r]\n\n", "[^u]: a\n\n    b\n\nend\n", "`a b` [a]( uri  'title' )\n\n", transitions[0]};
    unsigned fixture,eol,bom,operation,mode;uint64_t total=0;
    trivia_goldens();
    for(fixture=0;fixture<sizeof(source)/sizeof(*source);fixture++)for(eol=0;eol<3;eol++)for(bom=0;bom<2;bom++){
        uint64_t n,at;char* bytes=source_variant(source[fixture],eol,bom,&n);
        for(at=bom?3:0;at<=n;at++)for(operation=0;operation<3;operation++)for(mode=0;mode<4;mode++){
            fprintf(stderr,"Trivia transition fixture=%u eol=%u bom=%u at=%llu operation=%u mode=%u\n",fixture,eol,bom,(unsigned long long)at,operation,mode);
            if(at==n && operation==1 && mode<3)continue; /* No-op: no new undo entry. */
            transition_case(bytes,n,at,operation,mode);total++;
        }
        free(bytes);
    }
    printf("Trivia CST transitions: %llu SOURCE/Prepare/READY Continue/Undo/Redo; exact source, ordinary/definition arenas, complete public records/bindings/partition and full-parse HTML; memory graph and zero leaks\n",(unsigned long long)total);
    return 0;
}
