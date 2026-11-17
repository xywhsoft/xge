#ifndef XUI_FLOW_GRAPH_WIDGET_INTERNAL_H
#define XUI_FLOW_GRAPH_WIDGET_INTERNAL_H

#include "xui.h"

/* Only the actual native composite may rebind its owned canvas part. */
int xuiInternalFlowGraphWidgetSetGraph(xui_widget widget,xui_flow_graph graph,
    int ownGraph,xui_widget authority);
/* A late invalidate/cleanup error does not undo the borrowed slot publication. */
int xuiInternalFlowGraphWidgetSetGraphPublished(xui_widget widget,xui_flow_graph graph,
    int ownGraph,xui_widget authority,bool* published);
/* Destruction preparation only: stop a composite part borrowing its model
 * before the owning composite performs checked model release. */
int xuiInternalFlowGraphWidgetPrepareModelRelease(xui_widget widget,xui_widget authority);

#endif
