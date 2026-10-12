/* Ordinary parser syntax uses the same immutable arena layout as definition
 * syntax. Forest keys are raw bases plus one, never semantic node identities. */
typedef struct doc_forest_scope {
    uint64_t root, start, end, count, cursor;
    doc_blob* blob;
    doc_sequence* value;
} doc_forest_scope;
static int doc_forest_base(uint64_t key, int64_t shift, uint64_t* out)
{
    uint64_t base, magnitude;
    if (!key) return XUI_DOC_ERROR_SCHEMA;
    base = key-1;
    if (shift >= 0) {
        if ((uint64_t)shift > UINT64_MAX-base) return XUI_DOC_ERROR_SCHEMA;
        *out = base+(uint64_t)shift;
    } else {
        magnitude = (uint64_t)(-(shift+1))+1;
        if (magnitude > base) return XUI_DOC_ERROR_SCHEMA;
        *out = base-magnitude;
    }
    return XUI_OK;
}
static int doc_forest_value(doc_sequence* forest, uint64_t index, uint64_t limit,
    uint64_t* start, doc_source_cst_header* header, doc_sequence** value)
{
    int result;
    if (index >= doc_seq_size(forest)) return XUI_ERROR_NOT_FOUND;
    result = doc_forest_base(doc_seq_get_id(forest,index), doc_seq_source_shift(forest,index), start);
    if (result != XUI_OK) return result;
    *value = doc_seq_get_value_item(forest,index);
    if (!*value || doc_seq_size(*value)<sizeof(*header) ||
        doc_seq_read(*value,0,header,sizeof(*header))!=XUI_OK || !header->count ||
        header->count>(UINT64_MAX-sizeof(*header))/sizeof(doc_source_cst_record) ||
        sizeof(*header)+header->count*sizeof(doc_source_cst_record)!=doc_seq_size(*value) ||
        *start>limit || header->source_bytes>limit-*start) return XUI_DOC_ERROR_SCHEMA;
    return XUI_OK;
}
uint64_t doc_source_forest_count(const doc_state* state)
{ return state ? doc_seq_size(state->source_syntax) : 0; }
int doc_source_forest_info(doc_state* state, uint64_t index, uint64_t* start,
    doc_source_cst_header* out)
{
    doc_sequence* value;
    if (!state || !start || !out) return XUI_ERROR_INVALID_ARGUMENT;
    *start=0; memset(out,0,sizeof(*out));
    return doc_forest_value(state->source_syntax,index,doc_seq_size(state->source),start,out,&value);
}
int doc_source_forest_read(doc_state* state, uint64_t index, uint64_t ordinal,
    doc_source_cst_record* out)
{
    doc_sequence* value; doc_source_cst_header header; uint64_t start; int result;
    if (!state || !out) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));
    result=doc_forest_value(state->source_syntax,index,doc_seq_size(state->source),&start,&header,&value);
    if (result!=XUI_OK) return result;
    if (ordinal>=header.count) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read(value,sizeof(header)+ordinal*sizeof(*out),out,sizeof(*out));
}
static int doc_forest_order(const void* a, const void* b)
{
    const doc_forest_scope *left=a,*right=b;
    if (left->start!=right->start) return left->start<right->start?-1:1;
    if (left->end!=right->end) return left->end<right->end?-1:1;
    return left->root<right->root?-1:left->root>right->root;
}
static int doc_forest_lower(doc_sequence* forest, uint64_t position, uint64_t limit,
    uint64_t* out, const atomic_int* cancellation)
{
    uint64_t lo=0,hi=doc_seq_size(forest); int result;
    while(lo<hi) {
        uint64_t middle=lo+(hi-lo)/2,start; doc_source_cst_header header; doc_sequence* value;
        result=doc_cst_cancel(cancellation); if(result!=XUI_OK)return result;
        result=doc_forest_value(forest,middle,limit,&start,&header,&value); if(result!=XUI_OK)return result;
        if(start<position)lo=middle+1;else hi=middle;
    }
    *out=lo;return XUI_OK;
}
static int doc_forest_ordinary(const doc_source_cst_record* r)
{
    return r->kind==DOC_CST_BLOCK && r->detail!=MD_BLOCK_DOC &&
        r->detail!=MD_BLOCK_FOOTNOTE_DEF && r->detail!=MD_BLOCK_FOOTNOTE_DEF_SECTION;
}
/* A bounded directory groups byte-identical parser-authoritative relative
 * arenas. Prefer an exact arena from the prior state, then reuse bytes once per
 * group. Every group member receives that same immutable value. */
