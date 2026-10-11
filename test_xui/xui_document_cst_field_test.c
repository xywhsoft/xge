#define main collector_matrix_main
#include "xui_document_cst_collector_test.c"
#undef main
int main(void)
{
    uint64_t count=0;
    for(unsigned fixture=0;fixture<sizeof(cases)/sizeof(*cases);fixture++){
        fault_allocator a={-1,0};xui_document d=open_doc(cases[fixture],strlen(cases[fixture]),3,&a);
        for(uint64_t i=0;i<doc_seq_size(d->state->reference_syntax);i++){
            uint64_t definition=doc_seq_get_id(d->state->reference_syntax,i)-1;doc_source_cst_header header;
            CHECK(doc_source_cst_info(d->state,definition,&header)==XUI_OK);
            for(uint64_t j=0;j<header.count;j++){
                doc_source_cst_record r;CHECK(doc_source_cst_read(d->state,definition,j,&r)==XUI_OK);
                if(r.kind!=DOC_CST_FIELD)continue;
                if(r.metadata[0]<r.start || r.metadata[0]>r.metadata[1] || r.metadata[1]>r.end)
                    fprintf(stderr,"invalid field fixture=%u definition=%llu record=%llu detail=%u source=%llu..%llu content=%u..%u\n",fixture,(unsigned long long)definition,(unsigned long long)j,r.detail,(unsigned long long)r.start,(unsigned long long)r.end,r.metadata[0],r.metadata[1]);
                CHECK(r.metadata[0]>=r.start && r.metadata[0]<=r.metadata[1] && r.metadata[1]<=r.end);count++;
            }
        }
        xuiDocumentRelease(d);CHECK(!a.live);
    }
    printf("CST field content: %llu actual label/destination/title/heading/fence/candidate fields remain within raw tokens\n",(unsigned long long)count);return 0;
}
