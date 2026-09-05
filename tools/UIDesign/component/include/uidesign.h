#ifndef UIDESIGN_H
#define UIDESIGN_H
/* UIDesign 0.1: UTF-8 C API. All mutation and callbacks run on the host UI
 * thread. Returned views are borrowed until the next mutation. No XUI types
 * or editor-private structs are part of the document API. */
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define UID_API_VERSION 1u
#define UID_ID_CAPACITY 64
#define UID_MAX_NODES 256
#define UID_MAX_PROPERTIES 48
#define UID_MAX_EVENTS 8
#define UID_MAX_TEXT 4096
#define UID_MAX_DOCUMENT_BYTES (8u * 1024u * 1024u)
typedef uint64_t uid_id;
typedef struct uid_context uid_context;
typedef struct uid_document uid_document;
typedef struct uid_snapshot uid_snapshot;
enum {
    UID_OK = 0,
    UID_ERROR = -1,
    UID_INVALID = -2,
    UID_NOT_FOUND = -3,
    UID_LIMIT = -4,
    UID_BUSY = -5,
    UID_UNSUPPORTED = -6,
    UID_CONFLICT = -7
};
enum { UID_STRING = 1, UID_NUMBER, UID_BOOL, UID_COLOR };
enum { UID_READONLY = 1, UID_HIDDEN = 2, UID_MULTIEDIT = 4, UID_CUSTOM_EDITOR = 8 };
enum { UID_CONTAINER = 1, UID_ROOT = 2 };
typedef struct uid_value {
    int kind;
    double number;
    const char *text;
} uid_value;
typedef struct uid_property_desc {
    const char *id, *name, *category, *description;
    uid_value initial;
    uint32_t flags;
    double minimum, maximum;
    int (*validate)(const uid_document *, uid_id, const uid_value *, void *);
    void *user;
} uid_property_desc;
typedef struct uid_event_desc {
    const char *id, *name, *description;
    int argument_kind; /* 0 = no value, otherwise one typed value in 0.1. */
} uid_event_desc;
typedef struct uid_control_desc {
    size_t size;
    uint32_t version;
    const char *id, *name, *name_prefix, *default_event;
    uint32_t flags;
    const uid_property_desc *properties;
    int property_count;
    const uid_event_desc *events;
    int event_count;
} uid_control_desc;
typedef struct uid_node_info {
    uid_id id, parent;
    const char *type, *name, *slot;
    int property_count, binding_count;
} uid_node_info;
typedef struct uid_binding {
    char event_id[UID_ID_CAPACITY], handler[UID_ID_CAPACITY];
} uid_binding;
typedef struct uid_change {
    uid_id node;
    const char *property;
    uid_value value;
} uid_change;
typedef struct uid_notice {
    uid_document *document;
    uint64_t document_id, revision;
    const char *command;
    int dirty, can_undo, can_redo;
} uid_notice;
typedef void (*uid_notify_proc)(const uid_notice *, void *);

uid_value uidString(const char *text);
uid_value uidNumber(double number);
uid_value uidBool(int value);
uid_value uidColor(uint32_t rgba);
int uidContextCreate(uid_context **out);
int uidContextDestroy(uid_context *context); /* BUSY while docs/snapshots/views live. */
const char *uidLastError(const uid_context *context);
int uidRegisterControl(uid_context *, const uid_control_desc *); /* copies metadata */
int uidRegisterBuiltins(uid_context *);
int uidControlCount(const uid_context *);
const uid_control_desc *uidControlAt(const uid_context *, int index);
const uid_control_desc *uidFindControl(const uid_context *, const char *type);
const uid_property_desc *uidFindProperty(const uid_context *, const char *type, const char *id);
const uid_event_desc *uidFindEvent(const uid_context *, const char *type, const char *id);
int uidSubscribe(uid_context *, uid_notify_proc, void *user, uint64_t *token);
void uidUnsubscribe(uid_context *, uint64_t token);

int uidDocumentCreate(uid_context *, const char *name, uid_document **out);
int uidDocumentLoad(uid_context *, const char *json, size_t size, uid_document **out);
int uidDocumentDestroy(uid_document *);
uint64_t uidDocumentId(const uid_document *);
uint64_t uidDocumentRevision(const uid_document *);
int uidDocumentDirty(const uid_document *);
uid_id uidDocumentRoot(const uid_document *);
int uidNodeCount(const uid_document *);
int uidNodeAt(const uid_document *, int index, uid_node_info *out);
int uidNodeGet(const uid_document *, uid_id, uid_node_info *out);
uid_id uidNodeFindName(const uid_document *, const char *name);
int uidGetProperty(const uid_document *, uid_id, const char *id, uid_value *out);
/* Enumerates persisted properties, including unknown extension data. Name is
 * node identity and is returned by uidNodeGet, not duplicated in this list. */
int uidGetPropertyAt(const uid_document *, uid_id, int index, const char **id, uid_value *out);
int uidGetBinding(const uid_document *, uid_id, const char *event_id, uid_binding *out);
int uidGetBindingAt(const uid_document *, uid_id, int index, uid_binding *out);
int uidValidateProperty(const uid_document *, uid_id, const char *, const uid_value *);
int uidBegin(uid_document *, const char *command);
int uidCommit(uid_document *);
int uidCancel(uid_document *);
int uidAddNode(uid_document *, const char *type, uid_id parent, const char *slot, uid_id *out);
int uidDeleteNode(uid_document *, uid_id);
int uidRenameNode(uid_document *, uid_id, const char *name);
int uidReparentNode(uid_document *, uid_id, uid_id parent, const char *slot);
int uidSetProperty(uid_document *, uid_id, const char *, uid_value);
int uidSetProperties(uid_document *, const uid_change *, int count);
int uidResetProperty(uid_document *, uid_id, const char *);
int uidSetBinding(uid_document *, uid_id, const char *event_id, const char *handler);
int uidCanUndo(const uid_document *);
int uidCanRedo(const uid_document *);
int uidUndo(uid_document *);
int uidRedo(uid_document *);
int uidDocumentSerialize(const uid_document *, char **out, size_t *size);
void uidFree(void *);
int uidSnapshotCreate(const uid_document *, uid_snapshot **out);
void uidSnapshotDestroy(uid_snapshot *);
const uid_document *uidSnapshotDocument(const uid_snapshot *);
uint64_t uidSnapshotRevision(const uid_snapshot *);
int uidDocumentMarkSaved(uid_document *, const uid_snapshot *); /* exact saved content */
int uidAcceptResult(const uid_document *, uint64_t document_id, uint64_t revision);
int uidIdentifierValid(const char *);

/* In-memory C backend. Does not write files or invoke a compiler. */
typedef struct uid_c_artifacts {
    char *header, *source;
    uint64_t revision;
} uid_c_artifacts;
int uidGenerateC(const uid_snapshot *, const char *symbol, uid_c_artifacts *out);
void uidCArtifactsFree(uid_c_artifacts *);
#ifdef __cplusplus
}
#endif
#endif
