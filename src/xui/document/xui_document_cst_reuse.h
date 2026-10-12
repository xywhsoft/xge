/* Reuse source-ordered grammar cohorts as persistent forest subtrees. A
 * cohort matches complete relative grammar bytes and one raw-base delta;
 * equality of hashes alone never establishes a match. */
static int doc_forest_pair(doc_state* prior,uint64_t old_at,doc_state* state,uint64_t new_at,
    const atomic_int* cancellation,int* equal,int64_t* delta)
{
    doc_sequence *old_value,*new_value;doc_source_cst_header old_header,new_header;
    doc_cst_share_entry a,b;uint64_t old_start,new_start;int order,result=doc_cst_cancel(cancellation);
    *equal=0;*delta=0;if(result!=XUI_OK)return result;
    /* Draft source may already be patched; old grammar retains old bases. */
    result=doc_forest_value(prior->source_syntax,old_at,UINT64_MAX,&old_start,&old_header,&old_value);
    if(result==XUI_OK)result=doc_forest_value(state->source_syntax,new_at,doc_seq_size(state->source),&new_start,&new_header,&new_value);
    if(result!=XUI_OK)return result;
    if(old_start>INT64_MAX || new_start>INT64_MAX)return XUI_DOC_ERROR_LIMIT;
    if(old_value==new_value)order=0;
    else{
        result=doc_cst_share_key(old_value,cancellation,&a);
        if(result==XUI_OK)result=doc_cst_share_key(new_value,cancellation,&b);
        if(result==XUI_OK)result=doc_cst_share_compare(&a,&b,cancellation,&order);
        if(result!=XUI_OK)return result;
    }
    if(!order){*equal=1;*delta=(int64_t)new_start-(int64_t)old_start;}
    return XUI_OK;
}
static int doc_forest_catalog(doc_state* state,uint64_t first,uint64_t count,
    const atomic_int* cancellation,doc_cst_share_index* index)
{
    uint64_t i;int result;index->count=count;index->cancellation=cancellation;
    if(!count)return XUI_OK;
    if(count>SIZE_MAX/sizeof(*index->entries) || count>SIZE_MAX/sizeof(*index->order))return XUI_DOC_ERROR_LIMIT;
    index->entries=doc_alloc(state->allocator,(size_t)count*sizeof(*index->entries));
    index->order=doc_alloc(state->allocator,(size_t)count*sizeof(*index->order));
    if(!index->entries || !index->order)return XUI_ERROR_OUT_OF_MEMORY;
    for(i=0;i<count;i++){
        result=doc_cst_share_key(doc_seq_get_value_item(state->source_syntax,first+i),cancellation,index->entries+i);
        if(result!=XUI_OK)return result;
        index->order[i]=i;
    }
    for(i=count/2;i;){result=doc_cst_share_sift(index,--i,count);if(result!=XUI_OK)return result;}
    for(i=count;i>1;){
        uint64_t temporary=index->order[0];index->order[0]=index->order[--i];index->order[i]=temporary;
        result=doc_cst_share_sift(index,0,i);if(result!=XUI_OK)return result;
    }
    return XUI_OK;
}
int doc_source_forest_reuse(doc_state* prior,doc_state* state,const atomic_int* cancellation)
{
    doc_cst_share_index old={0},fresh={0};doc_sequence **canonical=NULL,*value=NULL;
    doc_sequence *insert=NULL,*shifted=NULL,*adjusted=NULL,*forest=NULL;
    uint64_t old_count,new_count,prefix=0,suffix=0,middle=0,i;int64_t prefix_delta=0,suffix_delta=0;
    int result=doc_cst_cancel(cancellation),equal;int64_t delta;
    if(result!=XUI_OK)return result;
    if(!prior || !state || prior==state || prior->allocator!=state->allocator)return XUI_ERROR_INVALID_ARGUMENT;
    old_count=doc_seq_size(prior->source_syntax);new_count=doc_seq_size(state->source_syntax);
    if(!old_count || !new_count || prior->source_syntax==state->source_syntax)return XUI_OK;
    while(prefix<old_count && prefix<new_count){
        result=doc_forest_pair(prior,prefix,state,prefix,cancellation,&equal,&delta);if(result!=XUI_OK)goto done;
        if(!equal || (prefix && delta!=prefix_delta))break;
        prefix_delta=delta;prefix++;
    }
    while(suffix<old_count-prefix && suffix<new_count-prefix){
        result=doc_forest_pair(prior,old_count-1-suffix,state,new_count-1-suffix,cancellation,&equal,&delta);
        if(result!=XUI_OK)goto done;
        if(!equal || (suffix && delta!=suffix_delta))break;
        suffix_delta=delta;suffix++;
    }
    if(!prefix)prefix_delta=suffix?suffix_delta:0;
    /* An unrepresentable difference only disables that reuse cohort. */
    if(suffix && ((prefix_delta>0 && suffix_delta<INT64_MIN+prefix_delta) ||
                  (prefix_delta<0 && suffix_delta>INT64_MAX+prefix_delta)))suffix=0;
    middle=new_count-prefix-suffix;
    if(middle){
        if(middle>SIZE_MAX/sizeof(*canonical)){result=XUI_DOC_ERROR_LIMIT;goto done;}
        canonical=doc_alloc(state->allocator,(size_t)middle*sizeof(*canonical));
        if(!canonical){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
        memset(canonical,0,(size_t)middle*sizeof(*canonical));
        result=doc_forest_catalog(prior,0,old_count,cancellation,&old);
        if(result==XUI_OK)result=doc_forest_catalog(state,prefix,middle,cancellation,&fresh);
        if(result!=XUI_OK)goto done;
        for(i=0;i<middle;){
            uint64_t end=i+1,representative=fresh.order[i],j,found,preferred=prefix+representative;int order;
            while(end<middle){
                result=doc_cst_share_compare(fresh.entries+representative,fresh.entries+fresh.order[end],cancellation,&order);
                if(result!=XUI_OK)goto done;
                if(order)break;
                end++;
            }
            if(preferred>=old_count)preferred=old_count-1;
            result=doc_cst_share_find(&old,fresh.entries+representative,preferred,&found);if(result!=XUI_OK)goto done;
            if(found!=DOC_NONE){value=old.entries[found].value;doc_seq_retain(value);}
            else{
                result=doc_seq_reuse_bytes(state->allocator,old.entries[preferred].value,fresh.entries[representative].value,cancellation,&value);
                if(result!=XUI_OK)goto done;
            }
            for(j=i;j<end;j++){
                if(!(j&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
                canonical[fresh.order[j]]=value;doc_seq_retain(value);
            }
            doc_seq_release(value);value=NULL;i=end;
        }
        for(i=0;i<middle;i++){
            doc_sequence *item=NULL,*next=NULL,*ignored;doc_source_cst_header h;uint64_t at;
            result=doc_cst_cancel(cancellation);
            if(result==XUI_OK)result=doc_forest_value(state->source_syntax,prefix+i,doc_seq_size(state->source),&at,&h,&ignored);
            if(result==XUI_OK){item=doc_seq_keyed_value_item(state->allocator,at+1,canonical[i]);if(!item)result=XUI_ERROR_OUT_OF_MEMORY;}
            if(result==XUI_OK)result=doc_seq_replace(state->allocator,insert,i,i,item,&next);
            if(result==XUI_OK){doc_seq_release(insert);insert=next;next=NULL;}
            doc_seq_release(item);doc_seq_release(next);if(result!=XUI_OK)goto done;
        }
    }
    result=doc_seq_shift_source_suffix(state->allocator,prior->source_syntax,0,prefix_delta,&shifted);
    if(result==XUI_OK && suffix && suffix_delta!=prefix_delta){
        result=doc_seq_shift_source_suffix(state->allocator,shifted,old_count-suffix,suffix_delta-prefix_delta,&adjusted);
        if(result==XUI_OK){doc_seq_release(shifted);shifted=adjusted;adjusted=NULL;}
    }
    if(result==XUI_OK)result=doc_seq_replace(state->allocator,shifted,prefix,old_count-suffix,insert,&forest);
    if(result==XUI_OK)result=doc_cst_cancel(cancellation);
    if(result==XUI_OK){doc_seq_release(state->source_syntax);state->source_syntax=forest;forest=NULL;}
done:
    if(canonical)for(i=0;i<middle;i++)doc_seq_release(canonical[i]);
    doc_free(canonical);doc_free(old.entries);doc_free(old.order);doc_free(fresh.entries);doc_free(fresh.order);
    doc_seq_release(value);doc_seq_release(insert);doc_seq_release(shifted);doc_seq_release(adjusted);doc_seq_release(forest);return result;
}
