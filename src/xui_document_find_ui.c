#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_EDITOR
#include "xui_document_find_ui.h"
#include "xui_document_view_internal.h"
#include "xui_internal.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The window owns only widgets. Matches and highlights remain in DocumentView;
 * editing goes through DocumentEditor and its normal transaction history. */
struct doc_find_ui {
    xui_widget editor, window, find, replacement, previous, next, all;
    xui_widget replace_one, replace_all, case_sensitive, whole_word, regex;
    xui_widget selection_only, results, status;
    uint64_t count, revision, identity, language_revision;
    uint32_t mode;
    int replace_mode, dirty, invalid_pattern;
    char position[48], preview[192];
};

static int doc_find_ui_refresh(doc_find_ui* ui);
static void doc_find_ui_action(doc_find_ui* ui, xui_widget button);

static uint32_t doc_find_ui_flags(const doc_find_ui* ui)
{
    uint32_t flags = 0;
    if (!xuiCheckBoxGetChecked(ui->case_sensitive)) flags |= XUI_DOC_FIND_IGNORE_CASE;
    if (xuiCheckBoxGetChecked(ui->whole_word)) flags |= XUI_DOC_FIND_WHOLE_WORD;
    if (xuiCheckBoxGetChecked(ui->regex)) flags |= XUI_DOC_FIND_REGEX;
    return flags;
}

static void doc_find_ui_status(doc_find_ui* ui, int result, uint64_t count, int replaced)
{
    xui_context context;
    char buffer[96];
    const char* message;
    if (!ui || !xuiInternalWidgetIsValid(ui->editor)) return;
    context = xuiWidgetGetContext(ui->editor);
    if (result == XUI_OK) {
        snprintf(buffer, sizeof(buffer), xuiTranslate(context,
            replaced ? XUI_TR_FIND_REPLACED_FMT : XUI_TR_FIND_MATCHES_FMT),
            (int)(count > INT_MAX ? INT_MAX : count));
        message = buffer;
    } else if (result == XUI_ERROR_NOT_FOUND) {
        message = xuiTranslate(context, XUI_TR_FIND_NOT_FOUND);
    } else {
        message = xuiTranslate(context, XUI_TR_FIND_INVALID_PATTERN);
    }
    (void)xuiLabelSetText(ui->status, message);
}

static void doc_find_ui_language(doc_find_ui* ui)
{
    xui_context c = xuiWidgetGetContext(ui->editor);
    xui_table_view_column_t columns[2] = {0};
    ui->language_revision = xuiGetLanguageRevision(c);
    (void)xuiWindowSetTitle(ui->window, xuiTranslate(c,
        ui->replace_mode ? XUI_TR_REPLACE_TITLE : XUI_TR_FIND_TITLE));
    (void)xuiInputSetPlaceholder(ui->find, xuiTranslate(c, XUI_TR_FIND_PLACEHOLDER));
    (void)xuiInputSetPlaceholder(ui->replacement, xuiTranslate(c, XUI_TR_REPLACE_PLACEHOLDER));
    (void)xuiButtonSetText(ui->previous, xuiTranslate(c, XUI_TR_FIND_PREVIOUS));
    (void)xuiButtonSetText(ui->next, xuiTranslate(c, XUI_TR_FIND_NEXT));
    (void)xuiButtonSetText(ui->all, xuiTranslate(c, XUI_TR_FIND_ALL));
    (void)xuiButtonSetText(ui->replace_one, xuiTranslate(c, XUI_TR_REPLACE_CURRENT));
    (void)xuiButtonSetText(ui->replace_all, xuiTranslate(c, XUI_TR_REPLACE_ALL));
    (void)xuiCheckBoxSetText(ui->case_sensitive, xuiTranslate(c, XUI_TR_FIND_CASE));
    (void)xuiCheckBoxSetText(ui->whole_word, xuiTranslate(c, XUI_TR_FIND_WORD));
    (void)xuiCheckBoxSetText(ui->regex, xuiTranslate(c, XUI_TR_FIND_REGEX));
    (void)xuiCheckBoxSetText(ui->selection_only, xuiTranslate(c, XUI_TR_FIND_SELECTION));
    columns[0].sTitle = xuiTranslate(c, XUI_TR_FIND_COL_POSITION);
    columns[0].fWidth = 88; columns[0].fMinWidth = 64;
    columns[1].sTitle = xuiTranslate(c, XUI_TR_FIND_COL_CONTENT);
    columns[1].fWidth = 480; columns[1].fMinWidth = 160;
    columns[0].bVisibleSet = columns[1].bVisibleSet = 1;
    columns[0].bVisible = columns[1].bVisible = 1;
    (void)xuiTableViewSetColumns(ui->results, columns, 2);
    if (ui->invalid_pattern) doc_find_ui_status(ui, XUI_ERROR_INVALID_ARGUMENT, 0, 0);
    else if (ui->count) doc_find_ui_status(ui, XUI_OK, ui->count, 0);
}

