#define main uninvoked_core_main
#include "xui_document_test.c"
#undef main

static doc_sequence* share_fixture(doc_allocator* a)
{
    doc_sequence *root=doc_seq_id(a,2),*left=doc_seq_id(a,1),*right=doc_seq_id(a,3);
    CHECK(root && left && right);root->priority=1;left->priority=2;right->priority=3;
    root->left=left;root->right=right;root->total=3;return root;
}
static int64_t share_index_shift(doc_sequence* p,uint64_t ordinal)
{
    int64_t result=0;
    while(p){uint64_t left=doc_seq_size(p->left);
        if(ordinal<left){result+=p->syntax_child_index;p=p->left;}
        else if(ordinal<left+p->length)return result+p->syntax_self_index;
        else{ordinal-=left+p->length;result+=p->syntax_child_index;p=p->right;}
    }
    CHECK(0);return 0;
}
static void share_expect(doc_sequence* out,int prepend,int64_t source,int64_t index)
{
    uint64_t i;CHECK(doc_seq_size(out)==4);
    for(i=0;i<4;i++){
        int inserted=prepend?i==0:i==3;uint64_t id=inserted?4:prepend?i:i+1;
        CHECK(doc_seq_get_id(out,i)==id);
        CHECK(doc_seq_source_shift(out,i)==(inserted?0:source));
        CHECK(share_index_shift(out,i)==(inserted?0:index));
    }
}
int main(void)
{
    unsigned mode;long total_faults=0;fail_allocator f={-1,0};xui_document d;xui_doc_desc_t desc={0};
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_RICH;desc.onAlloc=failing_alloc;desc.onFree=failing_free;desc.pAllocatorUser=&f;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);
    for(mode=0;mode<4;mode++){
        doc_sequence *base=share_fixture(d->allocator),*shifted=NULL,*insert=doc_seq_id(d->allocator,4),*out=NULL;
        int prepend=mode&1;int64_t source=prepend?-17:17,index=mode&2?-7:7;long point;unsigned live;
        CHECK(insert);insert->priority=UINT64_MAX;
        CHECK(doc_seq_shift_source_suffix(d->allocator,base,0,source,&shifted)==XUI_OK);
        shifted->syntax_self_index=shifted->syntax_child_index=index;
        live=f.live;
        for(point=0;point<32;point++){
            int result;f.remaining=point;result=doc_seq_replace(d->allocator,shifted,prepend?0:3,prepend?0:3,insert,&out);f.remaining=-1;
            if(result==XUI_OK){
                share_expect(out,prepend,source,index);
                CHECK(prepend?out->right==shifted->right:out->left==shifted->left);
                doc_seq_release(out);out=NULL;CHECK(f.live==live);break;
            }
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY && !out && f.live==live);total_faults++;
            CHECK(shifted->left==base->left && shifted->right==base->right);
        }
        CHECK(point<32);
        shifted->syntax_self_source=shifted->syntax_child_source=INT64_MIN;
        CHECK(doc_seq_replace(d->allocator,shifted,3,3,insert,&out)==XUI_DOC_ERROR_LIMIT && !out && f.live==live);
        doc_seq_release(insert);doc_seq_release(shifted);doc_seq_release(base);
    }
    printf("Persistent displaced join: four append/prepend/source/index directions, untouched sibling pointer retained, %ld OOM points atomic, overflow rejected, zero leaks\n",total_faults);
    xuiDocumentRelease(d);CHECK(!f.live);return 0;
}
