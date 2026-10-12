/* Private collector for parser-confirmed syntax without semantic nodes.
 * Included by markdown.c; every record belongs to its existing parsed state. */
typedef struct doc_cst_candidate_entry {
    uint64_t record,left,right;uint32_t height;
} doc_cst_candidate_entry;
typedef struct doc_cst_collector {
    doc_state* state;
    const char* source;
    uint64_t size, offset, definition, base, end;
    const atomic_int* cancellation;
    doc_source_cst_record* records;
    doc_cst_candidate_entry* candidate_entries;
    uint64_t candidate_count,candidate_capacity,candidate_root;
    uint64_t count, capacity, blocks[DOC_MAX_DEPTH], spans[DOC_MAX_DEPTH], table_slot_row;
    unsigned block_count, span_count;
    int active, source_only, pending, html;
    MD_TEXTTYPE pending_type, pending_content;
    MD_SIZE pending_size;
    uint64_t pending_start, pending_end;
} doc_cst_collector;
static int doc_cst_check(doc_cst_collector* c)
{ return c->cancellation ? atomic_load(c->cancellation) : XUI_OK; }
static uint64_t doc_cst_parent(doc_cst_collector* c)
{ return c->span_count ? c->spans[c->span_count-1] : c->block_count ? c->blocks[c->block_count-1] : 0; }
static int doc_cst_append(doc_cst_collector* c,uint32_t kind,uint32_t detail,uint32_t flags,
    uint64_t parent,uint64_t start,uint64_t end,uint64_t* ordinal)
{
    doc_source_cst_record* record;int result=doc_cst_check(c);
    if(result!=XUI_OK)return result;
    if(!c->active || start<c->base || end<start || end>c->end ||
        (c->count && parent>=c->count))return XUI_DOC_ERROR_FORMAT;
    if(c->count==c->capacity){
        uint64_t capacity=c->capacity?c->capacity+c->capacity/2:32;
        if(capacity>c->state->allocator->max_syntax_records)capacity=c->state->allocator->max_syntax_records;
        if(capacity<=c->count || capacity>SIZE_MAX/sizeof(*record))return XUI_DOC_ERROR_LIMIT;
        record=doc_realloc(c->state->allocator,c->records,(size_t)capacity*sizeof(*record));
        if(!record)return XUI_ERROR_OUT_OF_MEMORY;
        c->records=record;c->capacity=capacity;
    }
    if(ordinal)*ordinal=c->count;
    record=&c->records[c->count++];memset(record,0,sizeof(*record));
    record->kind=kind;record->detail=detail;record->flags=flags;record->parent=parent;
    record->start=start-c->base;record->end=end-c->base;return XUI_OK;
}
static int doc_cst_field(doc_cst_collector* c,uint32_t kind,uint64_t parent,
    uint64_t start,uint64_t end,uint64_t content_start,uint64_t content_end)
{
    uint64_t index;int result;
    if(start>content_start || content_start>content_end || content_end>end)return XUI_DOC_ERROR_FORMAT;
    result=doc_cst_append(c,DOC_CST_FIELD,kind,0,parent,start,end,&index);
    if(result!=XUI_OK)return result;
    if(content_end-c->base>UINT32_MAX)return XUI_DOC_ERROR_LIMIT;
    c->records[index].metadata[0]=(uint32_t)(content_start-c->base);
    c->records[index].metadata[1]=(uint32_t)(content_end-c->base);return XUI_OK;
}
static int doc_cst_link_field(doc_cst_collector* c, uint64_t parent, uint32_t detail,
    uint64_t start, uint64_t end, uint64_t content_start, uint64_t content_end)
{
    uint64_t record = c->count; int result = doc_cst_field(c, detail, parent, start, end, content_start, content_end);
    if (result == XUI_OK && start < content_start)
        result = doc_cst_append(c, DOC_CST_MARKER, detail, DOC_CST_MARKER_OPEN, record, start, start+1, NULL);
    if (result == XUI_OK && content_end < end)
        result = doc_cst_append(c, DOC_CST_MARKER, detail, DOC_CST_MARKER_CLOSE, record, end-1, end, NULL);
    return result;
}
static int doc_cst_link(doc_cst_collector* c, uint64_t parent,
    MD_OFFSET start, MD_OFFSET label_token_end, MD_OFFSET lb, MD_OFFSET le,
    MD_OFFSET db, MD_OFFSET de, MD_OFFSET dtb, MD_OFFSET dte,
    MD_OFFSET tb, MD_OFFSET te, MD_OFFSET ttb, MD_OFFSET tte,
    const MD_OFFSET* lines, MD_SIZE count)
{
    MD_SIZE i; uint64_t offset = c->offset; int result;
    if (!lines || !count || label_token_end <= start || dtb > db || db > de || de > dte)
        return XUI_DOC_ERROR_FORMAT;
    result = doc_cst_link_field(c, parent, DOC_CST_FIELD_LABEL,
        offset+start, offset+label_token_end, offset+lb, offset+le);
    if (result == XUI_OK) result = doc_cst_append(c, DOC_CST_MARKER, XUI_DOC_REFERENCE_LINK,
        DOC_CST_MARKER_SEPARATOR, parent, offset+label_token_end, offset+label_token_end+1, NULL);
    if (result == XUI_OK) result = doc_cst_link_field(c, parent, DOC_CST_FIELD_DESTINATION,
        offset+dtb, offset+dte, offset+db, offset+de);
    if (result == XUI_OK && ttb != UINT_MAX) result = doc_cst_link_field(c, parent, DOC_CST_FIELD_TITLE,
        offset+ttb, offset+tte, offset+tb, offset+te);
    for (i = 0; i < count && result == XUI_OK; i++) {
        const MD_OFFSET* line = lines + (size_t)i*6;
        uint64_t ordinal, at = line[0], end = line[3];
        if (line[0] > line[1] || line[1] > line[2] || line[2] > line[3] || line[3] > line[4])
            return XUI_DOC_ERROR_FORMAT;
        result = doc_cst_append(c, DOC_CST_INDENT, XUI_DOC_REFERENCE_LINK, DOC_CST_INDENT_DEFINITION,
            parent, offset+line[0], offset+line[1], &ordinal);
        if (result != XUI_OK) break;
        c->records[ordinal].metadata[0] = 0;
        c->records[ordinal].metadata[1] = c->records[ordinal].metadata[2] = line[5];
        result = doc_cst_append(c, DOC_CST_BREAK, 0, DOC_CST_BREAK_ENDING,
            parent, offset+line[3], offset+line[4], NULL);
        if (result != XUI_OK) break;
        while (at < end) {
            uint64_t begin;
            while (at < end && c->source[offset+at] != ' ' && c->source[offset+at] != '\t' &&
                c->source[offset+at] != '\v' && c->source[offset+at] != '\f') {
                if (!(at & 4095u) && (result = doc_cst_check(c)) != XUI_OK) return result;
                at++;
            }
            begin = at;
            while (at < end && (c->source[offset+at] == ' ' || c->source[offset+at] == '\t' ||
                c->source[offset+at] == '\v' || c->source[offset+at] == '\f')) {
                if (!(at & 4095u) && (result = doc_cst_check(c)) != XUI_OK) return result;
                at++;
            }
            if (at > begin) {
                result = doc_cst_field(c, DOC_CST_FIELD_WHITESPACE, parent, offset+begin, offset+at, offset+begin, offset+at);
                if (result != XUI_OK) break;
            }
        }
    }
    return result;
}
#include "xui_document_cst_trivia.h"
static int doc_cst_scope(doc_cst_collector* c,MD_OFFSET from,MD_OFFSET to,int event)
{
    xui_doc_reference_definition_t definition;uint64_t i;int result;
    if(event<0){c->active=0;c->count=c->block_count=c->span_count=0;c->pending=c->html=0;c->candidate_count=c->candidate_root=0;return XUI_OK;}
    if(event){
        if(c->active || from>to || to>c->size-c->offset)return XUI_DOC_ERROR_FORMAT;
        c->base=c->offset+from;c->end=c->offset+to;c->definition=DOC_NONE;
        for(i=0;i<doc_seq_size(c->state->references)/sizeof(definition);i++){
            if(!(i&255u) && (result=doc_cst_check(c))!=XUI_OK)return result;
            result=doc_seq_read(c->state->references,i*sizeof(definition),&definition,sizeof(definition));if(result!=XUI_OK)return result;
            if(definition.iKind==XUI_DOC_REFERENCE_FOOTNOTE && definition.iSourceStart==c->base && definition.iSourceEnd==c->end){c->definition=i;break;}
        }
        if(c->definition==DOC_NONE)return XUI_DOC_ERROR_SCHEMA;
        c->active=1;c->count=c->block_count=c->span_count=0;c->pending=c->html=0;c->candidate_count=c->candidate_root=0;
        result=doc_cst_append(c,DOC_CST_SCOPE,XUI_DOC_REFERENCE_FOOTNOTE,0,DOC_NONE,c->base,c->end,NULL);
        if(result==XUI_OK)result=doc_cst_field(c,DOC_CST_FIELD_LABEL,0,definition.iLabelStart,definition.iLabelEnd,definition.iLabelStart,definition.iLabelEnd);
        return result;
    }
    if(!c->active || c->base!=c->offset+from || c->end!=c->offset+to || c->span_count || c->pending || c->html ||
        c->block_count!=1 || c->records[c->blocks[0]].detail!=MD_BLOCK_FOOTNOTE_DEF)return XUI_DOC_ERROR_FORMAT;
    result=doc_cst_complete_trivia(c);
    if(result==XUI_OK)result=doc_source_cst_set_parsed(c->state,c->definition,c->records,c->count,c->size,c->cancellation);
    c->active=0;c->count=c->block_count=c->candidate_count=c->candidate_root=0;return result;
}
static int doc_cst_block(doc_cst_collector* c,MD_BLOCKTYPE type,MD_OFFSET from,MD_OFFSET to,int enter)
{
    uint64_t index,start=c->offset+from,end=c->offset+to;int result;
    if(c->span_count)return XUI_DOC_ERROR_FORMAT;
    if(enter){
        if(c->block_count>=DOC_MAX_DEPTH)return XUI_DOC_ERROR_LIMIT;
        result=doc_cst_append(c,DOC_CST_BLOCK,(uint32_t)type,0,doc_cst_parent(c),start,end,&index);
        if(result==XUI_OK)c->blocks[c->block_count++]=index;
        return result;
    }
    if(!c->block_count)return XUI_DOC_ERROR_FORMAT;
    index=c->blocks[c->block_count-1];
    if(c->records[index].detail!=(uint32_t)type || start<c->base || end<start || end>c->end)return XUI_DOC_ERROR_FORMAT;
    c->records[index].end=end-c->base;c->block_count--;return XUI_OK;
}
static int doc_cst_span(doc_cst_collector* c,MD_SPANTYPE type,MD_OFFSET from,MD_OFFSET to,int enter,
    MD_OFFSET destination_start,MD_OFFSET destination_end,MD_OFFSET destination_token_start,
    MD_OFFSET destination_token_end,MD_OFFSET title_token_start,MD_OFFSET title_token_end)
{
    uint64_t index,start=c->offset+from,end=c->offset+to;int result;
    (void)destination_start;(void)destination_end;
    if(enter){
        if(c->span_count>=DOC_MAX_DEPTH)return XUI_DOC_ERROR_LIMIT;
        result=doc_cst_append(c,DOC_CST_INLINE,(uint32_t)type,0,doc_cst_parent(c),start,end,&index);
        if(result!=XUI_OK)return result;
        if((type==MD_SPAN_A || type==MD_SPAN_IMG || type==MD_SPAN_FOOTNOTE_REF) &&
            destination_token_start==UINT_MAX && destination_start!=UINT_MAX)
            c->records[index].flags|=DOC_CST_INLINE_REFERENCE;
        c->spans[c->span_count++]=index;
        return doc_cst_append(c,DOC_CST_MARKER,(uint32_t)type,DOC_CST_MARKER_OPEN,index,start,end,NULL);
    }
    if(!c->span_count)return XUI_DOC_ERROR_FORMAT;
    index=c->spans[c->span_count-1];
    if(c->records[index].detail!=(uint32_t)type)return XUI_DOC_ERROR_FORMAT;
    result=doc_cst_append(c,DOC_CST_MARKER,(uint32_t)type,DOC_CST_MARKER_CLOSE,index,start,end,NULL);
    if(result!=XUI_OK)return result;
    c->records[index].end=end-c->base;
    if((type==MD_SPAN_A || type==MD_SPAN_IMG) && destination_token_start!=UINT_MAX){
        result=doc_cst_field(c,DOC_CST_FIELD_DESTINATION,index,c->offset+destination_token_start,c->offset+destination_token_end,
            c->offset+destination_start,c->offset+destination_end);if(result!=XUI_OK)return result;
        if(title_token_start!=UINT_MAX){
            if(title_token_end<title_token_start || title_token_end-title_token_start<2)return XUI_DOC_ERROR_FORMAT;
            result=doc_cst_field(c,DOC_CST_FIELD_TITLE,index,c->offset+title_token_start,c->offset+title_token_end,
                c->offset+title_token_start+1,c->offset+title_token_end-1);if(result!=XUI_OK)return result;
        }
    }
    c->span_count--;return XUI_OK;
}
static int doc_cst_markers(doc_cst_collector* c,int kind,MD_OFFSET a,MD_OFFSET z,MD_OFFSET aa,MD_OFFSET zz,
    MD_OFFSET tail,MD_OFFSET gap,MD_OFFSET task)
{
    uint64_t parent=doc_cst_parent(c);int result;
    result=doc_cst_append(c,DOC_CST_MARKER,(uint32_t)kind,DOC_CST_MARKER_OPEN,parent,c->offset+a,c->offset+z,NULL);
    if(result!=XUI_OK)return result;
    if(aa<zz){result=doc_cst_append(c,DOC_CST_MARKER,(uint32_t)kind,DOC_CST_MARKER_CLOSE,parent,c->offset+aa,c->offset+zz,NULL);if(result!=XUI_OK)return result;}
    if(kind==XUI_DOC_BLOCK_SYNTAX_FENCED_CODE && tail>z){result=doc_cst_append(c,DOC_CST_MARKER,(uint32_t)kind,DOC_CST_MARKER_TAIL,parent,c->offset+z,c->offset+tail,NULL);if(result!=XUI_OK)return result;}
    if(kind==XUI_DOC_BLOCK_SYNTAX_LIST_ITEM){
        if(gap>z){result=doc_cst_append(c,DOC_CST_INDENT,(uint32_t)kind,0,parent,c->offset+z,c->offset+gap,NULL);if(result!=XUI_OK)return result;}
        if(task>zz)return doc_cst_append(c,DOC_CST_INDENT,(uint32_t)kind,0,parent,c->offset+zz,c->offset+task,NULL);
    }
    return XUI_OK;
}
static int doc_cst_heading(doc_cst_collector* c,MD_OFFSET a,MD_OFFSET z)
{return doc_cst_field(c,DOC_CST_FIELD_HEADING,doc_cst_parent(c),c->offset+a,c->offset+z,c->offset+a,c->offset+z);}
static int doc_cst_fence(doc_cst_collector* c,MD_OFFSET a,MD_OFFSET z,MD_OFFSET language_end)
{
    int result=doc_cst_field(c,DOC_CST_FIELD_FENCE_INFO,doc_cst_parent(c),c->offset+a,c->offset+z,c->offset+a,c->offset+z);
    if(result==XUI_OK)result=doc_cst_field(c,DOC_CST_FIELD_FENCE_LANGUAGE,doc_cst_parent(c),c->offset+a,c->offset+language_end,c->offset+a,c->offset+language_end);
    return result;
}
static int doc_cst_indents(doc_cst_collector* c,const MD_OFFSET* records,MD_SIZE count,uint32_t family)
{
    MD_SIZE i;int result;if(!records || !count)return XUI_DOC_ERROR_FORMAT;
    for(i=0;i<count;i++){
        uint64_t index,parent=doc_cst_parent(c);size_t slot=(size_t)i*(family==DOC_CST_INDENT_QUOTE?7:6);
        const MD_OFFSET* r=records+slot;MD_OFFSET a=r[0],z=r[1];uint32_t flags=family;
        if(family==DOC_CST_INDENT_QUOTE){
            result=doc_cst_append(c,DOC_CST_MARKER,MD_BLOCK_QUOTE,0,parent,c->offset+a,c->offset+z,NULL);if(result!=XUI_OK)return result;
            a=r[6];
        }else if(family==DOC_CST_INDENT_LIST_FIRST && i){flags=DOC_CST_INDENT_LIST_CONTINUE;}
        else if(family==DOC_CST_INDENT_LIST_FIRST){a=r[5];}
        result=doc_cst_append(c,DOC_CST_INDENT,0,flags,parent,c->offset+a,c->offset+z,&index);if(result!=XUI_OK)return result;
        c->records[index].metadata[0]=r[2];c->records[index].metadata[1]=r[3];c->records[index].metadata[2]=r[4];
        c->records[index].metadata[3]=family==DOC_CST_INDENT_QUOTE?r[5]:family==DOC_CST_INDENT_CODE?r[5]:0;
    }
    return XUI_OK;
}
static int doc_cst_table(doc_cst_collector* c,int kind,MD_SIZE row,MD_SIZE ordinal,MD_OFFSET a,MD_OFFSET z,unsigned flags)
{
    uint64_t index;int result=doc_cst_append(c,DOC_CST_MARKER,(uint32_t)kind,DOC_CST_MARKER_TABLE,doc_cst_parent(c),c->offset+a,c->offset+z,&index);
    if(result==XUI_OK){c->records[index].metadata[0]=row;c->records[index].metadata[1]=ordinal;c->records[index].metadata[2]=flags;}return result;
}
static int doc_cst_table_slot(doc_cst_collector* c,int kind,MD_SIZE row,MD_SIZE ordinal,MD_OFFSET a,MD_OFFSET z,
    MD_OFFSET content_start,MD_OFFSET content_end)
{
    uint64_t parent=doc_cst_parent(c),index=c->count;int result;
    if(kind==2){
        parent=c->table_slot_row;
        if(parent>=c->count || c->records[parent].kind!=DOC_CST_FIELD ||
            c->records[parent].detail!=DOC_CST_FIELD_TABLE_ROW || c->records[parent].metadata[2]!=row)
            return XUI_DOC_ERROR_FORMAT;
    }else if(kind!=1)return XUI_DOC_ERROR_FORMAT;
    result=doc_cst_field(c,kind==1?DOC_CST_FIELD_TABLE_ROW:DOC_CST_FIELD_TABLE_CELL,parent,
        c->offset+a,c->offset+z,c->offset+content_start,c->offset+content_end);
    if(result==XUI_OK){
        c->records[index].metadata[2]=row;c->records[index].metadata[3]=ordinal;
        if(kind==1)c->table_slot_row=index;
    }
    return result;
}
/* Temporary AVL catalog of candidate grammar owners. Entries refer to the
 * collector's record ordinals, never pointers, source copies or NodeIds. */
