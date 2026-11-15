#ifndef XUI_FLOW_GRAPH_WIDGET_INTERNAL_H
#define XUI_FLOW_GRAPH_WIDGET_INTERNAL_H

#include "xui.h"

/* Only the actual native composite may rebind its owned canvas part. */
int xuiInternalFlowGraphWidgetSetGraph(xui_widget widget,xui_flow_graph graph,
    int ownGraph,xui_widget authority);

#endif
