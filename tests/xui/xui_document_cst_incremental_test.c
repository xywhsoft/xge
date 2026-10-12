#define main collector_matrix_main
#include "xui_document_cst_collector_test.c"
#undef main
int main(void)
{
    unsigned fixture;uint64_t total=0;
    for(fixture=0;fixture<sizeof(cases)/sizeof(*cases);fixture++){
        const char* source=cases[fixture];uint64_t n=strlen(source),at;
        for(at=0;at<=n;at++){
            char* expected=malloc((size_t)n+2);fault_allocator a={-1,0};xui_document d,oracle; xui_document_snapshot actual,full;
            xui_doc_txn_desc_t desc={0};xui_document_transaction t;int result;
            CHECK(expected);memcpy(expected,source,(size_t)at);expected[at]='x';memcpy(expected+at+1,source+at,(size_t)(n-at)+1);
            d=open_doc(source,n,3,&a);desc.iSize=sizeof(desc);desc.iDomain=XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d,&desc,&t)==XUI_OK);result=xuiDocumentTxnReplaceSource(t,at,at,"x",1);
            if(at<n && ((unsigned char)source[at]&0xc0)==0x80){
                CHECK(result==XUI_DOC_ERROR_UTF8);xuiDocumentTxnRelease(t);xuiDocumentRelease(d);CHECK(!a.live);free(expected);continue;
            }
            if(result!=XUI_OK)fprintf(stderr,"SOURCE failure fixture=%u at=%llu result=%d\n",fixture,(unsigned long long)at,result);
            CHECK(result==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
            oracle=open_doc(expected,n+1,3,&a);CHECK(xuiDocumentAcquireSnapshot(d,&actual)==XUI_OK && xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK);
            fprintf(stderr,"comparing fixture=%u at=%llu\n",fixture,(unsigned long long)at);
            compare_cst(actual->state,full->state);check_forest(actual->state);
            xuiDocumentSnapshotRelease(actual);xuiDocumentSnapshotRelease(full);xuiDocumentRelease(d);xuiDocumentRelease(oracle);CHECK(!a.live);free(expected);total++;
        }
    }
    printf("Hidden CST incremental: %llu source-byte inserts equal complete parse records\n",(unsigned long long)total);return 0;
}