static uint32_t doc_cst_candidate_height(doc_cst_collector* c,uint64_t root)
{ return root?c->candidate_entries[root-1].height:0; }
static void doc_cst_candidate_height_set(doc_cst_collector* c,uint64_t root)
{
    doc_cst_candidate_entry* e=c->candidate_entries+root-1;
    uint32_t a=doc_cst_candidate_height(c,e->left),b=doc_cst_candidate_height(c,e->right);
    e->height=(a>b?a:b)+1;
}
static uint64_t doc_cst_candidate_rotate(doc_cst_collector* c,uint64_t root,int left)
{
    doc_cst_candidate_entry* e=c->candidate_entries+root-1;
    uint64_t next=left?e->right:e->left;doc_cst_candidate_entry* n=c->candidate_entries+next-1;
    if(left){e->right=n->left;n->left=root;}else{e->left=n->right;n->right=root;}
    doc_cst_candidate_height_set(c,root);doc_cst_candidate_height_set(c,next);return next;
}
static uint64_t doc_cst_candidate_balance(doc_cst_collector* c,uint64_t root)
{
    doc_cst_candidate_entry* e=c->candidate_entries+root-1;
    uint32_t a=doc_cst_candidate_height(c,e->left),b=doc_cst_candidate_height(c,e->right);
    if(a>b+1){
        doc_cst_candidate_entry* n=c->candidate_entries+e->left-1;
        if(doc_cst_candidate_height(c,n->right)>doc_cst_candidate_height(c,n->left))
            e->left=doc_cst_candidate_rotate(c,e->left,1);
        return doc_cst_candidate_rotate(c,root,0);
    }
    if(b>a+1){
        doc_cst_candidate_entry* n=c->candidate_entries+e->right-1;
        if(doc_cst_candidate_height(c,n->left)>doc_cst_candidate_height(c,n->right))
            e->right=doc_cst_candidate_rotate(c,e->right,0);
        return doc_cst_candidate_rotate(c,root,1);
    }
    doc_cst_candidate_height_set(c,root);return root;
}
static int doc_cst_candidate_key(doc_cst_collector* c,uint64_t record,uint64_t parent,uint32_t kind,
    uint64_t start,uint64_t end,uint64_t label_start,uint64_t label_end)
{
    const doc_source_cst_record *r=c->records+record,*field=r+1;
    uint64_t a[]={r->parent,r->detail,r->start,r->end,field->start,field->end};
    uint64_t b[]={parent,kind,start,end,label_start,label_end};unsigned i;
    for(i=0;i<6;i++)if(a[i]!=b[i])return a[i]<b[i]?-1:1;
    return 0;
}
static uint64_t doc_cst_candidate_insert(doc_cst_collector* c,uint64_t root,uint64_t added)
{
    doc_cst_candidate_entry* e;doc_source_cst_record *r,*field;int order;
    if(!root)return added;
    e=c->candidate_entries+root-1;r=c->records+c->candidate_entries[added-1].record;field=r+1;
    order=doc_cst_candidate_key(c,e->record,r->parent,r->detail,r->start,r->end,field->start,field->end);
    if(order>0)e->left=doc_cst_candidate_insert(c,e->left,added);
    else e->right=doc_cst_candidate_insert(c,e->right,added);
    return doc_cst_candidate_balance(c,root);
}
static int doc_cst_candidate(doc_cst_collector* c,int kind,MD_OFFSET a,MD_OFFSET z,MD_OFFSET label_a,MD_OFFSET label_z)
{
    uint64_t parent=doc_cst_parent(c),index,at=c->candidate_root,start=c->offset+a,end=c->offset+z;
    uint64_t label_start=c->offset+label_a,label_end=c->offset+label_z;int result=doc_cst_check(c);
    if(result!=XUI_OK)return result;
    if(!c->active || kind<1 || kind>3 || start<c->base || start>label_start || label_start>label_end ||
        label_end>end || end>c->end || parent>=c->count)return XUI_DOC_ERROR_FORMAT;
    while(at){
        doc_cst_candidate_entry* e=c->candidate_entries+at-1;
        int order=doc_cst_candidate_key(c,e->record,parent,(uint32_t)kind,start-c->base,end-c->base,
            label_start-c->base,label_end-c->base);
        if(!order)return XUI_OK;
        if((result=doc_cst_check(c))!=XUI_OK)return result;
        at=order>0?e->left:e->right;
    }
    if(c->candidate_count==c->candidate_capacity){
        uint64_t capacity=c->candidate_capacity?c->candidate_capacity+c->candidate_capacity/2:8;
        uint64_t limit=c->state->allocator->max_syntax_records/2;doc_cst_candidate_entry* entries;
        if(capacity>limit)capacity=limit;
        if(capacity<=c->candidate_count || capacity>SIZE_MAX/sizeof(*entries))return XUI_DOC_ERROR_LIMIT;
        entries=doc_realloc(c->state->allocator,c->candidate_entries,(size_t)capacity*sizeof(*entries));
        if(!entries)return XUI_ERROR_OUT_OF_MEMORY;
        c->candidate_entries=entries;c->candidate_capacity=capacity;
    }
    result=doc_cst_append(c,DOC_CST_INLINE,(uint32_t)kind,DOC_CST_INLINE_CANDIDATE,parent,start,end,&index);
    if(result==XUI_OK)result=doc_cst_field(c,DOC_CST_FIELD_CANDIDATE_LABEL,index,label_start,label_end,label_start,label_end);
    if(result==XUI_OK){
        doc_cst_candidate_entry* e=c->candidate_entries+c->candidate_count;memset(e,0,sizeof(*e));e->record=index;e->height=1;
        c->candidate_count++;c->candidate_root=doc_cst_candidate_insert(c,c->candidate_root,c->candidate_count);
        result=doc_cst_check(c);
    }
    return result;
}

