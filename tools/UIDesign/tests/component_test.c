#include "uidesign_xui.h"
#include "test_xui/xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int checks;
#define T(e)                                                                                       \
    do {                                                                                           \
        ++checks;                                                                                  \
        if (!(e)) {                                                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #e);                           \
            return 0;                                                                              \
        }                                                                                          \
    } while (0)
static double number(const uid_document *d, uid_id id, const char *p) {
    uid_value v = {0};
    return uidGetProperty(d, id, p, &v) ? NAN : v.number;
}
typedef struct observer {
    int calls, reentrant, create_result;
    uint64_t token;
    uid_context *context;
} observer;
static void observe(const uid_notice *n, void *u) {
    observer *o = u;
    uid_document *attempt = NULL;
    ++o->calls;
    o->reentrant =
        uidSetProperty(n->document, uidDocumentRoot(n->document), "text", uidString("not allowed"));
    o->create_result = uidDocumentCreate(o->context, "NestedCreate", &attempt);
}
static int reject_seven(const uid_document *d, uid_id id, const uid_value *v, void *u) {
    int *calls = u;
    (void)d;
    (void)id;
    ++*calls;
    return v->number == 7 ? UID_INVALID : UID_OK;
}
static int core_tests(void) {
    uid_context *c = NULL;
    uid_document *d = NULL, *loaded = NULL, *other = NULL;
    uid_snapshot *s = NULL;
    uid_id root, a, b, p, old_id;
    uid_value v;
    uid_binding bind;
    uid_node_info n;
    observer o = {0};
    uint64_t rev;
    char *json = NULL, *again = NULL;
    size_t size;
    int calls = 0, i;
    const char *key;
    uid_change changes[2];
    uid_c_artifacts output = {0};
    T(!uidContextCreate(&c));
    T(!uidRegisterBuiltins(c));
    T(uidControlCount(c) == 6);
    T(uidRegisterBuiltins(c) == UID_CONFLICT);
    T(!uidDocumentCreate(c, "Form1", &d));
    root = uidDocumentRoot(d);
    T(root && !uidCanUndo(d) && !uidDocumentDirty(d));
    T(uidContextDestroy(c) == UID_BUSY);
    T(uidDeleteNode(d, root) == UID_INVALID);
    o.context = c;
    T(!uidSubscribe(c, observe, &o, &o.token));
    T(!uidBegin(d, "Add pair"));
    T(!uidAddNode(d, "xui.button", root, "content", &a));
    T(!uidAddNode(d, "xui.button", root, "content", &b));
    T(!uidCommit(d));
    T(o.calls == 1 && o.reentrant == UID_BUSY && o.create_result == UID_BUSY);
    uidUnsubscribe(c, o.token);
    T(!uidNodeGet(d, a, &n) && !strcmp(n.name, "button1"));
    T(uidRenameNode(d, b, "button1") == UID_CONFLICT);
    T(uidRenameNode(d, a, "invalid-name") == UID_INVALID);
    rev = uidDocumentRevision(d);
    T(!uidRenameNode(d, a, "btnOK"));
    T(uidDocumentRevision(d) == rev + 1);
    rev = uidDocumentRevision(d);
    T(!uidRenameNode(d, a, "btnOK") && uidDocumentRevision(d) == rev);
    T(!uidSetBinding(d, a, "click", "OnClick"));
    T(!uidRenameNode(d, a, "btnRenamed"));
    T(!uidGetBinding(d, a, "click", &bind) && !strcmp(bind.handler, "OnClick"));
    T(!uidUndo(d));
    T(uidNodeFindName(d, "btnOK") == a);
    T(!uidRedo(d));
    T(!uidSetBinding(d, a, "click", ""));
    T(uidGetBinding(d, a, "click", &bind) == UID_NOT_FOUND);
    T(!uidUndo(d));
    T(!uidGetBinding(d, a, "click", &bind));
    changes[0] = (uid_change){a, "width", uidNumber(200)};
    changes[1] = (uid_change){b, "width", uidNumber(-1)};
    T(uidSetProperties(d, changes, 2) == UID_INVALID);
    T(number(d, a, "width") == 180);
    T(!uidBegin(d, "Cancelled edit"));
    T(!uidSetProperty(d, a, "x", uidNumber(88)));
    T(uidSnapshotCreate(d, &s) == UID_BUSY);
    T(!uidCancel(d));
    T(number(d, a, "x") == 0);
    T(!uidAddNode(d, "xui.panel", root, "content", &p));
    T(!uidReparentNode(d, a, p, "content"));
    T(uidReparentNode(d, p, a, "content") == UID_INVALID);
    T(uidReparentNode(d, p, p, "content") == UID_INVALID);
    T(!uidDeleteNode(d, p));
    T(uidNodeGet(d, a, &n) == UID_NOT_FOUND);
    T(!uidUndo(d));
    T(!uidNodeGet(d, a, &n) && n.parent == p);
    T(!uidSnapshotCreate(d, &s));
    T(uidSetProperty((uid_document *)uidSnapshotDocument(s), a, "x", uidNumber(9)) == UID_INVALID);
    T(!uidDocumentMarkSaved(d, s) && !uidDocumentDirty(d));
    rev = uidDocumentRevision(d);
    T(!uidSetProperty(d, a, "text", uidString("quote \" slash \\ newline\n中文")));
    T(!uidAcceptResult(d, uidDocumentId(d), rev));
    T(!uidDocumentMarkSaved(d, s) && uidDocumentDirty(d));
    T(!uidUndo(d) && !uidDocumentDirty(d));
    T(!uidRedo(d));
    T(!uidGenerateC(s, "form", &output));
    T(strstr(output.header, "xui_widget btnRenamed;") != NULL);
    T(strstr(output.source, "event_") != NULL);
    uidCArtifactsFree(&output);
    T(uidGenerateC(s, "while", &output) == UID_INVALID);
    uidSnapshotDestroy(s);
    s = NULL;
    T(!uidDocumentSerialize(d, &json, &size));
    T(size == strlen(json));
    T(!uidDocumentLoad(c, json, size, &loaded));
    T(!uidDocumentDirty(loaded));
    T(uidDocumentId(loaded) != uidDocumentId(d));
    T(!uidDocumentSerialize(loaded, &again, NULL));
    T(!strcmp(json, again));
    free(again);
    again = NULL;
    T(!uidGetPropertyAt(loaded, a, 0, &key, &v) && *key);
    T(!uidDocumentCreate(c, "OtherForm", &other));
    T(uidNodeFindName(other, "btnRenamed") == 0 && !uidDocumentDirty(other));
    T(!uidSnapshotCreate(other, &s));
    T(uidDocumentMarkSaved(d, s) == UID_INVALID);
    uidSnapshotDestroy(s);
    s = NULL;
    T(!uidDeleteNode(d, b));
    old_id = b;
    T(!uidAddNode(d, "xui.button", root, "content", &b));
    T(b > old_id);
    T(!uidUndo(d));
    T(!uidAddNode(d, "xui.button", root, "content", &old_id));
    T(old_id > b);
    /* Metadata is copied and callbacks are invoked without allowing reentry. */
    {
        uid_property_desc prop = {
            "rating",      "Rating", "Custom", "Host-validated", {UID_NUMBER, 1, NULL},
            UID_MULTIEDIT, 0,        10,       reject_seven,     &calls};
        uid_control_desc type = {sizeof(type),
                                 UID_API_VERSION,
                                 "host.custom",
                                 "Custom",
                                 "custom",
                                 "",
                                 0,
                                 &prop,
                                 1,
                                 NULL,
                                 0};
        T(!uidRegisterControl(c, &type));
        prop.initial.number = 9;
        T(!uidAddNode(d, "host.custom", root, "content", &b));
        T(number(d, b, "rating") == 1);
        T(uidSetProperty(d, b, "rating", uidNumber(7)) == UID_INVALID && calls == 1);
        T(!uidSetProperty(d, b, "rating", uidNumber(8)) && calls == 2);
    }
    /* Unknown extensions are preserved, exported to the host and rejected by
       the optional built-in C backend rather than silently disappearing. */
    {
        uid_context *missing = NULL;
        uid_document *unknown = NULL;
        char *s1 = NULL, *s2 = NULL;
        T(!uidDocumentSerialize(d, &s1, NULL));
        T(!uidContextCreate(&missing));
        T(!uidRegisterBuiltins(missing));
        T(!uidDocumentLoad(missing, s1, strlen(s1), &unknown));
        T(!uidGetProperty(unknown, b, "rating", &v) && v.number == 8);
        T(!uidDocumentSerialize(unknown, &s2, NULL) && !strcmp(s1, s2));
        T(!uidSnapshotCreate(unknown, &s));
        T(uidGenerateC(s, "form", &output) == UID_UNSUPPORTED);
        uidSnapshotDestroy(s);
        s = NULL;
        free(s1);
        free(s2);
        T(!uidDocumentDestroy(unknown));
        T(!uidContextDestroy(missing));
    }
    /* Truncation / corruption never publish a partially loaded document. */
    for (i = 0; i < (int)size; i += (int)(size / 31 + 1)) {
        uid_document *bad = NULL;
        T(uidDocumentLoad(c, json, (size_t)i, &bad) != UID_OK && bad == NULL);
    }
    {
        const char *empty = "{\"format\":\"xui.uidesign.component\",\"version\":1,\"nodes\":[]}";
        uid_document *bad = NULL;
        T(uidDocumentLoad(c, empty, strlen(empty), &bad) == UID_INVALID && !bad);
    }
    {
        const char *legacy = "{\"format\":\"xui.uidesign\",\"version\":1}";
        uid_document *bad = NULL;
        T(uidDocumentLoad(c, legacy, strlen(legacy), &bad) == UID_UNSUPPORTED);
    }
    {
        uid_document *bad = NULL;
        char *corrupt = malloc(size + 16);
        T(corrupt != NULL);
        memcpy(corrupt, json, size);
        memcpy(corrupt + size, "garbage", 8);
        T(uidDocumentLoad(c, corrupt, size + 7, &bad) != UID_OK && !bad);
        memcpy(corrupt, json, size + 1);
        corrupt[size / 2] = 0;
        T(uidDocumentLoad(c, corrupt, size, &bad) == UID_INVALID && !bad);
        free(corrupt);
    }
    {
        uid_document *limited = NULL;
        uid_id node = 0;
        T(!uidDocumentCreate(c, "LimitTest", &limited));
        T(!uidBegin(limited, "Bulk add"));
        for (i = 1; i < UID_MAX_NODES; ++i)
            T(!uidAddNode(limited, "xui.label", uidDocumentRoot(limited), "content", &node));
        T(!uidCommit(limited));
        T(uidAddNode(limited, "xui.label", uidDocumentRoot(limited), "content", &node) ==
          UID_LIMIT);
        T(uidNodeCount(limited) == UID_MAX_NODES);
        T(!uidUndo(limited) && uidNodeCount(limited) == 1);
        T(!uidDocumentDestroy(limited));
    }
    T(!uidDocumentDestroy(other));
    T(!uidDocumentDestroy(loaded));
    T(!uidDocumentDestroy(d));
    free(json);
    T(!uidContextDestroy(c));
    return 1;
}

