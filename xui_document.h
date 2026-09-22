#ifndef XUI_DOCUMENT_H
#define XUI_DOCUMENT_H

/* Unified, window-independent document API. All input strings are UTF-8.
 * Owning output handles must be released. Snapshot slices are borrowed until
 * the snapshot is released. A live document has one writer; snapshots may be
 * read concurrently. No node, cell, or view owns a separate undo history. */
#include "xui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xui_document_t* xui_document;
typedef struct xui_doc_snapshot_t* xui_document_snapshot;
typedef struct xui_doc_transaction_t* xui_document_transaction;
typedef struct xui_doc_change_set_t* xui_document_change_set;
typedef uint64_t xui_doc_node_id;
#define XUI_DOCUMENT_ROOT UINT64_C(1)
#define XUI_DOCUMENT_APPEND UINT64_MAX

enum xui_doc_profile {
    XUI_DOCUMENT_RICH = 1,
    XUI_DOCUMENT_MARKDOWN = 2
};
enum xui_doc_markdown_dialect {
    XUI_MD_COMMONMARK = 1, XUI_MD_GFM = 2, XUI_MD_EXTENDED = 3
};
enum xui_doc_node_kind {
    XUI_DOC_ROOT = 1, XUI_DOC_PARAGRAPH, XUI_DOC_HEADING, XUI_DOC_TEXT,
    XUI_DOC_QUOTE, XUI_DOC_LIST, XUI_DOC_LIST_ITEM, XUI_DOC_CODE_BLOCK,
    XUI_DOC_TABLE, XUI_DOC_ROW, XUI_DOC_CELL, XUI_DOC_IMAGE,
    XUI_DOC_RULE, XUI_DOC_SOFT_BREAK, XUI_DOC_HARD_BREAK,
    XUI_DOC_HTML, XUI_DOC_MATH, XUI_DOC_DIAGRAM, XUI_DOC_FOOTNOTE,
    XUI_DOC_FOOTNOTE_REF, XUI_DOC_FRONT_MATTER, XUI_DOC_EXTENSION
};
enum xui_doc_mark {
    XUI_DOC_BOLD = 1u, XUI_DOC_ITALIC = 2u, XUI_DOC_UNDERLINE = 4u,
    XUI_DOC_STRIKE = 8u, XUI_DOC_CODE = 16u, XUI_DOC_SUBSCRIPT = 32u,
    XUI_DOC_SUPERSCRIPT = 64u, XUI_DOC_LINK = 128u, XUI_DOC_HIGHLIGHT = 256u
};
enum xui_doc_error {
    XUI_DOC_ERROR_SCHEMA = -100, XUI_DOC_ERROR_UTF8 = -101,
    XUI_DOC_ERROR_STALE = -102, XUI_DOC_ERROR_BUSY = -103,
    XUI_DOC_ERROR_LIMIT = -104, XUI_DOC_ERROR_FORMAT = -105,
    XUI_DOC_ERROR_DOMAIN = -106, XUI_DOC_ERROR_UNREPRESENTABLE = -107,
    XUI_DOC_ERROR_IO = -108
};
enum xui_doc_position_kind { XUI_DOC_POSITION_TEXT = 1, XUI_DOC_POSITION_GAP = 2, XUI_DOC_POSITION_SOURCE = 3 };
enum xui_doc_affinity { XUI_DOC_BEFORE = 0, XUI_DOC_AFTER = 1 };
enum xui_doc_domain { XUI_DOC_SEMANTIC = 1, XUI_DOC_SOURCE = 2 };
enum xui_doc_file_format { XUI_DOC_FILE_NATIVE = 1, XUI_DOC_FILE_MARKDOWN = 2, XUI_DOC_FILE_TEXT = 3, XUI_DOC_FILE_HTML = 4 };
enum xui_doc_change_flags {
    XUI_DOC_CHANGE_TEXT = 1u, XUI_DOC_CHANGE_STRUCTURE = 2u,
    XUI_DOC_CHANGE_STYLE = 4u, XUI_DOC_CHANGE_SOURCE = 8u,
    XUI_DOC_CHANGE_RESOURCE = 16u, XUI_DOC_CHANGE_RESET = 32u
};
enum xui_doc_operation_kind {
    XUI_DOC_OP_TEXT = 1, XUI_DOC_OP_INSERT, XUI_DOC_OP_DELETE,
    XUI_DOC_OP_MOVE, XUI_DOC_OP_ATTRIBUTES, XUI_DOC_OP_SOURCE,
    XUI_DOC_OP_SPLIT, XUI_DOC_OP_MERGE
};
enum xui_doc_mapping { XUI_DOC_MAP_EXACT = 0, XUI_DOC_MAP_DELETED = 1, XUI_DOC_MAP_APPROXIMATE = 2 };
enum xui_doc_node_flags {
    XUI_DOC_ORDERED = 1u, XUI_DOC_TASK = 2u, XUI_DOC_CHECKED = 4u,
    XUI_DOC_HEADER = 8u, XUI_DOC_BLOCK = 16u, XUI_DOC_TIGHT = 32u
};