static int doc_cst_text_scope(doc_cst_collector* c,MD_TEXTTYPE type,MD_OFFSET a,MD_OFFSET z,int enter)
{
    int result;if(type!=MD_TEXT_HTML || (enter && c->html) || (!enter && !c->html))return XUI_DOC_ERROR_FORMAT;
    if(enter){result=doc_cst_append(c,DOC_CST_OPAQUE,(uint32_t)type,0,doc_cst_parent(c),c->offset+a,c->offset+z,NULL);if(result!=XUI_OK)return result;}
    c->html=enter;return XUI_OK;
}
static int doc_cst_text_source(doc_cst_collector* c,MD_TEXTTYPE type,MD_TEXTTYPE content,MD_OFFSET a,MD_OFFSET z,MD_SIZE size)
{
    if(c->pending || a>z || z>c->size-c->offset)return XUI_DOC_ERROR_FORMAT;
    c->pending=1;c->pending_type=type;c->pending_content=content;c->pending_size=size;
    c->pending_start=c->offset+a;c->pending_end=c->offset+z;return XUI_OK;
}
static int doc_cst_text(doc_cst_collector* c,MD_TEXTTYPE type,const MD_CHAR* text,MD_SIZE size)
{
    uint64_t start,end,index;uint32_t flags=XUI_DOC_SOURCE_DIRECT;MD_TEXTTYPE content=type;
    uintptr_t base=(uintptr_t)c->source,at=(uintptr_t)text;int result;
    if(type==MD_TEXT_BR || type==MD_TEXT_SOFTBR){if(c->pending)return XUI_DOC_ERROR_FORMAT;return XUI_OK;}
    if(c->pending){
        if(type!=c->pending_type || size!=c->pending_size)return XUI_DOC_ERROR_FORMAT;
        start=c->pending_start;end=c->pending_end;content=c->pending_content;c->pending=0;flags=XUI_DOC_SOURCE_NORMALIZED;
    }else if(at>=base && at-base<=c->size && size<=c->size-(at-base)){
        start=at-base;end=start+size;if(type==MD_TEXT_ENTITY)flags=XUI_DOC_SOURCE_ENTITY;
    }else return XUI_DOC_ERROR_FORMAT;
    result=doc_cst_append(c,DOC_CST_TEXT,(uint32_t)type,flags,doc_cst_parent(c),start,end,&index);
    if(result==XUI_OK){c->records[index].metadata[0]=(uint32_t)content;c->records[index].metadata[1]=size;}return result;
}
static int doc_cst_break(doc_cst_collector* c,int kind,MD_OFFSET trailing,MD_OFFSET a,MD_OFFSET z,MD_OFFSET marker_a,MD_OFFSET marker_z)
{
    uint64_t parent=doc_cst_parent(c);int result;
    result=doc_cst_append(c,DOC_CST_BREAK,(uint32_t)kind,DOC_CST_BREAK_TRAILING,parent,c->offset+trailing,c->offset+a,NULL);if(result!=XUI_OK)return result;
    if(marker_a!=UINT_MAX || marker_z!=UINT_MAX){
        if(marker_a==UINT_MAX || marker_z==UINT_MAX)return XUI_DOC_ERROR_FORMAT;
        result=doc_cst_append(c,DOC_CST_BREAK,(uint32_t)kind,DOC_CST_BREAK_MARKER,parent,c->offset+marker_a,c->offset+marker_z,NULL);if(result!=XUI_OK)return result;
    }
    return doc_cst_append(c,DOC_CST_BREAK,(uint32_t)kind,DOC_CST_BREAK_ENDING,parent,c->offset+a,c->offset+z,NULL);
}

