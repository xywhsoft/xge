#include "../src/xui_document_md4c.h"
#include "../src/xui_document_cst_collect.h"
#include <stdio.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#define CHECK(c) do{if(!(c)){fprintf(stderr,"collector line %d: %s\n",__LINE__,#c);exit(1);}}while(0)
typedef struct fault_allocator{long remaining;unsigned live;}fault_allocator;
static void* fault_alloc(void* user,size_t bytes){fault_allocator* a=user;void* p;if(!a->remaining)return NULL;if(a->remaining>0)a->remaining--;p=malloc(bytes);if(p)a->live++;return p;}
static void fault_free(void* user,void* p){fault_allocator* a=user;CHECK(a->live);a->live--;free(p);}
static const char* cases[]={
    "top\n\n[^u]: **bold** [a](target \"title\")\n",
    "top[^v]\n\n[^v]: shown\n[^u]: *hidden* [^w]\n[^w]: final\n",
    "top[^v]\n\n[^v]: shown\n[^v]: **duplicate**\n",
    "[^u]: > - item\n    >   `code`\n    >\n",
    "[^u]:\n\n    ``` lang\n    x &amp; y\n    ```\n",
    "[^u]: a | b\n    :-: | ---\n    x | y\n",
    "> [^u]:\n>\n>     > text\n>\n",
    "[^u]: **bold**\n\n\tmore\n",
    "head\n\n[^u]: &amp; \\* *utf8* 中文\n",
    "head[^a]\n\n[^a]: text[^b]\n[^b]: shown\n[^z]: hidden[^a] [^u]\n[^u]: last\n",
    "[^u]:\n", "text [^missing]\n",
    "head\n\n[^u]: aa <b title='x'>raw</b>\n",
    "head\n\n[^u]:\n\n        indented\n\n        more\n",
    "head\n\n[^u]: [![*alt*](image 'asset')](link \"outer\")\n"
};
static char* source_variant(const char* source,unsigned eol,unsigned bom,uint64_t* size)
{
    size_t n=strlen(source),used=0,i;char* out=malloc(n*2+4);CHECK(out);
    if(bom){memcpy(out,"\xef\xbb\xbf",3);used=3;}
    for(i=0;i<n;i++){if(source[i]=='\n'){if(eol)out[used++]='\r';if(eol!=2)out[used++]='\n';}else out[used++]=source[i];}
    out[used]=0;*size=used;return out;
}
static xui_document open_doc(const char* source,uint64_t bytes,unsigned dialect,fault_allocator* a)
{
    xui_doc_desc_t desc={0};xui_document d;int result;
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=dialect;
    if(a){desc.onAlloc=fault_alloc;desc.onFree=fault_free;desc.pAllocatorUser=a;}
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);result=xuiDocumentLoadMarkdown(d,source,bytes);
    if(result!=XUI_OK)fprintf(stderr,"load result %d, input %s\n",result,source);
    CHECK(result==XUI_OK);return d;
}
static uint64_t check_forest(doc_state* state)
{
    uint64_t i,j,arenas=0,definitions=doc_seq_size(state->references)/sizeof(xui_doc_reference_definition_t);
    for(i=0;i<definitions;i++){
        doc_source_cst_header h;doc_source_cst_record r;int result=doc_source_cst_info(state,i,&h);
        if(result==XUI_ERROR_NOT_FOUND)continue;
        CHECK(result==XUI_OK);arenas++;
        for(j=0;j<h.count;j++){
            CHECK(doc_source_cst_read(state,i,j,&r)==XUI_OK && r.start<=r.end && r.end<=h.source_bytes);
            if(!j)CHECK(r.kind==DOC_CST_SCOPE && r.parent==DOC_NONE && !r.start && r.end==h.source_bytes);
            else{doc_source_cst_record parent;CHECK(r.parent<j && doc_source_cst_read(state,i,r.parent,&parent)==XUI_OK && parent.start<=r.start && parent.end>=r.end);}
        }
    }
    return arenas;
}
static uint64_t find_record(doc_state* state,uint32_t kind,uint32_t detail,uint64_t start,uint64_t end)
{
    uint64_t i,j,found=0;for(i=0;i<doc_seq_size(state->reference_syntax);i++){
        uint64_t definition=doc_seq_get_id(state->reference_syntax,i)-1;doc_source_cst_header h;xui_doc_reference_definition_t d;
        CHECK(doc_source_cst_info(state,definition,&h)==XUI_OK && doc_seq_read(state->references,definition*sizeof(d),&d,sizeof(d))==XUI_OK);
        for(j=0;j<h.count;j++){doc_source_cst_record r;CHECK(doc_source_cst_read(state,definition,j,&r)==XUI_OK);
            if(r.kind==kind && r.detail==detail && d.iSourceStart+r.start==start && d.iSourceStart+r.end==end)found++;}
    }return found;
}
static void golden(doc_state* state,const char* source,unsigned fixture)
{
    const char* p;
    if(fixture==0){
        p=strstr(source,"**bold**");CHECK(p && find_record(state,DOC_CST_INLINE,MD_SPAN_STRONG,p-source,p-source+8)==1);
        CHECK(find_record(state,DOC_CST_TEXT,MD_TEXT_NORMAL,p-source+2,p-source+6)==1);
        p=strstr(source,"target");CHECK(p && find_record(state,DOC_CST_FIELD,DOC_CST_FIELD_DESTINATION,p-source,p-source+6)==1);
        p=strstr(source,"\"title\"");CHECK(p && find_record(state,DOC_CST_FIELD,DOC_CST_FIELD_TITLE,p-source,p-source+7)==1);
    }
    if(fixture==8){p=strstr(source,"&amp;");CHECK(p && find_record(state,DOC_CST_TEXT,MD_TEXT_ENTITY,p-source,p-source+5)==1);}
}
static void compare_cst(doc_state* a,doc_state* b)
{
    uint64_t i,j,n=doc_seq_size(a->references)/sizeof(xui_doc_reference_definition_t);CHECK(doc_seq_size(a->references)==doc_seq_size(b->references));
    for(i=0;i<n;i++){
        doc_source_cst_header ah,bh;int ar=doc_source_cst_info(a,i,&ah),br=doc_source_cst_info(b,i,&bh);CHECK(ar==br);
        if(ar==XUI_ERROR_NOT_FOUND)continue;
        CHECK(ar==XUI_OK && !memcmp(&ah,&bh,sizeof(ah)));
        for(j=0;j<ah.count;j++){doc_source_cst_record x,y;CHECK(doc_source_cst_read(a,i,j,&x)==XUI_OK && doc_source_cst_read(b,i,j,&y)==XUI_OK && !memcmp(&x,&y,sizeof(x)));}
    }
}
static int matrix(int dump)
{
    unsigned fixture,dialect,eol,bom;uint64_t total=0;
#ifdef _WIN32
    if(dump)CHECK(_setmode(_fileno(stdout),_O_BINARY)!=-1);
#endif
    for(fixture=0;fixture<sizeof(cases)/sizeof(*cases);fixture++)for(dialect=1;dialect<=3;dialect++)for(eol=0;eol<3;eol++)for(bom=0;bom<2;bom++){
        uint64_t bytes;char* source=source_variant(cases[fixture],eol,bom,&bytes);xui_document d; xui_document_snapshot s;char *html,*native;uint64_t hn,nn;
        fault_allocator a={-1,0};d=open_doc(source,bytes,dialect,&a);CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK);
        CHECK(xuiDocumentExportHtml(s,&html,&hn)==XUI_OK && xuiDocumentSerialize(s,&native,&nn)==XUI_OK);
        if(dump){
            uint64_t header[]={fixture,dialect,eol,bom,bytes,hn,nn,atomic_load(&d->allocator->next_node_id),s->state->node_count,
                doc_seq_size(s->state->inline_syntax),doc_seq_size(s->state->references),doc_seq_size(s->state->reference_candidates)};
            CHECK(fwrite(header,sizeof(header),1,stdout)==1 && fwrite(html,1,(size_t)hn,stdout)==hn && fwrite(native,1,(size_t)nn,stdout)==nn);
            doc_sequence* sequences[]={s->state->inline_syntax,s->state->references,s->state->reference_candidates};
            for(unsigned k=0;k<3;k++){uint64_t size=doc_seq_size(sequences[k]);char* raw=malloc((size_t)size+1);CHECK(raw && doc_seq_read(sequences[k],0,raw,size)==XUI_OK && fwrite(raw,1,(size_t)size,stdout)==size);free(raw);}
        }else{
            xui_document restored; xui_document_snapshot rs;xui_doc_desc_t desc={0};desc.iSize=sizeof(desc);desc.onAlloc=fault_alloc;desc.onFree=fault_free;desc.pAllocatorUser=&a;
            check_forest(s->state);if(dialect==3)golden(s->state,source,fixture);
            CHECK(xuiDocumentDeserialize(&desc,native,nn,&restored)==XUI_OK && xuiDocumentAcquireSnapshot(restored,&rs)==XUI_OK);
            compare_cst(s->state,rs->state);xuiDocumentRelease(restored);check_forest(rs->state);xuiDocumentSnapshotRelease(rs);
        }
        xuiDocumentFreeBuffer(html);xuiDocumentFreeBuffer(native);xuiDocumentRelease(d);
        if(!dump)check_forest(s->state);
        xuiDocumentSnapshotRelease(s);CHECK(!a.live);free(source);total++;
    }
    if(!dump)printf("Hidden CST collection: %llu load/native/lifetime cases, three dialects/endings/BOM, actual parsed records, independent bold/link/entity ranges and parent containment; zero leaks\n",(unsigned long long)total);
    return 0;
}
int main(int argc,char** argv){return matrix(argc==2 && !strcmp(argv[1],"--dump"));}