typedef void* (*xui_doc_alloc_proc)(void* user, size_t bytes);
typedef void (*xui_doc_free_proc)(void* user, void* pointer);

typedef struct xui_doc_desc_t {
    uint32_t iSize;
    uint32_t iProfile;
    uint32_t iMarkdownDialect; /* 0 selects EXTENDED: GFM + math, footnotes, admonitions, front matter, Mermaid. */
    uint32_t iHistoryLimit;   /* 0 selects 256; bDisableHistory explicitly opts out. */
    int bDisableHistory;
    uint64_t iMaxTextBytes;  /* 0 selects 256 MiB. */
    uint64_t iMaxNodes;      /* 0 selects 1,000,000. */
    xui_doc_alloc_proc onAlloc;
    xui_doc_free_proc onFree;
    void* pAllocatorUser;   /* Must outlive every handle/snapshot using it. */
} xui_doc_desc_t;

typedef struct xui_doc_attributes_t {
    uint32_t iMarks;
    uint32_t iFlags;
    uint32_t iHeadingLevel;
    uint32_t iAlignment;    /* 0 left, 1 center, 2 right, 3 justified. */
    uint32_t iRowSpan;
    uint32_t iColumnSpan;
    uint64_t iListStart;
    uint32_t iTextColor;
    uint32_t iBackgroundColor;
    float fFontSize;
    float fWidth;
    float fHeight;
    float fParagraphSpacing;
    char sFontFamily[64];
} xui_doc_attributes_t;

typedef struct xui_doc_node_desc_t {
    uint32_t iSize;
    uint32_t iKind;
    xui_doc_attributes_t tAttributes;
    const char* sText;
    uint64_t iTextBytes;
    const char* sResource;  /* URI or extension identifier; copied. */
    const char* sInfo;      /* Code language, extension data type, or label. */
    const char* sTitle;
} xui_doc_node_desc_t;

typedef struct xui_doc_node_info_t {
    uint32_t iSize;
    uint32_t iKind;
    xui_doc_node_id iId;
    xui_doc_node_id iParentId;
    uint64_t iChildCount;
    uint64_t iTextBytes;
    xui_doc_attributes_t tAttributes;
    const char* sResource;
    const char* sInfo;
    const char* sTitle;
    uint64_t iSourceStart;
    uint64_t iSourceEnd;
    int bSourceExact;
} xui_doc_node_info_t;

typedef struct xui_doc_position_t {
    uint32_t iSize;
    uint32_t iKind;
    uint64_t iDocumentId;
    uint64_t iRevision;
    xui_doc_node_id iNodeId;
    uint64_t iOffset;
    uint32_t iAffinity;
    uint32_t iReserved;
} xui_doc_position_t;
typedef struct xui_doc_range_t { xui_doc_position_t tAnchor, tCaret; } xui_doc_range_t;

typedef struct xui_doc_txn_desc_t {
    uint32_t iSize;
    uint32_t iDomain;
    uint64_t iBaseRevision; /* 0 means the current revision. */
    uint64_t iOrigin;
    uint64_t iGroup;        /* Nonzero explicitly groups consecutive commits. */
} xui_doc_txn_desc_t;