static void doc_find_ui_layout(doc_find_ui* ui)
{
    float options_y = ui->replace_mode ? 92.0f : 52.0f;
    float y = options_y + 32.0f;
    xui_rect_t rect = xuiWidgetGetRect(ui->window);
    rect.fW = 640; rect.fH = ui->replace_mode ? 388 : 348;
    (void)xuiWidgetSetRect(ui->window, rect);
    (void)xuiWidgetSetRect(ui->find, (xui_rect_t){12, 12, 290, 28});
    (void)xuiWidgetSetRect(ui->previous, (xui_rect_t){312, 12, 86, 28});
    (void)xuiWidgetSetRect(ui->next, (xui_rect_t){404, 12, 80, 28});
    (void)xuiWidgetSetRect(ui->all, (xui_rect_t){490, 12, 128, 28});
    (void)xuiWidgetSetVisible(ui->replacement, ui->replace_mode);
    (void)xuiWidgetSetVisible(ui->replace_one, ui->replace_mode);
    (void)xuiWidgetSetVisible(ui->replace_all, ui->replace_mode);
    if (ui->replace_mode) {
        (void)xuiWidgetSetRect(ui->replacement, (xui_rect_t){12, 52, 290, 28});
        (void)xuiWidgetSetRect(ui->replace_one, (xui_rect_t){312, 52, 126, 28});
        (void)xuiWidgetSetRect(ui->replace_all, (xui_rect_t){444, 52, 174, 28});
    }
    (void)xuiWidgetSetRect(ui->case_sensitive, (xui_rect_t){12, options_y, 150, 24});
    (void)xuiWidgetSetRect(ui->whole_word, (xui_rect_t){168, options_y, 126, 24});
    (void)xuiWidgetSetRect(ui->regex, (xui_rect_t){300, options_y, 110, 24});
    (void)xuiWidgetSetRect(ui->selection_only, (xui_rect_t){416, options_y, 202, 24});
    (void)xuiWidgetSetRect(ui->results, (xui_rect_t){12, y, 606, 194});
    (void)xuiWidgetSetRect(ui->status, (xui_rect_t){12, y + 202, 606, 24});
}

static void doc_find_ui_readonly(doc_find_ui* ui)
{
    int enabled = !xuiDocumentEditorGetReadOnly(ui->editor);
    (void)xuiWidgetSetEnabled(ui->replace_one, enabled);
    (void)xuiWidgetSetEnabled(ui->replace_all, enabled);
}

static int doc_find_ui_count(xui_widget widget, void* user)
{
    doc_find_ui* ui = user; (void)widget;
    return ui->count > INT_MAX ? INT_MAX : (int)ui->count;
}

