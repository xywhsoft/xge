#include "xui_document_ui.h"
#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#define W 1280
#define H 780
#define DEMO_BUTTON_COUNT 12
typedef struct demo demo;
typedef struct action { demo* app; int command; } action;
struct demo {
    xui_context context; xui_proxy_t proxy; xui_surface target;
    xui_font font, bold, italic, mono; xui_widget root, rich, markdown, preview, status, active_editor;
    xui_widget caption, headings[3], buttons[DEMO_BUTTON_COUNT], format_menu, format_editor; xui_rect_t panels[3];
    xui_document rich_doc, md_doc, pending_open; action actions[DEMO_BUTTON_COUNT];
    xui_file_dialog file_dialog; xui_msgbox discard_box, close_box;
    xui_widget file_editor; int file_save;
    int close_requested, close_saving;
    int width, height, verify, frames, result;
    uint64_t source_pixels;
    uint64_t async_revision, async_source_pixels, async_preview_pixels, async_pending_pixels;
};
static const char* md_text =
    "# Shared Markdown document\n\n"
    "This **preview** and the editor use the *same Document*.\n\n"
    "![Native image](demo.surface)\n\n"
    "> A real nested block quote.\n> - Keep source and content together\n> - One shared undo history\n\n"
    "- [x] Source preserved\n- [ ] Visual structural commands\n\n"
    "| Feature | Status |\n| :--- | ---: |\n| Snapshot | Shared |\n| Tables | Native |\n\n"
    "```c\n// A code fence\nxuiDocumentUndo(document, NULL);\n```\n";
static const char* md_image_test_text =
    "# Missing image\n\n![Preview alt text](missing.surface)\n";
static int root_draw(xui_widget w, xui_draw_context draw, uint32_t state, void* user)
{
    demo* d = user; int i, result; (void)w; (void)state;
    result = d->proxy.drawRectFill(&d->proxy, draw, (xui_rect_t){0,0,d->width,d->height}, XUI_COLOR_RGBA(239,242,247,255));
    for (i = 0; i < 3 && result == XUI_OK; i++) result = d->proxy.drawRectFill(&d->proxy, draw, d->panels[i], XUI_COLOR_WHITE);
    return result;
}
static void show_error(xui_widget editor, int error, void* user)
{
    demo* d = user; char text[140]; (void)editor;
    snprintf(text, sizeof(text), "Operation returned %d. The document was not partially committed.", error);
    xuiLabelSetText(d->status, text);
}
static void image_destroy(xui_context context, void* handle, void* user)
{
    demo* d = user; (void)context;
    d->proxy.surfaceDestroy(&d->proxy, (xui_surface)handle);
}
/* Prepare the replacement preview before changing the editor. A failed open
 * keeps both old bindings and the old document available. */
static int replace_document(demo* d, xui_widget editor, xui_document document)
{
    xui_widget replacement = NULL; int result;
    if (editor == d->markdown) {
        xui_doc_view_desc_t view = {0};
        view.iSize = sizeof(view); view.pDocument = document;
        view.tRenderer.iSize = sizeof(view.tRenderer);
        view.tRenderer.tFonts = (xui_doc_font_set_t){d->font,d->bold,d->italic,d->bold,d->mono};
        result = xuiDocumentViewCreate(d->context, &view, &replacement);
        if (result == XUI_OK) result = xuiWidgetSetRect(replacement, xuiWidgetGetRect(d->preview));
        if (result == XUI_OK) result = xuiWidgetAddChild(d->root, replacement);
        if (result != XUI_OK) { if (replacement) xuiWidgetDestroy(replacement); return result; }
    }
    result = xuiDocumentViewSetDocument(editor, document);
    if (result != XUI_OK) { if (replacement) xuiWidgetDestroy(replacement); return result; }
    xuiDocumentRetain(document);
    if (editor == d->rich) { xuiDocumentRelease(d->rich_doc); d->rich_doc = document; }
    else {
        xuiWidgetDestroy(d->preview); d->preview = replacement;
        xuiDocumentRelease(d->md_doc); d->md_doc = document;
    }
    d->active_editor = editor;
    return XUI_OK;
}
static int editor_has_unpublished_work(xui_widget editor);
static void restore_editor_focus(demo* d, xui_widget editor)
{
    if (editor) {
        d->active_editor = editor;
        (void)xuiSetFocusWidget(d->context, editor);
    }
}
static void discard_result(xui_msgbox box, int answer, void* user)
{
    demo* d = user; int result = XUI_OK; (void)box;
    if (answer == XUI_MSGBOX_RESULT_YES && d->pending_open) {
        xui_document current = xuiDocumentViewGetDocument(d->file_editor);
        result = xuiDocumentHasPrepare(current) || editor_has_unpublished_work(d->file_editor) ?
            XUI_DOC_ERROR_BUSY : replace_document(d, d->file_editor, d->pending_open);
    }
    xuiDocumentRelease(d->pending_open); d->pending_open = NULL;
    restore_editor_focus(d, d->file_editor);
    if (result != XUI_OK) show_error(d->file_editor, result, d);
    else xuiLabelSetText(d->status, answer == XUI_MSGBOX_RESULT_YES ? "File opened." : "Open cancelled. Current document kept.");
}
static int file_action(demo* d, xui_widget editor, int save);
/* Native close can arrive before this frame's text events are dispatched.
 * Defer the decision until the UI has processed those events. */
