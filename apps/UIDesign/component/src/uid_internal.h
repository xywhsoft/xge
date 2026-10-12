#ifndef UID_INTERNAL_H
#define UID_INTERNAL_H
#include "uidesign.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define UID_CONTROLS 64
#define UID_HISTORY 32
#define UID_HISTORY_BYTES (32u * 1024u * 1024u)
typedef struct uid_prop_i {
    char id[UID_ID_CAPACITY];
    uid_value value;
} uid_prop_i;
typedef struct uid_node_i {
    uid_id id, parent;
    char type[UID_ID_CAPACITY], name[UID_ID_CAPACITY], slot[UID_ID_CAPACITY];
    uid_prop_i *properties;
    int property_count;
    uid_binding bindings[UID_MAX_EVENTS];
    int binding_count;
} uid_node_i;
typedef struct uid_history_i {
    char *before, *after;
} uid_history_i;
struct uid_context {
    uid_control_desc controls[UID_CONTROLS];
    int control_count, documents, snapshots, views, callbacks;
    uint64_t next_doc, next_subscription;
    struct {
        uint64_t token;
        uid_notify_proc fn;
        void *user;
    } observers[32];
    void *adapters[UID_CONTROLS];
    void (*free_adapter)(void *);
    char error[256];
};
struct uid_document {
    uid_context *context;
    uint64_t id, revision, next_node;
    uid_node_i *nodes;
    int count, views, snapshot, constructing;
    char *clean, *before;
    char command[80];
    uid_history_i undo[UID_HISTORY], redo[UID_HISTORY];
    int undo_count, redo_count;
};
struct uid_snapshot {
    uid_document data;
    uint64_t source_id, revision;
    char *json;
};
typedef struct uid_builder {
    char *text;
    size_t size, capacity;
    int error;
} uid_builder;
int uid_fail(uid_context *, int, const char *);
char *uid_dup(const char *);
uid_node_i *uid_node(const uid_document *, uid_id);
int uid_value_copy(uid_value *, const uid_value *);
void uid_value_clear(uid_value *);
void uid_clear_nodes(uid_document *);
int uid_parse(uid_document *, const char *, size_t);
void uid_notify(uid_document *, const char *);
void uid_append(uid_builder *, const char *, ...);
void uid_quote(uid_builder *, const char *);
double uid_num(const uid_document *, uid_id, const char *, double);
const char *uid_text(const uid_document *, uid_id, const char *, const char *);
#endif
