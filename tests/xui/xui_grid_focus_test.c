#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

static int g_iChecks;
static int g_iFailures;
static const char* g_sCase;
#define CHECK(expr, msg) do { g_iChecks++; if (!(expr)) { g_iFailures++; \
    printf("FAIL [%s]: %s (line %d)\n", g_sCase, msg, __LINE__); } } while (0)

enum { ROW_TEXT, ROW_NUMBER, ROW_ENUM, ROW_COLOR, ROW_DATE, ROW_TEXTAREA, ROW_BOOL, ROW_DISABLED, ROW_COUNT };
static const int g_arrTypes[] = {XUI_TABLE_CELL_TYPE_TEXT, XUI_TABLE_CELL_TYPE_INT,
    XUI_TABLE_CELL_TYPE_ENUM, XUI_TABLE_CELL_TYPE_COLOR, XUI_TABLE_CELL_TYPE_DATE,
    XUI_TABLE_CELL_TYPE_TEXTAREA, XUI_TABLE_CELL_TYPE_BOOL, XUI_TABLE_CELL_TYPE_TEXT};
static const char* g_arrValues[] = {"alpha", "12", "Draft", "#43A77C", "2026-09-05", "note", "false", "locked"};
static const char* g_arrEnum[] = {"Draft", "Ready", "Done"};

typedef struct grid_test_t {
    xui_test_proxy_state_t tProxy;
    xui_context pContext;
    xui_widget pRoot, pBefore, pAfter, pWidget, pGrid, pTable;
    xui_font pFont;
    xui_surface pSurface;
    xui_table_view_column_t arrColumns[2];
    char arrValues[ROW_COUNT][96];
    int arrProperties[ROW_COUNT];
    int iCategory, bProperty, iRows, iSetCount, iSelectCount;
} grid_test_t;

static int count_rows(xui_widget pWidget, void* pUser)
{
    return ((grid_test_t*)pUser)->iRows;
}

static int get_cell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
    grid_test_t* pTest = (grid_test_t*)pUser;
    if (iRow < 0 || iRow >= pTest->iRows) return 0;
    pCell->sText = pTest->arrValues[iRow];
    pCell->iType = g_arrTypes[iRow];
    pCell->bDisabled = iRow == ROW_DISABLED;
    return 1;
}

static void set_cell(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
    grid_test_t* pTest = (grid_test_t*)pUser;
    snprintf(pTest->arrValues[iRow], sizeof(pTest->arrValues[iRow]), "%s", sValue);
    pTest->iSetCount++;
}

static void property_change(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
    ((grid_test_t*)pUser)->iSetCount++;
}

static void table_select(xui_widget pWidget, int iRow, int iColumn, int iMode, void* pUser)
{
    ((grid_test_t*)pUser)->iSelectCount++;
}

static int validate(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
    return strcmp(sValue, "reject") != 0;
}

static int editor_config(xui_widget pWidget, int iRow, int iColumn, int iType,
    xui_table_grid_editor_config_t* pConfig, void* pUser)
{
    pConfig->arrEnumItems = g_arrEnum;
    pConfig->iEnumItemCount = 3;
    pConfig->iEnumSelected = -1;
    return 1;
}

static void key(grid_test_t* pTest, int iKey, uint32_t iModifiers)
{
    CHECK(xuiInputKeyDown(pTest->pContext, iKey, iModifiers) == XUI_OK, "key down");
    CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "dispatch key down");
    CHECK(xuiInputKeyUp(pTest->pContext, iKey, iModifiers) == XUI_OK, "key up");
    CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "dispatch key up");
}

static void type_text(grid_test_t* pTest, const char* sText)
{
    for (; *sText != '\0'; sText++) {
        CHECK(xuiInputText(pTest->pContext, (unsigned char)*sText) == XUI_OK, "text input");
        CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "dispatch text");
    }
}

static void render(grid_test_t* pTest)
{
    xui_rect_i_t tRect = {0, 0, 900, 650};
    CHECK(xuiLayout(pTest->pContext) == XUI_OK, "layout");
    CHECK(xuiRender(pTest->pContext, pTest->pSurface, &tRect, 1) == XUI_OK, "render");
}