typedef struct xui_doc_operation_t {
    uint32_t iKind;
    uint32_t iFlags;
    xui_doc_node_id iNodeId;
    xui_doc_node_id iParentId;
    xui_doc_node_id iOtherNodeId;
    uint64_t iOffset;
    uint64_t iOldLength;
    uint64_t iNewLength;
} xui_doc_operation_t;

typedef struct xui_doc_change_info_t {
    uint32_t iSize;
    uint32_t iFlags;
    uint64_t iBeforeRevision;
    uint64_t iAfterRevision;
    uint64_t iOrigin;
    uint64_t iOperationCount;
    const xui_doc_operation_t* pOperations;
    int bUndo;
} xui_doc_change_info_t;

typedef struct xui_doc_stats_t {
    uint32_t iSize;
    uint64_t iLiveBytes;
    uint64_t iPeakBytes;
    uint64_t iAllocations;
    uint64_t iNodes;
    uint64_t iUndoCount;
    uint64_t iRedoCount;
    uint64_t iMarkdownParses;
    uint64_t iMarkdownParsedBytes;
} xui_doc_stats_t;

typedef void (*xui_doc_change_proc)(xui_document document,
    xui_document_change_set change, void* user);

/* Lifetime, immutable reads, subscriptions and diagnostics. */
XUI_API int xuiDocumentCreate(const xui_doc_desc_t* desc, xui_document* out);
XUI_API void xuiDocumentRetain(xui_document document);
XUI_API void xuiDocumentRelease(xui_document document);
XUI_API uint64_t xuiDocumentGetRevision(xui_document document);
XUI_API uint64_t xuiDocumentGetIdentity(xui_document document);
XUI_API int xuiDocumentGetProfile(xui_document document);
XUI_API int xuiDocumentGetMarkdownDialect(xui_document document);
XUI_API int xuiDocumentAcquireSnapshot(xui_document document, xui_document_snapshot* out);
XUI_API void xuiDocumentSnapshotRetain(xui_document_snapshot snapshot);
XUI_API void xuiDocumentSnapshotRelease(xui_document_snapshot snapshot);
XUI_API uint64_t xuiDocumentSnapshotGetRevision(xui_document_snapshot snapshot);
XUI_API int xuiDocumentSnapshotGetNode(xui_document_snapshot snapshot, xui_doc_node_id id, xui_doc_node_info_t* out);
XUI_API int xuiDocumentSnapshotGetChild(xui_document_snapshot snapshot, xui_doc_node_id parent, uint64_t index, xui_doc_node_id* out);
/* Copy functions report the required byte length (without a terminator).
 * A NULL buffer queries the length. A supplied buffer needs length+1 bytes. */
XUI_API int xuiDocumentSnapshotCopyText(xui_document_snapshot snapshot, xui_doc_node_id node, char* buffer, uint64_t capacity, uint64_t* length);
XUI_API int xuiDocumentSnapshotCopySource(xui_document_snapshot snapshot, char* buffer, uint64_t capacity, uint64_t* length);
XUI_API int xuiDocumentSnapshotReadText(xui_document_snapshot snapshot, xui_doc_node_id node, uint64_t offset, void* buffer, uint64_t bytes);
XUI_API int xuiDocumentSnapshotReadSource(xui_document_snapshot snapshot, uint64_t offset, void* buffer, uint64_t bytes);
XUI_API int xuiDocumentSubscribe(xui_document document, xui_doc_change_proc callback, void* user, uint64_t* token);
XUI_API void xuiDocumentUnsubscribe(xui_document document, uint64_t token);
XUI_API int xuiDocumentGetStats(xui_document document, xui_doc_stats_t* out);
XUI_API int xuiDocumentIsDirty(xui_document document);
XUI_API int xuiDocumentMarkSaved(xui_document document, xui_document_snapshot saved);
/* UTF-8 paths. Snapshot export writes an exclusive temporary file in the target
 * directory and atomically replaces the target using XRT's filesystem layer.
 * Export never changes saved state. SaveFile accepts native or Markdown only.
 * For asynchronous saving, export a retained snapshot and MarkSaved that exact
 * snapshot on the document owner thread only after successful publication. */
