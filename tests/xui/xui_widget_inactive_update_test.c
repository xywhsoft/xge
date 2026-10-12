#include "xui.h"
#include <stdio.h>
#include <string.h>

static int updates, inactive_updates;

static int on_update(xui_widget widget, float delta, void* user)
{
    (void)widget; (void)user;
    updates++;
    if (delta == 0.0f) inactive_updates++;
    return XUI_OK;
}

#define CHECK(condition, step) do { \
    if (!(condition)) { fprintf(stderr, "inactive update: %s failed\n", step); return 2; } \
} while (0)

int main(void)
{
    xui_context context = NULL;
    xui_widget root = NULL, other_root = NULL, parent = NULL, child = NULL;
    xui_widget_type type = NULL;
    xui_widget_type_desc_t desc;
    CHECK(xuiCreate(&context) == XUI_OK, "context");
    memset(&desc, 0, sizeof(desc));
    desc.iSize = sizeof(desc);
    desc.sName = "test.inactive-update";
    desc.pParent = xuiWidgetGetBaseType();
    desc.iFlags = XUI_WIDGET_TYPE_UPDATE_ON_INACTIVE;
    desc.onUpdate = on_update;
    CHECK(xuiWidgetRegisterType(context, &type, &desc) == XUI_OK, "register");
    CHECK(xuiWidgetCreate(context, &root) == XUI_OK, "root");
    CHECK(xuiWidgetCreate(context, &other_root) == XUI_OK, "other root");
    CHECK(xuiSetRootWidget(context, root) == XUI_OK, "set root");
    CHECK(xuiWidgetCreate(context, &parent) == XUI_OK, "parent");
    CHECK(xuiWidgetCreateTyped(context, type, &child, NULL) == XUI_OK, "child");
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK, "attach parent");
    CHECK(xuiWidgetAddChild(parent, child) == XUI_OK, "attach child");
    CHECK(xuiWidgetIsAttachedToContext(child), "attached");
    CHECK(xuiUpdate(context, .016f) == XUI_OK && updates == 1 && !inactive_updates,
        "visible update");
    CHECK(xuiWidgetSetVisible(parent, 0) == XUI_OK &&
        updates == 2 && inactive_updates == 1, "hide sync");
    CHECK(xuiUpdate(context, .016f) == XUI_OK &&
        updates == 2 && inactive_updates == 1, "hidden skips frames");
    CHECK(xuiWidgetSetVisible(parent, 1) == XUI_OK, "show");
    CHECK(xuiUpdate(context, .016f) == XUI_OK && updates == 3, "shown update");
    CHECK(xuiWidgetRemoveFromParent(parent) == XUI_OK &&
        !xuiWidgetIsAttachedToContext(child) &&
        updates == 4 && inactive_updates == 2, "detach sync");
    CHECK(xuiUpdate(context, .016f) == XUI_OK && updates == 4,
        "detached skips frames");
    CHECK(xuiWidgetAddChild(root, parent) == XUI_OK, "reattach");
    CHECK(xuiUpdate(context, .016f) == XUI_OK && updates == 5, "reattached update");
    CHECK(xuiSetRootWidget(context, other_root) == XUI_OK &&
        !xuiWidgetIsAttachedToContext(child) &&
        updates == 6 && inactive_updates == 3, "root replacement sync");
    CHECK(xuiSetRootWidget(context, root) == XUI_OK, "restore root");
    CHECK(xuiUpdate(context, .016f) == XUI_OK && updates == 7,
        "restored root update");
    xuiWidgetDestroy(other_root);
    xuiDestroy(context);
    puts("XUI inactive widget update: hide, detach, root replacement and no hidden frame traversal passed");
    return 0;
}
