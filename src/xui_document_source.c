#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

XUI_API int xuiDocumentSnapshotGetSourceInfo(xui_document_snapshot snapshot, xui_doc_source_info_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    out->iInlineSyntaxCount = doc_seq_size(snapshot->state->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    out->iReferenceDefinitionCount = doc_seq_size(snapshot->state->references) / sizeof(xui_doc_reference_definition_t);
    out->iReferenceCandidateCount = doc_seq_size(snapshot->state->reference_candidates) / sizeof(xui_doc_inline_syntax_t);
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetInlineSyntax(xui_document_snapshot snapshot, uint64_t index, xui_doc_inline_syntax_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (index >= doc_seq_size(snapshot->state->inline_syntax) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read_syntax(snapshot->state->inline_syntax, index, out);
}
XUI_API int xuiDocumentSnapshotGetReferenceDefinition(xui_document_snapshot snapshot, uint64_t index, xui_doc_reference_definition_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (index >= doc_seq_size(snapshot->state->references) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read(snapshot->state->references, index * sizeof(*out), out, sizeof(*out));
}
XUI_API int xuiDocumentSnapshotGetReferenceCandidate(xui_document_snapshot snapshot, uint64_t index, xui_doc_inline_syntax_t* out)
{
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (index >= doc_seq_size(snapshot->state->reference_candidates) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    return doc_seq_read_syntax(snapshot->state->reference_candidates, index, out);
}

int doc_source_append(doc_allocator* a, doc_sequence** target, const xui_doc_source_segment_t* segment)
{
    doc_sequence *item, *next = NULL; uint64_t size = doc_seq_size(*target); int result;
    item = doc_seq_text(a, (const char*)segment, sizeof(*segment));
    if (!item) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_replace(a, *target, size, size, item, &next); doc_seq_release(item);
    if (result == XUI_OK) { doc_seq_release(*target); *target = next; }
    return result;
}

/* Rebase the packed source spans only when they are read. Unchanged suffix
 * nodes can retain the exact same persistent provenance sequence across edits. */
void doc_source_resolve_segment(const doc_state* state, const doc_node* node,
    xui_doc_source_segment_t* segment)
{
    int64_t shift = doc_node_block_shift(state, node);
    if (node->provenance_base != DOC_NONE)
        shift += (int64_t)node->provenance_current - (int64_t)node->provenance_base;
    if (segment->iSourceStart != DOC_NONE) segment->iSourceStart = (uint64_t)((int64_t)segment->iSourceStart + shift);
    if (segment->iSourceEnd != DOC_NONE) segment->iSourceEnd = (uint64_t)((int64_t)segment->iSourceEnd + shift);
}

int doc_source_shift_provenance(doc_node* node, uint64_t removed, uint64_t added)
{
    uint64_t i, base = DOC_NONE;
    if (!node->provenance || removed == added) return XUI_OK;
    if (node->provenance_base == DOC_NONE) {
        for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(xui_doc_source_segment_t)) {
            xui_doc_source_segment_t segment;
            int result = doc_seq_read(node->provenance, i, &segment, sizeof(segment));
            if (result != XUI_OK) return result;
            if (segment.iSourceStart != DOC_NONE && segment.iSourceStart < base) base = segment.iSourceStart;
            if (segment.iSourceEnd != DOC_NONE && segment.iSourceEnd < base) base = segment.iSourceEnd;
        }
        if (base == DOC_NONE) return XUI_OK;
        node->provenance_base = node->provenance_current = base;
    }
    if (node->provenance_current < removed || node->provenance_current - removed > UINT64_MAX - added)
        return XUI_ERROR_UNSUPPORTED;
    node->provenance_current = node->provenance_current - removed + added;
    return XUI_OK;
}

int doc_source_slice(doc_state* state, const doc_node* node, uint64_t start, uint64_t end,
    uint64_t target_offset, doc_sequence** out)
{
    uint64_t i;
    for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(xui_doc_source_segment_t)) {
        xui_doc_source_segment_t s; uint64_t from, to; int result;
        doc_seq_read(node->provenance, i, &s, sizeof(s));
        doc_source_resolve_segment(state, node, &s);
        if (s.iTextEnd <= start || s.iTextStart >= end) continue;
        from = s.iTextStart > start ? s.iTextStart : start; to = s.iTextEnd < end ? s.iTextEnd : end;
        if (s.iKind == XUI_DOC_SOURCE_DIRECT) {
            s.iSourceEnd = s.iSourceStart + to - s.iTextStart; s.iSourceStart += from - s.iTextStart;
        } else {
            if (from != s.iTextStart) s.iFlags |= XUI_DOC_SOURCE_PARTIAL_START;
            if (to != s.iTextEnd) s.iFlags |= XUI_DOC_SOURCE_PARTIAL_END;
        }
        s.iTextStart = target_offset + from - start; s.iTextEnd = target_offset + to - start;
        result = doc_source_append(state->allocator, out, &s); if (result != XUI_OK) return result;
    }
    return XUI_OK;
}

XUI_API int xuiDocumentSnapshotGetSourceSegment(xui_document_snapshot snapshot, uint64_t id,
    uint64_t index, xui_doc_source_segment_t* out)
{
    doc_node* node;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    node = doc_index_get(snapshot->state->index, id);
    if (!node || index >= doc_seq_size(node->provenance) / sizeof(*out)) return XUI_ERROR_NOT_FOUND;
    { int result = doc_seq_read(node->provenance, index * sizeof(*out), out, sizeof(*out));
      if (result == XUI_OK) doc_source_resolve_segment(snapshot->state, node, out);
      return result; }
}

/* A source line is a raw byte partition, independent of Markdown semantics.
 * Read bounded chunks so a long code/HTML line never needs a flat copy. */
XUI_API int xuiDocumentSnapshotGetSourceLine(xui_document_snapshot snapshot,
    uint64_t offset, xui_doc_source_line_t* out)
{
    doc_sequence* source;
    uint64_t size, at, start, indent_end, trailing_start;
    char buffer[4096], current;
    int body = 0, result;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    source = snapshot->state->source; size = doc_seq_size(source);
    if (offset > size) return XUI_ERROR_INVALID_ARGUMENT;
    if (offset == size) return XUI_ERROR_NOT_FOUND;
    at = offset;
    result = doc_seq_read(source, offset, &current, 1);
    if (result != XUI_OK) return result;
    if (current == '\n' && offset) {
        result = doc_seq_read(source, offset - 1, &current, 1);
        if (result != XUI_OK) return result;
        if (current == '\r') at--;
    }
    while (at) {
        uint64_t count = at < sizeof(buffer) ? at : sizeof(buffer), i;
        start = at - count;
        result = doc_seq_read(source, start, buffer, count);
        if (result != XUI_OK) return result;
        for (i = count; i; i--) if (buffer[i - 1] == '\r' || buffer[i - 1] == '\n') {
            at = start + i;
            goto found_start;
        }
        at = start;
    }
found_start:
    start = at; indent_end = trailing_start = start;
    out->iLineStart = start;
    while (at < size) {
        uint64_t count = size - at < sizeof(buffer) ? size - at : sizeof(buffer), i;
        result = doc_seq_read(source, at, buffer, count);
        if (result != XUI_OK) return result;
        for (i = 0; i < count; i++) {
            uint64_t position = at + i;
            current = buffer[i];
            if (current == '\r' || current == '\n') {
                out->iContentEnd = position;
                out->iLineEnd = position + 1;
                if (current == '\r' && out->iLineEnd < size) {
                    char next;
                    result = doc_seq_read(source, out->iLineEnd, &next, 1);
                    if (result != XUI_OK) return result;
                    if (next == '\n') out->iLineEnd++;
                }
                goto found_end;
            }
            if (current == ' ' || current == '\t') {
                if (!body) indent_end = position + 1;
            } else {
                body = 1; trailing_start = position + 1;
            }
        }
        at += count;
    }
    out->iContentEnd = out->iLineEnd = size;
found_end:
    out->iIndentEnd = indent_end;
    out->iTrailingStart = body ? trailing_start : indent_end;
    return XUI_OK;
}

XUI_API int xuiDocumentSnapshotGetBlockSyntax(xui_document_snapshot snapshot,
    xui_doc_node_id id, xui_doc_block_syntax_t* out)
{
    doc_node* node;
    doc_node_source_range range;
    uint64_t span;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    node = doc_index_get(snapshot->state->index, id);
    if (!node || !node->marker_kind || node->kind == XUI_DOC_SOFT_BREAK ||
        node->kind == XUI_DOC_HARD_BREAK) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(snapshot->state, node, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_end < range.syntax_start || range.syntax_end > doc_seq_size(snapshot->state->source))
        return XUI_DOC_ERROR_FORMAT;
    span = range.syntax_end - range.syntax_start;
    if (node->marker_primary_start > node->marker_primary_end || node->marker_primary_end > span ||
        node->marker_primary_start == node->marker_primary_end) return XUI_DOC_ERROR_FORMAT;
    out->iKind = node->marker_kind;
    out->iPrimaryStart = range.syntax_start + node->marker_primary_start;
    out->iPrimaryEnd = range.syntax_start + node->marker_primary_end;
    out->iSecondaryStart = out->iSecondaryEnd = DOC_NONE;
    out->iFenceTailStart = out->iFenceTailEnd = DOC_NONE;
    out->iFenceInfoStart = out->iFenceInfoEnd = DOC_NONE;
    out->iFenceLanguageStart = out->iFenceLanguageEnd = DOC_NONE;
    out->iListMarkerGapStart = out->iListMarkerGapEnd = DOC_NONE;
    out->iTaskMarkerGapStart = out->iTaskMarkerGapEnd = DOC_NONE;
    out->iHeadingContentStart = out->iHeadingContentEnd = DOC_NONE;
    if (node->marker_secondary_start || node->marker_secondary_end) {
        if (node->marker_secondary_start >= node->marker_secondary_end ||
            node->marker_secondary_end > span) return XUI_DOC_ERROR_FORMAT;
        out->iSecondaryStart = range.syntax_start + node->marker_secondary_start;
        out->iSecondaryEnd = range.syntax_start + node->marker_secondary_end;
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_ATX_HEADING ||
        node->marker_kind == XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING) {
        doc_heading_content_relative content;
        if (node->kind != XUI_DOC_HEADING || !node->syntax_aux ||
            node->syntax_aux->size != sizeof(content)) return XUI_DOC_ERROR_FORMAT;
        memcpy(&content, node->syntax_aux->data, sizeof(content));
        if (content.start > content.end || content.end > span ||
            (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_ATX_HEADING && content.start < node->marker_primary_end) ||
            (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING && content.end > node->marker_primary_start))
            return XUI_DOC_ERROR_FORMAT;
        out->iHeadingContentStart = range.syntax_start + content.start;
        out->iHeadingContentEnd = range.syntax_start + content.end;
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_FENCED_CODE) {
        doc_fence_info_relative info;
        if (node->marker_tail_end < node->marker_primary_end ||
            node->marker_tail_end > span || !node->syntax_aux ||
            node->syntax_aux->size != sizeof(info))
            return XUI_DOC_ERROR_FORMAT;
        out->iFenceTailStart = out->iPrimaryEnd;
        out->iFenceTailEnd = range.syntax_start + node->marker_tail_end;
        memcpy(&info, node->syntax_aux->data, sizeof(info));
        if (info.info_start < node->marker_primary_end ||
            info.info_start > info.language_end ||
            info.language_end > info.info_end ||
            info.info_end > node->marker_tail_end) return XUI_DOC_ERROR_FORMAT;
        out->iFenceInfoStart = range.syntax_start + info.info_start;
        out->iFenceInfoEnd = range.syntax_start + info.info_end;
        out->iFenceLanguageStart = out->iFenceInfoStart;
        out->iFenceLanguageEnd = range.syntax_start + info.language_end;
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN) {
        if (node->quote_prefixes && node->quote_prefixes->size % sizeof(uint32_t))
            return XUI_DOC_ERROR_FORMAT;
        out->iQuotePrefixCount = 1 + (node->quote_prefixes ?
            node->quote_prefixes->size / sizeof(uint32_t) : 0);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_TABLE_UNDERLINE) {
        if (!node->syntax_aux || node->syntax_aux->size % sizeof(doc_table_token_relative))
            return XUI_DOC_ERROR_FORMAT;
        out->iTableTokenCount = node->syntax_aux->size / sizeof(doc_table_token_relative);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE) {
        if (node->kind != XUI_DOC_CODE_BLOCK || !node->syntax_aux ||
            !node->syntax_aux->size ||
            node->syntax_aux->size % sizeof(doc_code_indent_relative))
            return XUI_DOC_ERROR_FORMAT;
        out->iCodeIndentCount = node->syntax_aux->size / sizeof(doc_code_indent_relative);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM && node->list_indents) {
        if (node->list_indents->size % sizeof(doc_list_indent_relative))
            return XUI_DOC_ERROR_FORMAT;
        out->iListIndentCount = node->list_indents->size / sizeof(doc_list_indent_relative);
    }
    if (node->marker_kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM) {
        out->iListMarkerGapStart = out->iPrimaryEnd;
        if (node->marker_secondary_end) {
            if (node->marker_secondary_start < node->marker_primary_end ||
                node->marker_tail_end < node->marker_secondary_end ||
                node->marker_tail_end > span) return XUI_DOC_ERROR_FORMAT;
            out->iListMarkerGapEnd = out->iSecondaryStart;
            out->iTaskMarkerGapStart = out->iSecondaryEnd;
            out->iTaskMarkerGapEnd = range.syntax_start + node->marker_tail_end;
        } else {
            if (node->marker_tail_end < node->marker_primary_end ||
                node->marker_tail_end > span) return XUI_DOC_ERROR_FORMAT;
            out->iListMarkerGapEnd = range.syntax_start + node->marker_tail_end;
        }
    }
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetBreakSyntax(xui_document_snapshot snapshot,
    xui_doc_node_id id, xui_doc_break_syntax_t* out)
{
    doc_node* node;
    doc_node_source_range range;
    uint64_t span;
    if (!snapshot || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    if (snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    node = doc_index_get(snapshot->state->index, id);
    if (!node || (node->kind != XUI_DOC_SOFT_BREAK && node->kind != XUI_DOC_HARD_BREAK) ||
        !node->marker_kind) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(snapshot->state, node, &range);
    if (range.source_start == DOC_NONE || range.source_end == DOC_NONE ||
        range.source_end < range.source_start ||
        range.source_end > doc_seq_size(snapshot->state->source)) return XUI_DOC_ERROR_FORMAT;
    span = range.source_end - range.source_start;
    if (node->marker_kind < XUI_DOC_BREAK_SOFT ||
        node->marker_kind > XUI_DOC_BREAK_HARD_FORCED ||
        (node->kind == XUI_DOC_SOFT_BREAK) != (node->marker_kind == XUI_DOC_BREAK_SOFT) ||
        node->marker_primary_start > node->marker_primary_end ||
        node->marker_primary_end >= node->marker_tail_end ||
        node->marker_tail_end != span) return XUI_DOC_ERROR_FORMAT;
    if (node->marker_secondary_start || node->marker_secondary_end) {
        if (node->marker_kind != XUI_DOC_BREAK_HARD_BACKSLASH &&
            node->marker_kind != XUI_DOC_BREAK_HARD_SPACES) return XUI_DOC_ERROR_FORMAT;
        if (node->marker_secondary_start >= node->marker_secondary_end ||
            node->marker_secondary_end > node->marker_primary_end)
            return XUI_DOC_ERROR_FORMAT;
    } else if (node->marker_kind == XUI_DOC_BREAK_HARD_BACKSLASH ||
        node->marker_kind == XUI_DOC_BREAK_HARD_SPACES) return XUI_DOC_ERROR_FORMAT;
    out->iKind = node->marker_kind;
    out->iTrailingStart = range.source_start + node->marker_primary_start;
    out->iTrailingEnd = range.source_start + node->marker_primary_end;
    out->iLineEndingStart = out->iTrailingEnd;
    out->iLineEndingEnd = range.source_start + node->marker_tail_end;
    out->iHardMarkerStart = out->iHardMarkerEnd = DOC_NONE;
    if (node->marker_secondary_end) {
        out->iHardMarkerStart = range.source_start + node->marker_secondary_start;
        out->iHardMarkerEnd = range.source_start + node->marker_secondary_end;
    }
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetQuotePrefix(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, uint64_t* start, uint64_t* end)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_node* node;
    doc_node_source_range range;
    uint32_t relative;
    int result;
    if (!start || !end) return XUI_ERROR_INVALID_ARGUMENT;
    *start = *end = DOC_NONE;
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN ||
        index >= syntax.iQuotePrefixCount) return XUI_ERROR_NOT_FOUND;
    if (!index) {
        *start = syntax.iPrimaryStart; *end = syntax.iPrimaryEnd;
        return XUI_OK;
    }
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->quote_prefixes->data + (index - 1) * sizeof(relative),
        sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative <= node->marker_primary_start ||
        relative >= range.syntax_end - range.syntax_start)
        return XUI_DOC_ERROR_FORMAT;
    *start = range.syntax_start + relative; *end = *start + 1;
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetListContinuationIndent(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, xui_doc_list_indent_t* out)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_node_source_range range;
    doc_node* node;
    doc_list_indent_relative relative;
    int result;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_LIST_ITEM ||
        index >= syntax.iListIndentCount) return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->list_indents->data + index * sizeof(relative), sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative.start >= relative.end ||
        relative.start <= node->marker_primary_end ||
        relative.end > range.syntax_end - range.syntax_start ||
        relative.start_column >= relative.content_column ||
        relative.content_column > relative.end_column) return XUI_DOC_ERROR_FORMAT;
    out->iSourceStart = range.syntax_start + relative.start;
    out->iSourceEnd = range.syntax_start + relative.end;
    out->iIndentStartColumn = relative.start_column;
    out->iContentColumn = relative.content_column;
    out->iIndentEndColumn = relative.end_column;
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetCodeIndent(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, xui_doc_code_indent_t* out)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_code_indent_relative relative;
    doc_node_source_range range;
    doc_node* node;
    int result;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE ||
        index >= syntax.iCodeIndentCount) return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->syntax_aux->data + index * sizeof(relative), sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative.start > relative.end ||
        relative.end > range.syntax_end - range.syntax_start ||
        relative.flags > XUI_DOC_CODE_INDENT_BLANK ||
        (relative.flags ? relative.content_column != UINT32_MAX ||
            relative.start_column != relative.end_column :
            relative.start == relative.end ||
            relative.start_column >= relative.content_column ||
            relative.content_column > relative.end_column ||
            relative.content_column - relative.start_column != 4))
        return XUI_DOC_ERROR_FORMAT;
    out->iFlags = relative.flags;
    out->iSourceStart = range.syntax_start + relative.start;
    out->iSourceEnd = range.syntax_start + relative.end;
    out->iIndentStartColumn = relative.start_column;
    out->iContentColumn = relative.content_column;
    out->iIndentEndColumn = relative.end_column;
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotGetTableToken(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint64_t index, xui_doc_table_token_t* out)
{
    xui_doc_block_syntax_t syntax = {0};
    doc_table_token_relative relative;
    doc_node_source_range range;
    doc_node* node;
    int result;
    if (!out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(snapshot, id, &syntax);
    if (result != XUI_OK) return result;
    if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_TABLE_UNDERLINE ||
        index >= syntax.iTableTokenCount) return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(snapshot->state->index, id);
    memcpy(&relative, node->syntax_aux->data + index * sizeof(relative), sizeof(relative));
    doc_node_source_range_get(snapshot->state, node, &range);
    if (relative.start >= relative.end ||
        relative.end > range.syntax_end - range.syntax_start ||
        (relative.kind != XUI_DOC_TABLE_TOKEN_PIPE &&
         relative.kind != XUI_DOC_TABLE_TOKEN_UNDERLINE) ||
        (relative.kind == XUI_DOC_TABLE_TOKEN_UNDERLINE && relative.row != 1) ||
        relative.flags > 3) return XUI_DOC_ERROR_FORMAT;
    out->iKind = relative.kind; out->iRow = relative.row;
    out->iOrdinal = relative.ordinal; out->iFlags = relative.flags;
    out->iSourceStart = range.syntax_start + relative.start;
    out->iSourceEnd = range.syntax_start + relative.end;
    return XUI_OK;
}

static unsigned doc_source_quality(int mapping)
{
    return mapping == XUI_DOC_MAP_EXACT ? 0 : mapping == XUI_DOC_MAP_COLLAPSED ? 1 :
        mapping == XUI_DOC_MAP_SYNTAX ? 2 : 3;
}
int doc_source_better(const doc_source_hit* a, const doc_source_hit* b, unsigned affinity)
{
    if (a->distance != b->distance) return a->distance < b->distance;
    if (a->mapping != b->mapping) return doc_source_quality(a->mapping) < doc_source_quality(b->mapping);
    if (a->rank != b->rank) return a->rank < b->rank;
    if (a->span != b->span) return a->span < b->span;
    return affinity == XUI_DOC_AFTER;
}
static int doc_source_segment_map(const xui_doc_source_segment_t* s, uint64_t offset,
    unsigned affinity, int from_source, doc_source_hit* hit)
{
    uint64_t begin = from_source ? s->iSourceStart : s->iTextStart;
    uint64_t end = from_source ? s->iSourceEnd : s->iTextEnd;
    uint64_t other_begin = from_source ? s->iTextStart : s->iSourceStart;
    uint64_t other_end = from_source ? s->iTextEnd : s->iSourceEnd;
    if (s->iSourceStart == DOC_NONE || s->iSourceEnd == DOC_NONE) return 0;
    hit->distance = offset < begin ? begin - offset : offset > end ? offset - end : 0;
    hit->span = end - begin;
    hit->rank = (offset == begin && affinity == XUI_DOC_BEFORE) || (offset == end && affinity == XUI_DOC_AFTER);
    if (hit->distance) {
        hit->offset = offset < begin ? other_begin : other_end;
        hit->mapping = from_source ? XUI_DOC_MAP_SYNTAX : XUI_DOC_MAP_APPROXIMATE;
    } else if (s->iKind == XUI_DOC_SOURCE_DIRECT && end - begin == other_end - other_begin) {
        hit->offset = other_begin + offset - begin; hit->mapping = XUI_DOC_MAP_EXACT;
    } else if (begin != end && offset == begin && !(s->iFlags & XUI_DOC_SOURCE_PARTIAL_START)) {
        hit->offset = other_begin; hit->mapping = XUI_DOC_MAP_EXACT;
    } else if (begin != end && offset == end && !(s->iFlags & XUI_DOC_SOURCE_PARTIAL_END)) {
        hit->offset = other_end; hit->mapping = XUI_DOC_MAP_EXACT;
    } else {
        hit->offset = affinity == XUI_DOC_AFTER ? other_end : other_begin; hit->mapping = XUI_DOC_MAP_COLLAPSED;
    }
    return 1;
}
static int doc_source_map(const doc_state* state, const doc_node* node,
    uint64_t offset, unsigned affinity, int from_source, doc_source_hit* out)
{
    uint64_t i; int found = 0;
    for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(xui_doc_source_segment_t)) {
        xui_doc_source_segment_t segment; doc_source_hit hit;
        doc_seq_read(node->provenance, i, &segment, sizeof(segment));
        doc_source_resolve_segment(state, node, &segment);
        if (doc_source_segment_map(&segment, offset, affinity, from_source, &hit) &&
            (!found || doc_source_better(&hit, out, affinity))) { *out = hit; found = 1; }
    }
    return found;
}
int doc_source_map_source(const doc_state* state, const doc_node* node,
    uint64_t offset, unsigned affinity, doc_source_hit* out)
{
    return doc_source_map(state, node, offset, affinity, 1, out);
}
int doc_source_map_text(const doc_state* state, const doc_node* node,
    uint64_t offset, unsigned affinity, uint64_t* out, int* mapping)
{
    doc_source_hit hit;
    if (!doc_source_map(state, node, offset, affinity, 0, &hit)) return XUI_ERROR_NOT_FOUND;
    *out = hit.offset; *mapping = hit.mapping; return XUI_OK;
}

#endif