XUI_API int xuiDocumentSnapshotExportFile(xui_document_snapshot snapshot, const char* path, uint32_t format);
XUI_API int xuiDocumentSaveFile(xui_document document, const char* path, uint32_t format);
/* Returns a new, clean document without an import undo entry. max_file_bytes=0
 * selects a 256 MiB input cap. Native and Markdown are accepted input formats. */
XUI_API int xuiDocumentOpenFile(const char* path, uint32_t format, const xui_doc_desc_t* desc, uint64_t max_file_bytes, xui_document* out);

/* Transactions are all-or-nothing. An operation failure poisons the transaction.
 * Commit on a failed transaction publishes nothing. Release implies Abort. */
XUI_API int xuiDocumentBeginTransaction(xui_document document, const xui_doc_txn_desc_t* desc, xui_document_transaction* out);
XUI_API int xuiDocumentTxnGetNode(xui_document_transaction transaction, xui_doc_node_id id, xui_doc_node_info_t* out);
XUI_API int xuiDocumentTxnInsertNode(xui_document_transaction transaction, xui_doc_node_id parent, uint64_t child_index, const xui_doc_node_desc_t* desc, xui_doc_node_id* out);
XUI_API int xuiDocumentTxnDeleteNode(xui_document_transaction transaction, xui_doc_node_id node);
XUI_API int xuiDocumentTxnMoveNode(xui_document_transaction transaction, xui_doc_node_id node, xui_doc_node_id parent, uint64_t child_index);
XUI_API int xuiDocumentTxnReplaceText(xui_document_transaction transaction, xui_doc_node_id node, uint64_t start, uint64_t end, const char* utf8, uint64_t bytes);
XUI_API int xuiDocumentTxnSetAttributes(xui_document_transaction transaction, xui_doc_node_id node, const xui_doc_attributes_t* attributes);
XUI_API int xuiDocumentTxnSetResource(xui_document_transaction transaction, xui_doc_node_id node, const char* resource, const char* info, const char* title);
XUI_API int xuiDocumentTxnSetMarks(xui_document_transaction transaction, const xui_doc_range_t* range, uint32_t set, uint32_t clear);
/* common: marks present throughout the selection; mixed: marks present on only
 * part of it. Structural gaps are supported, including a root Select All range. */
XUI_API int xuiDocumentSnapshotQueryMarks(xui_document_snapshot snapshot, const xui_doc_range_t* range, uint32_t* common, uint32_t* mixed);
/* Case-sensitive UTF-8 literal search over semantic text or Markdown source.
 * Matches may cross style runs and paragraph boundaries. A NULL scope searches
 * the complete document. total reports all non-overlapping matches; the caller
 * can first query with matches=NULL/capacity=0, then allocate its result array. */
XUI_API int xuiDocumentSnapshotFind(xui_document_snapshot snapshot, uint32_t domain,
    const char* pattern, uint64_t pattern_bytes, const xui_doc_range_t* scope,
    xui_doc_range_t* matches, uint64_t capacity, uint64_t* total);
/* Replace every match in one atomic root transaction. An unsupported structural
 * boundary poisons the entire transaction; no partial replacement is published. */
XUI_API int xuiDocumentTxnReplaceAll(xui_document_transaction transaction,
    const char* pattern, uint64_t pattern_bytes, const char* replacement, uint64_t replacement_bytes,
    const xui_doc_range_t* scope, uint64_t* replaced);
/* Structural text commands. The returned caret addresses the transaction draft;
 * set its revision to the committed revision after a successful commit. Newlines
 * split paragraphs; code/math/HTML literals retain their newlines. A range may
 * span sibling paragraphs, but cannot accidentally join independent cell scopes. */
XUI_API int xuiDocumentTxnReplaceRange(xui_document_transaction transaction, const xui_doc_range_t* range,
    const char* utf8, uint64_t bytes, xui_doc_position_t* caret);
