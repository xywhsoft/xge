#ifndef XGE_XUI_VIEWPORT_QUARANTINE_H
#define XGE_XUI_VIEWPORT_QUARANTINE_H

/*
 * Temporary build quarantine for the viewport infrastructure rebuild.
 *
 * The old VirtualView-dependent controls are deliberately not compiled
 * while the viewport stack is being rebuilt. ScrollModel, ScrollFrame,
 * ScrollView, Popup, ListView, TreeView, TableView, TextEdit, ColorPicker, and DatePicker are already restored.
 * xge_xui_page.c still contains old XSON
 * apply paths for the remaining quarantined controls; these macros keep those
 * old static paths compilable while runtime dispatch rejects the quarantined
 * widget types explicitly. PropertyGrid has been rebuilt on top of TableGrid
 * and is no longer part of this quarantine.
 */

#if !XGE_XUI_VIEWPORT_REBUILD

#define xgeXuiVirtualListInit(...) XGE_ERROR_UNSUPPORTED
#define xgeXuiVirtualListSetAdapter(...) ((void)0)
#define xgeXuiVirtualListSetColors(...) ((void)0)
#define xgeXuiVirtualListSetItemHeight(...) ((void)0)
#define xgeXuiVirtualListSetScroll(...) ((void)0)
#define xgeXuiVirtualListUnit(...) ((void)0)

#endif

#endif
