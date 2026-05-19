#ifndef XGE_XUI_VIEWPORT_QUARANTINE_H
#define XGE_XUI_VIEWPORT_QUARANTINE_H

/*
 * Temporary build quarantine for the viewport infrastructure rebuild.
 *
 * The old VirtualView-dependent controls are deliberately not compiled
 * while the viewport stack is being rebuilt. ScrollModel, ScrollFrame,
 * ScrollView, Popup, ListView, TreeView, TextEdit, ColorPicker, and DatePicker are already restored.
 * xge_xui_page.c still contains old XSON
 * apply paths for the remaining quarantined controls; these macros keep those
 * old static paths compilable while runtime dispatch rejects the quarantined
 * widget types explicitly.
 */

#if !XGE_XUI_VIEWPORT_REBUILD

#define xgeXuiVirtualListInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiVirtualListSetAdapter(...) ((void)0)
#define xgeXuiVirtualListSetColors(...) ((void)0)
#define xgeXuiVirtualListSetItemHeight(...) ((void)0)
#define xgeXuiVirtualListSetScroll(...) ((void)0)
#define xgeXuiVirtualListUnit(...) ((void)0)

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
