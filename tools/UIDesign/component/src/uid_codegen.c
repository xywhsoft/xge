#include "uid_xui_internal.h"

/* This backend is deliberately optional. A host may consume the same immutable
 * snapshot with its own language/backend. Never emit a partial successful form. */
static int c_identifier(const char *s) {
    static const char *const words[] = {
        "auto",           "break",        "case",     "char",     "const",      "continue",
        "default",        "do",           "double",   "else",     "enum",       "extern",
        "float",          "for",          "goto",     "if",       "inline",     "int",
        "long",           "register",     "restrict", "return",   "short",      "signed",
        "sizeof",         "static",       "struct",   "switch",   "typedef",    "union",
        "unsigned",       "void",         "volatile", "while",    "_Alignas",   "_Alignof",
        "_Atomic",        "_Bool",        "_Complex", "_Generic", "_Imaginary", "_Noreturn",
        "_Static_assert", "_Thread_local"};
    size_t i;
    if (!uidIdentifierValid(s) || s[0] == '_')
        return 0;
    for (i = 0; i < sizeof(words) / sizeof(words[0]); ++i)
        if (!strcmp(s, words[i]))
            return 0;
    return 1;
}

static void c_string(uid_builder *b, const char *s) {
    const unsigned char *p = (const unsigned char *)(s ? s : "");
    uid_append(b, "\"");
    for (; *p; ++p) {
        if (*p == '"' || *p == '\\')
            uid_append(b, "\\%c", *p);
        else if (*p < 32 || *p == 127 || *p == '?')
            uid_append(b, "\\%03o", *p);
        else
            uid_append(b, "%c", *p);
    }
    uid_append(b, "\"");
}

static int runtime_kind(const char *type) {
    static const char *const types[] = {"xui.form",  "xui.panel",  "xui.label",
                                        "xui.input", "xui.button", "xui.checkbox"};
    int i;
    for (i = 0; type && i < 6; ++i)
        if (!strcmp(type, types[i]))
            return i;
    return -1;
}

static int property_supported(const char *id, int kind) {
    static const char *const common[] = {"x",        "y",         "width",   "height", "text",
                                         "fontSize", "textColor", "visible", "enabled"};
    int i;
    for (i = 0; i < 9; ++i)
        if (!strcmp(id, common[i]))
            return 1;
    return kind == 5 && !strcmp(id, "checked");
}

void uidCArtifactsFree(uid_c_artifacts *a) {
    if (a) {
        free(a->header);
        free(a->source);
        memset(a, 0, sizeof(*a));
    }
}

