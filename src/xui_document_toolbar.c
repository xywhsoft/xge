#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_EDITOR
#include "../xui_document_ui.h"
#include <string.h>

typedef struct doc_toolbar_command {
    const char* label;
    int text_id;
    uint32_t command;
    uint32_t group;
    int toggle;
} doc_toolbar_command;

static const doc_toolbar_command doc_toolbar_commands[] = {
    {"Undo", XUI_TR_EDIT_UNDO, XUI_DOC_EDIT_UNDO, XUI_DOC_TOOLBAR_HISTORY, 0},
    {"Redo", XUI_TR_EDIT_REDO, XUI_DOC_EDIT_REDO, XUI_DOC_TOOLBAR_HISTORY, 0},
    {"Cut", XUI_TR_EDIT_CUT, XUI_DOC_EDIT_CUT, XUI_DOC_TOOLBAR_CLIPBOARD, 0},
    {"Copy", XUI_TR_EDIT_COPY, XUI_DOC_EDIT_COPY, XUI_DOC_TOOLBAR_CLIPBOARD, 0},
    {"Paste", XUI_TR_EDIT_PASTE, XUI_DOC_EDIT_PASTE, XUI_DOC_TOOLBAR_CLIPBOARD, 0},
    {"All", XUI_TR_EDIT_SELECT_ALL, XUI_DOC_EDIT_SELECT_ALL, XUI_DOC_TOOLBAR_CLIPBOARD, 0},
    {"B", XUI_TR_RICH_BOLD, XUI_DOC_EDIT_BOLD, XUI_DOC_TOOLBAR_INLINE_FORMAT, 1},
    {"I", XUI_TR_RICH_ITALIC, XUI_DOC_EDIT_ITALIC, XUI_DOC_TOOLBAR_INLINE_FORMAT, 1},
    {"U", XUI_TR_RICH_UNDERLINE, XUI_DOC_EDIT_UNDERLINE, XUI_DOC_TOOLBAR_INLINE_FORMAT, 1},
    {"S", XUI_TR_RICH_STRIKEOUT, XUI_DOC_EDIT_STRIKE, XUI_DOC_TOOLBAR_INLINE_FORMAT, 1},
    {"Left", XUI_TR_RICH_ALIGN_LEFT, XUI_DOC_EDIT_ALIGN_LEFT, XUI_DOC_TOOLBAR_ALIGNMENT, 1},
    {"Center", XUI_TR_RICH_ALIGN_CENTER, XUI_DOC_EDIT_ALIGN_CENTER, XUI_DOC_TOOLBAR_ALIGNMENT, 1},
    {"Right", XUI_TR_RICH_ALIGN_RIGHT, XUI_DOC_EDIT_ALIGN_RIGHT, XUI_DOC_TOOLBAR_ALIGNMENT, 1},
    {"Justify", XUI_TR_RICH_ALIGN_JUSTIFY, XUI_DOC_EDIT_ALIGN_JUSTIFY, XUI_DOC_TOOLBAR_ALIGNMENT, 1},
    {"P", XUI_TR_RICH_PARAGRAPH, XUI_DOC_EDIT_PARAGRAPH, XUI_DOC_TOOLBAR_BLOCKS, 1},
    {"H1", XUI_TR_RICH_HEADING_1, XUI_DOC_EDIT_HEADING_1, XUI_DOC_TOOLBAR_BLOCKS, 1},
    {"H2", XUI_TR_RICH_HEADING_2, XUI_DOC_EDIT_HEADING_2, XUI_DOC_TOOLBAR_BLOCKS, 1},
    {"H3", XUI_TR_RICH_HEADING_3, XUI_DOC_EDIT_HEADING_3, XUI_DOC_TOOLBAR_BLOCKS, 1},
    {"Quote", XUI_TR_RICH_BLOCK_QUOTE, XUI_DOC_EDIT_BLOCK_QUOTE, XUI_DOC_TOOLBAR_BLOCKS, 1},
    {"Rule", XUI_TR_RICH_HORIZONTAL_RULE, XUI_DOC_EDIT_INSERT_RULE, XUI_DOC_TOOLBAR_BLOCKS, 0},
    {"Code", XUI_TR_RICH_CODE_BLOCK, XUI_DOC_EDIT_INSERT_CODE_BLOCK, XUI_DOC_TOOLBAR_BLOCKS, 0},
    {"Bullets", XUI_TR_RICH_BULLET_LIST, XUI_DOC_EDIT_BULLET_LIST, XUI_DOC_TOOLBAR_LISTS, 1},
    {"Numbers", XUI_TR_RICH_NUMBER_LIST, XUI_DOC_EDIT_NUMBER_LIST, XUI_DOC_TOOLBAR_LISTS, 1},
    {"Tasks", XUI_TR_RICH_CHECK_LIST, XUI_DOC_EDIT_TASK_LIST, XUI_DOC_TOOLBAR_LISTS, 1},
    {"Indent", XUI_TR_RICH_INDENT, XUI_DOC_EDIT_INDENT_LIST, XUI_DOC_TOOLBAR_LISTS, 0},
    {"Outdent", XUI_TR_RICH_OUTDENT, XUI_DOC_EDIT_OUTDENT_LIST, XUI_DOC_TOOLBAR_LISTS, 0}
};

