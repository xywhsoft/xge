#ifndef UID_XUI_INTERNAL_H
#define UID_XUI_INTERNAL_H
#include "uid_internal.h"
#include "uidesign_xui.h"
typedef struct uid_instance {
    struct uid_preview *scene;
    uid_id id;
    xui_widget widget;
} uid_instance;
struct uid_preview {
    uid_context *context;
    const uid_document *document;
    uid_snapshot *snapshot;
    xui_context xui;
    xui_widget root;
    xui_font font;
    uid_runtime_host host;
    uid_instance items[UID_MAX_NODES];
    int count, design;
    float zoom;
};
const uid_xui_adapter *uid_adapter(const uid_context *, const char *);
int uid_scene_create(uid_context *, const uid_document *, xui_context, xui_widget, xui_font,
                     const uid_runtime_host *, int, float, uid_preview **);
void uid_scene_destroy(uid_preview *);
#endif