static void click_local(grid_test_t* pTest, xui_widget pWidget, xui_rect_t tRect)
{
    xui_rect_t tWorld = xuiWidgetGetWorldRect(pWidget);
    float fX = tWorld.fX + tRect.fX + tRect.fW * 0.5f;
    float fY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
    CHECK(tRect.fW > 0 && tRect.fH > 0, "clickable field rect");
    CHECK(xuiInputPointerDown(pTest->pContext, (int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "pointer down");
    CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "dispatch pointer down");
    CHECK(xuiInputPointerUp(pTest->pContext, (int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "pointer up");
    CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "dispatch pointer up");
}

static int setup(grid_test_t* pTest, int bProperty)
{
    xui_button_desc_t tButton = {0};
    xui_table_grid_desc_t tGrid = {0};
    xui_property_grid_desc_t tProperty = {0};
    xui_property_desc_t tProp = {0};
    xui_table_grid_editor_config_t tConfig = {0};
    int i;
    memset(pTest, 0, sizeof(*pTest));
    pTest->bProperty = bProperty;
    pTest->iRows = ROW_COUNT;
    xuiTestProxyInit(&pTest->tProxy);
    if (xuiCreate(&pTest->pContext) != XUI_OK) return 0;
    CHECK(xuiSetProxy(pTest->pContext, &pTest->tProxy.tProxy) == XUI_OK, "proxy");
    CHECK(pTest->tProxy.tProxy.fontLoadMemory(&pTest->tProxy.tProxy, &pTest->pFont,
        "focus", 5, 14.0f, XUI_FONT_FORMAT_TTF) == XUI_OK, "font");
    CHECK(xuiSetDefaultFont(pTest->pContext, pTest->pFont) == XUI_OK, "default font");
    CHECK(xuiInputViewport(pTest->pContext, 900, 650) == XUI_OK, "viewport");
    CHECK(xuiWidgetCreate(pTest->pContext, &pTest->pRoot) == XUI_OK, "root");
    CHECK(xuiWidgetSetRect(pTest->pRoot, (xui_rect_t){0, 0, 900, 650}) == XUI_OK, "root rect");
    CHECK(xuiSetRootWidget(pTest->pContext, pTest->pRoot) == XUI_OK, "root set");
    tButton.iSize = sizeof(tButton);
    tButton.pFont = pTest->pFont;
    tButton.sText = "Before";
    CHECK(xuiButtonCreate(pTest->pContext, &pTest->pBefore, &tButton) == XUI_OK, "before button");
    CHECK(xuiWidgetSetRect(pTest->pBefore, (xui_rect_t){20, 10, 90, 28}) == XUI_OK, "before rect");
    CHECK(xuiWidgetAddChild(pTest->pRoot, pTest->pBefore) == XUI_OK, "before add");
    for (i = 0; i < ROW_COUNT; i++) snprintf(pTest->arrValues[i], 96, "%s", g_arrValues[i]);
    if (bProperty) {
        tProperty.iSize = sizeof(tProperty);
        tProperty.pFont = pTest->pFont;
        tProperty.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
        tProperty.fRowHeight = 28;
        tProperty.fCategoryHeight = 28;
        CHECK(xuiPropertyGridCreate(pTest->pContext, &pTest->pWidget, &tProperty) == XUI_OK, "property create");
        pTest->pGrid = xuiPropertyGridGetTableGrid(pTest->pWidget);
        pTest->pTable = xuiPropertyGridGetTableView(pTest->pWidget);
        pTest->iCategory = xuiPropertyGridAddCategory(pTest->pWidget, "general", "General", 1);
        for (i = 0; i < ROW_COUNT; i++) {
            tProp.sId = g_arrValues[i];
            tProp.sName = g_arrValues[i];
            tProp.sValue = g_arrValues[i];
            tProp.iType = g_arrTypes[i];
            tProp.iFlags = i == ROW_DISABLED ? XUI_PROPERTY_FLAG_DISABLED : 0;
            pTest->arrProperties[i] = xuiPropertyGridAddProperty(pTest->pWidget, pTest->iCategory, &tProp);
        }
        tConfig.arrEnumItems = g_arrEnum;
        tConfig.iEnumItemCount = 3;
        tConfig.iEnumSelected = -1;
        CHECK(xuiPropertyGridSetEditorConfig(pTest->pWidget, pTest->arrProperties[ROW_ENUM], &tConfig) == XUI_OK, "enum config");
        CHECK(xuiPropertyGridSetChange(pTest->pWidget, property_change, pTest) == XUI_OK, "property change");
    } else {
        for (i = 0; i < 2; i++) {
            pTest->arrColumns[i].sTitle = "Value";
            pTest->arrColumns[i].fWidth = 200;
        }
        tGrid.iSize = sizeof(tGrid);
        tGrid.pFont = pTest->pFont;
        tGrid.arrColumns = pTest->arrColumns;
        tGrid.iColumnCount = 2;
        tGrid.iRowCount = ROW_COUNT;
        tGrid.onCount = count_rows;
        tGrid.onCell = get_cell;
        tGrid.onSet = set_cell;
        tGrid.pAdapterUser = pTest;
        tGrid.onValidate = validate;
        tGrid.onEditorConfig = editor_config;
        tGrid.iSelectionMode = XUI_TABLE_VIEW_SELECTION_CELL;
        tGrid.fDefaultRowHeight = 28;
        tGrid.fHeaderHeight = 28;
        CHECK(xuiTableGridCreate(pTest->pContext, &pTest->pWidget, &tGrid) == XUI_OK, "table grid create");
        pTest->pGrid = pTest->pWidget;
        pTest->pTable = xuiTableGridGetTableView(pTest->pGrid);
        CHECK(xuiTableViewSetSelect(pTest->pTable, table_select, pTest) == XUI_OK, "table select");
    }
    CHECK(xuiWidgetSetRect(pTest->pWidget, (xui_rect_t){20, 48, 600, 480}) == XUI_OK, "grid rect");
    CHECK(xuiWidgetAddChild(pTest->pRoot, pTest->pWidget) == XUI_OK, "grid add");
    tButton.sText = "After";
    CHECK(xuiButtonCreate(pTest->pContext, &pTest->pAfter, &tButton) == XUI_OK, "after button");
    CHECK(xuiWidgetSetRect(pTest->pAfter, (xui_rect_t){20, 550, 90, 28}) == XUI_OK, "after rect");
    CHECK(xuiWidgetAddChild(pTest->pRoot, pTest->pAfter) == XUI_OK, "after add");
    CHECK(xuiTestSurfaceCreate(&pTest->tProxy, &pTest->pSurface, 900, 650, XUI_SURFACE_USAGE_TARGET) == XUI_OK, "surface");
    render(pTest);
    return 1;
}

static void cleanup(grid_test_t* pTest)
{
    xuiDestroy(pTest->pContext);
    pTest->tProxy.tProxy.surfaceDestroy(&pTest->tProxy.tProxy, pTest->pSurface);
    pTest->tProxy.tProxy.fontDestroy(&pTest->tProxy.tProxy, pTest->pFont);
}

static void focus_order(grid_test_t* pTest)
{
    CHECK(!xuiWidgetGetTabStop(pTest->pWidget) && !xuiWidgetGetFocusable(pTest->pWidget), "wrapper is not a focus entry");
    CHECK(!xuiWidgetGetTabStop(pTest->pGrid), "table grid wrapper is not a tab stop");
    CHECK(xuiSetFocusWidget(pTest->pContext, pTest->pBefore) == XUI_OK, "focus before");
    key(pTest, XUI_KEY_TAB, 0);
    CHECK(xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "before -> table");
    key(pTest, XUI_KEY_TAB, 0);
    CHECK(xuiGetFocusWidget(pTest->pContext) == pTest->pAfter, "table -> after");
    key(pTest, XUI_KEY_TAB, 0);
    CHECK(xuiGetFocusWidget(pTest->pContext) == pTest->pBefore, "after -> before wrap");
    key(pTest, XUI_KEY_TAB, XUI_MOD_SHIFT);
    CHECK(xuiGetFocusWidget(pTest->pContext) == pTest->pAfter, "before -> after reverse wrap");
    key(pTest, XUI_KEY_TAB, XUI_MOD_SHIFT);
    CHECK(xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "after -> table reverse");
    key(pTest, XUI_KEY_TAB, XUI_MOD_SHIFT);
    CHECK(xuiGetFocusWidget(pTest->pContext) == pTest->pBefore, "table -> before reverse");
}

static void begin(grid_test_t* pTest, int iRow)
{
    (void)xuiTableGridEndEdit(pTest->pGrid, 0);
    CHECK(xuiSetFocusWidget(pTest->pContext, pTest->pTable) == XUI_OK, "focus table");
    CHECK(xuiTableViewSetSelectedCell(pTest->pTable, iRow + pTest->bProperty, 1) == XUI_OK, "selected cell");
    CHECK(xuiTableGridBeginEdit(pTest->pGrid, iRow + pTest->bProperty, 1) != 0, "begin editor");
    render(pTest);
}

static xui_widget editor(grid_test_t* pTest)
{
    xui_widget pChild;
    for (pChild = xuiWidgetGetFirstChild(pTest->pGrid); pChild != NULL; pChild = xuiWidgetGetNextSibling(pChild)) {
        if (pChild != pTest->pTable && xuiWidgetGetVisible(pChild)) return pChild;
    }
    return NULL;
}

static xui_widget open_popup(grid_test_t* pTest, int iRow)
{
    xui_widget pEditor = editor(pTest);
    xui_widget pPopup = NULL;
    if (iRow == ROW_ENUM) {
        CHECK(xuiComboBoxOpen(pEditor) == XUI_OK, "open enum");
        pPopup = xuiComboBoxGetPopupWidget(pEditor);
    } else if (iRow == ROW_COLOR) {
        CHECK(xuiColorPickerOpen(pEditor) == XUI_OK, "open color");
        pPopup = xuiColorPickerGetPopupWidget(pEditor);
    } else if (iRow == ROW_DATE) {
        CHECK(xuiDatePickerOpen(pEditor) == XUI_OK, "open date");
        pPopup = xuiDatePickerGetPopupWidget(pEditor);
    } else if (iRow == ROW_TEXTAREA) {
        pPopup = xuiGetFocusWidget(pTest->pContext);
        while (pPopup != NULL && !xuiWidgetIsType(pPopup, xuiPopupGetType(pTest->pContext))) pPopup = xuiWidgetGetParent(pPopup);
    }
    if (pPopup != NULL) CHECK(xuiPopupIsOpen(pPopup), "popup open");
    render(pTest);
    return pPopup;
}

static void activation(grid_test_t* pTest)
{
    int iRow = -1, iColumn = -1, iCount;
    CHECK(xuiSetFocusWidget(pTest->pContext, pTest->pTable) == XUI_OK, "focus navigation");
    CHECK(xuiTableViewSetSelectedCell(pTest->pTable, pTest->bProperty, 1) == XUI_OK, "initial selection");
    key(pTest, XUI_KEY_DOWN, 0);
    CHECK(xuiTableViewGetSelectedCell(pTest->pTable, &iRow, &iColumn) == XUI_OK && iRow == 1 + pTest->bProperty && iColumn == 1, "TableView down preserved");
    key(pTest, XUI_KEY_UP, 0);
    if (!pTest->bProperty) {
        key(pTest, XUI_KEY_LEFT, 0);
        CHECK(xuiTableViewGetSelectedCell(pTest->pTable, &iRow, &iColumn) == XUI_OK && iRow == 0 && iColumn == 0, "TableView left preserved");
        key(pTest, XUI_KEY_RIGHT, 0);
        CHECK(xuiTableViewGetSelectedCell(pTest->pTable, &iRow, &iColumn) == XUI_OK && iRow == 0 && iColumn == 1, "TableView right preserved");
        key(pTest, XUI_KEY_HOME, 0);
        CHECK(xuiTableViewGetSelectedCell(pTest->pTable, &iRow, &iColumn) == XUI_OK && iColumn == 0, "TableView Home preserved");
        key(pTest, XUI_KEY_END, 0);
        CHECK(xuiTableViewGetSelectedCell(pTest->pTable, &iRow, &iColumn) == XUI_OK && iColumn == 1, "TableView End preserved");
    }
    iCount = pTest->iSelectCount;
    key(pTest, XUI_KEY_ENTER, 0);
    CHECK(xuiTableGridIsEditing(pTest->pGrid), "Enter starts selected editor");
    CHECK(pTest->iSelectCount == iCount, "activation does not duplicate selection callback");
    if (!xuiTableGridIsEditing(pTest->pGrid)) begin(pTest, ROW_TEXT);
    CHECK(xuiInputSetText(xuiGetFocusWidget(pTest->pContext), "changed") == XUI_OK, "text value");
    iCount = pTest->iSetCount;
    key(pTest, XUI_KEY_ENTER, 0);
    CHECK(!xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "Enter commits and restores table");
    CHECK(pTest->iSetCount == iCount + 1, "commit once");
    key(pTest, XUI_KEY_SPACE, 0);
    CHECK(xuiTableGridIsEditing(pTest->pGrid), "Space starts selected editor");
    if (xuiTableGridIsEditing(pTest->pGrid)) CHECK(xuiInputSetText(xuiGetFocusWidget(pTest->pContext), "cancelled") == XUI_OK, "cancel draft");
    key(pTest, XUI_KEY_ESCAPE, 0);
    CHECK(!xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "Escape restores table");
    CHECK(pTest->iSetCount == iCount + 1, "cancel never commits");
    begin(pTest, ROW_NUMBER);
    CHECK(xuiInputSetText(xuiGetFocusWidget(pTest->pContext), "42") == XUI_OK, "numeric value");
    key(pTest, XUI_KEY_ENTER, 0);
    CHECK(!xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "numeric Enter commits grid, not only numeric input");
    (void)xuiTableGridEndEdit(pTest->pGrid, 0);
    CHECK(xuiTableViewSetSelectedCell(pTest->pTable, ROW_BOOL + pTest->bProperty, 1) == XUI_OK, "bool select");
    iCount = pTest->iSetCount;
    key(pTest, XUI_KEY_SPACE, 0);
    CHECK(!xuiTableGridIsEditing(pTest->pGrid) && pTest->iSetCount == iCount + 1, "bool Space toggles exactly once");
    CHECK(!xuiTableGridBeginEdit(pTest->pGrid, ROW_DISABLED + pTest->bProperty, 1), "disabled cell cannot edit");
}

static void editor_exit(grid_test_t* pTest)
{
    int iRow, bReverse, iCancels;
    xui_widget pPopup, pEditor;
    for (iRow = ROW_TEXT; iRow <= ROW_TEXTAREA; iRow++) {
        for (bReverse = 0; bReverse <= 1; bReverse++) {
            begin(pTest, iRow);
            pPopup = open_popup(pTest, iRow);
            key(pTest, XUI_KEY_TAB, bReverse ? XUI_MOD_SHIFT : 0);
            CHECK(!xuiTableGridIsEditing(pTest->pGrid), "Tab ends editing");
            CHECK(xuiGetFocusWidget(pTest->pContext) == (bReverse ? pTest->pBefore : pTest->pAfter), "editor Tab reaches correct external sentinel");
            if (pPopup != NULL) CHECK(!xuiPopupIsOpen(pPopup), "Tab closes editor popup");
        }
        begin(pTest, iRow);
        pEditor = editor(pTest);
        pPopup = open_popup(pTest, iRow);
        iCancels = iRow == ROW_DATE ? xuiDatePickerGetCancelCount(pEditor) : 0;
        key(pTest, XUI_KEY_ESCAPE, 0);
        CHECK(!xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "Escape cancels complete edit");
        if (pPopup != NULL) CHECK(!xuiPopupIsOpen(pPopup), "Escape closes popup");
        if (iRow == ROW_DATE) CHECK(xuiDatePickerGetCancelCount(pEditor) == iCancels + 1, "date keeps native cancel notification");
        begin(pTest, iRow);
        pPopup = open_popup(pTest, iRow);
        key(pTest, XUI_KEY_ENTER, 0);
        CHECK(!xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "Enter completes editor including unchanged value");
        if (pPopup != NULL) CHECK(!xuiPopupIsOpen(pPopup), "Enter closes popup");
    }
    (void)xuiTableGridEndEdit(pTest->pGrid, 0);
}

static void edge_cases(grid_test_t* pTest)
{
    const int arrDisabled[] = {0, 0, 0};
    const int arrEnabled[] = {1, 1, 1};
    xui_widget pEditor, pPopup, pText, pButton;
    int iCount, i;
    begin(pTest, ROW_TEXT);
    CHECK(xuiInputSetText(xuiGetFocusWidget(pTest->pContext), "tabbed") == XUI_OK, "Tab draft");
    iCount = pTest->iSetCount;
    key(pTest, XUI_KEY_TAB, 0);
    CHECK(!xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pTest->pAfter, "Tab commits and exits");
    CHECK(pTest->iSetCount == iCount + 1, "Tab commit callback once");
    CHECK(strcmp(pTest->bProperty ? xuiPropertyGridGetValue(pTest->pWidget, pTest->arrProperties[ROW_TEXT]) : pTest->arrValues[ROW_TEXT], "tabbed") == 0, "Tab stores draft value");
    begin(pTest, ROW_TEXTAREA);
    pText = xuiGetFocusWidget(pTest->pContext);
    CHECK(xuiTextEditSetText(pText, "") == XUI_OK, "multiline reset");
    key(pTest, XUI_KEY_ENTER, XUI_MOD_SHIFT);
    CHECK(xuiTableGridIsEditing(pTest->pGrid) && strcmp(xuiTextEditGetText(pText), "\n") == 0, "Shift Enter inserts newline");
    key(pTest, XUI_KEY_ENTER, 0);
    CHECK(!xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pTest->pTable, "multiline Enter completes edit");
    for (i = 0; i < 2; i++) {
        begin(pTest, ROW_TEXTAREA);
        pPopup = open_popup(pTest, ROW_TEXTAREA);
        pButton = xuiWidgetGetNextSibling(xuiGetFocusWidget(pTest->pContext));
        if (i) pButton = xuiWidgetGetNextSibling(pButton);
        CHECK(xuiSetFocusWidget(pTest->pContext, pButton) == XUI_OK, "focus popup action button");
        key(pTest, XUI_KEY_TAB, i ? XUI_MOD_SHIFT : 0);
        CHECK(!xuiPopupIsOpen(pPopup) && !xuiTableGridIsEditing(pTest->pGrid), "action button Tab closes popup");
        CHECK(xuiGetFocusWidget(pTest->pContext) == (i ? pTest->pBefore : pTest->pAfter), "action button Tab leaves composite");
    }
    begin(pTest, ROW_ENUM);
    pEditor = editor(pTest);
    pPopup = open_popup(pTest, ROW_ENUM);
    key(pTest, XUI_KEY_DOWN, 0);
    iCount = pTest->iSetCount;
    key(pTest, XUI_KEY_TAB, 0);
    CHECK(!xuiPopupIsOpen(pPopup) && !xuiTableGridIsEditing(pTest->pGrid), "enum Tab commits highlighted choice");
    CHECK(xuiComboBoxGetSelected(pEditor) == 1 && pTest->iSetCount == iCount + 1, "enum choice committed once");
    begin(pTest, ROW_ENUM);
    pEditor = editor(pTest);
    pPopup = open_popup(pTest, ROW_ENUM);
    CHECK(xuiComboBoxSetEnabledItems(pEditor, arrDisabled, 3) == XUI_OK, "disable enum items");
    key(pTest, XUI_KEY_TAB, XUI_MOD_SHIFT);
    CHECK(!xuiPopupIsOpen(pPopup) && !xuiTableGridIsEditing(pTest->pGrid), "all-disabled enum can leave");
    CHECK(xuiGetFocusWidget(pTest->pContext) == pTest->pBefore, "all-disabled enum reverse sentinel");
    CHECK(xuiComboBoxSetEnabledItems(pEditor, arrEnabled, 3) == XUI_OK, "enable enum items");
    begin(pTest, ROW_COLOR);
    pEditor = editor(pTest);
    pPopup = open_popup(pTest, ROW_COLOR);
    click_local(pTest, xuiColorPickerGetPanelWidget(pEditor), xuiColorPickerGetHexRect(pEditor));
    key(pTest, 'A', XUI_MOD_CTRL);
    type_text(pTest, "123456");
    key(pTest, XUI_KEY_TAB, 0);
    CHECK(!xuiPopupIsOpen(pPopup) && xuiGetFocusWidget(pTest->pContext) == pTest->pAfter, "color field Tab is not swallowed");
    CHECK(xuiColorPickerGetColor(pEditor) == XUI_COLOR_RGBA(0x12, 0x34, 0x56, 255), "color field committed before leaving");
    for (i = 0; i < 2; i++) {
        begin(pTest, ROW_COLOR);
        pEditor = editor(pTest);
        pPopup = open_popup(pTest, ROW_COLOR);
        click_local(pTest, xuiColorPickerGetPanelWidget(pEditor), xuiColorPickerGetHexRect(pEditor));
        key(pTest, 'A', XUI_MOD_CTRL);
        type_text(pTest, "1");
        iCount = pTest->iSetCount;
        key(pTest, i ? XUI_KEY_TAB : XUI_KEY_ENTER, 0);
        CHECK(xuiPopupIsOpen(pPopup) && xuiTableGridIsEditing(pTest->pGrid), "invalid color field keeps edit open");
        CHECK(xuiGetFocusWidget(pTest->pContext) == xuiColorPickerGetPanelWidget(pEditor), "invalid color field retains focus");
        CHECK(pTest->iSetCount == iCount, "invalid color does not commit");
        key(pTest, XUI_KEY_ESCAPE, 0);
    }
    begin(pTest, ROW_DATE);
    pEditor = editor(pTest);
    CHECK(xuiDatePickerSetMode(pEditor, XUI_DATE_PICKER_MODE_DATETIME) == XUI_OK, "date time mode");
    pPopup = open_popup(pTest, ROW_DATE);
    click_local(pTest, xuiDatePickerGetPanelWidget(pEditor), xuiDatePickerGetTimeRect(pEditor, 0, 0));
    key(pTest, 'A', XUI_MOD_CTRL);
    type_text(pTest, "13");
    key(pTest, XUI_KEY_TAB, XUI_MOD_SHIFT);
    CHECK(!xuiPopupIsOpen(pPopup) && xuiGetFocusWidget(pTest->pContext) == pTest->pBefore, "date field reverse Tab is not swallowed");
    CHECK((xuiDatePickerGetValue(pEditor) / XRT_TIME_HOUR) % 24 == 13, "date draft committed before leaving");
    if (!pTest->bProperty) {
        begin(pTest, ROW_TEXT);
        pEditor = xuiGetFocusWidget(pTest->pContext);
        CHECK(xuiInputSetText(pEditor, "reject") == XUI_OK, "invalid value");
        iCount = xuiTableGridGetRejectCount(pTest->pGrid);
        key(pTest, XUI_KEY_TAB, 0);
        CHECK(xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pEditor, "rejected Tab retains editor focus");
        CHECK(xuiTableGridGetRejectCount(pTest->pGrid) == iCount + 1, "rejected once");
        key(pTest, XUI_KEY_TAB, XUI_MOD_SHIFT);
        CHECK(xuiTableGridIsEditing(pTest->pGrid) && xuiGetFocusWidget(pTest->pContext) == pEditor, "rejected reverse Tab retains editor");
        key(pTest, XUI_KEY_ESCAPE, 0);
        CHECK(!xuiTableGridIsEditing(pTest->pGrid), "invalid editor can cancel");
        pTest->iRows = 0;
        CHECK(xuiTableViewRefresh(pTest->pTable) == XUI_OK, "empty table refresh");
        key(pTest, XUI_KEY_DOWN, 0);
        key(pTest, XUI_KEY_ENTER, 0);
        CHECK(!xuiTableGridIsEditing(pTest->pGrid), "empty table cannot edit");
        focus_order(pTest);
    }
}

static void property_categories(grid_test_t* pTest)
{
    int iRow = -1, iColumn = -1;
    CHECK(xuiSetFocusWidget(pTest->pContext, pTest->pTable) == XUI_OK, "category focus");
    CHECK(xuiTableViewSetSelectedCell(pTest->pTable, 0, 0) == XUI_OK, "category select");
    key(pTest, XUI_KEY_ENTER, 0);
    CHECK(!xuiPropertyGridGetCategoryExpanded(pTest->pWidget, pTest->iCategory), "Enter collapses category");
    CHECK(!xuiTableGridIsEditing(pTest->pGrid), "category is not a text editor");
    key(pTest, XUI_KEY_SPACE, 0);
    CHECK(xuiPropertyGridGetCategoryExpanded(pTest->pWidget, pTest->iCategory), "Space expands category");
    key(pTest, XUI_KEY_LEFT, 0);
    CHECK(!xuiPropertyGridGetCategoryExpanded(pTest->pWidget, pTest->iCategory), "Left collapses category");
    key(pTest, XUI_KEY_RIGHT, 0);
    CHECK(xuiPropertyGridGetCategoryExpanded(pTest->pWidget, pTest->iCategory), "Right expands category");
    key(pTest, XUI_KEY_DOWN, 0);
    CHECK(xuiTableViewGetSelectedCell(pTest->pTable, &iRow, &iColumn) == XUI_OK && iRow == 1 && iColumn == 1, "category Down reaches editable value column");
    key(pTest, XUI_KEY_ENTER, 0);
    CHECK(xuiTableGridIsEditing(pTest->pGrid), "expanded property is keyboard editable");
    key(pTest, XUI_KEY_ESCAPE, 0);
    key(pTest, XUI_KEY_UP, 0);
    CHECK(xuiTableViewGetSelectedCell(pTest->pTable, &iRow, &iColumn) == XUI_OK && iRow == 0, "property Up returns to category");
    render(pTest);
    focus_order(pTest);
}

int main(void)
{
    grid_test_t tTest;
    int bProperty;
    for (bProperty = 0; bProperty <= 1; bProperty++) {
        g_sCase = bProperty ? "PropertyGrid" : "TableGrid";
        if (!setup(&tTest, bProperty)) return 1;
        focus_order(&tTest);
        activation(&tTest);
        editor_exit(&tTest);
        edge_cases(&tTest);
        if (bProperty) property_categories(&tTest);
        CHECK(xuiWidgetSetRect(tTest.pWidget, (xui_rect_t){20, 48, 300, 120}) == XUI_OK, "overflow grid rect");
        if (!bProperty) {
            tTest.iRows = ROW_COUNT;
            CHECK(xuiTableViewRefresh(tTest.pTable) == XUI_OK, "restore table rows");
        }
        render(&tTest);
        focus_order(&tTest);
        CHECK(xuiWidgetSetEnabled(tTest.pWidget, 0) == XUI_OK, "disable grid");
        CHECK(xuiSetFocusWidget(tTest.pContext, tTest.pBefore) == XUI_OK, "disabled before");
        key(&tTest, XUI_KEY_TAB, 0);
        CHECK(xuiGetFocusWidget(tTest.pContext) == tTest.pAfter, "disabled subtree skipped");
        cleanup(&tTest);
    }
    printf("grid focus: %d checks, %d failures\n", g_iChecks, g_iFailures);
    return g_iFailures ? 1 : 0;
}
