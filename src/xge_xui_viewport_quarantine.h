#ifndef XGE_XUI_VIEWPORT_QUARANTINE_H
#define XGE_XUI_VIEWPORT_QUARANTINE_H

/*
 * Temporary build quarantine for the viewport infrastructure rebuild.
 *
 * The old VirtualView-dependent controls are deliberately not compiled
 * while the viewport stack is being rebuilt. ScrollModel, ScrollFrame, and
 * ScrollView and Popup are already restored. xge_xui_page.c still contains old XSON
 * apply paths for the remaining quarantined controls; these macros keep those
 * old static paths compilable while runtime dispatch rejects the quarantined
 * widget types explicitly.
 */

#if !XGE_XUI_VIEWPORT_REBUILD

#define xgeXuiTextEditInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiTextEditUnit(...) ((void)0)
#define xgeXuiTextEditSetText(pEdit, sText) do { (void)(pEdit); (void)(sText); } while (0)
#define xgeXuiTextEditSetDisabledColors(...) ((void)0)
#define xgeXuiTextEditSetFrameColors(...) ((void)0)
#define xgeXuiTextEditSetLineNumbers(...) ((void)0)
#define xgeXuiTextEditSetReadonly(...) ((void)0)
#define xgeXuiTextEditSetScrollbarColors(...) ((void)0)
#define xgeXuiTextEditSetScrollbarMode(...) ((void)0)
#define xgeXuiTextEditSetWordWrap(...) ((void)0)

#define xgeXuiColorPickerGetRGBA(pPicker, pR, pG, pB, pA) do { if ((pR) != NULL) { *(pR) = 0; } if ((pG) != NULL) { *(pG) = 0; } if ((pB) != NULL) { *(pB) = 0; } if ((pA) != NULL) { *(pA) = 255; } } while (0)
#define xgeXuiColorPickerInit(pPicker, pContext, pWidget, pFont) ((void)(pPicker), (void)(pContext), (void)(pWidget), (void)(pFont), XGE_ERROR_UNSUPPORTED)
#define xgeXuiColorPickerSetAlphaEnabled(...) ((void)0)
#define xgeXuiColorPickerSetColor(...) ((void)0)
#define xgeXuiColorPickerSetColors(...) ((void)0)
#define xgeXuiColorPickerSetHex(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiColorPickerSetPalette(pPicker, pColors, iCount) do { (void)(pPicker); (void)(pColors); (void)(iCount); } while (0)
#define xgeXuiColorPickerSetRGBA(...) ((void)0)
#define xgeXuiColorPickerUnit(...) ((void)0)

#define xgeXuiComboBoxGetSelectedValue(...) 0
#define xgeXuiComboBoxInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiComboBoxSetColors(...) ((void)0)
#define xgeXuiComboBoxSetEnabledItems(pCombo, arrEnabled, iCount) do { (void)(pCombo); (void)(arrEnabled); (void)(iCount); } while (0)
#define xgeXuiComboBoxSetFont(...) ((void)0)
#define xgeXuiComboBoxSetItemColors(...) ((void)0)
#define xgeXuiComboBoxSetItemData(...) ((void)0)
#define xgeXuiComboBoxSetItems(...) ((void)0)
#define xgeXuiComboBoxSetMetrics(...) ((void)0)
#define xgeXuiComboBoxSetPopupHeight(...) ((void)0)
#define xgeXuiComboBoxSetPopupMaxHeight(...) ((void)0)
#define xgeXuiComboBoxSetPopupPlacement(...) ((void)0)
#define xgeXuiComboBoxSetSelected(...) ((void)0)
#define xgeXuiComboBoxSetSelectedValue(...) ((void)0)
#define xgeXuiComboBoxUnit(...) ((void)0)

#define xgeXuiMenuInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiMenuClose(...) ((void)0)
#define xgeXuiMenuIsOpen(...) 0
#define xgeXuiMenuOpenAt(...) ((void)0)
#define xgeXuiMenuSetColors(...) ((void)0)
#define xgeXuiMenuSetFont(...) ((void)0)
#define xgeXuiMenuSetItems(...) ((void)0)
#define xgeXuiMenuSetItemState(pMenu, iIndex, iState) do { (void)(pMenu); (void)(iIndex); (void)(iState); } while (0)
#define xgeXuiMenuSetMetrics(...) ((void)0)
#define xgeXuiMenuSetSelect(...) ((void)0)
#define xgeXuiMenuUnit(...) ((void)0)

