/* Reference host: only public component/XUI/XGE headers are used here. */
#include "host_files.h"
#include "host_api.h"
#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

enum {
    CMD_DESIGN = 1,
    CMD_CODE,
    CMD_RUN,
    CMD_STOP,
    CMD_SAVE,
    CMD_OPEN,
    CMD_NEW,
    CMD_SAVE_AS,
    CMD_LOG
};
typedef struct shell shell;
typedef struct action {
    shell *host;
    int command;
    xui_widget widget;
} action;
struct shell {
    uid_context *designer;
    uid_document *document;
    uid_editor *editor;
    xui_context xui;
    xui_proxy_t proxy;
    xui_surface surface;
    xui_widget root, editor_parent, code, preview_parent, status, title, output;
    xui_font fonts[97];
    xui_font code_font;
    action actions[9];
    int width, height, page, pending, frame, max_frames, exercise, failed;
    char directory[HOST_PATH], sdk[HOST_PATH], compiler[HOST_PATH], work[HOST_PATH],
        project[HOST_PATH];
    char message[8192], last_log[512], *saved_code, *building_code;
    PROCESS_INFORMATION process;
    HANDLE build_log, build_job;
    int building, cancel_build, generation;
    uid_snapshot *build_snapshot;
    HMODULE module;
    void *view;
    void *(*create_view)(xui_context, xui_widget, xui_font, const uid_runtime_host *);
    void (*destroy_view)(void *);
    xui_widget (*find_view)(void *, const char *);
    shell_invoke_proc invoke;
    shell_services services;
    int calls, ensure_calls;
    uint64_t subscription;
    uid_id exercise_button, exercise_input, exercise_check;
    int exercise_phase, exercise_wait, first_calls;
};

static const char initial_code[] =
    "/* Host-owned business code. F7: code, Shift+F7: design, F5: compile/run.\n"
    " * Double-click a control in Design to create its default handler.\n"
    " * The form generator never rewrites this file. */\n"
    "#include \"host_api.h\"\n\n";