static const doc_toolbar_command* doc_toolbar_lookup(uint32_t command)
{
    size_t i;
    for (i = 0; i < sizeof(doc_toolbar_commands) / sizeof(doc_toolbar_commands[0]); i++)
        if (doc_toolbar_commands[i].command == command) return &doc_toolbar_commands[i];
    return NULL;
}

static int doc_toolbar_valid(xui_widget editor, xui_widget toolbar)
{
    xui_context context;
    if (!editor || !toolbar) return 0;
    context = xuiWidgetGetContext(editor);
    return context && xuiWidgetGetContext(toolbar) == context &&
        xuiWidgetIsType(editor, xuiDocumentEditorGetType(context)) &&
        xuiWidgetIsType(toolbar, xuiToolbarGetType(context));
}

XUI_API int xuiDocumentEditorSetupToolbar(xui_widget editor, xui_widget toolbar,
    uint32_t groups)
{
    xui_toolbar_item_t items[XUI_TOOLBAR_ITEM_CAPACITY] = {0};
    uint32_t previous_group = 0;
    size_t i;
    int count = 0, result;
    if (!doc_toolbar_valid(editor, toolbar)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!groups) groups = XUI_DOC_TOOLBAR_DEFAULT;
    if (groups & ~(XUI_DOC_TOOLBAR_HISTORY | XUI_DOC_TOOLBAR_CLIPBOARD |
        XUI_DOC_TOOLBAR_INLINE_FORMAT | XUI_DOC_TOOLBAR_ALIGNMENT |
        XUI_DOC_TOOLBAR_BLOCKS | XUI_DOC_TOOLBAR_LISTS))
        return XUI_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < sizeof(doc_toolbar_commands) / sizeof(doc_toolbar_commands[0]); i++) {
        const doc_toolbar_command* command = &doc_toolbar_commands[i];
        if (!(groups & command->group)) continue;
        if (previous_group && previous_group != command->group) {
            items[count].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
            count++;
        }
        items[count].sText = command->label;
        items[count].sTooltip = xuiTranslate(xuiWidgetGetContext(editor), command->text_id);
        items[count].iType = command->toggle ? XUI_TOOLBAR_ITEM_TOGGLE : XUI_TOOLBAR_ITEM_BUTTON;
        items[count].iState = XUI_TOOLBAR_ITEM_ENABLED;
        items[count].iValue = (int)command->command;
        items[count].iGroup = (int)command->group;
        count++; previous_group = command->group;
    }
    result = xuiToolbarSetItems(toolbar, items, count);
    return result == XUI_OK ? xuiDocumentEditorSyncToolbar(editor, toolbar) : result;
}

XUI_API int xuiDocumentEditorSyncToolbar(xui_widget editor, xui_widget toolbar)
{
    xui_context context;
    int i, count, result;
    if (!doc_toolbar_valid(editor, toolbar)) return XUI_ERROR_INVALID_ARGUMENT;
    context = xuiWidgetGetContext(editor);
    count = xuiToolbarGetItemCount(toolbar);
    for (i = 0; i < count; i++) {
        const xui_toolbar_item_t* item = xuiToolbarGetItem(toolbar, i);
        const doc_toolbar_command* command;
        xui_doc_command_state_t state = {0};
        if (!item || item->iType == XUI_TOOLBAR_ITEM_SEPARATOR) continue;
        command = doc_toolbar_lookup((uint32_t)item->iValue);
        if (!command) continue;
        state.iSize = sizeof(state);
        result = xuiDocumentEditorQueryCommand(editor, command->command, &state);
        if (result != XUI_OK) return result;
        result = xuiToolbarSetItemTooltip(toolbar, i,
            xuiTranslate(context, command->text_id));
        if (result == XUI_OK) result = xuiToolbarSetItemEnabled(toolbar, i, state.bEnabled);
        if (result == XUI_OK) result = xuiToolbarSetItemChecked(toolbar, i,
            command->toggle && (state.bActive || state.bMixed));
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}

XUI_API int xuiDocumentEditorExecuteToolbarItem(xui_widget editor,
    xui_widget toolbar, int index)
{
    const xui_toolbar_item_t* item;
    const doc_toolbar_command* command;
    int result, sync_result;
    if (!doc_toolbar_valid(editor, toolbar)) return XUI_ERROR_INVALID_ARGUMENT;
    item = xuiToolbarGetItem(toolbar, index);
    if (!item || item->iType == XUI_TOOLBAR_ITEM_SEPARATOR) return XUI_ERROR_INVALID_ARGUMENT;
    command = doc_toolbar_lookup((uint32_t)item->iValue);
    if (!command) return XUI_ERROR_INVALID_ARGUMENT;
    result = xuiDocumentEditorExecute(editor, command->command);
    sync_result = xuiDocumentEditorSyncToolbar(editor, toolbar);
    return result == XUI_OK ? sync_result : result;
}

#endif