#define xgeXuiVirtualListInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiVirtualListSetAdapter(...) ((void)0)
#define xgeXuiVirtualListSetColors(...) ((void)0)
#define xgeXuiVirtualListSetItemHeight(...) ((void)0)
#define xgeXuiVirtualListSetScroll(...) ((void)0)
#define xgeXuiVirtualListUnit(...) ((void)0)

#define xgeXuiListViewInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiListViewSetColors(...) ((void)0)
#define xgeXuiListViewSetDisabledTextColor(...) ((void)0)
#define xgeXuiListViewSetEnabledItems(...) ((void)0)
#define xgeXuiListViewSetFont(...) ((void)0)
#define xgeXuiListViewSetItemHeight(...) ((void)0)
#define xgeXuiListViewSetItems(...) ((void)0)
#define xgeXuiListViewSetItemSelected(...) ((void)0)
#define xgeXuiListViewSetScroll(...) ((void)0)
#define xgeXuiListViewSetScrollbarMode(...) ((void)0)
#define xgeXuiListViewSetSelected(...) ((void)0)
#define xgeXuiListViewSetSelectionBuffer(...) ((void)0)
#define xgeXuiListViewSetSelectionMode(...) ((void)0)
#define xgeXuiListViewUnit(...) ((void)0)

#define xgeXuiTreeViewAddNode(pTree, iId, iParent, sText) ((void)(pTree), (void)(iId), (void)(iParent), (void)(sText), XGE_ERROR_UNSUPPORTED)
#define xgeXuiTreeViewInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiTreeViewSetColors(...) ((void)0)
#define xgeXuiTreeViewSetFont(...) ((void)0)
#define xgeXuiTreeViewSetMetrics(...) ((void)0)
#define xgeXuiTreeViewSetNodeExpanded(pTree, iId, iExpanded) do { (void)(pTree); (void)(iId); (void)(iExpanded); } while (0)
#define xgeXuiTreeViewSetScroll(...) ((void)0)
#define xgeXuiTreeViewSetSelected(...) ((void)0)
#define xgeXuiTreeViewUnit(...) ((void)0)

#define xgeXuiTableViewInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiTableViewSetAdapter(...) ((void)0)
#define xgeXuiTableViewSetColors(...) ((void)0)
#define xgeXuiTableViewSetColumns(...) ((void)0)
#define xgeXuiTableViewSetFont(...) ((void)0)
#define xgeXuiTableViewSetMetrics(...) ((void)0)
#define xgeXuiTableViewSetScroll(...) ((void)0)
#define xgeXuiTableViewSetSelected(...) ((void)0)
#define xgeXuiTableViewUnit(...) ((void)0)

#define xgeXuiPropertyGridAddCategory(pGrid, sName, bExpanded) ((void)(pGrid), (void)(sName), (void)(bExpanded), XGE_ERROR_UNSUPPORTED)
#define xgeXuiPropertyGridAddProperty(pGrid, iCategory, sName, sValue, iEditor) ((void)(pGrid), (void)(iCategory), (void)(sName), (void)(sValue), (void)(iEditor), XGE_ERROR_UNSUPPORTED)
#define xgeXuiPropertyGridClear(...) ((void)0)
#define xgeXuiPropertyGridInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiPropertyGridSetColors(...) ((void)0)
#define xgeXuiPropertyGridSetEnumItems(...) ((void)0)
#define xgeXuiPropertyGridSetFont(...) ((void)0)
#define xgeXuiPropertyGridSetMetrics(...) ((void)0)
#define xgeXuiPropertyGridSetPropertyFlags(...) ((void)0)
#define xgeXuiPropertyGridSetSelected(...) ((void)0)
#define xgeXuiPropertyGridUnit(...) ((void)0)

#endif

#endif