static int doc_forest_intern(doc_state* state,doc_forest_scope* scopes,uint64_t count,
    const atomic_int* cancellation)
{
    doc_cst_share_index index={0};doc_sequence* reused=NULL;uint64_t i;int result=doc_cst_cancel(cancellation);
    if(result!=XUI_OK)return result;
    if(!count)return XUI_OK;
    if(count>1){
        if(count>SIZE_MAX/sizeof(*index.entries) || count>SIZE_MAX/sizeof(*index.order))return XUI_DOC_ERROR_LIMIT;
        index.entries=doc_alloc(state->allocator,(size_t)count*sizeof(*index.entries));
        index.order=doc_alloc(state->allocator,(size_t)count*sizeof(*index.order));
        if(!index.entries || !index.order){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
        index.count=count;index.cancellation=cancellation;
    }
    for(i=0;i<count;i++){
        doc_forest_scope* scope=scopes+i;
        if((result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
        if(scope->cursor!=scope->count){result=XUI_DOC_ERROR_SCHEMA;goto done;}
        scope->value=doc_seq_blob_range(state->allocator,scope->blob,0,scope->blob->size);
        if(!scope->value){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
        if(count>1){
            result=doc_cst_share_key(scope->value,cancellation,index.entries+i);if(result!=XUI_OK)goto done;
            index.order[i]=i;
        }
    }
    if(count==1)goto done;
    for(i=count/2;i;){result=doc_cst_share_sift(&index,--i,count);if(result!=XUI_OK)goto done;}
    for(i=count;i>1;){
        uint64_t temporary=index.order[0];index.order[0]=index.order[--i];index.order[i]=temporary;
        result=doc_cst_share_sift(&index,0,i);if(result!=XUI_OK)goto done;
    }
    for(i=0;i<count;){
        uint64_t end=i+1,j,representative=index.order[i];int order;
        doc_sequence* canonical=scopes[representative].value;doc_sequence* partial=NULL;
        while(end<count){
            result=doc_cst_share_compare(index.entries+representative,index.entries+index.order[end],cancellation,&order);
            if(result!=XUI_OK)goto done;
            if(order)break;
            end++;
        }
        for(j=i;j<end;j++){
            uint64_t at=index.order[j],old_at,old_start;doc_source_cst_header old_header;doc_sequence* old_value;
            doc_cst_share_entry old_key;
            result=doc_forest_lower(state->source_syntax,scopes[at].start,doc_seq_size(state->source),&old_at,cancellation);
            if(result!=XUI_OK)goto done;
            if(old_at==doc_seq_size(state->source_syntax))continue;
            result=doc_forest_value(state->source_syntax,old_at,doc_seq_size(state->source),&old_start,&old_header,&old_value);
            if(result!=XUI_OK)goto done;
            if(old_start!=scopes[at].start)continue;
            if(!partial)partial=old_value;
            result=doc_cst_share_key(old_value,cancellation,&old_key);
            if(result==XUI_OK)result=doc_cst_share_compare(index.entries+representative,&old_key,cancellation,&order);
            if(result!=XUI_OK)goto done;
            if(!order){canonical=old_value;break;}
        }
        if(canonical==scopes[representative].value && partial){
            result=doc_seq_reuse_bytes(state->allocator,partial,canonical,cancellation,&reused);
            if(result!=XUI_OK)goto done;
            canonical=reused;
        }
        for(j=i;j<end;j++){
            uint64_t at=index.order[j];
            if(!(j&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
            doc_seq_retain(canonical);doc_seq_release(scopes[at].value);
            scopes[at].value=canonical;index.entries[at].value=canonical;
            doc_blob_release(scopes[at].blob);scopes[at].blob=NULL;
        }
        doc_seq_release(reused);reused=NULL;i=end;
    }
    result=doc_cst_cancel(cancellation);
done:
    doc_seq_release(reused);doc_free(index.entries);doc_free(index.order);return result;
}

int doc_source_forest_build_parsed(doc_state* state, const doc_source_cst_record* records,
    uint64_t count, uint64_t input_bytes, const atomic_int* cancellation)
{
    uint64_t *owners=NULL,*mapped=NULL,doc=DOC_NONE,roots=0,i,j,bytes;
    doc_forest_scope* scopes=NULL; doc_sequence* forest=NULL;
    int result=doc_cst_cancel(cancellation);
    if(result!=XUI_OK)return result;
    if(!state || !records || !count)return XUI_ERROR_INVALID_ARGUMENT;
    if(count>state->allocator->max_syntax_records || count>SIZE_MAX/sizeof(*owners))return XUI_DOC_ERROR_LIMIT;
    /* Validate the complete parser tree, including definition-owned records
     * that remain in the existing definition forest. No partial publication. */
    for(i=0;i<count;i++) {
        const doc_source_cst_record* r=records+i;
        if(!(i&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)return result;
        if(r->kind<DOC_CST_SCOPE || r->kind>DOC_CST_OPAQUE || r->reserved ||
            r->start>r->end || r->end>input_bytes)return XUI_DOC_ERROR_SCHEMA;
        if(!i) {
            if(r->kind!=DOC_CST_SCOPE || r->parent!=DOC_NONE || r->start || r->end!=input_bytes)return XUI_DOC_ERROR_SCHEMA;
        } else {
            if(r->kind==DOC_CST_SCOPE || r->parent>=i || records[r->parent].start>r->start ||
                records[r->parent].end<r->end)return XUI_DOC_ERROR_SCHEMA;
            if(r->kind==DOC_CST_BLOCK && r->detail==MD_BLOCK_DOC) {
                if(doc!=DOC_NONE || r->parent)return XUI_DOC_ERROR_SCHEMA;
                doc=i;
            }
        }
        if(r->kind==DOC_CST_FIELD && (r->start>r->metadata[0] || r->metadata[0]>r->metadata[1] ||
            r->metadata[1]>r->end))return XUI_DOC_ERROR_SCHEMA;
    }
    if(doc==DOC_NONE)return XUI_DOC_ERROR_SCHEMA;
    owners=doc_alloc(state->allocator,(size_t)count*sizeof(*owners));
    mapped=doc_alloc(state->allocator,(size_t)count*sizeof(*mapped));
    if(!owners || !mapped){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
    memset(mapped,0,(size_t)count*sizeof(*mapped));owners[0]=DOC_NONE;
    for(i=1;i<count;i++) {
        if(!(i&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
        owners[i]=records[i].parent==doc ? (doc_forest_ordinary(records+i)?i:DOC_NONE) : owners[records[i].parent];
        if(owners[i]!=DOC_NONE) { mapped[owners[i]]++; if(owners[i]==i)roots++; }
    }
    if(roots>SIZE_MAX/sizeof(*scopes)){result=XUI_DOC_ERROR_LIMIT;goto done;}
    if(roots){scopes=doc_alloc(state->allocator,(size_t)roots*sizeof(*scopes));if(!scopes){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}}
    for(i=1,j=0;i<count;i++)if(owners[i]==i) {
        doc_forest_scope* scope=scopes+j++; memset(scope,0,sizeof(*scope));
        scope->root=i;scope->start=records[i].start;scope->end=records[i].end;scope->count=mapped[i]+1;
    }
    if(roots)qsort(scopes,(size_t)roots,sizeof(*scopes),doc_forest_order);
    for(i=0;i<roots;i++) {
        doc_forest_scope* scope=scopes+i;doc_source_cst_header header;doc_source_cst_record root={0};
        if((result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
        if((i && scopes[i-1].end>scope->start) || scope->start==UINT64_MAX ||
            scope->count>(UINT64_MAX-sizeof(header))/sizeof(root)){result=XUI_DOC_ERROR_SCHEMA;goto done;}
        bytes=sizeof(header)+scope->count*sizeof(root);if(bytes>SIZE_MAX){result=XUI_DOC_ERROR_LIMIT;goto done;}
        scope->blob=doc_blob_allocate(state->allocator,bytes);if(!scope->blob){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
        header.source_bytes=scope->end-scope->start;header.count=scope->count;
        memcpy(scope->blob->data,&header,sizeof(header));root.kind=DOC_CST_SCOPE;root.detail=records[scope->root].detail;
        root.parent=DOC_NONE;root.end=header.source_bytes;memcpy(scope->blob->data+sizeof(header),&root,sizeof(root));
        scope->cursor=1;owners[scope->root]=i;
    }
    for(i=1;i<count;i++) {
        uint64_t root,slot,local;doc_forest_scope* scope;doc_source_cst_record r;
        if(!(i&255u) && (result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
        if(records[i].parent==doc && doc_forest_ordinary(records+i))root=i;
        else{root=owners[i];if(root==DOC_NONE)continue;}
        slot=owners[root];if(slot>=roots){result=XUI_DOC_ERROR_SCHEMA;goto done;}
        scope=scopes+slot;local=scope->cursor++;mapped[i]=local;r=records[i];
        r.parent=i==root?0:mapped[r.parent];r.start-=scope->start;r.end-=scope->start;
        if(r.kind==DOC_CST_FIELD){r.metadata[0]-=(uint32_t)scope->start;r.metadata[1]-=(uint32_t)scope->start;}
        memcpy(scope->blob->data+sizeof(doc_source_cst_header)+local*sizeof(r),&r,sizeof(r));
    }
    result=doc_forest_intern(state,scopes,roots,cancellation);if(result!=XUI_OK)goto done;
    for(i=0;i<roots;i++) {
        doc_forest_scope* scope=scopes+i;doc_sequence *value=NULL,*reused=NULL,*item=NULL,*next=NULL;
        uint64_t old_at;doc_source_cst_header old_header;doc_sequence* old_value;uint64_t old_start;
        if(scope->cursor!=scope->count){result=XUI_DOC_ERROR_SCHEMA;goto done;}
        value=scope->value;doc_seq_retain(value);
        if(roots==1){
        result=doc_forest_lower(state->source_syntax,scope->start,doc_seq_size(state->source),&old_at,cancellation);
        if(result==XUI_OK && old_at<doc_seq_size(state->source_syntax)) {
            result=doc_forest_value(state->source_syntax,old_at,doc_seq_size(state->source),&old_start,&old_header,&old_value);
            if(result==XUI_OK && old_start==scope->start) {
                result=doc_seq_reuse_bytes(state->allocator,old_value,value,cancellation,&reused);
                if(result==XUI_OK){doc_seq_release(value);value=reused;reused=NULL;}
            }
        }
        }
        if(result==XUI_OK){item=doc_seq_keyed_value_item(state->allocator,scope->start+1,value);if(!item)result=XUI_ERROR_OUT_OF_MEMORY;}
        if(result==XUI_OK)result=doc_seq_replace(state->allocator,forest,doc_seq_size(forest),doc_seq_size(forest),item,&next);
        if(result==XUI_OK){doc_seq_release(forest);forest=next;next=NULL;}
        doc_seq_release(value);doc_seq_release(reused);doc_seq_release(item);doc_seq_release(next);
        if(result!=XUI_OK)goto done;
    }
    result=doc_cst_cancel(cancellation);
    if(result==XUI_OK){doc_seq_release(state->source_syntax);state->source_syntax=forest;forest=NULL;}
done:
    if(scopes)for(i=0;i<roots;i++){doc_blob_release(scopes[i].blob);doc_seq_release(scopes[i].value);}
    doc_free(scopes);doc_free(owners);doc_free(mapped);doc_seq_release(forest);return result;
}
int doc_source_forest_build(doc_state* state, const doc_source_cst_record* records,
    uint64_t count, uint64_t input_bytes, const atomic_int* cancellation)
{
    if(!state || input_bytes>doc_seq_size(state->source))return XUI_ERROR_INVALID_ARGUMENT;
    return doc_source_forest_build_parsed(state,records,count,input_bytes,cancellation);
}
/* A masked dependency parse can create one trivia arena across an omitted
 * ordinary block and its neighboring separators. Slice only that source-only
 * grammar at the graft boundary; every retained leaf keeps its original role.
 * Ordinary grammar and a physical CRLF cannot be cut here. */
static int doc_forest_clip_trivia(doc_state* state, doc_sequence* value,
    const doc_source_cst_header* old, uint64_t from, uint64_t to,
    const atomic_int* cancellation, doc_sequence** out)
{
    doc_source_cst_record root, block, record;doc_source_cst_header header;
    doc_blob* blob=NULL;doc_sequence* result_value=NULL;uint64_t i,count=2,previous=0,cursor=2,bytes;
    int result=doc_cst_cancel(cancellation);*out=NULL;
    if(result!=XUI_OK)return result;
    if(from>=to || to>old->source_bytes || old->count<2)return XUI_DOC_ERROR_UNREPRESENTABLE;
    if(doc_seq_read(value,sizeof(*old),&root,sizeof(root))!=XUI_OK ||
       doc_seq_read(value,sizeof(*old)+sizeof(root),&block,sizeof(block))!=XUI_OK)return XUI_DOC_ERROR_SCHEMA;
    if(root.kind!=DOC_CST_SCOPE || root.detail!=DOC_CST_BLOCK_TRIVIA || root.parent!=DOC_NONE ||
       root.start || root.end!=old->source_bytes || block.kind!=DOC_CST_BLOCK ||
       block.detail!=DOC_CST_BLOCK_TRIVIA || block.parent || block.start || block.end!=old->source_bytes)
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    for(i=2;i<old->count;i++){
        if((result=doc_cst_cancel(cancellation))!=XUI_OK)return result;
        if(doc_seq_read(value,sizeof(*old)+i*sizeof(record),&record,sizeof(record))!=XUI_OK)return XUI_DOC_ERROR_SCHEMA;
        if(record.parent!=1 || record.start!=previous || record.start>=record.end || record.end>old->source_bytes)
            return XUI_DOC_ERROR_SCHEMA;
        previous=record.end;
        if(record.kind==DOC_CST_FIELD){
            if(record.detail!=DOC_CST_FIELD_WHITESPACE || record.metadata[0]!=record.start || record.metadata[1]!=record.end)
                return XUI_DOC_ERROR_UNREPRESENTABLE;
        }else if(record.kind==DOC_CST_BREAK){
            if(record.flags!=DOC_CST_BREAK_ENDING ||
               (from>record.start && from<record.end) || (to>record.start && to<record.end))
                return XUI_DOC_ERROR_UNREPRESENTABLE;
        }else return XUI_DOC_ERROR_UNREPRESENTABLE;
        if(record.start<to && record.end>from)count++;
    }
    if(previous!=old->source_bytes)return XUI_DOC_ERROR_SCHEMA;
    if(count>state->allocator->max_syntax_records || count>(SIZE_MAX-sizeof(header))/sizeof(record))return XUI_DOC_ERROR_LIMIT;
    bytes=sizeof(header)+count*sizeof(record);blob=doc_blob_allocate(state->allocator,bytes);
    if(!blob)return XUI_ERROR_OUT_OF_MEMORY;
    header.source_bytes=to-from;header.count=count;root.end=block.end=header.source_bytes;
    memcpy(blob->data,&header,sizeof(header));memcpy(blob->data+sizeof(header),&root,sizeof(root));
    memcpy(blob->data+sizeof(header)+sizeof(root),&block,sizeof(block));
    for(i=2;i<old->count;i++){
        if((result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
        result=doc_seq_read(value,sizeof(*old)+i*sizeof(record),&record,sizeof(record));if(result!=XUI_OK)goto done;
        if(record.start>=to || record.end<=from)continue;
        record.start=(record.start<from?from:record.start)-from;
        record.end=(record.end>to?to:record.end)-from;
        if(record.kind==DOC_CST_FIELD){record.metadata[0]=(uint32_t)record.start;record.metadata[1]=(uint32_t)record.end;}
        memcpy(blob->data+sizeof(header)+cursor++*sizeof(record),&record,sizeof(record));
    }
    if(cursor!=count){result=XUI_DOC_ERROR_SCHEMA;goto done;}
    result_value=doc_seq_blob_range(state->allocator,blob,0,bytes);
    if(!result_value){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
    result=doc_cst_cancel(cancellation);
    if(result==XUI_OK){*out=result_value;result_value=NULL;}
done:
    doc_seq_release(result_value);doc_blob_release(blob);return result;
}
static int doc_forest_append(doc_state* state,doc_sequence** forest,uint64_t at,doc_sequence* value)
{
    doc_sequence *item=NULL,*next=NULL;int result;
    if(at==UINT64_MAX)return XUI_DOC_ERROR_LIMIT;
    item=doc_seq_keyed_value_item(state->allocator,at+1,value);if(!item)return XUI_ERROR_OUT_OF_MEMORY;
    result=doc_seq_replace(state->allocator,*forest,doc_seq_size(*forest),doc_seq_size(*forest),item,&next);
    if(result==XUI_OK){doc_seq_release(*forest);*forest=next;next=NULL;}
    doc_seq_release(item);doc_seq_release(next);return result;
}
static int doc_forest_splice(doc_state* state, uint64_t start, uint64_t old_end,
    uint64_t new_end, doc_state* parsed, const atomic_int* cancellation, int clip_trivia)
{
    doc_sequence *insert=NULL,*shifted=NULL,*next=NULL,*prefix=NULL,*suffix=NULL;uint64_t old_bytes,lo,hi,i,total,new_bytes;
    int64_t delta;int result=doc_cst_cancel(cancellation);
    if(result!=XUI_OK)return result;
    if(!state || !parsed || state==parsed || state->allocator!=parsed->allocator ||
        old_end<start || new_end<start)return XUI_ERROR_INVALID_ARGUMENT;
    new_bytes=doc_seq_size(state->source);
    if(new_end>new_bytes || old_end-start>UINT64_MAX-(new_bytes-(new_end-start)))return XUI_ERROR_INVALID_ARGUMENT;
    old_bytes=new_bytes-(new_end-start)+(old_end-start);
    if(old_end>INT64_MAX || new_end>INT64_MAX)return XUI_DOC_ERROR_LIMIT;
    delta=(int64_t)new_end-(int64_t)old_end;
    result=doc_forest_lower(state->source_syntax,start,old_bytes,&lo,cancellation);
    if(result==XUI_OK)result=doc_forest_lower(state->source_syntax,old_end,old_bytes,&hi,cancellation);
    if(result!=XUI_OK)return result;
    if(lo){uint64_t at;doc_source_cst_header h;doc_sequence* value;
        result=doc_forest_value(state->source_syntax,lo-1,old_bytes,&at,&h,&value);
        if(result!=XUI_OK)goto done;
        if(at+h.source_bytes>start){
            if(!clip_trivia)return XUI_DOC_ERROR_UNREPRESENTABLE;
            result=doc_forest_clip_trivia(state,value,&h,0,start-at,cancellation,&prefix);
            if(result==XUI_OK)result=doc_forest_append(state,&insert,at,prefix);
            if(result!=XUI_OK)goto done;
            lo--;
        }
    }
    if(hi>lo){uint64_t at;doc_source_cst_header h;doc_sequence* value;
        result=doc_forest_value(state->source_syntax,hi-1,old_bytes,&at,&h,&value);
        if(result!=XUI_OK)goto done;
        if(at+h.source_bytes>old_end){
            if(!clip_trivia){result=XUI_DOC_ERROR_UNREPRESENTABLE;goto done;}
            result=doc_forest_clip_trivia(state,value,&h,old_end-at,h.source_bytes,cancellation,&suffix);
            if(result!=XUI_OK)goto done;
        }
    }
    total=doc_seq_size(parsed->source_syntax);
    for(i=0;i<total;i++) {
        doc_sequence *value,*item=NULL,*joined=NULL,*reused=NULL;doc_source_cst_header header;uint64_t at;
        if((result=doc_cst_cancel(cancellation))!=XUI_OK)goto done;
        result=doc_forest_value(parsed->source_syntax,i,UINT64_MAX,&at,&header,&value);
        if(result!=XUI_OK)goto done;
        if(at>=new_end-start && header.source_bytes)continue;
        if(at>new_end-start || header.source_bytes>new_end-start-at){result=XUI_DOC_ERROR_UNREPRESENTABLE;goto done;}
        if(i<hi-lo) {
            uint64_t old_at;doc_source_cst_header old_header;doc_sequence* old_value;
            result=doc_forest_value(state->source_syntax,lo+i,old_bytes,&old_at,&old_header,&old_value);
            if(result==XUI_OK)result=doc_seq_reuse_bytes(state->allocator,old_value,value,cancellation,&reused);
            if(result!=XUI_OK)goto done;
            value=reused;
        }
        item=doc_seq_keyed_value_item(state->allocator,start+at+1,value);
        if(!item)result=XUI_ERROR_OUT_OF_MEMORY;
        if(result==XUI_OK)result=doc_seq_replace(state->allocator,insert,doc_seq_size(insert),doc_seq_size(insert),item,&joined);
        if(result==XUI_OK){doc_seq_release(insert);insert=joined;joined=NULL;}
        doc_seq_release(item);doc_seq_release(joined);doc_seq_release(reused);
        if(result!=XUI_OK)goto done;
    }
    if(suffix){result=doc_forest_append(state,&insert,new_end,suffix);if(result!=XUI_OK)goto done;}
    result=doc_seq_shift_source_suffix(state->allocator,state->source_syntax,hi,delta,&shifted);
    if(result==XUI_OK)result=doc_seq_replace(state->allocator,shifted,lo,hi,insert,&next);
    if(result==XUI_OK)result=doc_cst_cancel(cancellation);
    if(result==XUI_OK){doc_seq_release(state->source_syntax);state->source_syntax=next;next=NULL;}
done:
    doc_seq_release(prefix);doc_seq_release(suffix);doc_seq_release(insert);doc_seq_release(shifted);doc_seq_release(next);return result;
}

int doc_source_forest_splice(doc_state* state,uint64_t start,uint64_t old_end,
    uint64_t new_end,doc_state* parsed,const atomic_int* cancellation)
{return doc_forest_splice(state,start,old_end,new_end,parsed,cancellation,0);}

/* Full parse correctness baseline: relative grammar arenas can share even
 * when raw bases move. Equality proves all bytes; hashes only select candidates. */
#include "xui_document_cst_reuse.h"
/* Restore a genuinely omitted unchanged source range after dependency-cohort
 * parsing. Only bases move; all relative arenas remain shared. */
int doc_source_forest_graft(doc_state* state,doc_state* prior,uint64_t old_start,
    uint64_t old_end,uint64_t new_start,const atomic_int* cancellation)
{
    doc_state parsed={0};doc_sequence* forest=NULL;uint64_t lo,hi,i;int result;
    if(!state || !prior || state==prior || state->allocator!=prior->allocator || old_end<old_start ||
        new_start>doc_seq_size(state->source) || old_end-old_start>doc_seq_size(state->source)-new_start)
        return XUI_ERROR_INVALID_ARGUMENT;
    result=doc_forest_lower(prior->source_syntax,old_start,UINT64_MAX,&lo,cancellation);
    if(result==XUI_OK)result=doc_forest_lower(prior->source_syntax,old_end,UINT64_MAX,&hi,cancellation);
    if(result!=XUI_OK)return result;
    /* The borrowed interval must contain complete prior arenas as well as
     * admit safe clipping in the masked destination. Otherwise lower_bound
     * can skip a prior ordinary block and silently graft an empty forest. */
    if(lo){
        uint64_t at;doc_source_cst_header h;doc_sequence* value;
        result=doc_forest_value(prior->source_syntax,lo-1,UINT64_MAX,&at,&h,&value);
        if(result!=XUI_OK)return result;
        if(h.source_bytes>old_start-at)return XUI_DOC_ERROR_UNREPRESENTABLE;
    }
    for(i=lo;i<hi;i++){
        uint64_t at;doc_source_cst_header h;doc_sequence *value,*item=NULL,*next=NULL;
        result=doc_forest_value(prior->source_syntax,i,UINT64_MAX,&at,&h,&value);
        if(result==XUI_OK && h.source_bytes>old_end-at)result=XUI_DOC_ERROR_UNREPRESENTABLE;
        if(result==XUI_OK){item=doc_seq_keyed_value_item(state->allocator,at-old_start+1,value);if(!item)result=XUI_ERROR_OUT_OF_MEMORY;}
        if(result==XUI_OK)result=doc_seq_replace(state->allocator,forest,i-lo,i-lo,item,&next);
        if(result==XUI_OK){doc_seq_release(forest);forest=next;next=NULL;}
        doc_seq_release(item);doc_seq_release(next);if(result!=XUI_OK)goto done;
    }
    parsed.allocator=state->allocator;parsed.source=state->source;parsed.source_syntax=forest;
    result=doc_forest_splice(state,new_start,new_start+old_end-old_start,
        new_start+old_end-old_start,&parsed,cancellation,1);
done:
    doc_seq_release(forest);return result;
}
