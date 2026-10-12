/* Complete only trivia between parser-authoritative direct child ranges.
 * Content, markers and opaque payloads already owned by a child stay intact.
 * This is a source collection pass, with no parser, semantic nodes or copies. */
typedef struct doc_cst_trivia_child { uint64_t parent,start,end; } doc_cst_trivia_child;
static int doc_cst_trivia_order(const doc_cst_trivia_child* a,const doc_cst_trivia_child* b)
{
    if(a->parent!=b->parent)return a->parent<b->parent?-1:1;
    if(a->start!=b->start)return a->start<b->start?-1:1;
    return a->end<b->end?-1:a->end>b->end;
}
static int doc_cst_trivia_sift(doc_cst_collector* c,doc_cst_trivia_child* children,uint64_t at,uint64_t count)
{
    doc_cst_trivia_child saved=children[at];int result;
    while(at<count/2){
        uint64_t next=at*2+1;
        if((result=doc_cst_check(c))!=XUI_OK)return result;
        if(next+1<count && doc_cst_trivia_order(children+next,children+next+1)<0)next++;
        if(doc_cst_trivia_order(&saved,children+next)>=0)break;
        children[at]=children[next];at=next;
    }
    children[at]=saved;return XUI_OK;
}
static unsigned doc_cst_trivia_class(unsigned char byte)
{return byte==' ' || byte=='\t'?1:byte=='\r' || byte=='\n'?2:0;}
static int doc_cst_trivia_range(doc_cst_collector* c,uint64_t parent,uint64_t start,uint64_t end)
{
    uint64_t at=start+c->base,limit=end+c->base;int result;
    int document=c->records[parent].kind==DOC_CST_BLOCK && c->records[parent].detail==MD_BLOCK_DOC;
    while(at<limit){
        uint64_t begin=at,owner=parent,last;unsigned kind;
        if((result=doc_cst_check(c))!=XUI_OK)return result;
        if(!doc_cst_trivia_class((unsigned char)c->source[at])){at++;continue;}
        while(at<limit && doc_cst_trivia_class((unsigned char)c->source[at])){
            if(!(at&4095u) && (result=doc_cst_check(c))!=XUI_OK)return result;
            at++;
        }
        last=at;
        if(document){
            result=doc_cst_append(c,DOC_CST_BLOCK,DOC_CST_BLOCK_TRIVIA,0,parent,begin,last,&owner);
            if(result!=XUI_OK)return result;
        }
        for(at=begin;at<last;){
            uint64_t next=at+1;kind=doc_cst_trivia_class((unsigned char)c->source[at]);
            if(kind==1){
                while(next<last && doc_cst_trivia_class((unsigned char)c->source[next])==1){
                    if(!(next&4095u) && (result=doc_cst_check(c))!=XUI_OK)return result;
                    next++;
                }
                result=doc_cst_field(c,DOC_CST_FIELD_WHITESPACE,owner,at,next,at,next);
            }else{
                if(c->source[at]=='\r' && next<last && c->source[next]=='\n')next++;
                result=doc_cst_append(c,DOC_CST_BREAK,0,DOC_CST_BREAK_ENDING,owner,at,next,NULL);
            }
            if(result!=XUI_OK)return result;
            at=next;
        }
    }
    return XUI_OK;
}
static int doc_cst_complete_trivia(doc_cst_collector* c)
{
    doc_cst_trivia_child* children=NULL;uint64_t original=c->count,count=0,i,j=0;int result=doc_cst_check(c);
    if(result!=XUI_OK)return result;
    if(!original)return XUI_DOC_ERROR_SCHEMA;
    if(original>SIZE_MAX/sizeof(*children))return XUI_DOC_ERROR_LIMIT;
    if(original>1){
        children=doc_alloc(c->state->allocator,(size_t)(original-1)*sizeof(*children));
        if(!children)return XUI_ERROR_OUT_OF_MEMORY;
    }
    for(i=1;i<original;i++){
        doc_source_cst_record r=c->records[i];
        if(!(i&255u) && (result=doc_cst_check(c))!=XUI_OK)goto done;
        if(r.parent>=i || r.start>r.end || c->records[r.parent].start>r.start ||
           c->records[r.parent].end<r.end){result=XUI_DOC_ERROR_SCHEMA;goto done;}
        /* Raw table slots describe the same syntax without claiming its
         * content. Keep existing text/marker children authoritative for trivia. */
        if(r.kind==DOC_CST_FIELD && (r.detail==DOC_CST_FIELD_TABLE_ROW || r.detail==DOC_CST_FIELD_TABLE_CELL))continue;
        if(r.start<r.end){children[count].parent=r.parent;children[count].start=r.start;children[count++].end=r.end;}
    }
    for(i=count/2;i;){result=doc_cst_trivia_sift(c,children,--i,count);if(result!=XUI_OK)goto done;}
    for(i=count;i>1;){
        doc_cst_trivia_child temporary=children[0];children[0]=children[--i];children[i]=temporary;
        result=doc_cst_trivia_sift(c,children,0,i);if(result!=XUI_OK)goto done;
    }
    for(i=0;i<original;i++){
        doc_source_cst_record r=c->records[i];uint64_t at=r.start;
        int eligible=r.kind==DOC_CST_BLOCK || (r.kind==DOC_CST_INLINE && !(r.flags&DOC_CST_INLINE_CANDIDATE));
        if(!(i&255u) && (result=doc_cst_check(c))!=XUI_OK)goto done;
        while(j<count && children[j].parent<i)j++;
        while(j<count && children[j].parent==i){
            if(eligible && children[j].start>at){result=doc_cst_trivia_range(c,i,at,children[j].start);if(result!=XUI_OK)goto done;}
            if(children[j].end>at)at=children[j].end;
            j++;
        }
        if(eligible && at<r.end){result=doc_cst_trivia_range(c,i,at,r.end);if(result!=XUI_OK)goto done;}
    }
    result=doc_cst_check(c);
done:
    doc_free(children);return result;
}