static int doc_find_ui_cell(xui_widget widget, int row, int column,
    xui_table_view_cell_t* cell, void* user)
{
    doc_find_ui* ui = user;
    xui_doc_range_t range;
    xui_document_snapshot snapshot = NULL;
    uint64_t length = 0, begin, end;
    int result;
    (void)widget;
    if (!cell || row < 0 || (uint64_t)row >= ui->count) return 0;
    if (column == 0) {
        snprintf(ui->position, sizeof(ui->position), "%d", row + 1);
        cell->sText = ui->position; return 1;
    }
    if (column != 1 || !xuiInternalWidgetIsValid(ui->editor) ||
        xuiDocumentViewGetFindResult(ui->editor, (uint64_t)row, &range, NULL) != XUI_OK)
        return 0;
    ui->preview[0] = 0;
    result = xuiDocumentAcquireSnapshot(xuiDocumentViewGetDocument(ui->editor), &snapshot);
    if (result != XUI_OK) return 0;
    if (range.tAnchor.iKind == XUI_DOC_POSITION_SOURCE) {
        result = xuiDocumentSnapshotCopySource(snapshot, NULL, 0, &length);
    } else {
        result = xuiDocumentSnapshotCopyText(snapshot, range.tAnchor.iNodeId, NULL, 0, &length);
    }
    if (result == XUI_OK) {
        begin = range.tAnchor.iOffset > 32 ? range.tAnchor.iOffset - 32 : 0;
        if (begin > length) begin = length;
        end = length - begin > sizeof(ui->preview) - 1 ? begin + sizeof(ui->preview) - 1 : length;
        /* Keep the UTF-8 slice on codepoint boundaries. */
        while (begin < end) {
            unsigned char byte = 0;
            result = range.tAnchor.iKind == XUI_DOC_POSITION_SOURCE ?
                xuiDocumentSnapshotReadSource(snapshot, begin, &byte, 1) :
                xuiDocumentSnapshotReadText(snapshot, range.tAnchor.iNodeId, begin, &byte, 1);
            if (result != XUI_OK || (byte & 0xc0) != 0x80) break;
            begin++;
        }
        if (result == XUI_OK && end > begin) {
            result = range.tAnchor.iKind == XUI_DOC_POSITION_SOURCE ?
                xuiDocumentSnapshotReadSource(snapshot, begin, ui->preview, end - begin) :
                xuiDocumentSnapshotReadText(snapshot, range.tAnchor.iNodeId, begin, ui->preview, end - begin);
            if (result == XUI_OK) {
                size_t n = (size_t)(end - begin), i;
                if (end < length) {
                    unsigned char next = 0;
                    result = range.tAnchor.iKind == XUI_DOC_POSITION_SOURCE ?
                        xuiDocumentSnapshotReadSource(snapshot, end, &next, 1) :
                        xuiDocumentSnapshotReadText(snapshot, range.tAnchor.iNodeId, end, &next, 1);
                    if (result == XUI_OK && (next & 0xc0) == 0x80) {
                        while (n && ((unsigned char)ui->preview[n - 1] & 0xc0) == 0x80) n--;
                        if (n) n--;
                    }
                }
                ui->preview[n] = 0;
                for (i = 0; i < n; i++) if (ui->preview[i] == '\n' || ui->preview[i] == '\r' || ui->preview[i] == '\t') ui->preview[i] = ' ';
            }
        }
    }
    xuiDocumentSnapshotRelease(snapshot);
    cell->sText = result == XUI_OK ? ui->preview : xuiInputGetText(ui->find);
    return 1;
}

static void doc_find_ui_select(xui_widget widget, int row, int column,
    int selection_mode, void* user)
{
    doc_find_ui* ui = user;
    (void)widget; (void)column; (void)selection_mode;
    if (row >= 0 && xuiInternalWidgetIsValid(ui->editor))
        (void)xuiDocumentViewActivateFindResult(ui->editor, (uint64_t)row, NULL);
}

static void doc_find_ui_change(xui_widget widget, const char* text, void* user)
{
    doc_find_ui* ui = user; (void)widget; (void)text;
    ui->dirty = 1;
    (void)doc_find_ui_refresh(ui);
}