int uidGenerateC(const uid_snapshot *snapshot, const char *symbol, uid_c_artifacts *out) {
    const uid_document *d;
    uid_context *c;
    uid_builder h = {0}, s = {0};
    int kinds[UID_MAX_NODES], order[UID_MAX_NODES], done[UID_MAX_NODES] = {0};
    int i, j, pass, count = 0;
    if (!snapshot || !out)
        return UID_INVALID;
    memset(out, 0, sizeof(*out));
    d = &snapshot->data;
    c = d->context;
    if (!c_identifier(symbol) || !d->count)
        return uid_fail(c, UID_INVALID, "C export needs a non-reserved C identifier and a form");
    for (i = 0; i < d->count; ++i) {
        const uid_node_i *n = &d->nodes[i];
        const uid_xui_adapter *adapter = uid_adapter(c, n->type);
        kinds[i] = runtime_kind(adapter ? adapter->c_runtime_type : n->type);
        if (kinds[i] < 0 || !uidFindControl(c, n->type))
            return uid_fail(c, UID_UNSUPPORTED,
                            "C export: a control has no supported runtime contract");
        if (!c_identifier(n->name))
            return uid_fail(c, UID_INVALID, "C export: a control name is a reserved C identifier");
        if (strcmp(n->slot, "content") || (!i && kinds[i] != 0))
            return uid_fail(c, UID_UNSUPPORTED,
                            "C export only supports a form root and content slots");
        for (j = 0; j < n->property_count; ++j) {
            const uid_prop_i *p = &n->properties[j];
            int expected = !strcmp(p->id, "text")        ? UID_STRING
                           : !strcmp(p->id, "textColor") ? UID_COLOR
                           : (!strcmp(p->id, "visible") || !strcmp(p->id, "enabled") ||
                              !strcmp(p->id, "checked"))
                               ? UID_BOOL
                               : UID_NUMBER;
            if (!property_supported(p->id, kinds[i]) || p->value.kind != expected)
                return uid_fail(c, UID_UNSUPPORTED,
                                "C export: unsupported property (preserved in document)");
        }
        for (j = 0; j < n->binding_count; ++j) {
            const char *event = kinds[i] == 4   ? "click"
                                : kinds[i] == 3 ? "textChanged"
                                : kinds[i] == 5 ? "checkedChanged"
                                                : "";
            const uid_event_desc *schema = uidFindEvent(c, n->type, event);
            if (strcmp(event, n->bindings[j].event_id) || !c_identifier(n->bindings[j].handler) ||
                !schema ||
                schema->argument_kind != (kinds[i] == 3   ? UID_STRING
                                          : kinds[i] == 5 ? UID_BOOL
                                                          : 0))
                return uid_fail(c, UID_UNSUPPORTED,
                                "C export: unsupported event or reserved handler name");
        }
    }
    /* Imported documents may list children before parents. */
    for (pass = 0; pass < d->count && count < d->count; ++pass) {
        for (i = 0; i < d->count; ++i)
            if (!done[i]) {
                const uid_node_i *n = &d->nodes[i];
                if (n->parent) {
                    for (j = 0; j < d->count; ++j)
                        if (d->nodes[j].id == n->parent)
                            break;
                    if (j == d->count || !done[j])
                        continue;
                    if (kinds[j] > 1)
                        return uid_fail(c, UID_UNSUPPORTED, "C export: parent is not a container");
                }
                done[i] = 1;
                order[count++] = i;
            }
    }
    if (count != d->count)
        return UID_INVALID;

    uid_append(&h,
               "/* Generated by UIDesign 0.1. Do not edit this file. */\n"
               "#ifndef %s_GENERATED_H\n#define %s_GENERATED_H\n"
               "#include \"uidesign_xui.h\"\n"
               "#if defined(_WIN32)\n#define UID_FORM_EXPORT __declspec(dllexport)\n"
               "#else\n#define UID_FORM_EXPORT\n#endif\n"
               "typedef struct %s_controls {\n",
               symbol, symbol, symbol);
    for (i = 0; i < d->count; ++i)
        uid_append(&h, "    xui_widget %s;\n", d->nodes[i].name);
    uid_append(&h,
               "} %s_controls;\n"
               "typedef struct %s {\n    %s_controls controls;\n"
               "    uid_runtime_host host;\n} %s;\n"
               "UID_FORM_EXPORT %s *%s_create(xui_context, xui_widget, xui_font, const "
               "uid_runtime_host *);\n"
               "UID_FORM_EXPORT void %s_destroy(%s *);\n"
               "UID_FORM_EXPORT xui_widget %s_find(%s *, const char *name);\n#endif\n",
               symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol);
    uid_append(&s,
               "/* Generated UI only. Business code lives in a separate host-owned file. */\n"
               "#include \"%s.generated.h\"\n#include <stdlib.h>\n#include <string.h>\n"
               "static int form_box(xui_widget w,xui_draw_context draw,uint32_t state,void *u) {\n"
               "    xui_proxy_t p; xui_rect_t r=xuiWidgetGetContentRect(w); (void)state;\n"
               "    if(xuiGetProxy(xuiWidgetGetContext(w),&p)) return XUI_ERROR;\n"
               "    p.drawRectFill(&p,draw,r,(uint32_t)(uintptr_t)u);\n"
               "    return p.drawRectStroke(&p,draw,r,1,0xB9C7D8FFu);\n}\n",
               symbol);
    for (i = 0; i < d->count; ++i) {
        const uid_node_i *n = &d->nodes[i];
        for (j = 0; j < n->binding_count; ++j) {
            const uid_binding *binding = &n->bindings[j];
            uid_append(&s,
                       "static void event_%d(xui_widget w,%svoid *user) {\n"
                       "    %s *v=(%s*)user; uid_runtime_event e={0};\n"
                       "    e.node=%llu; e.name=",
                       i,
                       kinds[i] == 3   ? "const char *text,"
                       : kinds[i] == 5 ? "int checked,"
                                       : "",
                       symbol, symbol, (unsigned long long)n->id);
            c_string(&s, n->name);
            uid_append(&s, "; e.event_id=");
            c_string(&s, binding->event_id);
            uid_append(&s, "; e.handler=");
            c_string(&s, binding->handler);
            uid_append(&s, "; e.sender=w;\n");
            if (kinds[i] == 3)
                uid_append(&s, "    e.value.kind=UID_STRING; e.value.text=text;\n");
            if (kinds[i] == 5)
                uid_append(&s, "    e.value.kind=UID_BOOL; e.value.number=checked;\n");
            uid_append(&s, "    if(v->host.dispatch) v->host.dispatch(&e,v->host.user);\n}\n");
        }
    }
    uid_append(&s,
               "%s *%s_create(xui_context ctx,xui_widget parent,xui_font fallback,const "
               "uid_runtime_host *host) {\n"
               "    %s *v; xui_widget w=NULL; xui_font font; int r;\n"
               "    if(!ctx||!parent||(host&&host->size!=sizeof(*host))) return NULL;\n"
               "    v=(%s*)calloc(1,sizeof(*v)); if(!v) return NULL; if(host) v->host=*host;\n",
               symbol, symbol, symbol, symbol);
    for (pass = 0; pass < count; ++pass) {
        const uid_node_i *n;
        const char *parent = NULL;
        i = order[pass];
        n = &d->nodes[i];
        if (n->parent)
            parent = uid_node(d, n->parent)->name;
        uid_append(&s,
                   "    font=host&&host->font?host->font(%.6ff,host->user):fallback; if(!font) "
                   "font=fallback;\n    {\n",
                   uid_num(d, n->id, "fontSize", 18));
        if (kinds[i] <= 1)
            uid_append(&s,
                       "        xui_cache_policy_t p={0}; r=xuiWidgetCreate(ctx,&w);\n"
                       "        if(!r) { "
                       "p.iSize=sizeof(p);p.iPolicy=XUI_CACHE_POLICY_SELF;p.iFlags=XUI_CACHE_CLEAR_"
                       "ON_UPDATE;\n"
                       "            r=xuiWidgetSetCachePolicy(w,&p); "
                       "xuiWidgetSetLayoutType(w,XUI_LAYOUT_MANUAL);\n"
                       "            "
                       "xuiWidgetSetCacheRenderCallback(w,form_box,(void*)(uintptr_t)0x%08Xu); }\n",
                       kinds[i] ? 0xF3F6FAFFu : 0xFFFFFFFFu);
        else {
            const char *desc = kinds[i] == 2   ? "label"
                               : kinds[i] == 3 ? "input"
                               : kinds[i] == 4 ? "button"
                                               : "checkbox";
            const char *create = kinds[i] == 2   ? "Label"
                                 : kinds[i] == 3 ? "Input"
                                 : kinds[i] == 4 ? "Button"
                                                 : "CheckBox";
            uid_append(&s,
                       "        xui_%s_desc_t a={0};a.iSize=sizeof(a);a.pFont=font;a.sText=", desc);
            c_string(&s, uid_text(d, n->id, "text", ""));
            uid_append(&s, ";a.iTextColor=0x%08Xu;\n",
                       (uint32_t)uid_num(d, n->id, "textColor", 0x233246FFu));
            if (kinds[i] == 2)
                uid_append(
                    &s, "        "
                        "a.iTextFlags=XUI_TEXT_ALIGN_LEFT|XUI_TEXT_ALIGN_MIDDLE|XUI_TEXT_CLIP;\n");
            if (kinds[i] == 3)
                uid_append(&s, "        a.iBackgroundColor=0xFFFFFFFFu;\n");
            if (kinds[i] == 5)
                uid_append(&s, "        a.bChecked=%d;\n", (int)uid_num(d, n->id, "checked", 0));
            uid_append(&s, "        r=xui%sCreate(ctx,&w,&a);\n", create);
        }
        uid_append(&s,
                   "    }\n    if(r) { if(w) xuiWidgetDestroy(w); goto fail; }\n"
                   "    r=xuiWidgetAddChild(%s%s,w); if(r) { xuiWidgetDestroy(w); goto fail; }\n"
                   "    v->controls.%s=w;\n"
                   "    xuiWidgetSetRect(w,(xui_rect_t){%d,%d,%d,%d});\n"
                   "    xuiWidgetSetVisible(w,%d);xuiWidgetSetEnabled(w,%d);\n",
                   parent ? "v->controls." : "", parent ? parent : "parent", n->name,
                   (int)lround(uid_num(d, n->id, "x", 0)), (int)lround(uid_num(d, n->id, "y", 0)),
                   (int)lround(uid_num(d, n->id, "width", 180)),
                   (int)lround(uid_num(d, n->id, "height", 36)),
                   (int)uid_num(d, n->id, "visible", 1), (int)uid_num(d, n->id, "enabled", 1));
        if (n->binding_count)
            uid_append(&s, "    xui%s(w,event_%d,v);\n",
                       kinds[i] == 3   ? "InputSetChange"
                       : kinds[i] == 5 ? "CheckBoxSetChange"
                                       : "ButtonSetClick",
                       i);
        uid_append(&s, "    w=NULL;\n");
    }
    uid_append(&s,
               "    return v;\nfail: %s_destroy(v); return NULL;\n}\n"
               "void %s_destroy(%s *v) { if(v) { if(v->controls.%s) "
               "xuiWidgetDestroy(v->controls.%s); free(v); } }\n"
               "xui_widget %s_find(%s *v,const char *name) {\n    if(!v||!name) return NULL;\n",
               symbol, symbol, symbol, d->nodes[0].name, d->nodes[0].name, symbol, symbol);
    for (i = 0; i < d->count; ++i)
        uid_append(&s, "    if(!strcmp(name,\"%s\")) return v->controls.%s;\n", d->nodes[i].name,
                   d->nodes[i].name);
    uid_append(&s, "    return NULL;\n}\n");
    if (h.error || s.error) {
        free(h.text);
        free(s.text);
        return UID_ERROR;
    }
    out->header = h.text;
    out->source = s.text;
    out->revision = snapshot->revision;
    return UID_OK;
}