static int doc_cst_front_matter(doc_cst_collector* c,uint64_t start,
    uint64_t content_start,uint64_t content_end,uint64_t end)
{
    uint64_t block,field;int result;
    if(start>content_start || content_start>content_end || content_end>end ||
        content_start-start<3 || end-content_end<3)return XUI_DOC_ERROR_FORMAT;
    result=doc_cst_append(c,DOC_CST_BLOCK,DOC_CST_BLOCK_FRONT_MATTER,0,
        doc_cst_parent(c),start,end,&block);
    if(result==XUI_OK)result=doc_cst_field(c,DOC_CST_FIELD_FRONT_MATTER,block,
        content_start,content_end,content_start,content_end);
    field=c->count-1;
    if(result==XUI_OK)result=doc_cst_append(c,DOC_CST_MARKER,XUI_DOC_BLOCK_SYNTAX_FRONT_MATTER,
        DOC_CST_MARKER_OPEN,block,start,start+3,NULL);
    if(result==XUI_OK)result=doc_cst_append(c,DOC_CST_MARKER,XUI_DOC_BLOCK_SYNTAX_FRONT_MATTER,
        DOC_CST_MARKER_CLOSE,block,content_end,content_end+3,NULL);
    if(result==XUI_OK)result=doc_cst_append(c,DOC_CST_BREAK,0,DOC_CST_BREAK_ENDING,
        block,start+3,content_start,NULL);
    if(result==XUI_OK)result=doc_cst_append(c,DOC_CST_BREAK,0,DOC_CST_BREAK_ENDING,
        block,content_end+3,end,NULL);
    if(result==XUI_OK)result=doc_cst_append(c,DOC_CST_OPAQUE,XUI_DOC_FRONT_MATTER,0,
        field,content_start,content_end,NULL);
    return result;
}
