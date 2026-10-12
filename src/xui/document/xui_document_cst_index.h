/* Snapshot-owned ordinary and definition syntax use one source projection.
 * Parent links use source records; no semantic nodes or source copies. */
_Static_assert((int)DOC_CST_SCOPE == (int)XUI_DOC_SOURCE_SYNTAX_SCOPE &&
    (int)DOC_CST_OPAQUE == (int)XUI_DOC_SOURCE_SYNTAX_OPAQUE, "CST source record kinds");
static uint32_t doc_si_cst_span_kind(uint32_t type)
{
    switch (type) {
    case MD_SPAN_EM: return XUI_DOC_SYNTAX_EMPHASIS;
    case MD_SPAN_STRONG: return XUI_DOC_SYNTAX_STRONG;
    case MD_SPAN_U: case MD_SPAN_INS: return XUI_DOC_SYNTAX_UNDERLINE;
    case MD_SPAN_DEL: return XUI_DOC_SYNTAX_STRIKE;
    case MD_SPAN_CODE: return XUI_DOC_SYNTAX_CODE;
    case MD_SPAN_A: return XUI_DOC_SYNTAX_LINK;
    case MD_SPAN_IMG: return XUI_DOC_SYNTAX_IMAGE;
    case MD_SPAN_LATEXMATH: return XUI_DOC_SYNTAX_MATH;
    case MD_SPAN_LATEXMATH_DISPLAY: return XUI_DOC_SYNTAX_DISPLAY_MATH;
    case MD_SPAN_FOOTNOTE_REF: return XUI_DOC_SYNTAX_FOOTNOTE_REF;
    case MD_SPAN_MARK: return XUI_DOC_SYNTAX_HIGHLIGHT;
    case MD_SPAN_SUBSCRIPT: return XUI_DOC_SYNTAX_SUBSCRIPT;
    case MD_SPAN_SUPERSCRIPT: return XUI_DOC_SYNTAX_SUPERSCRIPT;
    default: return 0;
    }
}
static uint32_t doc_si_cst_block_kind(uint32_t type)
{
    switch (type) {
    case MD_BLOCK_DOC: return XUI_DOC_ROOT;
    case DOC_CST_BLOCK_FRONT_MATTER: return XUI_DOC_FRONT_MATTER;
    case MD_BLOCK_QUOTE: case MD_BLOCK_ADMONITION: return XUI_DOC_QUOTE;
    case MD_BLOCK_UL: case MD_BLOCK_OL: return XUI_DOC_LIST;
    case MD_BLOCK_LI: return XUI_DOC_LIST_ITEM;
    case MD_BLOCK_HR: return XUI_DOC_RULE;
    case MD_BLOCK_H: return XUI_DOC_HEADING;
    case MD_BLOCK_P: return XUI_DOC_PARAGRAPH;
    case MD_BLOCK_CODE: return XUI_DOC_CODE_BLOCK;
    case MD_BLOCK_HTML: return XUI_DOC_HTML;
    case MD_BLOCK_TABLE: case MD_BLOCK_THEAD: case MD_BLOCK_TBODY: return XUI_DOC_TABLE;
    case MD_BLOCK_TR: return XUI_DOC_ROW;
    case MD_BLOCK_TH: case MD_BLOCK_TD: return XUI_DOC_CELL;
    case MD_BLOCK_FOOTNOTE_DEF: case MD_BLOCK_FOOTNOTE_DEF_SECTION: return XUI_DOC_FOOTNOTE;
    default: return 0;
    }
}
static int doc_si_cst_origin(xui_document_source_index index, uint64_t record,
    uint64_t ordinal, doc_source_cst_record* syntax, uint64_t* base)
{
    const doc_si_record_value* r; xui_doc_reference_definition_t definition; int result;
    if (record >= index->info.iRecordCount) return XUI_DOC_ERROR_FORMAT;
    r = index->records + record;
    if ((r->iKindDetail & UINT16_MAX) != XUI_DOC_SOURCE_RECORD_SYNTAX &&
        (r->iKindDetail & UINT16_MAX) != XUI_DOC_SOURCE_RECORD_DEFINITION)
        return XUI_DOC_ERROR_FORMAT;
    if (r->iFlags & XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT) {
        doc_source_cst_header header;
        if ((r->iKindDetail & UINT16_MAX) != XUI_DOC_SOURCE_RECORD_SYNTAX) return XUI_DOC_ERROR_FORMAT;
        result = doc_source_forest_info(index->snapshot->state, r->iOriginIndex, base, &header);
        if (result != XUI_OK) return result;
        return doc_source_forest_read(index->snapshot->state, r->iOriginIndex, ordinal, syntax);
    }
    result = doc_source_cst_read(index->snapshot->state, r->iOriginIndex, ordinal, syntax);
    if (result != XUI_OK) return result;
    result = doc_seq_read(index->snapshot->state->references,
        r->iOriginIndex * sizeof(definition), &definition, sizeof(definition));
    if (result != XUI_OK) return result;
    *base = definition.iSourceStart; return XUI_OK;
}
static int doc_si_cst_read(doc_state* state, uint64_t owner, uint64_t ordinal,
    int ordinary, doc_source_cst_record* out)
{ return ordinary ? doc_source_forest_read(state, owner, ordinal, out) : doc_source_cst_read(state, owner, ordinal, out); }
static int doc_si_cst_records(doc_si_builder* b, uint64_t definition, uint64_t root,
    uint64_t base, const doc_source_cst_header* header, int ordinary)
{
    uint64_t i, first = b->index->info.iRecordCount;
    const uint32_t confirmed = XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED;
    const uint32_t origin = ordinary ? XUI_DOC_SOURCE_BINDING_DOCUMENT_SYNTAX : XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX;
    if (base > b->index->info.iSourceBytes || header->source_bytes > b->index->info.iSourceBytes - base)
        return b->error = XUI_DOC_ERROR_FORMAT;
    for (i = 0; i < header->count; i++) {
        doc_source_cst_record s, parent; uint64_t record, start, end, content_start, content_end;
        uint32_t kind = 0, detail = 0, flags = confirmed, record_flags = 0; unsigned priority = 1;
        if (doc_si_step(b)) return b->error;
        b->error = doc_si_cst_read(b->index->snapshot->state, definition, i, ordinary, &s);
        if (b->error) return b->error;
        if (s.start > s.end || s.end > header->source_bytes ||
            (i && s.parent >= i) || (!i && (s.kind != DOC_CST_SCOPE ||
            s.parent != DOC_NONE || s.start || s.end != header->source_bytes)))
            return b->error = XUI_DOC_ERROR_FORMAT;
        start = base + s.start; end = base + s.end; content_start = start; content_end = end;
        if (!i) {
            doc_si_add_origin(b, root, ordinary ? 0 : XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX, ordinary ? 0 : s.detail,
                confirmed, start, end, ordinary ? 0 : 40, origin, i);
            continue;
        }
        b->error = doc_si_cst_read(b->index->snapshot->state, definition, s.parent, ordinary, &parent);
        if (b->error) return b->error;
        if (parent.start > s.start || parent.end < s.end) return b->error = XUI_DOC_ERROR_FORMAT;
        if (s.kind == DOC_CST_FIELD) {
            if (s.start > s.metadata[0] || s.metadata[0] > s.metadata[1] || s.metadata[1] > s.end)
                return b->error = XUI_DOC_ERROR_FORMAT;
            content_start = base + s.metadata[0]; content_end = base + s.metadata[1];
        }
        if (s.kind == DOC_CST_INLINE || s.kind == DOC_CST_MARKER ||
            s.kind == DOC_CST_INDENT || s.kind == DOC_CST_BREAK) record_flags = s.flags << 2;
        if (s.kind == DOC_CST_OPAQUE || (s.kind == DOC_CST_TEXT && s.detail == MD_TEXT_HTML))
            record_flags |= XUI_DOC_SOURCE_RECORD_OPAQUE;
        if (s.kind == DOC_CST_FIELD && s.detail == DOC_CST_FIELD_FENCE_INFO)
            record_flags |= XUI_DOC_SOURCE_RECORD_SYNTAX_FENCE_INFO;
        if (s.kind == DOC_CST_FIELD && s.detail == DOC_CST_FIELD_FENCE_LANGUAGE)
            record_flags |= XUI_DOC_SOURCE_RECORD_SYNTAX_FENCE_LANGUAGE;
        if (ordinary) record_flags |= XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT;
        record = doc_si_record(b, XUI_DOC_SOURCE_RECORD_SYNTAX, s.kind, record_flags,
            s.parent ? first + s.parent - 1 : root, 0, definition, start, end, content_start, content_end);
        if (record == DOC_NONE) return b->error;
        switch (s.kind) {
        case DOC_CST_BLOCK: detail = doc_si_cst_block_kind(s.detail); break;
        case DOC_CST_INLINE:
            detail = s.flags & DOC_CST_INLINE_CANDIDATE ? XUI_DOC_SYNTAX_CANDIDATE_LINK + s.detail - 1 : doc_si_cst_span_kind(s.detail);
            break;
        case DOC_CST_TEXT:
            kind = XUI_DOC_SOURCE_TOKEN_CONTENT; detail = XUI_DOC_TEXT; priority = ordinary ? 79 : 80;
            if (s.detail == MD_TEXT_HTML) { kind = XUI_DOC_SOURCE_TOKEN_HTML; flags |= XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD; }
            break;
        case DOC_CST_MARKER:
            priority = 100;
            if (s.flags & DOC_CST_MARKER_TABLE) { kind = XUI_DOC_SOURCE_TOKEN_TABLE; detail = s.detail; priority = 120; }
            else if (s.flags & DOC_CST_MARKER_SEPARATOR) { kind = XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX; detail = XUI_DOC_REFERENCE_LINK; }
            else if (parent.kind == DOC_CST_FIELD) {
                kind = parent.detail == DOC_CST_FIELD_LABEL ? XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX : XUI_DOC_SOURCE_TOKEN_INLINE_MARKER;
                detail = parent.detail == DOC_CST_FIELD_LABEL ? XUI_DOC_REFERENCE_LINK :
                    parent.detail == DOC_CST_FIELD_DESTINATION ? XUI_DOC_SYNTAX_DESTINATION : XUI_DOC_SYNTAX_TITLE;
            }
            else if (s.flags & DOC_CST_MARKER_TAIL) { detail = XUI_DOC_BLOCK_SYNTAX_FENCED_CODE; priority = 1; }
            else if (parent.kind == DOC_CST_INLINE) { kind = XUI_DOC_SOURCE_TOKEN_INLINE_MARKER; detail = doc_si_cst_span_kind(s.detail); priority = 90; }
            else { kind = XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER; detail = s.flags ? s.detail : XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN; }
            break;
        case DOC_CST_FIELD:
            kind = XUI_DOC_SOURCE_TOKEN_INLINE_MARKER; priority = 90;
            switch (s.detail) {
            case DOC_CST_FIELD_LABEL: kind = XUI_DOC_SOURCE_TOKEN_REFERENCE_LABEL;
                detail = (parent.kind == DOC_CST_SCOPE && parent.detail == XUI_DOC_REFERENCE_LINK) ||
                    (parent.kind == DOC_CST_FIELD && parent.detail == DOC_CST_FIELD_DEFINITION) ?
                    XUI_DOC_REFERENCE_LINK : XUI_DOC_REFERENCE_FOOTNOTE; break;
            case DOC_CST_FIELD_DEFINITION: kind = XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX; detail = XUI_DOC_REFERENCE_LINK; priority = 40; break;
            case DOC_CST_FIELD_WHITESPACE: kind = XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE; detail = 0; priority = 100; break;
            case DOC_CST_FIELD_TABLE_ROW: case DOC_CST_FIELD_TABLE_CELL:
                /* Zero is the builder's enclosing scope; its public SCOPE
                 * kind is assigned only when publishing overlapping bindings. */
                kind = 0;
                detail = s.detail == DOC_CST_FIELD_TABLE_ROW ? XUI_DOC_ROW : XUI_DOC_CELL; priority = 1; break;
            case DOC_CST_FIELD_DESTINATION: detail = XUI_DOC_SYNTAX_DESTINATION; break;
            case DOC_CST_FIELD_TITLE: detail = XUI_DOC_SYNTAX_TITLE; break;
            case DOC_CST_FIELD_HEADING: kind = XUI_DOC_SOURCE_TOKEN_CONTENT; detail = XUI_DOC_HEADING; if (ordinary) priority = 79; break;
            case DOC_CST_FIELD_FRONT_MATTER: kind = XUI_DOC_SOURCE_TOKEN_OPAQUE; detail = XUI_DOC_FRONT_MATTER;
                flags |= XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD; priority = 29; break;
            case DOC_CST_FIELD_FENCE_INFO: case DOC_CST_FIELD_FENCE_LANGUAGE:
                kind = XUI_DOC_SOURCE_TOKEN_FENCE_INFO; detail = XUI_DOC_BLOCK_SYNTAX_FENCED_CODE; break;
            case DOC_CST_FIELD_CANDIDATE_LABEL: kind = XUI_DOC_SOURCE_TOKEN_REFERENCE_LABEL;
                detail = XUI_DOC_SYNTAX_CANDIDATE_LINK + parent.detail - 1; if (ordinary) priority = 79; break;
            default: return b->error = XUI_DOC_ERROR_FORMAT;
            }
            break;
        case DOC_CST_INDENT:
            priority = 100; kind = XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT;
            if (s.flags & DOC_CST_INDENT_CODE) kind = XUI_DOC_SOURCE_TOKEN_CODE_INDENT;
            else if (!s.flags) { kind = XUI_DOC_SOURCE_TOKEN_LIST_GAP; detail = s.detail; }
            else if (s.flags & (DOC_CST_INDENT_QUOTE | DOC_CST_INDENT_LIST_FIRST)) priority = 70;
            else if (s.flags & DOC_CST_INDENT_DEFINITION) { detail = XUI_DOC_REFERENCE_LINK; priority = 70; }
            break;
        case DOC_CST_BREAK:
            detail = s.detail; priority = 110;
            kind = s.flags & DOC_CST_BREAK_TRAILING ? XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE :
                s.flags & DOC_CST_BREAK_MARKER ? XUI_DOC_SOURCE_TOKEN_BREAK_MARKER : XUI_DOC_SOURCE_TOKEN_LINE_ENDING;
            if (s.flags & DOC_CST_BREAK_MARKER) priority = 120;
            break;
        case DOC_CST_OPAQUE:
            kind = XUI_DOC_SOURCE_TOKEN_HTML; detail = XUI_DOC_HTML; flags |= XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD; priority = 30;
            if (parent.kind == DOC_CST_FIELD && parent.detail == DOC_CST_FIELD_FRONT_MATTER) {
                kind = XUI_DOC_SOURCE_TOKEN_OPAQUE; detail = XUI_DOC_FRONT_MATTER; priority = 29;
            } else if (end - start >= 4) { char prefix[4]; if (doc_si_read(b, start, prefix, sizeof(prefix))) return b->error;
                if (!memcmp(prefix, "<!--", 4)) { kind = XUI_DOC_SOURCE_TOKEN_HTML_COMMENT; priority = ordinary ? 79 : 85; } }
            break;
        default: return b->error = XUI_DOC_ERROR_FORMAT;
        }
        doc_si_add_origin(b, record, kind, detail, flags, start, end, priority,
            origin, i);
        if (!b->error && s.kind == DOC_CST_FIELD) {
            uint32_t content_kind = kind == XUI_DOC_SOURCE_TOKEN_INLINE_MARKER ? XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT : kind;
            uint32_t content_detail = detail;
            /* Definition payloads keep their reference roles in both the
             * overlapping bindings and the final byte partition. Inline
             * links inside footnote content still use ordinary fields. */
            if ((parent.kind == DOC_CST_SCOPE && parent.detail == XUI_DOC_REFERENCE_LINK) ||
                (parent.kind == DOC_CST_FIELD && parent.detail == DOC_CST_FIELD_DEFINITION)) {
                if (s.detail == DOC_CST_FIELD_DESTINATION)
                    content_kind = XUI_DOC_SOURCE_TOKEN_REFERENCE_DESTINATION;
                else if (s.detail == DOC_CST_FIELD_TITLE)
                    content_kind = XUI_DOC_SOURCE_TOKEN_REFERENCE_TITLE;
                if (s.detail == DOC_CST_FIELD_DESTINATION || s.detail == DOC_CST_FIELD_TITLE)
                    content_detail = XUI_DOC_REFERENCE_LINK;
            }
            if (content_kind != kind || content_start != start || content_end != end)
                doc_si_add_origin(b, record, content_kind, content_detail, flags, content_start, content_end, content_kind ? 110 : priority,
                    origin, i);
        }
        if (b->error) return b->error;
    }
    return b->error;
}
static int doc_si_cst_metadata(xui_document_source_index index, uint64_t record,
    uint64_t ordinal, xui_doc_source_binding_t* result)
{
    doc_source_cst_record s; uint64_t base; int code = doc_si_cst_origin(index, record, ordinal, &s, &base);
    if (code != XUI_OK) return code;
    result->iMetadata |= XUI_DOC_SOURCE_BINDING_SOURCE_ONLY;
    if (s.kind == DOC_CST_INDENT && s.flags) {
        result->iMetadata |= XUI_DOC_SOURCE_BINDING_COLUMNS;
        result->iStartColumn = s.metadata[0];
        if (s.flags & DOC_CST_INDENT_QUOTE) {
            result->iMarkerColumn = s.metadata[1]; result->iContentColumn = s.metadata[2]; result->iEndColumn = s.metadata[3];
            if (result->iStartColumn > result->iMarkerColumn || result->iMarkerColumn >= result->iContentColumn ||
                result->iContentColumn > result->iEndColumn) return XUI_DOC_ERROR_FORMAT;
        } else {
            result->iContentColumn = s.metadata[1] == UINT32_MAX ? DOC_NONE : s.metadata[1]; result->iEndColumn = s.metadata[2];
            if (result->iStartColumn > result->iEndColumn || (result->iContentColumn != DOC_NONE &&
                (result->iStartColumn > result->iContentColumn || result->iContentColumn > result->iEndColumn))) return XUI_DOC_ERROR_FORMAT;
            if (s.flags & DOC_CST_INDENT_CODE) result->iMarkerFlags = s.metadata[3];
        }
    } else if (s.kind == DOC_CST_MARKER && (s.flags & DOC_CST_MARKER_TABLE)) {
        result->iMetadata |= XUI_DOC_SOURCE_BINDING_TABLE;
        result->iRow = s.metadata[0]; result->iOrdinal = s.metadata[1]; result->iMarkerFlags = s.metadata[2];
    } else if (s.kind == DOC_CST_FIELD && (s.detail == DOC_CST_FIELD_TABLE_ROW || s.detail == DOC_CST_FIELD_TABLE_CELL)) {
        result->iMetadata |= XUI_DOC_SOURCE_BINDING_TABLE;
        result->iRow = s.metadata[2]; result->iOrdinal = s.metadata[3];
    }
    return XUI_OK;
}