static int request_close(void* user)
{
    demo* d = user; d->close_requested = 1; return 0;
}
static void close_result(xui_msgbox box, int answer, void* user)
{
    demo* d = user; (void)box;
    d->close_requested = 0; d->close_saving = 0;
    if (answer == XUI_MSGBOX_RESULT_YES) d->close_saving = 1;
    else if (answer == XUI_MSGBOX_RESULT_NO) xgeQuit();
    else {
        restore_editor_focus(d, d->active_editor);
        xuiLabelSetText(d->status, "Close cancelled. Current documents kept.");
    }
}
static int editor_has_unpublished_work(xui_widget editor)
{
    xui_doc_editor_pending_work_t work = {0}; work.iSize = sizeof(work);
    return xuiDocumentEditorGetPendingWork(editor, &work) != XUI_OK || work.bHasInput || work.iQueuedEvents || work.iResult != XUI_OK;
}
static int service_close(demo* d)
{
    int result;
    if ((!d->close_requested && !d->close_saving) ||
        (d->file_dialog && xuiFileDialogIsOpen(d->file_dialog)) ||
        (d->discard_box && xuiMsgBoxIsOpen(d->discard_box)) ||
        (d->close_box && xuiMsgBoxIsOpen(d->close_box))) return XUI_OK;
    if (d->close_saving) {
        int rich = xuiDocumentEditorFlush(d->rich), markdown = xuiDocumentEditorFlush(d->markdown);
        if (rich == XUI_DOC_ERROR_BUSY || markdown == XUI_DOC_ERROR_BUSY ||
            xuiDocumentHasPrepare(d->rich_doc) || xuiDocumentHasPrepare(d->md_doc) ||
            editor_has_unpublished_work(d->rich) || editor_has_unpublished_work(d->markdown)) {
            xuiLabelSetText(d->status, "Waiting for pending edits before saving. Close again to cancel or discard.");
            if (!d->close_requested) return XUI_OK;
            d->close_saving = 0;
        } else if (rich != XUI_OK || markdown != XUI_OK) {
            d->close_saving = d->close_requested = 0;
            return rich != XUI_OK ? rich : markdown;
        } else {
            d->close_requested = 0;
            if (xuiDocumentIsDirty(d->rich_doc)) result = file_action(d, d->rich, 1);
            else if (xuiDocumentIsDirty(d->md_doc)) result = file_action(d, d->markdown, 1);
            else { d->close_saving = 0; xgeQuit(); return XUI_OK; }
            if (result != XUI_OK) d->close_saving = 0;
            return result;
        }
    }
    if (!xuiDocumentIsDirty(d->rich_doc) && !xuiDocumentIsDirty(d->md_doc) &&
        !xuiDocumentHasPrepare(d->rich_doc) && !xuiDocumentHasPrepare(d->md_doc) &&
        !editor_has_unpublished_work(d->rich) && !editor_has_unpublished_work(d->markdown)) {
        d->close_requested = 0; xgeQuit(); return XUI_OK;
    }
    {
        xui_msgbox_desc_t desc = {0}; desc.iSize = sizeof(desc);
        desc.sTitle = "Close documents"; desc.sMessage = "Save unsaved documents before closing?";
        desc.pFont = d->font; desc.iButtons = XUI_MSGBOX_BUTTON_YES_NO_CANCEL;
        desc.bModal = desc.bHasModal = 1;
        if (d->close_box) { xuiMsgBoxDestroy(d->close_box); d->close_box = NULL; }
        result = xuiMsgBoxCreate(d->context, &d->close_box, &desc);
        if (result == XUI_OK) result = xuiMsgBoxSetPresetButtonTitle(d->close_box, XUI_MSGBOX_BUTTON_TITLE_YES, "Save");
        if (result == XUI_OK) result = xuiMsgBoxSetPresetButtonTitle(d->close_box, XUI_MSGBOX_BUTTON_TITLE_NO, "Discard");
        if (result == XUI_OK) result = xuiMsgBoxSetResult(d->close_box, close_result, d);
        if (result == XUI_OK) result = xuiMsgBoxSetOpen(d->close_box, 1);
        d->close_requested = 0;
        return result;
    }
}
static void file_result(xui_file_dialog dialog, int answer, const char* path, void* user)
{
    demo* d = user; xui_document current = xuiDocumentViewGetDocument(d->file_editor);
    uint32_t format = d->file_editor == d->rich ? XUI_DOC_FILE_NATIVE : XUI_DOC_FILE_MARKDOWN;
    int result; (void)dialog;
    restore_editor_focus(d, d->file_editor);
    if (answer != XUI_FILE_DIALOG_RESULT_OK) { d->close_saving = d->close_requested = 0; return; }
    /* Recheck at confirmation: a host edit can start after the dialog opens.
     * Keep its unpublished input instead of replacing it or saving only the
     * previously committed revision. */
    if (xuiDocumentHasPrepare(current) || editor_has_unpublished_work(d->file_editor)) {
        d->close_saving = d->close_requested = 0;
        show_error(d->file_editor, XUI_DOC_ERROR_BUSY, d);
        return;
    }
    if (d->file_save) {
        result = xuiDocumentSaveFile(current, path, format);
        if (result == XUI_OK) xuiLabelSetText(d->status, "File saved.");
    } else {
        result = xuiDocumentOpenFile(path, format, NULL, 0, &d->pending_open);
        if (result == XUI_OK && xuiDocumentGetProfile(d->pending_open) != xuiDocumentGetProfile(current)) result = XUI_ERROR_INVALID_ARGUMENT;
        if (result == XUI_OK && xuiDocumentIsDirty(current)) {
            xui_msgbox_desc_t desc = {0}; desc.iSize = sizeof(desc);
            desc.sTitle = "Open document"; desc.sMessage = "Discard current unsaved changes and open this file?";
            desc.pFont = d->font; desc.iButtons = XUI_MSGBOX_BUTTON_YES_NO;
            desc.bModal = desc.bHasModal = 1;
            if (d->discard_box) { xuiMsgBoxDestroy(d->discard_box); d->discard_box = NULL; }
            result = xuiMsgBoxCreate(d->context, &d->discard_box, &desc);
            if (result == XUI_OK) result = xuiMsgBoxSetResult(d->discard_box, discard_result, d);
            if (result == XUI_OK) result = xuiMsgBoxSetOpen(d->discard_box, 1);
            if (result == XUI_OK) return;
        } else if (result == XUI_OK) {
            result = replace_document(d, d->file_editor, d->pending_open);
            if (result == XUI_OK) xuiLabelSetText(d->status, "File opened.");
        }
        xuiDocumentRelease(d->pending_open); d->pending_open = NULL;
    }
    if (result != XUI_OK) {
        d->close_saving = d->close_requested = 0;
        show_error(d->file_editor, result, d);
    }
}
static int file_action(demo* d, xui_widget editor, int save)
{
    xui_file_dialog_desc_t desc = {0};
    if (xuiDocumentHasPrepare(xuiDocumentViewGetDocument(editor))) return XUI_DOC_ERROR_BUSY;
    if (d->file_dialog) { xuiFileDialogDestroy(d->file_dialog); d->file_dialog = NULL; }
    d->file_editor = editor; d->file_save = save;
    desc.iSize = sizeof(desc); desc.sTitle = save ? "Save document" : "Open document";
    desc.sInitialDir = "."; desc.pFont = d->font; desc.bModal = 1;
    desc.sFilter = editor == d->rich ? "XUI documents (*.xdoc)|*.xdoc" : "Markdown (*.md)|*.md|Text (*.txt)|*.txt";
    if (save) desc.sFileName = editor == d->rich ? "document.xdoc" : "document.md";
    desc.onResult = file_result; desc.pResultUser = d;
    return save ? xuiSaveFileDialog(d->context, &d->file_dialog, &desc) : xuiOpenFileDialog(d->context, &d->file_dialog, &desc);
}
static const struct { const char* label; uint32_t command; } formats[] = {
    {"Paragraph",XUI_DOC_EDIT_PARAGRAPH},{"Heading 1",XUI_DOC_EDIT_HEADING_1},
    {"Heading 2",XUI_DOC_EDIT_HEADING_2},{"Heading 3",XUI_DOC_EDIT_HEADING_3},{NULL,0},
    {"Bullet list",XUI_DOC_EDIT_BULLET_LIST},{"Numbered list",XUI_DOC_EDIT_NUMBER_LIST},
    {"Task list",XUI_DOC_EDIT_TASK_LIST},{"Block quote",XUI_DOC_EDIT_BLOCK_QUOTE},
    {"Code block",XUI_DOC_EDIT_INSERT_CODE_BLOCK},{NULL,0},
    {"Underline",XUI_DOC_EDIT_UNDERLINE},{"Strikethrough",XUI_DOC_EDIT_STRIKE},
    {"Inline code",XUI_DOC_EDIT_CODE},{"Clear formatting",XUI_DOC_EDIT_CLEAR_FORMATTING}
};
static void format_select(xui_widget menu,int index,int value,void* user)
{
    demo* d=user;xui_widget editor=d->format_editor;int result;
    (void)menu;(void)index;
    if(!editor || value<=0)return;
    result=xuiDocumentEditorExecute(editor,(uint32_t)value);
    restore_editor_focus(d,editor);
    if(result!=XUI_OK)show_error(editor,result,d);
    else xuiLabelSetText(d->status,"Format applied. Ctrl+Z/Y uses the same document history.");
}
static int open_format(demo* d,xui_widget button,xui_widget editor)
{
    xui_menu_item_t items[sizeof(formats)/sizeof(formats[0])]={0};
    xui_rect_t anchor=xuiWidgetGetWorldRect(button);size_t i;int result;
    if(!d->format_menu) {
        xui_menu_desc_t desc={0};desc.iSize=sizeof(desc);desc.pOwner=editor;desc.pFont=d->font;
        result=xuiMenuCreate(d->context,&d->format_menu,&desc);
        if(result!=XUI_OK)return result;
        result=xuiMenuSetSelect(d->format_menu,format_select,d);
        if(result!=XUI_OK)return result;
        xuiWidgetSetUserData(button,d->format_menu);
    }
    for(i=0;i<sizeof(formats)/sizeof(formats[0]);i++) {
        xui_doc_command_state_t state={0};state.iSize=sizeof(state);
        if(!formats[i].command) {items[i].iType=XUI_MENU_ITEM_SEPARATOR;continue;}
        result=xuiDocumentEditorQueryCommand(editor,formats[i].command,&state);
        if(result!=XUI_OK)return result;
        items[i].sText=formats[i].label;items[i].iValue=(int)formats[i].command;items[i].iType=XUI_MENU_ITEM_CHECK;
        items[i].iState=(state.bEnabled?XUI_MENU_ITEM_ENABLED:0)|(state.bActive?XUI_MENU_ITEM_CHECKED:0);
    }
    result=xuiMenuSetItems(d->format_menu,items,(int)(sizeof(formats)/sizeof(formats[0])));
    if(result!=XUI_OK)return result;
    d->format_editor=editor;
    return xuiMenuOpenAt(d->format_menu,editor,anchor.fX,anchor.fY+anchor.fH);
}
static void click(xui_widget button, void* user)
{
    action* a = user; demo* d = a->app; xui_widget editor = xuiGetFocusWidget(d->context); int result;
    if (editor != d->rich && editor != d->markdown) editor = d->active_editor ? d->active_editor : d->markdown;
    if (a->command == 300) result = open_format(d,button,editor);
    else if (a->command >= 250) result = a->command == 250 ? xuiDocumentEditorOpenFind(editor) : xuiDocumentEditorOpenReplace(editor);
    else if (a->command >= 200) result = file_action(d, editor, a->command == 201);
    else if (a->command >= 100) { editor = d->markdown; result = xuiDocumentViewSetMode(editor,
        a->command == 100 ? XUI_DOC_SOURCE_TEXT : a->command == 101 ? XUI_DOC_VISUAL : XUI_DOC_LIVE_MARKDOWN); }
    else result = xuiDocumentEditorExecute(editor, (uint32_t)a->command);
    if (result != XUI_OK) show_error(editor, result, d);
    else xuiLabelSetText(d->status, "Ctrl+Z undo | Ctrl+B/I format | Ctrl+wheel zoom | Source and preview share one document.");
    d->active_editor = editor;
    if (a->command < 200 || result != XUI_OK) xuiSetFocusWidget(d->context, editor);
}
static int label(demo* d, const char* text, xui_rect_t rect, xui_widget* out)
{
    xui_label_desc_t desc = {0}; int r; desc.iSize = sizeof(desc); desc.sText = text; desc.pFont = d->font; desc.iTextColor = XUI_COLOR_RGBA(39,51,70,255);
    r = xuiLabelCreate(d->context, out, &desc); if (r != XUI_OK) return r;
    xuiWidgetSetRect(*out, rect); return xuiWidgetAddChild(d->root, *out);
}
static int arrange(demo* d, int width, int height)
{
    xui_widget views[] = {d->rich,d->markdown,d->preview};
    int columns = (width-32)/96, rows, extra, i, result;
    if (columns < 1) columns = 1;
    if (columns > DEMO_BUTTON_COUNT) columns = DEMO_BUTTON_COUNT;
    rows = (DEMO_BUTTON_COUNT+columns-1)/columns; extra = (rows-1)*34;
#define SET(widget,...) do { result=xuiWidgetSetRect(widget,(xui_rect_t){__VA_ARGS__}); if(result!=XUI_OK) return result; } while(0)
    result = xuiInputViewport(d->context,width,height); if (result != XUI_OK) return result;
    SET(d->root,0,0,width,height);
    SET(d->caption,18,8,width>36?width-36:1,28);
    for (i=0; i<DEMO_BUTTON_COUNT; i++) SET(d->buttons[i],20+(i%columns)*96,37+(i/columns)*34,88,26);
    for (i=0; i<3; i++) {
        int label_y=65+extra;
        if (width>=900) {
            int delta=(width-W)/3;
            int left=i==0?14:i==1?414+delta:836+2*delta;
            int span=i==0?388+delta:i==1?410+delta:width-850-2*delta;
            d->panels[i]=(xui_rect_t){left,94+extra,span,height-142-extra>1?height-142-extra:1};
        } else {
            int cell=(height-48-label_y)/3;
            if (cell<60) cell=60;
            label_y+=i*cell;
            d->panels[i]=(xui_rect_t){14,label_y+29,width>28?width-28:1,cell-35};
        }
        SET(d->headings[i],d->panels[i].fX+6,label_y,d->panels[i].fW-8,24);
        SET(views[i],d->panels[i].fX+10,d->panels[i].fY+12,
            d->panels[i].fW>20?d->panels[i].fW-20:1,d->panels[i].fH>24?d->panels[i].fH-24:1);
    }
    SET(d->status,18,height>36?height-36:0,width>36?width-36:1,26);
    d->width=width; d->height=height;
    return XUI_OK;
#undef SET
}
static int resize_target(demo* d, int width, int height)
{
    xui_surface_desc_t surface={0}; xui_surface candidate=NULL; int result;
    if (width<=0 || height<=0 || (width==d->width && height==d->height)) return XUI_OK;
    surface.iKind=XUI_SURFACE_KIND_TEXTURE; surface.iFormat=XUI_SURFACE_FORMAT_RGBA8;
    surface.iWidth=width; surface.iHeight=height;
    surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    result=d->proxy.surfaceCreate(&d->proxy,&candidate,&surface);
    if (result!=XUI_OK) return result;
    result=arrange(d,width,height);
    if (result!=XUI_OK) { d->proxy.surfaceDestroy(&d->proxy,candidate); return result; }
    d->proxy.surfaceDestroy(&d->proxy,d->target); d->target=candidate;
    return XUI_OK;
}
static int setup(demo* d)
{
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t editor = {0}; xui_doc_view_desc_t view = {0};
    const char* labels[] = {"Undo", "Redo", "Bold", "Italic", "Source", "Visual", "Live MD", "Open", "Save as", "Find", "Replace", "Format"};
    const int commands[] = {XUI_DOC_EDIT_UNDO,XUI_DOC_EDIT_REDO,XUI_DOC_EDIT_BOLD,XUI_DOC_EDIT_ITALIC,100,101,102,200,201,250,251,300};
    int i, result;
#define OK(expr) do { result = (expr); if (result != XUI_OK) { fprintf(stderr, "line %d: %s returned %d\n", __LINE__, #expr, result); return result; } } while (0)
    OK(xuiWidgetCreate(d->context, &d->root)); OK(xuiSetRootWidget(d->context, d->root));
    xuiWidgetSetRect(d->root, (xui_rect_t){0,0,W,H}); xuiWidgetSetCacheRenderCallback(d->root, root_draw, d);
    OK(label(d, "Unified Document / Native Rich Text + Markdown", (xui_rect_t){18,8,W-36,28}, &d->caption));
    OK(label(d, "Rich text editor", (xui_rect_t){20,65,380,24}, &d->headings[0]));
    OK(label(d, "Markdown editor", (xui_rect_t){420,65,400,24}, &d->headings[1]));
    OK(label(d, "Shared Markdown preview", (xui_rect_t){842,65,410,24}, &d->headings[2]));
    OK(label(d, "Ctrl+Z undo | Ctrl+B/I format | Ctrl+wheel zoom | Source and preview share one document.", (xui_rect_t){18,H-36,W-36,26}, &d->status));
    OK(xuiDocumentCreate(NULL, &d->rich_doc)); md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    OK(xuiDocumentCreate(&md, &d->md_doc));
    {
        const char* source = d->verify == 3 ? md_image_test_text : md_text;
        OK(xuiDocumentLoadMarkdown(d->md_doc, source, strlen(source)));
    }
    editor.iSize = sizeof(editor); editor.tView.iSize = sizeof(editor.tView); editor.tView.pDocument = d->rich_doc;
    editor.tView.tRenderer.iSize = sizeof(editor.tView.tRenderer); editor.tView.tRenderer.tFonts = (xui_doc_font_set_t){d->font,d->bold,d->italic,d->bold,d->mono};
    editor.onError = show_error; editor.pUser = d;
    OK(xuiDocumentEditorCreate(d->context, &editor, &d->rich));
    OK(xuiDocumentEditorInsertText(d->rich, "A new document kernel\n\nRich paragraphs and Markdown share storage, transactions, snapshots and undo.\n\nTry selecting text, applying bold, pasting paragraphs, or typing Chinese.\n\n", strlen("A new document kernel\n\nRich paragraphs and Markdown share storage, transactions, snapshots and undo.\n\nTry selecting text, applying bold, pasting paragraphs, or typing Chinese.\n\n")));
    { xui_document_transaction t; uint64_t table;
      OK(xuiDocumentBeginTransaction(d->rich_doc, NULL, &t));
      result = xuiDocumentTxnInsertTable(t, 1, XUI_DOCUMENT_APPEND, 3, 2, 1, &table);
      if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
      xuiDocumentTxnRelease(t); if (result != XUI_OK) return result; }
    xuiWidgetSetRect(d->rich, (xui_rect_t){24,106,368,H-166}); OK(xuiWidgetAddChild(d->root, d->rich));
    OK(xuiDocumentViewSetScroll(d->rich, 0, 0));
    editor.tView.pDocument = d->md_doc; editor.iMode = XUI_DOC_SOURCE_TEXT;
    if (d->verify == 2) editor.iAsyncSourceThresholdBytes = 1;
    OK(xuiDocumentEditorCreate(d->context, &editor, &d->markdown));
    xuiWidgetSetRect(d->markdown, (xui_rect_t){424,106,390,H-166}); OK(xuiWidgetAddChild(d->root, d->markdown));
    view = editor.tView; view.pDocument = d->md_doc;
    OK(xuiDocumentViewCreate(d->context, &view, &d->preview));
    xuiWidgetSetRect(d->preview, (xui_rect_t){846,106,W-870,H-166}); OK(xuiWidgetAddChild(d->root, d->preview));
    for (i = 0; i < DEMO_BUTTON_COUNT; i++) {
        xui_widget button; xui_button_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.sText = labels[i]; desc.pFont = d->font;
        OK(xuiButtonCreate(d->context, &button, &desc)); d->buttons[i]=button; d->actions[i] = (action){d,commands[i]};
        xuiButtonSetClick(button, click, &d->actions[i]); xuiWidgetSetRect(button, (xui_rect_t){20+i*96,37,88,26}); xuiWidgetSetFocusable(button, 0);
        OK(xuiWidgetAddChild(d->root, button));
    }
    OK(arrange(d,W,H));
    if (!d->verify) OK(xuiSetFocusWidget(d->context, d->markdown));
    xgeSetQuitRequestCallback(request_close, d);
    return XUI_OK;
#undef OK
}
static const char* demo_font_path(const char* variable, const char* fallback)
{
    const char* path = getenv(variable);
    return path && *path ? path : fallback;
}
static int resources(demo* d)
{
    xui_surface_desc_t surface = {0}; xui_resource_desc_t image_resource = {0};
    xui_surface image = NULL; xui_resource decoded = NULL; int r;
    const char* primary; const char* bold; const char* italic; const char* mono;
    const char* fallback = getenv("XUI_DOCUMENT_CJK_FONT");
    if (fallback && *fallback) {
        r = xgeFontFallbackSet(fallback, 16);
        if (r != XGE_OK) { fprintf(stderr, "Cannot load configured fallback font: %s (%d)\n", fallback, r); return r; }
    }
#ifdef _WIN32
    /* Each proxy font retains its own sized copy of this fallback. */
    if ((!fallback || !*fallback) && xgeFontFallbackSet("C:\\Windows\\Fonts\\msyh.ttc", 16) != XGE_OK &&
        xgeFontFallbackSet("C:\\Windows\\Fonts\\simsun.ttc", 16) != XGE_OK)
        fprintf(stderr, "No Chinese fallback font found; provide one through xgeFontFallbackSet.\n");
    primary = demo_font_path("XUI_DOCUMENT_FONT", "C:\\Windows\\Fonts\\segoeui.ttf");
    bold = demo_font_path("XUI_DOCUMENT_FONT_BOLD", "C:\\Windows\\Fonts\\segoeuib.ttf");
    italic = demo_font_path("XUI_DOCUMENT_FONT_ITALIC", "C:\\Windows\\Fonts\\segoeuii.ttf");
    mono = demo_font_path("XUI_DOCUMENT_FONT_MONO", "C:\\Windows\\Fonts\\consola.ttf");
#else
    primary = demo_font_path("XUI_DOCUMENT_FONT", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    bold = demo_font_path("XUI_DOCUMENT_FONT_BOLD", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");
    italic = demo_font_path("XUI_DOCUMENT_FONT_ITALIC", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Oblique.ttf");
    mono = demo_font_path("XUI_DOCUMENT_FONT_MONO", "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
#endif
    d->proxy = xuiProxyXge(); if (xuiCreate(&d->context) != XUI_OK || xuiSetProxy(d->context, &d->proxy) != XUI_OK) return XUI_ERROR;
    r = d->proxy.fontLoadFile(&d->proxy, &d->font, primary, 16, XUI_FONT_FORMAT_TTF);
    if (r != XUI_OK) { fprintf(stderr, "Cannot load document font: %s (%d); set XUI_DOCUMENT_FONT.\n", primary, r); return r; }
    d->proxy.fontLoadFile(&d->proxy, &d->bold, bold, 16, XUI_FONT_FORMAT_TTF);
    d->proxy.fontLoadFile(&d->proxy, &d->italic, italic, 16, XUI_FONT_FORMAT_TTF);
    d->proxy.fontLoadFile(&d->proxy, &d->mono, mono, 15, XUI_FONT_FORMAT_TTF);
    xuiSetDefaultFont(d->context, d->font); xuiInputViewport(d->context, W, H);
    surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8; surface.iWidth = W; surface.iHeight = H;
    surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    r = d->proxy.surfaceCreate(&d->proxy, &d->target, &surface);
    if (r != XUI_OK) return r;
    surface.iWidth = 96; surface.iHeight = 56;
    r = d->proxy.surfaceCreate(&d->proxy, &image, &surface);
    if (r != XUI_OK) return r;
    r = d->proxy.surfaceClear(&d->proxy, image, XUI_COLOR_RGBA(42,190,126,255));
    if (r != XUI_OK) { d->proxy.surfaceDestroy(&d->proxy, image); return r; }
    image_resource.iSize = sizeof(image_resource); image_resource.sName = "demo.surface";
    image_resource.iKind = XUI_RESOURCE_SURFACE; image_resource.pHandle = image;
    image_resource.pUser = d; image_resource.onDestroy = image_destroy;
    r = xuiResourceSet(d->context, NULL, &image_resource);
    if (r != XUI_OK) { d->proxy.surfaceDestroy(&d->proxy, image); return r; }
    if (d->verify) {
        r = xuiDocumentImageResourceLoadFile(d->context, "demo.decoded.probe", "res/xui/assets/msgbox_info.png", NULL, &decoded);
        if (r != XUI_OK) return r;
        r = d->proxy.surfaceGetDesc(&d->proxy, (xui_surface)xuiResourceGetHandle(decoded), &surface);
        if (r != XUI_OK || surface.iWidth <= 0 || surface.iHeight <= 0) return XUI_ERROR_BACKEND_FAILED;
        r = xuiResourceRemove(decoded);
        if (r != XUI_OK) return r;
        {
            xui_doc_image_request request = NULL; unsigned attempt;
            r = xuiDocumentImageResourceLoadFileAsync(d->context, "demo.async.probe", "res/xui/assets/msgbox_info.png", NULL, &request);
            if (r != XUI_OK) return r;
            for (attempt = 0; attempt < 5000; attempt++) {
                r = xuiDocumentImageResourcePoll(request, &decoded);
                if (r != XUI_DOC_ERROR_BUSY) break;
                xgeSleep(1);
            }
            if (r == XUI_OK && decoded) {
                r = d->proxy.surfaceGetDesc(&d->proxy, (xui_surface)xuiResourceGetHandle(decoded), &surface);
                if (r == XUI_OK && surface.iWidth > 0 && surface.iHeight > 0)
                    r = xuiResourceRemove(decoded);
                else r = XUI_ERROR_BACKEND_FAILED;
            } else if (r == XUI_OK) r = XUI_ERROR_BACKEND_FAILED;
            xuiDocumentImageResourceRelease(request);
            if (r != XUI_OK) return r;
        }
    }
    return setup(d);
}
static int capture_async(demo* d, const char* path, uint64_t* source, uint64_t* preview)
{
    unsigned char* pixels = malloc((size_t)W * H * 4); int x, y, result;
    if (!pixels) return XUI_ERROR_OUT_OF_MEMORY;
    *source = *preview = UINT64_C(14695981039346656037);
    result = d->proxy.surfaceReadRGBA(&d->proxy, d->target, pixels, W * 4);
    if (result == XUI_OK) for (y = 110; y < H - 80; y++) for (x = 424; x < W - 30; x++) {
        uint64_t* hash = x < 814 ? source : x >= 846 ? preview : NULL;
        if (hash) { *hash ^= pixels[((size_t)y * W + x) * 4]; *hash *= UINT64_C(1099511628211); }
    }
    if (result == XUI_OK && path) result = xgeImageSavePNG(path, W, H, pixels, W * 4);
    free(pixels); return result;
}
static int same_document_content(xui_document first, xui_document second)
{
    xui_document_snapshot a = NULL, b = NULL; char *left = NULL, *right = NULL;
    uint64_t left_bytes = 0, right_bytes = 0; int result;
    result = xuiDocumentAcquireSnapshot(first, &a);
    if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(second, &b);
    if (result == XUI_OK) result = xuiDocumentSerialize(a, &left, &left_bytes);
    if (result == XUI_OK) result = xuiDocumentSerialize(b, &right, &right_bytes);
    if (result == XUI_OK && (left_bytes != right_bytes || memcmp(left, right, (size_t)left_bytes))) result = XUI_ERROR_INVALID_STATE;
    xuiDocumentFreeBuffer(left); xuiDocumentFreeBuffer(right);
    xuiDocumentSnapshotRelease(a); xuiDocumentSnapshotRelease(b);
    return result;
}
static int verify_files(demo* d)
{
    unsigned i; int result = XUI_OK;
    for (i = 0; i < 2 && result == XUI_OK; i++) {
        xui_widget editor = i ? d->markdown : d->rich;
        xui_document before = xuiDocumentViewGetDocument(editor), opened = NULL, reopened = NULL;
        uint32_t format = i ? XUI_DOC_FILE_MARKDOWN : XUI_DOC_FILE_NATIVE;
        char path[256];
#ifdef _WIN32
        unsigned long process = GetCurrentProcessId();
#elif defined(__unix__) || defined(__APPLE__)
        unsigned long process = (unsigned long)getpid();
#else
        unsigned long process = 0;
#endif
        snprintf(path, sizeof(path), "artifacts/xui-document-rebuild/demo-%lu-中文-%s", process, i ? "markdown.md" : "rich.xdoc");
        xuiDocumentRetain(before);
        result = xuiDocumentSaveFile(before, path, format);
        if (result == XUI_OK && xuiDocumentIsDirty(before)) result = XUI_ERROR_INVALID_STATE;
        if (result == XUI_OK) result = xuiDocumentOpenFile(path, format, NULL, 0, &opened);
        if (result == XUI_OK) result = same_document_content(before, opened);
        if (result == XUI_OK && xuiDocumentIsDirty(opened)) result = XUI_ERROR_INVALID_STATE;
        if (result == XUI_OK) result = replace_document(d, editor, opened);
        if (result == XUI_OK && (xuiDocumentViewGetDocument(editor) != opened || (i && xuiDocumentViewGetDocument(d->preview) != opened))) result = XUI_ERROR_INVALID_STATE;
        if (result == XUI_OK) result = xuiDocumentEditorInsertText(editor, "中文 edited\n", strlen("中文 edited\n"));
        if (result == XUI_OK && !xuiDocumentIsDirty(opened)) result = XUI_ERROR_INVALID_STATE;
        if (result == XUI_OK) result = xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO);
        if (result == XUI_OK) result = same_document_content(before, opened);
        if (result == XUI_OK && xuiDocumentIsDirty(opened)) result = XUI_ERROR_INVALID_STATE;
        if (result == XUI_OK) result = xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO);
        if (result == XUI_OK) result = xuiDocumentSaveFile(opened, path, format);
        if (result == XUI_OK) result = xuiDocumentOpenFile(path, format, NULL, 0, &reopened);
        if (result == XUI_OK) result = same_document_content(opened, reopened);
        if (result == XUI_OK) result = replace_document(d, editor, reopened);
        if (result == XUI_OK) {
            xui_document invalid = NULL;
            int failure = xuiDocumentOpenFile("artifacts/xui-document-rebuild/nonexistent-folder/no-file", format, NULL, 0, &invalid);
            if (failure != XUI_DOC_ERROR_IO || invalid || xuiDocumentViewGetDocument(editor) != reopened ||
                (i && xuiDocumentViewGetDocument(d->preview) != reopened)) result = XUI_ERROR_INVALID_STATE;
            xuiDocumentRelease(invalid);
        }
        xuiDocumentRelease(before); xuiDocumentRelease(opened); xuiDocumentRelease(reopened);
        if (result == XUI_OK) {
#ifdef _WIN32
            wchar_t native_path[256];
            if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, native_path, 256) || !DeleteFileW(native_path)) result = XUI_DOC_ERROR_IO;
#else
            if (remove(path)) result = XUI_DOC_ERROR_IO;
#endif
        }
    }
    return result;
}
static int verify_async(demo* d)
{
    static const char prefix[] = "Pending source input; preview updates after publication.\n\n";
    uint64_t source_hash = 0, preview_hash = 0; int result = XUI_OK;
    if (d->frames == 1) {
        result = capture_async(d, NULL, &d->async_source_pixels, &d->async_preview_pixels);
        d->async_revision = xuiDocumentGetRevision(d->md_doc);
        if (result == XUI_OK) result = xuiDocumentEditorInsertText(d->markdown, prefix, sizeof(prefix) - 1);
    } else if (d->frames == 2) {
        xui_doc_prepare_info_t info = {0}; xui_doc_range_t range; info.iSize = sizeof(info);
        result = xuiDocumentEditorGetPendingInput(d->markdown, &info);
        if (result == XUI_OK) result = xuiDocumentViewGetSelection(d->markdown, &range);
        if (result == XUI_OK && (xuiDocumentGetRevision(d->md_doc) != d->async_revision || !range.tCaret.iInputGeneration)) result = XUI_ERROR_INVALID_STATE;
        if (result == XUI_OK) result = capture_async(d, "artifacts/xui-document-rebuild/native-async-pending.png", &source_hash, &preview_hash);
        if (result == XUI_OK && (source_hash == d->async_source_pixels || preview_hash != d->async_preview_pixels)) result = XUI_ERROR_BACKEND_FAILED;
        d->async_pending_pixels = source_hash;
    } else if (d->frames > 2 && !xuiDocumentHasPrepare(d->md_doc)) {
        xui_document_snapshot snapshot = NULL; char buffer[2048]; uint64_t bytes;
        result = capture_async(d, "artifacts/xui-document-rebuild/native-async-committed.png", &source_hash, &preview_hash);
        if (result == XUI_OK && (source_hash != d->async_pending_pixels || preview_hash == d->async_preview_pixels ||
            xuiDocumentGetRevision(d->md_doc) != d->async_revision + 1)) result = XUI_ERROR_BACKEND_FAILED;
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d->md_doc, &snapshot);
        if (result == XUI_OK) result = xuiDocumentSnapshotCopySource(snapshot, buffer, sizeof(buffer), &bytes);
        if (result == XUI_OK && (strncmp(buffer, prefix, sizeof(prefix) - 1) || strcmp(buffer + sizeof(prefix) - 1, md_text))) result = XUI_ERROR_INVALID_STATE;
        xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
        if (result == XUI_OK) result = xuiDocumentEditorExecute(d->markdown, XUI_DOC_EDIT_UNDO);
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d->md_doc, &snapshot);
        if (result == XUI_OK) result = xuiDocumentSnapshotCopySource(snapshot, buffer, sizeof(buffer), &bytes);
        if (result == XUI_OK && strcmp(buffer, md_text)) result = XUI_ERROR_INVALID_STATE;
        xuiDocumentSnapshotRelease(snapshot); d->result = result; xgeQuit();
    } else if (d->frames > 600) result = XUI_DOC_ERROR_BUSY;
    return result;
}
static int frame(void* user)
{
    demo* d = user; xui_rect_i_t damage = {0,0,W,H}; int r;
#ifdef _WIN32
    if (d->verify && !d->frames) ShowWindow((HWND)xgePlatformNativeHandle(), SW_HIDE);
#endif
    if (!d->target) { r = resources(d); if (r != XUI_OK) { fprintf(stderr, "Document resources failed: %d\n", r); goto failed; } }
    if (!d->verify) { r=resize_target(d,xgeGetWidth(),xgeGetHeight()); if(r!=XUI_OK) goto failed; }
    damage=(xui_rect_i_t){0,0,d->width,d->height};
    r = xgeBegin(); if (r != XGE_OK) goto failed;
    if (!d->verify) {
        xui_widget focused = xuiGetFocusWidget(d->context);
        if (focused == d->rich || focused == d->markdown) d->active_editor = focused;
        xuiProxyXgePumpInputRect(d->context, (xui_rect_t){0,0,d->width,d->height}); xuiDispatchPendingEvents(d->context);
    }
    r = xuiLayout(d->context); if (r != XUI_OK) goto failed;
    r = xuiUpdate(d->context, .016f); if (r != XUI_OK) goto failed;
    if (!d->verify) {
        r = service_close(d);
        if (r != XUI_OK) { show_error(d->active_editor, r, d); r = XUI_OK; }
    }
    d->proxy.surfaceClear(&d->proxy, d->target, XUI_COLOR_WHITE);
    r = xuiRender(d->context, d->target, &damage, 1); if (r != XUI_OK) goto failed;
    if (!d->verify) { xgeClear(XUI_COLOR_WHITE); d->proxy.surfaceDraw(&d->proxy, d->target, (xui_rect_t){0,0,d->width,d->height}, (xui_rect_t){0,0,d->width,d->height}, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE); }
    r = xgeEnd();
    if (r != XGE_OK) goto failed;
    d->frames++;
    if (d->verify == 2) { r = verify_async(d); if (r != XUI_OK) goto failed; return r; }
    if (d->verify == 4 && d->frames == 1) {
        r = verify_files(d); if (r != XUI_OK) goto failed;
    }
    if (d->verify == 4 && d->frames == 3) {
        uint64_t source, preview;
        r = capture_async(d, "artifacts/xui-document-rebuild/native-files.png", &source, &preview);
        d->result = r; xgeQuit(); return r;
    }
    if (d->verify == 4) return r;
    if (d->verify == 3 && d->frames == 3) {
        unsigned char* pixels = malloc((size_t)W * H * 4);
        unsigned background = 0, border = 0; int x, y;
        if (!pixels) r = XUI_ERROR_OUT_OF_MEMORY;
        else {
            r = d->proxy.surfaceReadRGBA(&d->proxy, d->target, pixels, W * 4);
            if (r == XUI_OK) {
                for (y = 110; y < H - 80; y++) for (x = 846; x < W - 30; x++) {
                    const unsigned char* p = pixels + ((size_t)y * W + x) * 4;
                    if (p[3] > 200 && p[0] == 238 && p[1] == 241 && p[2] == 245) background++;
                    if (p[3] > 200 && p[0] == 170 && p[1] == 180 && p[2] == 192) border++;
                }
                if (background < 200 || border < 10) {
                    fprintf(stderr, "Missing image placeholder pixels: background=%u border=%u\n",
                        background, border);
                    r = XUI_ERROR_BACKEND_FAILED;
                }
            }
            if (r == XUI_OK) r = xgeImageSavePNG(
                "artifacts/xui-document-rebuild/native-image-placeholder.png",
                W, H, pixels, W * 4);
            free(pixels);
        }
        d->result = r; xgeQuit(); return r;
    }
    if ((d->frames == 3 || d->frames == 6) && d->verify) {
        unsigned char* pixels = malloc((size_t)W*H*4); unsigned counts[3] = {0}, image_pixels = 0; int x, y; uint64_t hash = UINT64_C(14695981039346656037);
        if (!pixels) r = XUI_ERROR_OUT_OF_MEMORY;
        else {
            if (r == XUI_OK) r = d->proxy.surfaceReadRGBA(&d->proxy, d->target, pixels, W*4);
            if (r == XUI_OK) {
                for (y = 110; y < H-80; y++) for (x = 24; x < W-30; x++) {
                    unsigned char* p = pixels + ((size_t)y*W+x)*4;
                    if (p[3] > 200 && p[0] < 140 && p[1] < 140 && p[2] < 140) counts[x < 410 ? 0 : x < 836 ? 1 : 2]++;
                    if (x >= 846 && p[0] < 80 && p[1] > 150 && p[2] > 90 && p[2] < 170) image_pixels++;
                    if (x >= 424 && x < 814) { hash ^= p[0]; hash *= UINT64_C(1099511628211); }
                }
                if (counts[0] < 200 || counts[1] < 200 || counts[2] < 200) { fprintf(stderr, "Missing text pixels: %u, %u, %u\n", counts[0], counts[1], counts[2]); r = XUI_ERROR_BACKEND_FAILED; }
                if (image_pixels < 1000) { fprintf(stderr, "Missing native image pixels: %u\n", image_pixels); r = XUI_ERROR_BACKEND_FAILED; }
            }
            if (r == XUI_OK) r = xgeImageSavePNG(d->frames == 3 ? "artifacts/xui-document-rebuild/native-smoke.png" :
                "artifacts/xui-document-rebuild/native-live-smoke.png", W, H, pixels, W*4);
            free(pixels);
        }
        if (r == XUI_OK && d->frames == 3) {
            d->source_pixels = hash; r = xuiDocumentViewSetMode(d->markdown, XUI_DOC_LIVE_MARKDOWN);
        } else if (r == XUI_OK && hash == d->source_pixels) { fprintf(stderr, "Live projection did not change the source editor pixels.\n"); r = XUI_ERROR_BACKEND_FAILED; }
        if (r != XUI_OK || d->frames == 6) { d->result = r; xgeQuit(); }
    }
    return r;
failed:
    d->result = r; xgeQuit(); return r;
}
int main(int argc, char** argv)
{
    demo d = {0}; xge_desc_t desc = {0}; int r;
    d.verify = argc > 1 && !strcmp(argv[1], "--verify");
    if (argc > 1 && !strcmp(argv[1], "--verify-async")) d.verify = 2;
    if (argc > 1 && !strcmp(argv[1], "--verify-image-placeholder")) d.verify = 3;
    if (argc > 1 && !strcmp(argv[1], "--verify-files")) d.verify = 4;
    desc.iWidth = W; desc.iHeight = H; desc.sTitle = "XUI Unified Document"; desc.iFlags = d.verify ? XGE_INIT_OFFSCREEN : XGE_INIT_VSYNC;
    desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 1;
    r = xgeRun(frame, &d); if (r == XUI_OK) r = d.result;
    xgeSetQuitRequestCallback(NULL, NULL);
    if (d.file_dialog) xuiFileDialogDestroy(d.file_dialog);
    if (d.discard_box) xuiMsgBoxDestroy(d.discard_box);
    if (d.close_box) xuiMsgBoxDestroy(d.close_box);
    xuiDocumentRelease(d.pending_open);
    xuiDestroy(d.context); xuiDocumentRelease(d.rich_doc); xuiDocumentRelease(d.md_doc);
    if (d.proxy.surfaceDestroy) d.proxy.surfaceDestroy(&d.proxy, d.target);
    if (d.proxy.fontDestroy) { d.proxy.fontDestroy(&d.proxy, d.mono); d.proxy.fontDestroy(&d.proxy, d.italic); d.proxy.fontDestroy(&d.proxy, d.bold); d.proxy.fontDestroy(&d.proxy, d.font); }
    xgeFontFallbackClear();
    xgeUnit();
    if (r != XUI_OK) fprintf(stderr, "Document demo failed: %d\n", r);
    else if (d.verify == 2) puts("Native XGE async Document: source pixels change before publication, preview pixels change only after publication, shared Undo passed.");
    else if (d.verify == 3) puts("Native XGE missing-image placeholder: background and border pixels passed.");
    else if (d.verify == 4) puts("Native XGE files: Rich and Markdown save/reopen, shared preview, Unicode edit, Undo/Redo, dirty state and failed-open preservation passed.");
    else if (d.verify) puts("Native XGE Document rendering passed; captured native-smoke.png and native-live-smoke.png.");
    return r == XUI_OK ? 0 : 1;
}