XUI_API int xuiDocumentTxnSplitBlock(xui_document_transaction transaction, const xui_doc_position_t* at, xui_doc_position_t* caret);
XUI_API int xuiDocumentTxnJoinBlocks(xui_document_transaction transaction, xui_doc_node_id left, xui_doc_node_id right, xui_doc_position_t* caret);
XUI_API int xuiDocumentSnapshotCopyRange(xui_document_snapshot snapshot, const xui_doc_range_t* range, char** out, uint64_t* bytes);
/* Table commands preserve a rectangular grid including row/column spans.
 * Row/column indices are zero based. Deleting the last row/column deletes the
 * table. Splitting keeps merged content in the upper-left cell; undo restores
 * the exact pre-merge distribution. */
XUI_API int xuiDocumentTxnInsertTable(xui_document_transaction transaction, xui_doc_node_id parent, uint64_t index,
    uint32_t rows, uint32_t columns, int header, xui_doc_node_id* out);
XUI_API int xuiDocumentTxnInsertTableRow(xui_document_transaction transaction, xui_doc_node_id table, uint32_t row);
XUI_API int xuiDocumentTxnDeleteTableRow(xui_document_transaction transaction, xui_doc_node_id table, uint32_t row);
XUI_API int xuiDocumentTxnInsertTableColumn(xui_document_transaction transaction, xui_doc_node_id table, uint32_t column);
XUI_API int xuiDocumentTxnDeleteTableColumn(xui_document_transaction transaction, xui_doc_node_id table, uint32_t column);
XUI_API int xuiDocumentTxnMergeCells(xui_document_transaction transaction, xui_doc_node_id table,
    uint32_t row, uint32_t column, uint32_t rows, uint32_t columns, xui_doc_node_id* out);
XUI_API int xuiDocumentTxnSplitCell(xui_document_transaction transaction, xui_doc_node_id cell);
XUI_API int xuiDocumentTxnReplaceSource(xui_document_transaction transaction, uint64_t start, uint64_t end, const char* utf8, uint64_t bytes);
XUI_API int xuiDocumentTxnCommit(xui_document_transaction transaction, xui_document_change_set* out);
XUI_API void xuiDocumentTxnAbort(xui_document_transaction transaction);
XUI_API void xuiDocumentTxnRelease(xui_document_transaction transaction);
XUI_API int xuiDocumentUndo(xui_document document, xui_document_change_set* out);
XUI_API int xuiDocumentRedo(xui_document document, xui_document_change_set* out);
XUI_API int xuiDocumentCanUndo(xui_document document);
XUI_API int xuiDocumentCanRedo(xui_document document);
XUI_API void xuiDocumentChangeSetRetain(xui_document_change_set change);
XUI_API void xuiDocumentChangeSetRelease(xui_document_change_set change);
XUI_API int xuiDocumentChangeSetGetInfo(xui_document_change_set change, xui_doc_change_info_t* out);
XUI_API int xuiDocumentMapPosition(xui_document_change_set change, const xui_doc_position_t* before, xui_doc_position_t* after, int* mapping);
XUI_API int xuiDocumentSourceToPosition(xui_document_snapshot snapshot, uint64_t source_offset, xui_doc_position_t* out, int* mapping);
XUI_API int xuiDocumentPositionToSource(xui_document_snapshot snapshot, const xui_doc_position_t* position, uint64_t* out, int* mapping);
XUI_API int xuiDocumentSnapshotComparePositions(xui_document_snapshot snapshot, const xui_doc_position_t* a, const xui_doc_position_t* b, int* order);
XUI_API int xuiDocumentSnapshotCopyPlainText(xui_document_snapshot snapshot, char** out, uint64_t* bytes);

/* Loading is transactional; failed input leaves the document unchanged. */
XUI_API int xuiDocumentLoadMarkdown(xui_document document, const char* utf8, uint64_t bytes);
XUI_API int xuiDocumentSerialize(xui_document_snapshot snapshot, char** out, uint64_t* bytes);
XUI_API int xuiDocumentDeserialize(const xui_doc_desc_t* desc, const char* data, uint64_t bytes, xui_document* out);
XUI_API int xuiDocumentExportHtml(xui_document_snapshot snapshot, char** out, uint64_t* bytes);
/* Buffers returned by serialize/export are released with this function. */
XUI_API void xuiDocumentFreeBuffer(void* buffer);

#ifdef __cplusplus
}
#endif
#endif