static void doc_find_ui_option_change(xui_widget widget, int checked, void* user)
{
    doc_find_ui* ui = user;
    if (widget == ui->selection_only) {
        int result;
        if (checked) {
            xui_doc_range_t selection;
            result = xuiDocumentViewGetSelection(ui->editor, &selection);
            if (result == XUI_OK)
                result = xuiDocumentViewSetFindScope(ui->editor, &selection);
            if (result != XUI_OK) {
                (void)xuiCheckBoxSetChecked(widget, 0);
                doc_find_ui_status(ui, XUI_ERROR_NOT_FOUND, 0, 0);
            }
        } else (void)xuiDocumentViewSetFindScope(ui->editor, NULL);
    }
    ui->dirty = 1;
    (void)doc_find_ui_refresh(ui);
}

static void doc_find_ui_clear_query(doc_find_ui* ui)
{
    doc_view_data* view = doc_view_get(ui->editor);
    if (!view) return;
    doc_view_find_query_clear(view);
    (void)xuiWidgetInvalidate(ui->editor,
        XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void doc_find_ui_click(xui_widget button, void* user)
{
    doc_find_ui_action(user, button);
}

static void doc_find_ui_close(xui_widget window, void* user)
{
    doc_find_ui* ui = user; (void)window;
    if (xuiInternalWidgetIsValid(ui->editor))
        (void)xuiSetFocusWidget(xuiWidgetGetContext(ui->editor), ui->editor);
}

static int doc_find_ui_key(xui_widget widget, const xui_event_t* event, void* user)
{
    doc_find_ui* ui = user; int key;
    (void)widget;
    if (!event || event->iType != XUI_EVENT_KEY_DOWN) return XUI_OK;
    key = event->iKey;
    if (key >= 'a' && key <= 'z') key += 'A' - 'a';
    if (event->iModifiers & XUI_MOD_CTRL) {
        if (key == 'F' || key == 'H') {
            ui->replace_mode = key == 'H'; doc_find_ui_layout(ui); doc_find_ui_language(ui);
            (void)xuiSetFocusWidget(xuiWidgetGetContext(ui->editor), ui->find);
            return XUI_EVENT_DISPATCH_STOP;
        }
    }
    if (key == XUI_KEY_ENTER || key == XUI_KEY_F3) {
        doc_find_ui_action(ui, event->iModifiers & XUI_MOD_SHIFT ? ui->previous : ui->next);
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (key == XUI_KEY_ESCAPE) {
        (void)xuiWindowSetOpen(ui->window, 0);
        if (xuiInternalWidgetIsValid(ui->editor))
            (void)xuiSetFocusWidget(xuiWidgetGetContext(ui->editor), ui->editor);
        return XUI_EVENT_DISPATCH_STOP;
    }
    return XUI_OK;
}

static int doc_find_ui_refresh(doc_find_ui* ui)
{
    const char* pattern;
    xui_document document;
    xui_widget editor;
    uint64_t count = 0;
    int result;
    if (!ui || !xuiInternalWidgetIsValid(ui->editor)) return XUI_ERROR_INVALID_STATE;
    editor = ui->editor;
    pattern = xuiInputGetText(ui->find);
    if (xuiCheckBoxGetChecked(ui->selection_only)) {
        xui_doc_range_t scope;
        if (xuiDocumentViewGetFindScope(editor, &scope) != XUI_OK) {
            doc_find_ui_clear_query(ui);
            ui->count = 0; ui->dirty = 0;
            (void)xuiTableViewRefreshAdapter(ui->results);
            doc_find_ui_status(ui, XUI_ERROR_NOT_FOUND, 0, 0);
            return XUI_ERROR_NOT_FOUND;
        }
    }
    if (!pattern || !*pattern) {
        doc_find_ui_clear_query(ui); result = XUI_OK;
    } else result = xuiDocumentViewSetFindQueryEx(editor, pattern,
        (uint64_t)strlen(pattern), doc_find_ui_flags(ui));
    if (!xuiInternalWidgetIsValid(editor)) return XUI_ERROR_INVALID_STATE;
    if (result == XUI_ERROR_INVALID_ARGUMENT) {
        doc_find_ui_clear_query(ui);
        ui->count = 0; ui->dirty = 0; ui->invalid_pattern = 1;
        (void)xuiTableViewRefreshAdapter(ui->results);
        doc_find_ui_status(ui, result, 0, 0);
        return result;
    }
    if (result != XUI_OK) return result;
    if (pattern && *pattern) result = xuiDocumentViewGetFindResultCount(ui->editor, &count);
    if (result != XUI_OK) return result;
    ui->count = count; ui->dirty = 0; ui->invalid_pattern = 0;
    document = xuiDocumentViewGetDocument(ui->editor);
    ui->identity = xuiDocumentGetIdentity(document);
    ui->revision = xuiDocumentGetRevision(document);
    ui->mode = xuiDocumentViewGetMode(ui->editor);
    (void)xuiTableViewRefreshAdapter(ui->results);
    if (pattern && *pattern) doc_find_ui_status(ui, XUI_OK, count, 0);
    else (void)xuiLabelSetText(ui->status, "");
    return XUI_OK;
}

static void doc_find_ui_action(doc_find_ui* ui, xui_widget button)
{
    const char *pattern, *replacement;
    xui_widget editor;
    xui_doc_range_t scope;
    const xui_doc_range_t* scope_ptr = NULL;
    uint64_t count = 0;
    xui_doc_range_t match;
    int result;
    if (!ui || !xuiInternalWidgetIsValid(ui->editor)) return;
    editor = ui->editor;
    pattern = xuiInputGetText(ui->find);
    if (!pattern || !*pattern) return;
    replacement = xuiInputGetText(ui->replacement);
    if (!replacement) replacement = "";
    result = doc_find_ui_refresh(ui);
    if (!xuiInternalWidgetIsValid(editor) || result != XUI_OK) return;
    if (xuiCheckBoxGetChecked(ui->selection_only)) {
        result = xuiDocumentViewGetFindScope(editor, &scope);
        if (result != XUI_OK) return;
        scope_ptr = &scope;
    }
    { uint32_t replace_flags = doc_find_ui_flags(ui);
      if (replace_flags & XUI_DOC_FIND_REGEX)
          replace_flags |= XUI_DOC_REPLACE_EXPAND;
    if (button == ui->previous || button == ui->next) {
        result = xuiDocumentViewFindEx(ui->editor, pattern, (uint64_t)strlen(pattern),
            doc_find_ui_flags(ui), button == ui->previous, 1, &match);
    } else if (button == ui->replace_one) {
        result = xuiDocumentEditorReplaceCurrentEx(ui->editor, pattern,
            (uint64_t)strlen(pattern), replacement, (uint64_t)strlen(replacement),
            replace_flags);
        if (result == XUI_ERROR_NOT_FOUND) {
            result = xuiDocumentViewFindEx(ui->editor, pattern, (uint64_t)strlen(pattern),
                doc_find_ui_flags(ui), 0, 1, &match);
            if (result == XUI_OK) result = xuiDocumentEditorReplaceCurrentEx(ui->editor, pattern,
                (uint64_t)strlen(pattern), replacement, (uint64_t)strlen(replacement),
                replace_flags);
        }
    } else if (button == ui->replace_all) {
        result = xuiDocumentEditorReplaceAllEx(ui->editor, pattern, (uint64_t)strlen(pattern),
            replacement, (uint64_t)strlen(replacement), replace_flags, scope_ptr, &count);
    } else result = XUI_OK;
    }
    if (!xuiInternalWidgetIsValid(editor)) return;
    if (result == XUI_OK) {
        ui->dirty = 1;
        result = doc_find_ui_refresh(ui);
        if (!xuiInternalWidgetIsValid(editor) || result != XUI_OK) return;
        if (button == ui->replace_all) doc_find_ui_status(ui, XUI_OK, count, 1);
        else {
            uint64_t i;
            for (i = 0; i < ui->count && i < INT_MAX; i++) {
                int active = 0;
                if (xuiDocumentViewGetFindResult(ui->editor, i, &match, &active) == XUI_OK && active) {
                    (void)xuiTableViewSetSelectedRow(ui->results, (int)i);
                    (void)xuiTableViewEnsureCellVisible(ui->results, (int)i, 0);
                    break;
                }
            }
        }
    } else doc_find_ui_status(ui, result, 0, 0);
}

static int doc_find_ui_add(xui_widget window, xui_widget child)
{
    int result = xuiWindowAddChild(window, child);
    if (result != XUI_OK) xuiWidgetDestroy(child);
    return result;
}

static int doc_find_ui_button(doc_find_ui* ui, xui_widget* out, int text_id)
{
    xui_context c = xuiWidgetGetContext(ui->editor);
    xui_button_desc_t desc = {0}; int result;
    desc.iSize = sizeof(desc); desc.pFont = xuiGetDefaultFont(c);
    desc.sText = xuiTranslate(c, text_id); desc.fBorderWidth = 1;
    result = xuiButtonCreate(c, out, &desc);
    if (result == XUI_OK) result = doc_find_ui_add(ui->window, *out);
    if (result == XUI_OK) result = xuiButtonSetClick(*out, doc_find_ui_click, ui);
    return result;
}

static int doc_find_ui_checkbox(doc_find_ui* ui, xui_widget* out,
    int text_id, int checked)
{
    xui_context c = xuiWidgetGetContext(ui->editor);
    xui_checkbox_desc_t desc = {0}; int result;
    desc.iSize = sizeof(desc); desc.pFont = xuiGetDefaultFont(c);
    desc.sText = xuiTranslate(c, text_id); desc.bChecked = checked;
    desc.fIndicatorSize = 14; desc.fGap = 4;
    result = xuiCheckBoxCreate(c, out, &desc);
    if (result == XUI_OK) result = doc_find_ui_add(ui->window, *out);
    if (result == XUI_OK) result = xuiCheckBoxSetChange(*out,
        doc_find_ui_option_change, ui);
    return result;
}

static int doc_find_ui_create(xui_widget editor, doc_find_ui** out)
{
    xui_context c = xuiWidgetGetContext(editor);
    xui_widget root = xuiGetRootWidget(c), client;
    xui_window_desc_t wd = {0}; xui_input_desc_t id = {0};
    xui_table_view_desc_t td = {0}; xui_label_desc_t ld = {0};
    doc_find_ui* ui; int result;
    if (!root) return XUI_ERROR_NOT_INITIALIZED;
    ui = calloc(1, sizeof(*ui)); if (!ui) return XUI_ERROR_OUT_OF_MEMORY;
    ui->editor = editor; ui->dirty = 1;
    wd.iSize = sizeof(wd); wd.sTitle = xuiTranslate(c, XUI_TR_FIND_TITLE);
    wd.pFont = xuiGetDefaultFont(c); wd.bClosed = 1; wd.bTopMost = 1;
    wd.bHideCollapse = wd.bHideMaximize = wd.bNotResizable = 1;
    wd.fTitleBarHeight = 28; wd.fBorderWidth = 1; wd.fButtonSize = 18;
    result = xuiWindowCreate(c, &ui->window, &wd);
    if (result != XUI_OK) goto fail;
    result = xuiWidgetAddChild(root, ui->window);
    if (result != XUI_OK) goto fail;
    client = xuiWindowGetClientWidget(ui->window);
    (void)xuiWidgetSetLayoutType(client, XUI_LAYOUT_MANUAL);
    (void)xuiWidgetSetFlowMode(client, XUI_FLOW_ABSOLUTE);
    (void)xuiWidgetSetPadding(client, (xui_thickness_t){0, 0, 0, 0});
    (void)xuiWidgetSetGap(client, 0);
    (void)xuiWindowSetClose(ui->window, doc_find_ui_close, ui);
    (void)xuiWidgetSetEventHandler(client, XUI_EVENT_KEY_DOWN, doc_find_ui_key, ui);
    id.iSize = sizeof(id); id.pFont = xuiGetDefaultFont(c); id.fBorderWidth = 1;
    id.sPlaceholder = xuiTranslate(c, XUI_TR_FIND_PLACEHOLDER);
    result = xuiInputCreate(c, &ui->find, &id);
    if (result == XUI_OK) result = doc_find_ui_add(ui->window, ui->find);
    if (result != XUI_OK) goto fail;
    id.sPlaceholder = xuiTranslate(c, XUI_TR_REPLACE_PLACEHOLDER);
    result = xuiInputCreate(c, &ui->replacement, &id);
    if (result == XUI_OK) result = doc_find_ui_add(ui->window, ui->replacement);
    if (result != XUI_OK) goto fail;
    (void)xuiInputSetChange(ui->find, doc_find_ui_change, ui);
    result = doc_find_ui_button(ui, &ui->previous, XUI_TR_FIND_PREVIOUS);
    if (result == XUI_OK) result = doc_find_ui_button(ui, &ui->next, XUI_TR_FIND_NEXT);
    if (result == XUI_OK) result = doc_find_ui_button(ui, &ui->all, XUI_TR_FIND_ALL);
    if (result == XUI_OK) result = doc_find_ui_button(ui, &ui->replace_one, XUI_TR_REPLACE_CURRENT);
    if (result == XUI_OK) result = doc_find_ui_button(ui, &ui->replace_all, XUI_TR_REPLACE_ALL);
    if (result == XUI_OK) result = doc_find_ui_checkbox(ui, &ui->case_sensitive,
        XUI_TR_FIND_CASE, 1);
    if (result == XUI_OK) result = doc_find_ui_checkbox(ui, &ui->whole_word,
        XUI_TR_FIND_WORD, 0);
    if (result == XUI_OK) result = doc_find_ui_checkbox(ui, &ui->regex,
        XUI_TR_FIND_REGEX, 0);
    if (result == XUI_OK) result = doc_find_ui_checkbox(ui, &ui->selection_only,
        XUI_TR_FIND_SELECTION, 0);
    if (result != XUI_OK) goto fail;
    { xui_doc_range_t scope;
      if (xuiDocumentViewGetFindScope(editor, &scope) == XUI_OK)
          (void)xuiCheckBoxSetChecked(ui->selection_only, 1); }
    td.iSize = sizeof(td); td.pFont = xuiGetDefaultFont(c);
    td.fDefaultRowHeight = 22; td.fHeaderHeight = 24;
    td.iSelectionMode = XUI_TABLE_VIEW_SELECTION_ROW;
    td.onCount = doc_find_ui_count; td.onCell = doc_find_ui_cell; td.pAdapterUser = ui;
    result = xuiTableViewCreate(c, &ui->results, &td);
    if (result == XUI_OK) result = doc_find_ui_add(ui->window, ui->results);
    if (result != XUI_OK) goto fail;
    (void)xuiTableViewSetSelect(ui->results, doc_find_ui_select, ui);
    ld.iSize = sizeof(ld); ld.pFont = xuiGetDefaultFont(c); ld.sText = "";
    ld.iTextColor = XUI_COLOR_RGBA(90, 105, 124, 255);
    ld.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
    result = xuiLabelCreate(c, &ui->status, &ld);
    if (result == XUI_OK) result = doc_find_ui_add(ui->window, ui->status);
    if (result != XUI_OK) goto fail;
    (void)xuiWidgetSetRect(ui->window, (xui_rect_t){32, 32, 640, 316});
    doc_find_ui_language(ui); doc_find_ui_layout(ui); doc_find_ui_readonly(ui);
    *out = ui; return XUI_OK;
fail:
    doc_find_ui_destroy(ui); return result;
}

int doc_find_ui_open(xui_widget editor, doc_find_ui** slot, int replace)
{
    doc_find_ui* ui;
    xui_doc_range_t selection;
    xui_document_snapshot snapshot = NULL;
    char* selected = NULL; uint64_t bytes = 0;
    xui_rect_t owner, rect;
    int result;
    if (!editor || !slot) return XUI_ERROR_INVALID_ARGUMENT;
    if (*slot && !xuiInternalWidgetIsValid((*slot)->window)) {
        doc_find_ui_destroy(*slot);
        *slot = NULL;
    }
    if (!*slot) {
        result = doc_find_ui_create(editor, slot);
        if (result != XUI_OK) return result;
    }
    ui = *slot; ui->replace_mode = !!replace;
    if (xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tAnchor.iKind == selection.tCaret.iKind &&
        selection.tAnchor.iNodeId == selection.tCaret.iNodeId &&
        selection.tCaret.iOffset > selection.tAnchor.iOffset &&
        selection.tCaret.iOffset - selection.tAnchor.iOffset <= 256 &&
        xuiDocumentAcquireSnapshot(xuiDocumentViewGetDocument(editor), &snapshot) == XUI_OK) {
        if (xuiDocumentSnapshotCopyRange(snapshot, &selection, &selected, &bytes) == XUI_OK &&
            bytes && bytes <= 256 && !memchr(selected, '\n', (size_t)bytes))
            (void)xuiInputSetText(ui->find, selected);
    }
    xuiDocumentFreeBuffer(selected); xuiDocumentSnapshotRelease(snapshot);
    if (!xuiInternalWidgetIsValid(editor)) return XUI_ERROR_INVALID_STATE;
    doc_find_ui_language(ui); doc_find_ui_layout(ui);
    doc_find_ui_readonly(ui);
    owner = xuiWidgetGetWorldRect(editor); rect = xuiWidgetGetRect(ui->window);
    rect.fX = owner.fX + 16; rect.fY = owner.fY + 16;
    (void)xuiWidgetSetRect(ui->window, rect);
    (void)xuiWindowSetOpen(ui->window, 1);
    (void)xuiWindowBringToFront(ui->window);
    (void)xuiSetFocusWidget(xuiWidgetGetContext(editor), ui->find);
    ui->dirty = 1;
    (void)doc_find_ui_refresh(ui);
    return XUI_OK;
}

void doc_find_ui_update(doc_find_ui* ui)
{
    xui_document document;
    if (!ui || !xuiInternalWidgetIsValid(ui->editor) ||
        !xuiInternalWidgetIsValid(ui->window)) return;
    if (ui->language_revision != xuiGetLanguageRevision(xuiWidgetGetContext(ui->editor)))
        doc_find_ui_language(ui);
    doc_find_ui_readonly(ui);
    document = xuiDocumentViewGetDocument(ui->editor);
    if (!document) return;
    if (ui->dirty || ui->identity != xuiDocumentGetIdentity(document) ||
        ui->revision != xuiDocumentGetRevision(document) ||
        ui->mode != xuiDocumentViewGetMode(ui->editor))
        (void)doc_find_ui_refresh(ui);
}

xui_widget doc_find_ui_window(doc_find_ui* ui)
{
    return ui && xuiInternalWidgetIsValid(ui->window) ? ui->window : NULL;
}

void doc_find_ui_destroy(doc_find_ui* ui)
{
    if (!ui) return;
    if (ui->window && xuiInternalWidgetIsValid(ui->window)) {
        xui_widget client = xuiWindowGetClientWidget(ui->window);
        (void)xuiWindowSetClose(ui->window, NULL, NULL);
        if (client) (void)xuiWidgetSetEventHandler(client, XUI_EVENT_KEY_DOWN, NULL, NULL);
        if (ui->find && xuiInternalWidgetIsValid(ui->find))
            (void)xuiInputSetChange(ui->find, NULL, NULL);
        if (ui->case_sensitive && xuiInternalWidgetIsValid(ui->case_sensitive))
            (void)xuiCheckBoxSetChange(ui->case_sensitive, NULL, NULL);
        if (ui->whole_word && xuiInternalWidgetIsValid(ui->whole_word))
            (void)xuiCheckBoxSetChange(ui->whole_word, NULL, NULL);
        if (ui->regex && xuiInternalWidgetIsValid(ui->regex))
            (void)xuiCheckBoxSetChange(ui->regex, NULL, NULL);
        if (ui->selection_only && xuiInternalWidgetIsValid(ui->selection_only))
            (void)xuiCheckBoxSetChange(ui->selection_only, NULL, NULL);
        if (ui->results && xuiInternalWidgetIsValid(ui->results))
            (void)xuiTableViewSetSelect(ui->results, NULL, NULL);
        xuiWidgetDestroy(ui->window);
    }
    free(ui);
}

#endif