static uid_emit_event_proc custom_emit;
static void *custom_emitter;
static int custom_create(xui_context c, xui_font f, const uid_document *d, uid_id id, xui_widget *w,
                         void *u) {
    xui_button_desc_t p = {0};
    (void)d;
    (void)id;
    (void)u;
    p.iSize = sizeof(p);
    p.pFont = f;
    p.sText = "Extension";
    return xuiButtonCreate(c, w, &p);
}
static int custom_apply(xui_widget w, const uid_document *d, uid_id id, float z, void *u) {
    (void)d;
    (void)id;
    (void)z;
    (void)u;
    return xuiWidgetSetRect(w, (xui_rect_t){10, 10, 120, 36});
}
static int custom_bind(xui_widget w, uid_emit_event_proc emit, void *emitter, void *u) {
    (void)w;
    (void)u;
    custom_emit = emit;
    custom_emitter = emitter;
    return UID_OK;
}
static void custom_event(const uid_runtime_event *e, void *u) {
    int *called = u;
    if (!strcmp(e->event_id, "accepted") && !strcmp(e->handler, "OnAccept") &&
        e->value.number == 42)
        ++*called;
}
typedef struct view_host {
    int selections, edits;
} view_host;
static void selection_changed(uid_editor *e, const uid_id *ids, int count, void *user) {
    view_host *h = user;
    (void)e;
    (void)ids;
    (void)count;
    ++h->selections;
}
static int edit_property(uid_document *d, const uid_id *ids, int count, const uid_property_desc *p,
                         xui_rect_t anchor, uid_value *out, void *user) {
    view_host *h = user;
    (void)d;
    (void)ids;
    (void)count;
    (void)p;
    (void)anchor;
    ++h->edits;
    *out = uidNumber(42);
    return UID_OK;
}
static int view_tests(void) {
    xui_test_proxy_state_t proxy;
    xui_context x = NULL;
    xui_widget root, left, right, grid, focus, unrelated;
    xui_font font = NULL;
    uid_context *c = NULL;
    uid_document *a = NULL, *b = NULL;
    uid_editor *ea = NULL, *eb = NULL;
    uid_editor_desc desc = {0};
    view_host services = {0};
    uid_preview *preview = NULL;
    uid_snapshot *snapshot = NULL;
    uid_id button, second, extension;
    uid_value v;
    int row, called = 0;
    uint64_t rev;
    xuiTestProxyInit(&proxy);
    T(!xuiCreate(&x));
    T(!xuiSetProxy(x, &proxy.tProxy));
    T(!proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test", 18, 0));
    T(!xuiSetDefaultFont(x, font));
    T(!xuiWidgetCreate(x, &root));
    T(!xuiSetRootWidget(x, root));
    T(!xuiWidgetCreate(x, &left));
    T(!xuiWidgetCreate(x, &right));
    T(!xuiWidgetCreate(x, &unrelated));
    T(!xuiWidgetAddChild(root, left));
    T(!xuiWidgetAddChild(root, right));
    T(!xuiWidgetAddChild(root, unrelated));
    T(!uidContextCreate(&c));
    T(!uidXuiRegisterBuiltins(c));
    T(!uidDocumentCreate(c, "First", &a));
    T(!uidDocumentCreate(c, "Second", &b));
    T(!uidAddNode(a, "xui.button", uidDocumentRoot(a), "content", &button));
    T(!uidAddNode(a, "xui.button", uidDocumentRoot(a), "content", &second));
    desc.size = sizeof(desc);
    desc.version = UID_API_VERSION;
    desc.xui = x;
    desc.font = font;
    desc.font_size = 18;
    desc.parent = left;
    desc.user = &services;
    desc.selection_changed = selection_changed;
    desc.edit_property = edit_property;
    T(!uidEditorCreate(c, a, &desc, &ea));
    desc.parent = right;
    T(!uidEditorCreate(c, b, &desc, &eb));
    T(uidDocumentDestroy(a) == UID_BUSY);
    T(uidContextDestroy(c) == UID_BUSY);
    xuiSetViewportSize(x, 2000, 900);
    xuiWidgetSetRect(root, (xui_rect_t){0, 0, 2000, 900});
    xuiWidgetSetRect(left, (xui_rect_t){0, 0, 1000, 900});
    xuiWidgetSetRect(right, (xui_rect_t){1000, 0, 1000, 900});
    xuiWidgetSetRect(uidEditorRoot(ea), (xui_rect_t){0, 0, 1000, 900});
    xuiWidgetSetRect(uidEditorRoot(eb), (xui_rect_t){0, 0, 1000, 900});
    T(!uidEditorUpdate(ea) && !uidEditorUpdate(eb));
    T(services.selections == 2);
    {
        uid_id parent;
        T(!uidAddNode(a, "xui.panel", uidDocumentRoot(a), "content", &parent));
        T(!uidReparentNode(a, button, parent, "content"));
        T(!uidEditorUpdate(ea));
        T(xuiTreeViewGetNodeCount(uidEditorTree(ea)) == 4);
        T(!uidUndo(a));
        T(!uidUndo(a));
        T(!uidEditorUpdate(ea));
    }
    T(!xuiLayout(x));
    T(!uidEditorSelect(ea, button, 0));
    T(!uidEditorUpdate(ea));
    T(!xuiLayout(x));
    rev = uidDocumentRevision(a);
    T(!uidEditorSelect(ea, second, 1));
    T(uidDocumentRevision(a) == rev);
    T(uidEditorSelection(ea, NULL, 0) == 2 && uidEditorSelection(eb, NULL, 0) == 1);
    T(!uidEditorSetSelectionProperty(ea, "width", uidNumber(240)));
    T(number(a, button, "width") == 240 && number(a, second, "width") == 240);
    T(!uidUndo(a));
    T(number(a, button, "width") == 180 && number(a, second, "width") == 180);
    T(!uidDocumentDirty(b));
    T(!uidEditorSelect(ea, button, 0));
    T(!uidEditorUpdate(ea));
    T(!xuiLayout(x));
    grid = uidEditorPropertyGrid(ea);
    row = xuiPropertyGridFindProperty(grid, "name");
    T(row >= 0);
    T(xuiPropertyGridBeginEdit(grid, row));
    focus = xuiGetFocusWidget(x);
    T(focus);
    T(!xuiInputSetText(focus, "btnEdited"));
    T(!uidEditorFlushPendingEdits(ea));
    T(uidNodeFindName(a, "btnEdited") == button);
    T(!uidEditorUpdate(ea));
    T(!xuiLayout(x));
    row = xuiPropertyGridFindProperty(grid, "width");
    T(row >= 0);
    T(xuiPropertyGridBeginEdit(grid, row));
    focus = xuiGetFocusWidget(x);
    T(!xuiInputSetText(focus, "-100"));
    T(uidEditorFlushPendingEdits(ea) != UID_OK);
    T(number(a, button, "width") == 180);
    T(xuiPropertyGridIsEditing(grid));
    T(xuiPropertyGridEndEdit(grid, 0));
    T(!uidEditorSetZoom(ea, 0.5f));
    T(!uidEditorUpdate(ea));
    T(number(a, button, "width") == 180);
    /* Default events without a host code service fail explicitly. */
    T(uidEditorEditDefaultEvent(ea, button) == UID_UNSUPPORTED);
    /* Custom metadata + widget adapter + arbitrary typed event emitter. */
    {
        uid_event_desc event = {"accepted", "Accepted", "Custom event", UID_NUMBER};
        uid_property_desc rating = {
            "rating",          "Rating", "Extension", "Host picker", {UID_NUMBER, 1, NULL},
            UID_CUSTOM_EDITOR, 0,        100,         NULL,          NULL};
        uid_control_desc type = {sizeof(type),
                                 UID_API_VERSION,
                                 "host.extension",
                                 "Extension",
                                 "extension",
                                 "accepted",
                                 0,
                                 &rating,
                                 1,
                                 &event,
                                 1};
        uid_xui_adapter adapter = {
            sizeof(adapter), type.id, custom_create, custom_apply, NULL, NULL, NULL, custom_bind};
        T(!uidRegisterControl(c, &type));
        T(!uidXuiRegisterAdapter(c, &adapter));
        T(!uidAddNode(b, type.id, uidDocumentRoot(b), "content", &extension));
        T(!uidSetBinding(b, extension, "accepted", "OnAccept"));
    }
    T(!uidEditorSelect(eb, extension, 0));
    T(!uidEditorUpdate(eb));
    T(xuiPropertyGridFindProperty(uidEditorPropertyGrid(eb), "rating") >= 0);
    T(!uidEditorEditProperty(eb, "rating") && services.edits == 1 &&
      number(b, extension, "rating") == 42);
    T(!uidUndo(b) && number(b, extension, "rating") == 1);
    T(!uidSnapshotCreate(b, &snapshot));
    {
        uid_runtime_host host = {sizeof(host), &called, custom_event, NULL};
        T(!uidPreviewCreate(c, snapshot, x, unrelated, font, &host, &preview));
    }
    uidSnapshotDestroy(snapshot);
    snapshot = NULL;
    T(custom_emit != NULL);
    custom_emit("accepted", uidNumber(42), custom_emitter);
    T(called == 1);
    custom_emit("accepted", uidString("wrong type"), custom_emitter);
    T(called == 1);
    T(!uidGetProperty(b, uidDocumentRoot(b), "text", &v) && !strcmp(v.text, "New Form"));
    uidPreviewDestroy(preview);
    custom_emit = NULL;
    custom_emitter = NULL;
    uidEditorDestroy(ea);
    uidEditorDestroy(eb);
    T(xuiWidgetGetChildCount(left) == 0 && xuiWidgetGetChildCount(right) == 0);
    T(xuiWidgetGetChildCount(root) == 3); /* host widgets survived */
    T(!uidDocumentDestroy(a));
    T(!uidDocumentDestroy(b));
    T(!uidContextDestroy(c));
    xuiDestroy(x);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    return 1;
}
int main(void) {
    if (!core_tests() || !view_tests())
        return 1;
    printf("COMPONENT PASS: %d checks (model, registry, history, snapshots, codec, generation, "
           "multi-instance, property edits, custom events, ownership)\n",
           checks);
    return 0;
}