static char *copy_text(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}
static void status(const char *s, void *u) {
    shell *h = u;
    snprintf(h->message, sizeof(h->message), "%s", s ? s : "");
    if (h->status)
        xuiLabelSetText(h->status, h->message);
    printf("[host] %s\n", h->message);
    fflush(stdout);
    xgeRenderRequest();
}
static void invalidate(void *u) {
    (void)u;
    xgeRenderRequest();
}
static xui_font resolve_font(float size, void *u) {
    shell *h = u;
    int key = (int)lroundf(size);
    char path[HOST_PATH], windows[HOST_PATH];
    if (key < 10)
        key = 10;
    if (key > 96)
        key = 96;
    if (!h->fonts[key]) {
        DWORD n = GetEnvironmentVariableA("WINDIR", windows, sizeof(windows));
        if (!n || n >= sizeof(windows))
            return NULL;
        if (!host_path(path, sizeof(path), windows, "Fonts/msyh.ttc"))
            return NULL;
        if (h->proxy.fontLoadFile(&h->proxy, &h->fonts[key], path, (float)key, 0)) {
            if (!host_path(path, sizeof(path), windows, "Fonts/segoeui.ttf"))
                return NULL;
            h->proxy.fontLoadFile(&h->proxy, &h->fonts[key], path, (float)key, 0);
        }
    }
    return h->fonts[key];
}
static void page(shell *h, int which) {
    if (h->editor && uidEditorFlushPendingEdits(h->editor))
        return;
    h->page = which;
    xuiWidgetSetVisible(h->editor_parent, which == 0);
    xuiWidgetSetVisible(h->code, which == 1);
    xuiWidgetSetVisible(h->preview_parent, which == 2);
    xuiWidgetSetVisible(h->output, which == 3);
    xuiWidgetInvalidate(h->root, XUI_WIDGET_DIRTY_LAYOUT);
    xgeRenderRequest();
}
static void show_code(const char *name, void *u) {
    shell *h = u;
    const char *text = xuiCodeEditGetText(h->code), *at = strstr(text, name);
    page(h, 1);
    xuiSetFocusWidget(h->xui, h->code);
    if (at) {
        xuiCodeSelectionGotoOffset(xuiCodeEditGetSelection(h->code),
                                   xuiCodeEditGetDocument(h->code), (int)(at - text), 0);
        xuiCodeEditEnsureCaretVisible(h->code);
    }
}
static int validate_handler(const uid_event_desc *event, const char *name, void *u) {
    shell *h = u;
    char signature[100];
    (void)event;
    if (!uidIdentifierValid(name))
        return UID_INVALID;
    snprintf(signature, sizeof(signature), "void %s(", name);
    if (!strstr(xuiCodeEditGetText(h->code), signature)) {
        status("Handler not found. Use the event row's ... action to create it, or define void "
               "Name(const uid_runtime_event *, const shell_services *).",
               h);
        return UID_NOT_FOUND;
    }
    return UID_OK;
}
static int ensure_handler(uid_document *doc, uid_id id, const uid_event_desc *event,
                          const char *existing, char *out, size_t capacity, void *u) {
    shell *h = u;
    uid_node_info node;
    host_buffer b = {0};
    char name[64], signature[96];
    const char *code;
    if (uidNodeGet(doc, id, &node))
        return UID_NOT_FOUND;
    if (existing && *existing)
        snprintf(name, sizeof(name), "%s", existing);
    else {
        int n = snprintf(name, sizeof(name), "%s_%s", node.name, event->id);
        if (n < 0 || n >= (int)sizeof(name))
            return UID_LIMIT;
    }
    snprintf(signature, sizeof(signature), "void %s(", name);
    code = xuiCodeEditGetText(h->code);
    if (!strstr(code, signature)) {
        ++h->ensure_calls;
        host_append(&b,
                    "%s\nvoid %s(const uid_runtime_event *e, const shell_services *host)\n{\n"
                    "    host->log(\"Handler called\", host->user);\n",
                    code, name);
        if (!strcmp(event->id, "click"))
            host_append(&b, "    xuiButtonSetText(e->sender, \"Clicked / 已执行\");\n");
        else if (event->argument_kind == UID_STRING)
            host_append(&b, "    host->log(e->value.text, host->user);\n");
        else if (event->argument_kind == UID_BOOL)
            host_append(&b, "    host->log(e->value.number ? \"checked=true\" : \"checked=false\", "
                            "host->user);\n");
        else
            host_append(&b, "    (void)e;\n");
        host_append(&b, "}\n");
        if (b.failed || b.size > HOST_CODE_LIMIT) {
            free(b.data);
            return UID_LIMIT;
        }
        if (xuiCodeEditSetText(h->code, b.data)) {
            free(b.data);
            return UID_ERROR;
        }
        free(b.data);
    }
    if (strlen(name) >= capacity)
        return UID_LIMIT;
    snprintf(out, capacity, "%s", name);
    return UID_OK;
}
static int create_editor(shell *h) {
    uid_editor_desc d = {0};
    d.size = sizeof(d);
    d.version = UID_API_VERSION;
    d.xui = h->xui;
    d.parent = h->editor_parent;
    d.font = resolve_font(18, h);
    d.font_size = 18;
    d.user = h;
    d.invalidate = invalidate;
    d.ensure_handler = ensure_handler;
    d.show_code = show_code;
    d.validate_handler = validate_handler;
    d.status = status;
    d.resolve_font = resolve_font;
    return uidEditorCreate(h->designer, h->document, &d, &h->editor);
}
static int code_dirty(shell *h) {
    return !h->saved_code || strcmp(h->saved_code, xuiCodeEditGetText(h->code));
}
static void notice(const uid_notice *n, void *u) {
    shell *h = u;
    char text[256];
    if (n->document != h->document)
        return;
    snprintf(text, sizeof(text), "UIDesign 0.1  |  %s  |  revision %llu%s", n->command,
             (unsigned long long)n->revision, n->dirty ? "  *" : "");
    xuiLabelSetText(h->title, text);
    xgeRenderRequest();
}
static void stop_view(shell *h) {
    int was_running = h->view != NULL;
    if (h->building) {
        h->cancel_build = 1;
        if (h->build_job) {
            CloseHandle(h->build_job);
            h->build_job = NULL;
        }
        status("Owned compiler process tree cancelled; its output will be discarded.", h);
    }
    if (h->view)
        h->destroy_view(h->view);
    h->view = NULL;
    if (h->module)
        FreeLibrary(h->module);
    h->module = NULL;
    h->invoke = NULL;
    h->create_view = NULL;
    h->destroy_view = NULL;
    h->find_view = NULL;
    page(h, 0);
    if (was_running)
        status("Stopped. Runtime changes were not written into the design document.", h);
}
static int save_project(shell *h, int choose) {
    char path[HOST_PATH], *json = NULL, *new_baseline = NULL;
    size_t size;
    uid_snapshot *s = NULL;
    host_buffer b = {0};
    const char *code;
    int ok = 0;
    if (uidEditorFlushPendingEdits(h->editor))
        return 0;
    snprintf(path, sizeof(path), "%s", h->project);
    if ((choose || !*path) && !host_choose_file(1, path, sizeof(path)))
        return 0;
    code = xuiCodeEditGetText(h->code);
    if (strlen(code) > HOST_CODE_LIMIT)
        return 0;
    new_baseline = copy_text(code);
    if (!new_baseline || uidSnapshotCreate(h->document, &s) ||
        uidDocumentSerialize(uidSnapshotDocument(s), &json, &size))
        goto done;
    host_append(&b, "UIDSHELL1\n%zu\n%zu\n%s%s", size, strlen(code), json, code);
    if (b.failed || !host_atomic_write(path, b.data, b.size)) {
        status("Save failed; original project and dirty state were preserved.", h);
        goto done;
    }
    if (uidDocumentMarkSaved(h->document, s))
        goto done;
    free(h->saved_code);
    h->saved_code = new_baseline;
    new_baseline = NULL;
    snprintf(h->project, sizeof(h->project), "%s", path);
    status("Saved design + business code atomically.", h);
    ok = 1;
done:
    free(new_baseline);
    free(b.data);
    uidFree(json);
    uidSnapshotDestroy(s);
    return ok;
}
static int allow_discard(shell *h) {
    int answer;
    if (uidEditorFlushPendingEdits(h->editor))
        return 0;
    if (!uidDocumentDirty(h->document) && !code_dirty(h))
        return 1;
    answer = MessageBoxW(NULL, L"Save changes to the form and business code?", L"UIDesign 0.1",
                         MB_YESNOCANCEL | MB_ICONQUESTION);
    return answer == IDNO || (answer == IDYES && save_project(h, 0));
}
static int open_project(shell *h, const char *path) {
    char *data, *p, *end, *json = NULL, *code = NULL;
    size_t size, docsize, codesize, remaining;
    uid_document *doc = NULL, *old;
    uid_editor *old_editor, *new_editor = NULL;
    int r = UID_INVALID;
    data = host_read(path, &size, UID_MAX_DOCUMENT_BYTES + HOST_CODE_LIMIT + 128);
    if (!data)
        return UID_ERROR;
    if (size < 14 || memcmp(data, "UIDSHELL1\n", 10))
        goto done;
    p = data + 10;
    docsize = (size_t)strtoull(p, &end, 10);
    if (end == p || *end != '\n')
        goto done;
    p = end + 1;
    codesize = (size_t)strtoull(p, &end, 10);
    if (end == p || *end != '\n')
        goto done;
    p = end + 1;
    remaining = size - (size_t)(p - data);
    if (docsize > UID_MAX_DOCUMENT_BYTES || codesize > HOST_CODE_LIMIT || docsize > remaining ||
        codesize != remaining - docsize || memchr(p, 0, remaining))
        goto done;
    json = malloc(docsize + 1);
    code = malloc(codesize + 1);
    if (!json || !code) {
        r = UID_ERROR;
        goto done;
    }
    memcpy(json, p, docsize);
    json[docsize] = 0;
    memcpy(code, p + docsize, codesize);
    code[codesize] = 0;
    r = uidDocumentLoad(h->designer, json, docsize, &doc);
    if (r)
        goto done;
    /* Build a replacement view before destroying the user's current document. */
    old = h->document;
    old_editor = h->editor;
    h->document = doc;
    h->editor = NULL;
    r = create_editor(h);
    new_editor = h->editor;
    h->document = old;
    h->editor = old_editor;
    if (r) {
        uidDocumentDestroy(doc);
        doc = NULL;
        goto done;
    }
    stop_view(h);
    uidEditorDestroy(old_editor);
    uidDocumentDestroy(old);
    h->document = doc;
    h->editor = new_editor;
    doc = NULL;
    xuiCodeEditSetText(h->code, code);
    free(h->saved_code);
    h->saved_code = code;
    code = NULL;
    snprintf(h->project, sizeof(h->project), "%s", path);
    page(h, 0);
    status("Project reopened. Stable names, event bindings and business code restored.", h);
done:
    free(json);
    free(code);
    free(data);
    return r;
}
static void runtime_log(const char *s, void *u) {
    shell *h = u;
    snprintf(h->last_log, sizeof(h->last_log), "%s", s ? s : "");
    status(s, h);
}
static void runtime_dispatch(const uid_runtime_event *event, void *u) {
    shell *h = u;
    if (!h->invoke)
        return;
    ++h->calls;
    printf("[runtime] node=%llu name=%s event=%s handler=%s kind=%d\n",
           (unsigned long long)event->node, event->name, event->event_id, event->handler,
           event->value.kind);
    h->invoke(event, &h->services);
}
static int export_file(shell *h, const char *name, const char *text) {
    char path[HOST_PATH];
    return host_path(path, sizeof(path), h->work, name) && host_write(path, text, strlen(text));
}
static int start_run(shell *h) {
    uid_c_artifacts a = {0};
    host_buffer b = {0};
    uid_node_info n;
    uid_binding binding;
    const uid_document *d;
    char output[HOST_PATH], leaf[96];
    char failure[512] =
        "Build could not start. Check SDK/compiler paths, output permissions and handler bindings.";
    char handlers[UID_MAX_NODES * UID_MAX_EVENTS][64];
    int i, j, k, count = 0, r = UID_ERROR;
    if (h->building) {
        status("A build is already running.", h);
        return UID_BUSY;
    }
    if (uidEditorFlushPendingEdits(h->editor))
        return UID_INVALID;
    stop_view(h);
    h->cancel_build = 0;
    if (uidSnapshotCreate(h->document, &h->build_snapshot))
        return UID_ERROR;
    r = uidGenerateC(h->build_snapshot, "form", &a);
    if (r) {
        snprintf(failure, sizeof(failure), "%s", uidLastError(h->designer));
        goto fail;
    }
    h->building_code = copy_text(xuiCodeEditGetText(h->code));
    if (!h->building_code || strlen(h->building_code) > HOST_CODE_LIMIT) {
        r = UID_LIMIT;
        goto fail;
    }
    if (!host_path(output, sizeof(output), h->directory, "generated") || !host_mkdir(output))
        goto fail;
    snprintf(leaf, sizeof(leaf), "run-%lu-%llu-%d", GetCurrentProcessId(),
             (unsigned long long)GetTickCount64(), ++h->generation);
    if (!host_path(h->work, sizeof(h->work), output, leaf) || !host_mkdir(h->work))
        goto fail;
    host_append(&b,
                "%s\n#include <string.h>\n"
                "__declspec(dllexport) void uid_shell_invoke(const uid_runtime_event *e,const "
                "shell_services *host)\n{\n",
                h->building_code);
    d = uidSnapshotDocument(h->build_snapshot);
    for (i = 0; i < uidNodeCount(d); ++i) {
        uidNodeAt(d, i, &n);
        for (j = 0; j < n.binding_count; ++j) {
            uidGetBindingAt(d, n.id, j, &binding);
            for (k = 0; k < count; ++k)
                if (!strcmp(handlers[k], binding.handler))
                    break;
            if (k < count)
                continue;
            snprintf(handlers[count++], 64, "%s", binding.handler);
            host_append(&b, "    if(!strcmp(e->handler,\"%s\")) { %s(e,host); return; }\n",
                        binding.handler, binding.handler);
        }
    }
    host_append(&b, "    host->log(\"Handler is not in this compiled form\",host->user);\n}\n");
    if (b.failed || !export_file(h, "form.generated.h", a.header) ||
        !export_file(h, "form.generated.c", a.source) || !export_file(h, "business.c", b.data))
        goto fail;
    {
        char path[HOST_PATH], *api;
        size_t bytes;
        if (!host_path(path, sizeof(path), h->sdk, "tools/UIDesign/shell/host_api.h"))
            goto fail;
        api = host_read(path, &bytes, 16384);
        if (!api)
            goto fail;
        i = export_file(h, "host_api.h", api);
        free(api);
        if (!i)
            goto fail;
    }
    if (!host_start_compiler(h->compiler, h->sdk, h->work, &h->process, &h->build_log,
                             &h->build_job))
        goto fail;
    h->building = 1;
    status("Compiling generated UI + host-owned C handlers... (UI remains responsive)", h);
    uidCArtifactsFree(&a);
    free(b.data);
    return UID_OK;
fail:
    uidCArtifactsFree(&a);
    free(b.data);
    uidSnapshotDestroy(h->build_snapshot);
    h->build_snapshot = NULL;
    free(h->building_code);
    h->building_code = NULL;
    if (!r)
        r = UID_ERROR;
    status(failure, h);
    return r;
}
static void poll_build(shell *h) {
    DWORD result;
    char path[HOST_PATH], *log;
    uid_runtime_host runtime = {sizeof(runtime), h, runtime_dispatch, resolve_font};
    if (!h->building || WaitForSingleObject(h->process.hProcess, 0) == WAIT_TIMEOUT)
        return;
    GetExitCodeProcess(h->process.hProcess, &result);
    CloseHandle(h->process.hProcess);
    CloseHandle(h->process.hThread);
    CloseHandle(h->build_log);
    h->build_log = NULL;
    if (h->build_job) {
        CloseHandle(h->build_job);
        h->build_job = NULL;
    }
    h->building = 0;
    if (h->cancel_build) {
        status("Build result discarded.", h);
        goto done;
    }
    if (result) {
        host_path(path, sizeof(path), h->work, "build.log");
        log = host_read(path, NULL, 65536);
        xuiCodeEditSetText(h->output,
                           log ? log : "Compiler failed; see generated/run-*/build.log.");
        page(h, 3);
        status("Compilation failed. Full diagnostics are shown in Build log; business code was "
               "preserved.",
               h);
        if (log)
            printf("%s\n", log);
        free(log);
        goto done;
    }
    if (!uidAcceptResult(h->document, uidDocumentId(uidSnapshotDocument(h->build_snapshot)),
                         uidSnapshotRevision(h->build_snapshot)) ||
        strcmp(h->building_code, xuiCodeEditGetText(h->code))) {
        status(
            "Design or code changed during compilation. Stale result discarded; press Run again.",
            h);
        goto done;
    }
    host_path(path, sizeof(path), h->work, "form.dll");
    h->module = host_load_library(path);
    if (!h->module) {
        status("Compiled DLL could not be loaded.", h);
        goto done;
    }
    h->create_view = (void *(*)(xui_context, xui_widget, xui_font,
                                const uid_runtime_host *))GetProcAddress(h->module, "form_create");
    h->destroy_view = (void (*)(void *))GetProcAddress(h->module, "form_destroy");
    h->find_view = (xui_widget (*)(void *, const char *))GetProcAddress(h->module, "form_find");
    h->invoke = (shell_invoke_proc)GetProcAddress(h->module, "uid_shell_invoke");
    if (!h->create_view || !h->destroy_view || !h->find_view || !h->invoke) {
        stop_view(h);
        status("Generated module has an incompatible ABI.", h);
        goto done;
    }
    h->view = h->create_view(h->xui, h->preview_parent, resolve_font(18, h), &runtime);
    if (!h->view) {
        stop_view(h);
        status("Compiled form creation failed.", h);
        goto done;
    }
    h->services = (shell_services){sizeof(h->services), h, runtime_log, h->view, h->find_view};
    page(h, 2);
    status("RUNNING: real XUI controls, compiled C business code. Click / type / toggle to call "
           "handlers.",
           h);
done:
    uidSnapshotDestroy(h->build_snapshot);
    h->build_snapshot = NULL;
    free(h->building_code);
    h->building_code = NULL;
}
static int paint(xui_widget w, xui_draw_context draw, uint32_t state, void *u) {
    shell *h = u;
    (void)state;
    return h->proxy.drawRectFill(&h->proxy, draw, xuiWidgetGetContentRect(w), 0xE9EFF7FFu);
}
static int layout(xui_widget w, xui_rect_t rect, void *u) {
    shell *h = u;
    int i, width = rect.fW, height = rect.fH;
    xuiLayoutArrangeChild(w, h->title, (xui_rect_t){16, 8, width - 32, 30});
    for (i = 0; i < 9; ++i)
        xuiLayoutArrangeChild(w, h->actions[i].widget, (xui_rect_t){16 + i * 126, 44, 118, 34});
    xuiLayoutArrangeChild(w, h->editor_parent, (xui_rect_t){8, 88, width - 16, height - 150});
    xuiLayoutArrangeChild(w, h->code, (xui_rect_t){16, 88, width - 32, height - 150});
    xuiLayoutArrangeChild(w, h->output, (xui_rect_t){16, 88, width - 32, height - 150});
    xuiLayoutArrangeChild(w, h->preview_parent, (xui_rect_t){32, 106, width - 64, height - 168});
    xuiLayoutArrangeChild(w, h->status, (xui_rect_t){16, height - 54, width - 32, 48});
    return XUI_OK;
}
static int editor_parent_layout(xui_widget w, xui_rect_t rect, void *u) {
    shell *h = u;
    if (h->editor)
        xuiLayoutArrangeChild(w, uidEditorRoot(h->editor), (xui_rect_t){0, 0, rect.fW, rect.fH});
    return XUI_OK;
}
static void button_click(xui_widget w, void *u) {
    action *a = u;
    (void)w;
    a->host->pending = a->command;
}
static int key_event(xui_widget w, const xui_event_t *e, void *u) {
    shell *h = u;
    (void)w;
    if (e->iType != XUI_EVENT_HOTKEY)
        return XUI_OK;
    if (e->iKey == XUI_KEY_F5)
        h->pending = (e->iModifiers & XUI_MOD_SHIFT) ? CMD_STOP : CMD_RUN;
    else if (e->iKey == XUI_KEY_F7)
        h->pending = (e->iModifiers & XUI_MOD_SHIFT) ? CMD_DESIGN : CMD_CODE;
    else if ((e->iModifiers & XUI_MOD_CTRL) && e->iKey == 'S')
        h->pending = CMD_SAVE;
    else
        return XUI_OK;
    return XUI_EVENT_DISPATCH_STOP;
}
static void commands(shell *h) {
    char path[HOST_PATH] = "";
    int cmd = h->pending;
    h->pending = 0;
    if (cmd == CMD_DESIGN)
        page(h, 0);
    else if (cmd == CMD_CODE)
        page(h, 1);
    else if (cmd == CMD_LOG)
        page(h, 3);
    else if (cmd == CMD_RUN)
        start_run(h);
    else if (cmd == CMD_STOP)
        stop_view(h);
    else if (cmd == CMD_SAVE || cmd == CMD_SAVE_AS)
        save_project(h, cmd == CMD_SAVE_AS);
    else if (cmd == CMD_OPEN && allow_discard(h) && host_choose_file(0, path, sizeof(path))) {
        if (open_project(h, path))
            status("Open failed. Current project was retained.", h);
    } else if (cmd == CMD_NEW && allow_discard(h)) {
        uid_document *doc = NULL;
        if (!uidDocumentCreate(h->designer, "Form1", &doc)) {
            stop_view(h);
            uidEditorDestroy(h->editor);
            uidDocumentDestroy(h->document);
            h->document = doc;
            h->editor = NULL;
            create_editor(h);
            xuiCodeEditSetText(h->code, initial_code);
            free(h->saved_code);
            h->saved_code = copy_text(initial_code);
            *h->project = 0;
            page(h, 0);
        }
    }
}
static int quit_request(void *u) {
    shell *h = u;
    if (h->exercise || h->max_frames)
        return 1;
    if (h->building) {
        status("Wait for the current compiler process to finish before closing.", h);
        return 0;
    }
    return allow_discard(h);
}
static int setup(shell *h) {
    xui_button_desc_t button = {0};
    xui_label_desc_t label = {0};
    xui_code_edit_desc_t code = {0};
    xui_cache_policy_t cache = {0};
    char path[HOST_PATH], *config, *newline;
    int i, r;
    const char *labels[] = {"Design / 设计", "Code / 代码", "Run / F5",   "Stop",     "Save / 保存",
                            "Open / 打开",   "New / 新建",  "Save as...", "Build log"};
    if (!host_executable_directory(h->directory, sizeof(h->directory)) ||
        !host_path(path, sizeof(path), h->directory, "sdk.paths"))
        return UID_ERROR;
    config = host_read(path, NULL, HOST_PATH * 2);
    if (!config)
        return UID_ERROR;
    newline = strchr(config, '\n');
    if (!newline) {
        free(config);
        return UID_ERROR;
    }
    *newline++ = 0;
    config[strcspn(config, "\r")] = 0;
    newline[strcspn(newline, "\r\n")] = 0;
    if (strlen(config) >= sizeof(h->sdk) || strlen(newline) >= sizeof(h->compiler)) {
        free(config);
        return UID_LIMIT;
    }
    strcpy(h->sdk, config);
    strcpy(h->compiler, newline);
    free(config);
    h->proxy = xuiProxyXge();
    if ((r = xuiCreate(&h->xui)))
        return r;
    xuiSetProxy(h->xui, &h->proxy);
    if (!resolve_font(18, h))
        return UID_ERROR;
    xuiSetDefaultFont(h->xui, resolve_font(18, h));
    h->code_font = resolve_font(18, h);
    /* Optional IDE-style monospace font; UI text stays in Microsoft YaHei. */
    if (host_path(path, sizeof(path), h->sdk, "tools/UIDesign/release/font/UIDesign.ttf") &&
        GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
        h->proxy.fontLoadFile(&h->proxy, &h->code_font, path, 18, 0);
    if (!h->code_font)
        h->code_font = resolve_font(18, h);
    if (xuiWidgetCreate(h->xui, &h->root))
        return UID_ERROR;
    xuiSetRootWidget(h->xui, h->root);
    xuiWidgetSetLayoutType(h->root, XUI_LAYOUT_MANUAL);
    xuiWidgetSetLayoutChildrenCallback(h->root, layout, h);
    cache.iSize = sizeof(cache);
    cache.iPolicy = XUI_CACHE_POLICY_SELF;
    cache.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
    xuiWidgetSetCachePolicy(h->root, &cache);
    xuiWidgetSetCacheRenderCallback(h->root, paint, h);
    xuiHotKeyRegister(h->xui, h->root, XUI_KEY_F5, 0, key_event, h);
    xuiHotKeyRegister(h->xui, h->root, XUI_KEY_F5, XUI_MOD_SHIFT, key_event, h);
    xuiHotKeyRegister(h->xui, h->root, XUI_KEY_F7, 0, key_event, h);
    xuiHotKeyRegister(h->xui, h->root, XUI_KEY_F7, XUI_MOD_SHIFT, key_event, h);
    xuiHotKeyRegister(h->xui, h->root, 'S', XUI_MOD_CTRL, key_event, h);
    label.iSize = sizeof(label);
    label.pFont = resolve_font(18, h);
    label.iTextColor = 0x233246FFu;
    label.sText = "Embeddable Form Designer 0.1 / 可嵌入窗体设计器";
    if (xuiLabelCreate(h->xui, &h->title, &label))
        return UID_ERROR;
    xuiWidgetAddChild(h->root, h->title);
    label.sText = "Draw controls, edit properties, double-click to create events, then Run.";
    if (xuiLabelCreate(h->xui, &h->status, &label))
        return UID_ERROR;
    xuiWidgetAddChild(h->root, h->status);
    for (i = 0; i < 9; ++i) {
        h->actions[i] = (action){h, i + 1, NULL};
        button.iSize = sizeof(button);
        button.pFont = resolve_font(18, h);
        button.sText = labels[i];
        button.iTextColor = 0x233246FFu;
        if (xuiButtonCreate(h->xui, &h->actions[i].widget, &button))
            return UID_ERROR;
        xuiWidgetAddChild(h->root, h->actions[i].widget);
        xuiButtonSetClick(h->actions[i].widget, button_click, &h->actions[i]);
    }
    if (xuiWidgetCreate(h->xui, &h->editor_parent) || xuiWidgetCreate(h->xui, &h->preview_parent))
        return UID_ERROR;
    xuiWidgetAddChild(h->root, h->editor_parent);
    xuiWidgetAddChild(h->root, h->preview_parent);
    xuiWidgetSetLayoutChildrenCallback(h->editor_parent, editor_parent_layout, h);
    code.iSize = sizeof(code);
    code.pFont = h->code_font;
    code.sText = initial_code;
    code.sLanguage = "c";
    code.bShowLineNumbers = 1;
    code.bShowFoldMargin = 1;
    code.iTabColumns = 4;
    code.iIndentColumns = 4;
    if (xuiCodeEditCreate(h->xui, &h->code, &code))
        return UID_ERROR;
    xuiWidgetAddChild(h->root, h->code);
    code.bReadonly = 1;
    code.sText = "Compiler diagnostics appear here.";
    code.sLanguage = "text";
    if (xuiCodeEditCreate(h->xui, &h->output, &code))
        return UID_ERROR;
    xuiWidgetAddChild(h->root, h->output);
    if (uidContextCreate(&h->designer) || uidXuiRegisterBuiltins(h->designer) ||
        uidDocumentCreate(h->designer, "Form1", &h->document) || create_editor(h))
        return UID_ERROR;
    uidSubscribe(h->designer, notice, h, &h->subscription);
    h->saved_code = copy_text(initial_code);
    xgeSetQuitRequestCallback(quit_request, h);
    page(h, 0);
    return UID_OK;
}

static int screenshot(shell *h, const char *name) {
    char path[HOST_PATH];
    void *pixels = malloc((size_t)h->width * h->height * 4);
    int r = UID_ERROR;
    if (pixels && host_path(path, sizeof(path), h->directory, name) &&
        !h->proxy.surfaceReadRGBA(&h->proxy, h->surface, pixels, h->width * 4))
        r = xgeImageSavePNG(path, h->width, h->height, pixels, h->width * 4);
    free(pixels);
    return r;
}
/* Black-box interaction checks: input is dispatched through XUI, never through
 * private editor structs or direct runtime click callbacks. */
static void pointer(shell *h, int type, int x, int y) {
    if (type == XUI_EVENT_POINTER_DOWN)
        xuiInputPointerDown(h->xui, x, y, XUI_POINTER_BUTTON_LEFT, 1);
    else if (type == XUI_EVENT_POINTER_UP)
        xuiInputPointerUp(h->xui, x, y, XUI_POINTER_BUTTON_LEFT, 0);
    else
        xuiInputPointerMove(h->xui, x, y, 1);
    xuiDispatchPendingEvents(h->xui);
}
static void draw_control(shell *h, const char *type, int x, int y, int width, int height) {
    xui_rect_t root =
        xuiWidgetGetWorldRect(uidEditorNodeWidget(h->editor, uidDocumentRoot(h->document)));
    uidEditorSetTool(h->editor, type);
    pointer(h, XUI_EVENT_POINTER_DOWN, root.fX + x, root.fY + y);
    pointer(h, XUI_EVENT_POINTER_MOVE, root.fX + x + width, root.fY + y + height);
    pointer(h, XUI_EVENT_POINTER_UP, root.fX + x + width, root.fY + y + height);
}
static void click_widget(shell *h, xui_widget w, int twice) {
    xui_rect_t r = xuiWidgetGetWorldRect(w);
    int x = r.fX + r.fW / 2, y = r.fY + r.fH / 2;
    pointer(h, XUI_EVENT_POINTER_DOWN, x, y);
    pointer(h, XUI_EVENT_POINTER_UP, x, y);
    if (twice) {
        pointer(h, XUI_EVENT_POINTER_DOWN, x, y);
        pointer(h, XUI_EVENT_POINTER_UP, x, y);
    }
}
#define CHECK(H, EXPR)                                                                             \
    do {                                                                                           \
        if (!(EXPR)) {                                                                             \
            fprintf(stderr, "E2E FAIL line %d: %s\n", __LINE__, #EXPR);                            \
            (H)->failed = 1;                                                                       \
            xgeQuit();                                                                             \
            return;                                                                                \
        }                                                                                          \
    } while (0)
static void exercise(shell *h) {
    uid_binding binding;
    uid_value value;
    xui_widget w;
    char path[HOST_PATH];
    if (++h->exercise_wait > 1800) {
        CHECK(h, 0);
    }
    switch (h->exercise_phase) {
    case 0:
        if (h->frame < 3)
            return;
        draw_control(h, "xui.button", 48, 56, 216, 48);
        CHECK(h, uidNodeCount(h->document) == 2);
        h->exercise_button = uidNodeFindName(h->document, "button1");
        CHECK(h, h->exercise_button);
        CHECK(h, !uidRenameNode(h->document, h->exercise_button, "btnHello"));
        CHECK(h, !uidSetProperty(h->document, h->exercise_button, "text",
                                 uidString("运行事件 / Click me")));
        ++h->exercise_phase;
        break;
    case 1:
        draw_control(h, "xui.input", 48, 128, 304, 40);
        h->exercise_input = uidNodeFindName(h->document, "input1");
        CHECK(h, h->exercise_input);
        ++h->exercise_phase;
        break;
    case 2:
        draw_control(h, "xui.checkbox", 48, 196, 240, 40);
        h->exercise_check = uidNodeFindName(h->document, "checkBox1");
        CHECK(h, h->exercise_check);
        CHECK(h,
              !uidSetProperty(h->document, h->exercise_check, "text", uidString("同意 / Agree")));
        h->exercise_phase = 20;
        break;
    case 20: {
        xui_rect_t r = xuiWidgetGetWorldRect(uidEditorNodeWidget(h->editor, h->exercise_button));
        pointer(h, XUI_EVENT_POINTER_DOWN, r.fX + 30, r.fY + 20);
        pointer(h, XUI_EVENT_POINTER_MOVE, r.fX + 46, r.fY + 28);
        pointer(h, XUI_EVENT_POINTER_UP, r.fX + 46, r.fY + 28);
        CHECK(h,
              !uidGetProperty(h->document, h->exercise_button, "x", &value) && value.number == 64);
        CHECK(h, !uidUndo(h->document));
        CHECK(h,
              !uidGetProperty(h->document, h->exercise_button, "x", &value) && value.number == 48);
        ++h->exercise_phase;
        break;
    }
    case 21: {
        xui_rect_t r = xuiWidgetGetWorldRect(uidEditorNodeWidget(h->editor, h->exercise_button));
        pointer(h, XUI_EVENT_POINTER_DOWN, r.fX + r.fW - 2, r.fY + r.fH - 2);
        pointer(h, XUI_EVENT_POINTER_MOVE, r.fX + r.fW + 30, r.fY + r.fH + 14);
        pointer(h, XUI_EVENT_POINTER_UP, r.fX + r.fW + 30, r.fY + r.fH + 14);
        CHECK(h, !uidGetProperty(h->document, h->exercise_button, "width", &value) &&
                     value.number == 248);
        CHECK(h, !uidUndo(h->document));
        ++h->exercise_phase;
        break;
    }
    case 22:
        draw_control(h, "xui.panel", 420, 56, 300, 210);
        CHECK(h, uidNodeFindName(h->document, "panel1"));
        ++h->exercise_phase;
        break;
    case 23: {
        uid_node_info n;
        uid_id label;
        draw_control(h, "xui.label", 444, 80, 246, 36);
        label = uidNodeFindName(h->document, "label1");
        CHECK(h, label);
        CHECK(h, !uidNodeGet(h->document, label, &n) &&
                     n.parent == uidNodeFindName(h->document, "panel1"));
        CHECK(h, !uidSetProperty(h->document, label, "text", uidString("嵌入 / Nested panel")));
        ++h->exercise_phase;
        break;
    }
    case 24: {
        xui_rect_t r = xuiWidgetGetWorldRect(uidEditorNodeWidget(h->editor, h->exercise_button));
        pointer(h, XUI_EVENT_POINTER_DOWN, r.fX + 30, r.fY + 20);
        pointer(h, XUI_EVENT_POINTER_MOVE, r.fX + 78, r.fY + 52);
        xuiInputKeyDown(h->xui, XUI_KEY_ESCAPE, 0);
        xuiInputKeyUp(h->xui, XUI_KEY_ESCAPE, 0);
        xuiDispatchPendingEvents(h->xui);
        pointer(h, XUI_EVENT_POINTER_UP, r.fX + 78, r.fY + 52);
        CHECK(h,
              !uidGetProperty(h->document, h->exercise_button, "x", &value) && value.number == 48);
        CHECK(h, !uidEditorSetZoom(h->editor, 0.5f));
        ++h->exercise_phase;
        break;
    }
    case 25:
        CHECK(h,
              xuiWidgetGetWorldRect(uidEditorNodeWidget(h->editor, h->exercise_button)).fW == 108);
        CHECK(h, !uidGetProperty(h->document, h->exercise_button, "width", &value) &&
                     value.number == 216);
        CHECK(h, !uidEditorSetZoom(h->editor, 1));
        h->exercise_phase = 3;
        break;
    case 3:
        screenshot(h, "design.png");
        click_widget(h, uidEditorNodeWidget(h->editor, h->exercise_button), 1);
        CHECK(h, !uidGetBinding(h->document, h->exercise_button, "click", &binding));
        CHECK(h,
              !strcmp(binding.handler, "btnHello_click") && h->ensure_calls == 1 && h->page == 1);
        ++h->exercise_phase;
        break;
    case 4: {
        const char *code = xuiCodeEditGetText(h->code), *at = strstr(code, "Handler called");
        host_buffer b = {0};
        CHECK(h, at);
        host_append(&b, "%.*s业务代码已执行%s", (int)(at - code), code,
                    at + strlen("Handler called"));
        CHECK(h, !b.failed && !xuiCodeEditSetText(h->code, b.data));
        free(b.data);
        CHECK(h, !uidEditorEditDefaultEvent(h->editor, h->exercise_button) && h->ensure_calls == 1);
        CHECK(h, !uidEditorEditDefaultEvent(h->editor, h->exercise_input));
        CHECK(h, !uidEditorEditDefaultEvent(h->editor, h->exercise_check));
        host_path(h->project, sizeof(h->project), h->directory, "exercise.uidproj");
        CHECK(h, save_project(h, 0) && !uidDocumentDirty(h->document) && !code_dirty(h));
        snprintf(path, sizeof(path), "%s", h->project);
        CHECK(h, !open_project(h, path));
        CHECK(h, uidNodeCount(h->document) == 6);
        CHECK(h, strstr(xuiCodeEditGetText(h->code), "业务代码已执行"));
        CHECK(h, !uidGetBinding(h->document, h->exercise_button, "click", &binding));
        page(h, 1);
        ++h->exercise_phase;
        break;
    }
    case 5:
        screenshot(h, "code.png");
        xuiSetFocusWidget(h->xui, h->code);
        xuiInputKeyDown(h->xui, XUI_KEY_F5, 0);
        xuiInputKeyUp(h->xui, XUI_KEY_F5, 0);
        xuiDispatchPendingEvents(h->xui);
        CHECK(h, h->pending == CMD_RUN);
        ++h->exercise_phase;
        break;
    case 6:
        if (h->building)
            return;
        CHECK(h, h->view && h->page == 2);
        ++h->exercise_phase;
        break;
    case 7:
        w = h->find_view(h->view, "btnHello");
        CHECK(h, w);
        click_widget(h, w, 0);
        CHECK(h, h->calls == 1 && !strcmp(h->last_log, "业务代码已执行"));
        CHECK(h, !strcmp(xuiButtonGetText(w), "Clicked / 已执行"));
        ++h->exercise_phase;
        break;
    case 8: {
        const char *p = "hello";
        w = h->find_view(h->view, "input1");
        CHECK(h, w);
        click_widget(h, w, 0);
        for (; *p; ++p)
            xuiInputText(h->xui, (unsigned char)*p);
        xuiDispatchPendingEvents(h->xui);
        CHECK(h, h->calls >= 2 && !strcmp(h->last_log, "hello"));
        ++h->exercise_phase;
        break;
    }
    case 9:
        w = h->find_view(h->view, "checkBox1");
        CHECK(h, w);
        click_widget(h, w, 0);
        CHECK(h, !strcmp(h->last_log, "checked=true"));
        CHECK(h,
              !uidGetProperty(h->document, h->exercise_check, "checked", &value) && !value.number);
        CHECK(h, !uidDocumentDirty(h->document));
        ++h->exercise_phase;
        break;
    case 10:
        CHECK(h, !screenshot(h, "run.png"));
        h->first_calls = h->calls;
        stop_view(h);
        CHECK(h, !h->view && !h->module);
        CHECK(h, !start_run(h));
        ++h->exercise_phase;
        break;
    case 11:
        if (h->building)
            return;
        CHECK(h, h->view);
        ++h->exercise_phase;
        break;
    case 12:
        click_widget(h, h->find_view(h->view, "btnHello"), 0);
        CHECK(h, h->calls == h->first_calls + 1);
        stop_view(h);
        {
            host_buffer code = {0};
            host_append(&code, "%s\n#error deliberate_compile_failure\n", h->saved_code);
            CHECK(h, !code.failed && !xuiCodeEditSetText(h->code, code.data));
            free(code.data);
        }
        CHECK(h, !start_run(h));
        ++h->exercise_phase;
        break;
    case 13:
        if (h->building)
            return;
        CHECK(h, !h->view && h->page == 3 &&
                     strstr(xuiCodeEditGetText(h->output), "deliberate_compile_failure"));
        CHECK(h, !xuiCodeEditSetText(h->code, h->saved_code));
        CHECK(h, !start_run(h));
        CHECK(h, !uidSetProperty(h->document, h->exercise_button, "text",
                                 uidString("changed while building")));
        ++h->exercise_phase;
        break;
    case 14:
        if (h->building)
            return;
        CHECK(h, !h->view && strstr(h->message, "Stale result discarded"));
        CHECK(h, !uidUndo(h->document) && !uidDocumentDirty(h->document));
        CHECK(h, !start_run(h));
        stop_view(h);
        ++h->exercise_phase;
        break;
    case 15:
        if (h->building)
            return;
        CHECK(h, !h->view && !h->module && !h->build_job && strstr(h->message, "discarded"));
        printf(
            "E2E PASS: draw=5 nested-container=1 move/resize/undo/cancel=1 zoom=1 "
            "default-event=3 idempotent=1 save/reopen=1 compiled-click=2 typed-events=2 restart=1 "
            "compiler-error=1 stale-result=1 cancel-build=1\n");
        fflush(stdout);
        h->exercise_phase = 16;
        xgeQuit();
        break;
    }
}
#undef CHECK
static int frame(void *u) {
    shell *h = u;
    int width = xgeGetWidth(), height = xgeGetHeight(), r;
    xui_rect_i_t full;
    xui_rect_t rect;
    if (width <= 0 || height <= 0)
        return XGE_OK;
    if (width != h->width || height != h->height) {
        xui_surface_desc_t desc = {0};
        if (h->surface)
            h->proxy.surfaceDestroy(&h->proxy, h->surface);
        h->surface = NULL;
        h->width = width;
        h->height = height;
        desc.iKind = XUI_SURFACE_KIND_TEXTURE;
        desc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
        desc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
        desc.iWidth = width;
        desc.iHeight = height;
        if (h->proxy.surfaceCreate(&h->proxy, &h->surface, &desc))
            return XGE_ERROR;
        xuiSetViewportSize(h->xui, (float)width, (float)height);
        xuiInputViewport(h->xui, (float)width, (float)height);
        xuiWidgetSetRect(h->root, (xui_rect_t){0, 0, width, height});
    }
    if ((r = xgeBegin()))
        return r;
    xuiProxyXgePumpInput(h->xui);
    xuiDispatchPendingEvents(h->xui);
    commands(h);
    poll_build(h);
    if ((r = uidEditorUpdate(h->editor))) {
        fprintf(stderr, "Editor update failed: %s\n", uidLastError(h->designer));
        return r;
    }
    if ((r = xuiLayout(h->xui)))
        return r;
    xuiUpdate(h->xui, xgeGetDelta());
    h->proxy.surfaceClear(&h->proxy, h->surface, 0xE9EFF7FFu);
    full = (xui_rect_i_t){0, 0, width, height};
    if ((r = xuiRender(h->xui, h->surface, &full, 1)))
        return r;
    xgeClear(0xE9EFF7FFu);
    rect = (xui_rect_t){0, 0, width, height};
    h->proxy.surfaceDraw(&h->proxy, h->surface, rect, rect, XUI_COLOR_WHITE,
                         XUI_SURFACE_DRAW_SCREEN_SPACE);
    r = xgeEnd();
    ++h->frame;
    if (h->exercise)
        exercise(h);
    else if (h->max_frames && h->frame >= h->max_frames) {
        screenshot(h, "shell.png");
        xgeQuit();
    }
    return r;
}
static void cleanup(shell *h) {
    int i;
    xgeSetQuitRequestCallback(NULL, NULL);
    if (h->building) {
        if (h->build_job) {
            CloseHandle(h->build_job);
            h->build_job = NULL;
        }
        WaitForSingleObject(h->process.hProcess, 1000);
        CloseHandle(h->process.hProcess);
        CloseHandle(h->process.hThread);
        CloseHandle(h->build_log);
        h->building = 0;
    }
    uidSnapshotDestroy(h->build_snapshot);
    free(h->building_code);
    if (h->view)
        h->destroy_view(h->view);
    if (h->module)
        FreeLibrary(h->module);
    uidUnsubscribe(h->designer, h->subscription);
    uidEditorDestroy(h->editor);
    uidDocumentDestroy(h->document);
    if (h->designer && uidContextDestroy(h->designer)) {
        fprintf(stderr, "Component ownership leak\n");
        h->failed = 1;
    }
    if (h->xui)
        xuiDestroy(h->xui);
    if (h->surface)
        h->proxy.surfaceDestroy(&h->proxy, h->surface);
    if (h->code_font && h->code_font != h->fonts[18])
        h->proxy.fontDestroy(&h->proxy, h->code_font);
    for (i = 0; i < 97; ++i)
        if (h->fonts[i])
            h->proxy.fontDestroy(&h->proxy, h->fonts[i]);
    free(h->saved_code);
}
int main(int argc, char **argv) {
    shell *h = calloc(1, sizeof(*h));
    xge_desc_t d = {0};
    int i, r;
    if (!h)
        return 1;
    setvbuf(stdout, NULL, _IONBF, 0);
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--exercise"))
            h->exercise = 1;
        else if (!strcmp(argv[i], "--frames") && i + 1 < argc)
            h->max_frames = atoi(argv[++i]);
        else {
            fprintf(stderr, "Usage: uidesign_shell.exe [--exercise | --frames N]\n");
            free(h);
            return 1;
        }
    }
    d.iWidth = 1440;
    d.iHeight = 920;
    d.sTitle = "UIDesign 0.1 - Embeddable Form Designer";
    d.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
    d.iRunMode = XGE_RUN_GAME_LOOP;
    d.iTargetFPS = 60;
    r = xgeInit(&d);
    if (r) {
        free(h);
        return 1;
    }
    r = setup(h);
    if (!r)
        r = xgeRun(frame, h);
    else
        fprintf(stderr, "Shell initialization failed: %d\n", r);
    cleanup(h);
    xgeUnit();
    i = r || h->failed || (h->exercise && h->exercise_phase != 16);
    free(h);
    return i ? 1 : 0;
}
