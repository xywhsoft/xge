# 草稿包：xui.h / flowgraph（71 条 API）

> 生成 2026-09-10 02:58 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiFlowGraphCreate
- 位置: xui.h:10313  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCreate(xui_flow_graph* ppGraph);`
- 实现: src/xui_flow_graph.c:817（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/tutorial_capture/ch182_main1.c:14; examples/xui_flowgraph/main.c:154; test_xui/xui_flow_graph_scale_test.c:49

## xuiFlowGraphDestroy
- 位置: xui.h:10314  已注释: 否
- 签名: `XUI_API void xuiFlowGraphDestroy(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:845（体 24 行）
- 用法: examples/xui_flowgraph/main.c:326; test_xui/xui_flow_graph_scale_test.c:106; test_xui/xui_flow_graph_test.c:363

## xuiFlowGraphAddNode
- 位置: xui.h:10315  已注释: 否
- 签名: `XUI_API int xuiFlowGraphAddNode(xui_flow_graph pGraph, const xui_flow_node_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_flow_graph.c:870（体 50 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/tutorial_capture/ch182_main1.c:24; examples/tutorial_capture/ch182_main1.c:34; examples/xui_flowgraph/main.c:112

## xuiFlowGraphAddPort
- 位置: xui.h:10316  已注释: 否
- 签名: `XUI_API int xuiFlowGraphAddPort(xui_flow_graph pGraph, int iNode, const xui_flow_port_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_flow_graph.c:978（体 47 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/tutorial_capture/ch182_main1.c:28; examples/tutorial_capture/ch182_main1.c:38; examples/xui_flowgraph/main.c:125

## xuiFlowGraphRebuildNodeDynamicPorts
- 位置: xui.h:10317  已注释: 否
- 签名: `XUI_API int xuiFlowGraphRebuildNodeDynamicPorts(xui_flow_graph pGraph, const char* sNodeId, const xui_flow_port_desc_t* pPorts, int iPortCount);`
- 实现: src/xui_flow_graph.c:1047（体 43 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiFlowGraphAddEdge
- 位置: xui.h:10318  已注释: 否
- 签名: `XUI_API int xuiFlowGraphAddEdge(xui_flow_graph pGraph, const xui_flow_edge_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_flow_graph.c:1091（体 78 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_flowgraph/main.c:141; test_xui/xui_flow_graph_scale_test.c:83; test_xui/xui_flow_graph_test.c:169

## xuiFlowGraphAddEdgePreserveInvalid
- 位置: xui.h:10319  已注释: 否
- 签名: `XUI_API int xuiFlowGraphAddEdgePreserveInvalid(xui_flow_graph pGraph, const xui_flow_edge_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_flow_graph.c:1170（体 57 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_workflow_xson_test.c:162

## xuiFlowGraphRemoveNode
- 位置: xui.h:10320  已注释: 否
- 签名: `XUI_API int xuiFlowGraphRemoveNode(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:921（体 38 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_flow_graph_test.c:212

## xuiFlowGraphRemoveEdge
- 位置: xui.h:10321  已注释: 否
- 签名: `XUI_API int xuiFlowGraphRemoveEdge(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:960（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_flow_graph_test.c:202

## xuiFlowGraphFindNode
- 位置: xui.h:10322  已注释: 否
- 签名: `XUI_API int xuiFlowGraphFindNode(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:1228（体 7 行）
- 用法: examples/xui_flowgraph/main.c:230; examples/xui_flowgraph/main.c:248; test_xui/xui_flow_graph_scale_test.c:89

## xuiFlowGraphFindEdge
- 位置: xui.h:10323  已注释: 否
- 签名: `XUI_API int xuiFlowGraphFindEdge(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:1236（体 7 行）
- 用法: examples/xui_flowgraph/main.c:249; test_xui/xui_flow_graph_scale_test.c:90; test_xui/xui_flow_graph_test.c:174

## xuiFlowGraphGetNodeCount
- 位置: xui.h:10324  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetNodeCount(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1244（体 4 行）
- 用法: examples/xui_flowgraph/main.c:250; examples/xui_flowgraph/main.c:356; examples/xui_flowgraph/main.c:378

## xuiFlowGraphGetEdgeCount
- 位置: xui.h:10325  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetEdgeCount(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1249（体 4 行）
- 用法: examples/xui_flowgraph/main.c:251; examples/xui_flowgraph/main.c:356; examples/xui_flowgraph/main.c:378

## xuiFlowGraphGetNodePortCount
- 位置: xui.h:10326  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetNodePortCount(xui_flow_graph pGraph, int iNode);`
- 实现: src/xui_flow_graph.c:1254（体 10 行）
- 用法: test_xui/xui_flow_graph_test.c:149; test_xui/xui_flow_graph_test.c:159; test_xui/xui_flow_graph_test.c:330

## xuiFlowGraphGetRevision
- 位置: xui.h:10327  已注释: 否
- 签名: `XUI_API uint32_t xuiFlowGraphGetRevision(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1265（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:50; test_xui/xui_flow_graph_test.c:65; test_xui/xui_flow_graph_test.c:67

## xuiFlowGraphGetNode
- 位置: xui.h:10328  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetNode(xui_flow_graph pGraph, int iNode, xui_flow_node_info_t* pInfo);`
- 实现: src/xui_flow_graph.c:1270（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_flowgraph/main.c:230; test_xui/xui_flow_graph_scale_test.c:95; test_xui/xui_flow_graph_test.c:91

## xuiFlowGraphGetPort
- 位置: xui.h:10329  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetPort(xui_flow_graph pGraph, int iNode, int iPort, xui_flow_port_info_t* pInfo);`
- 实现: src/xui_flow_graph.c:1293（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiFlowGraphGetEdge
- 位置: xui.h:10330  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetEdge(xui_flow_graph pGraph, int iEdge, xui_flow_edge_info_t* pInfo);`
- 实现: src/xui_flow_graph.c:1319（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_scale_test.c:101; test_xui/xui_workflow_interaction_test.c:268; test_xui/xui_workflow_schema_test.c:486

## xuiFlowGraphSetNodePosition
- 位置: xui.h:10331  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetNodePosition(xui_flow_graph pGraph, const char* sId, float fX, float fY);`
- 实现: src/xui_flow_graph.c:1349（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_flowgraph/main.c:226; test_xui/xui_flow_graph_test.c:87

## xuiFlowGraphSetNodeBounds
- 位置: xui.h:10332  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetNodeBounds(xui_flow_graph pGraph, const char* sId, float fX, float fY, float fW, float fH);`
- 实现: src/xui_flow_graph.c:1371（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiFlowGraphSetNodeSummary
- 位置: xui.h:10333  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetNodeSummary(xui_flow_graph pGraph, const char* sId, const char* sSummary);`
- 实现: src/xui_flow_graph.c:1394（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_style_flow_graph_test.c:97

## xuiFlowGraphSetNodeRunState
- 位置: xui.h:10334  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetNodeRunState(xui_flow_graph pGraph, const char* sId, int iState, const char* sPreview);`
- 实现: src/xui_flow_graph.c:1426（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_flow_graph_widget_test.c:157; test_xui/xui_style_flow_graph_test.c:115

## xuiFlowGraphSetEdgeRunState
- 位置: xui.h:10335  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetEdgeRunState(xui_flow_graph pGraph, const char* sId, int iState, const char* sPreview);`
- 实现: src/xui_flow_graph.c:1451（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_style_flow_graph_test.c:116

## xuiFlowGraphSetEdgeRoute
- 位置: xui.h:10336  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetEdgeRoute(xui_flow_graph pGraph, const char* sId, int iRouteStyle, float fRouteBias, float fSourceOffset, float fTargetOffset);`
- 实现: src/xui_flow_graph.c:1476（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:227; examples/xui_workflow_editor/main.c:228; examples/xui_workflow_editor/main.c:229

## xuiFlowGraphSetNodeConfig
- 位置: xui.h:10337  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetNodeConfig(xui_flow_graph pGraph, const char* sId, xvalue* pConfig);`
- 实现: src/xui_flow_graph.c:1501（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:446; test_xui/xui_workflow_schema_test.c:450; test_xui/xui_workflow_xson_test.c:125

## xuiFlowGraphGetNodeConfig
- 位置: xui.h:10338  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetNodeConfig(xui_flow_graph pGraph, const char* sId, xvalue** ppConfig);`
- 实现: src/xui_flow_graph.c:1528（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:101; test_xui/xui_flow_graph_test.c:110; test_xui/xui_flow_graph_test.c:116

## xuiFlowGraphCommandAddNode
- 位置: xui.h:10339  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandAddNode(xui_flow_graph pGraph, const xui_flow_node_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_flow_graph.c:1549（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:271; test_xui/xui_flow_graph_test.c:342; test_xui/xui_flow_graph_test.c:347

## xuiFlowGraphCommandRemoveNode
- 位置: xui.h:10340  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandRemoveNode(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:1581（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:326

## xuiFlowGraphCommandMoveNode
- 位置: xui.h:10341  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandMoveNode(xui_flow_graph pGraph, const xui_flow_move_node_desc_t* pDesc);`
- 实现: src/xui_flow_graph.c:1612（体 49 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_flow_graph_test.c:283

## xuiFlowGraphCommandRecordMoveNode
- 位置: xui.h:10342  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandRecordMoveNode(xui_flow_graph pGraph, const char* sId, float fOldX, float fOldY, float fNewX, float fNewY);`
- 实现: src/xui_flow_graph.c:1662（体 13 行）

## xuiFlowGraphCommandRecordMoveNodes
- 位置: xui.h:10343  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandRecordMoveNodes(xui_flow_graph pGraph, const xui_flow_move_node_record_t* pRecords, int iRecordCount);`
- 实现: src/xui_flow_graph.c:1676（体 35 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY

## xuiFlowGraphCommandSetNodeConfig
- 位置: xui.h:10344  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandSetNodeConfig(xui_flow_graph pGraph, const char* sId, xvalue* pConfig);`
- 实现: src/xui_flow_graph.c:1712（体 46 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:99; test_xui/xui_flow_graph_test.c:108; test_xui/xui_flow_graph_test.c:126

## xuiFlowGraphCommandAddEdge
- 位置: xui.h:10345  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandAddEdge(xui_flow_graph pGraph, const xui_flow_edge_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_flow_graph.c:1759（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:312

## xuiFlowGraphCommandRemoveEdge
- 位置: xui.h:10346  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCommandRemoveEdge(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:1791（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:318

## xuiFlowGraphCanUndo
- 位置: xui.h:10347  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCanUndo(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1822（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:100; test_xui/xui_flow_graph_test.c:272; test_xui/xui_flow_graph_test.c:276

## xuiFlowGraphCanRedo
- 位置: xui.h:10348  已注释: 否
- 签名: `XUI_API int xuiFlowGraphCanRedo(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1827（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:274; test_xui/xui_flow_graph_test.c:315; test_xui/xui_workflow_interaction_test.c:192

## xuiFlowGraphUndo
- 位置: xui.h:10349  已注释: 否
- 签名: `XUI_API int xuiFlowGraphUndo(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1832（体 50 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_flow_graph_test.c:114; test_xui/xui_flow_graph_test.c:273; test_xui/xui_flow_graph_test.c:287

## xuiFlowGraphRedo
- 位置: xui.h:10350  已注释: 否
- 签名: `XUI_API int xuiFlowGraphRedo(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1883（体 50 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_flow_graph_test.c:120; test_xui/xui_flow_graph_test.c:275; test_xui/xui_flow_graph_test.c:291

## xuiFlowGraphSetCommandHistoryLimit
- 位置: xui.h:10351  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetCommandHistoryLimit(xui_flow_graph pGraph, int iLimit);`
- 实现: src/xui_flow_graph.c:1934（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:333

## xuiFlowGraphGetCommandHistoryLimit
- 位置: xui.h:10352  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetCommandHistoryLimit(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1944（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:334

## xuiFlowGraphMarkClean
- 位置: xui.h:10353  已注释: 否
- 签名: `XUI_API int xuiFlowGraphMarkClean(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1949（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:52; test_xui/xui_flow_graph_test.c:68

## xuiFlowGraphIsDirty
- 位置: xui.h:10354  已注释: 否
- 签名: `XUI_API int xuiFlowGraphIsDirty(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1958（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:51; test_xui/xui_flow_graph_test.c:66; test_xui/xui_flow_graph_test.c:69

## xuiFlowGraphGetCleanRevision
- 位置: xui.h:10355  已注释: 否
- 签名: `XUI_API uint32_t xuiFlowGraphGetCleanRevision(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:1963（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:53

## xuiFlowGraphMakeNodeId
- 位置: xui.h:10356  已注释: 否
- 签名: `XUI_API int xuiFlowGraphMakeNodeId(xui_flow_graph pGraph, char* sBuffer, int iCapacity);`
- 实现: src/xui_flow_graph.c:1968（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:175

## xuiFlowGraphMakeEdgeId
- 位置: xui.h:10357  已注释: 否
- 签名: `XUI_API int xuiFlowGraphMakeEdgeId(xui_flow_graph pGraph, char* sBuffer, int iCapacity);`
- 实现: src/xui_flow_graph.c:1979（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:177

## xuiFlowGraphMakePortId
- 位置: xui.h:10358  已注释: 否
- 签名: `XUI_API int xuiFlowGraphMakePortId(xui_flow_graph pGraph, int iNode, char* sBuffer, int iCapacity);`
- 实现: src/xui_flow_graph.c:1990（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:179

## xuiFlowGraphClearSelection
- 位置: xui.h:10359  已注释: 否
- 签名: `XUI_API int xuiFlowGraphClearSelection(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:2006（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_flowgraph/main.c:235; examples/xui_flowgraph/main.c:241; test_xui/xui_flow_graph_test.c:217

## xuiFlowGraphSelectNode
- 位置: xui.h:10360  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSelectNode(xui_flow_graph pGraph, const char* sId, int bSelected);`
- 实现: src/xui_flow_graph.c:2019（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_flowgraph/main.c:202; examples/xui_flowgraph/main.c:242; test_xui/xui_flow_graph_test.c:182

## xuiFlowGraphSelectEdge
- 位置: xui.h:10361  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSelectEdge(xui_flow_graph pGraph, const char* sId, int bSelected);`
- 实现: src/xui_flow_graph.c:2033（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_flowgraph/main.c:203; examples/xui_flowgraph/main.c:236; test_xui/xui_flow_graph_test.c:186

## xuiFlowGraphIsNodeSelected
- 位置: xui.h:10362  已注释: 否
- 签名: `XUI_API int xuiFlowGraphIsNodeSelected(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:2047（体 4 行）
- 用法: examples/xui_flowgraph/main.c:244; test_xui/xui_flow_graph_test.c:183; test_xui/xui_flow_graph_test.c:191

## xuiFlowGraphIsEdgeSelected
- 位置: xui.h:10363  已注释: 否
- 签名: `XUI_API int xuiFlowGraphIsEdgeSelected(xui_flow_graph pGraph, const char* sId);`
- 实现: src/xui_flow_graph.c:2052（体 4 行）
- 用法: examples/xui_flowgraph/main.c:238; test_xui/xui_flow_graph_test.c:187; test_xui/xui_flow_graph_test.c:205

## xuiFlowGraphGetSelectedNodeCount
- 位置: xui.h:10364  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetSelectedNodeCount(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:2057（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:188; test_xui/xui_flow_graph_test.c:218; test_xui/xui_flow_graph_test.c:262

## xuiFlowGraphGetSelectedEdgeCount
- 位置: xui.h:10365  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetSelectedEdgeCount(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:2062（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:189; test_xui/xui_flow_graph_test.c:218; test_xui/xui_flow_graph_test.c:262

## xuiFlowGraphDeleteSelection
- 位置: xui.h:10366  已注释: 否
- 签名: `XUI_API int xuiFlowGraphDeleteSelection(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:2067（体 35 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_flowgraph/main.c:239; examples/xui_flowgraph/main.c:245; test_xui/xui_flow_graph_test.c:260

## xuiFlowGraphSetViewport
- 位置: xui.h:10367  已注释: 否
- 签名: `XUI_API int xuiFlowGraphSetViewport(xui_flow_graph pGraph, const xui_flow_viewport_t* pViewport);`
- 实现: src/xui_flow_graph.c:2103（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_flowgraph/main.c:163; examples/xui_workflow_editor/main.c:206; test_xui/xui_flow_graph_test.c:63

## xuiFlowGraphGetViewport
- 位置: xui.h:10368  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetViewport(xui_flow_graph pGraph, xui_flow_viewport_t* pViewport);`
- 实现: src/xui_flow_graph.c:2117（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_workflow_xson_test.c:204

## xuiFlowGraphWorldToScreen
- 位置: xui.h:10369  已注释: 否
- 签名: `XUI_API int xuiFlowGraphWorldToScreen(xui_flow_graph pGraph, float fWorldX, float fWorldY, float* pScreenX, float* pScreenY);`
- 实现: src/xui_flow_graph.c:2127（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:70

## xuiFlowGraphScreenToWorld
- 位置: xui.h:10370  已注释: 否
- 签名: `XUI_API int xuiFlowGraphScreenToWorld(xui_flow_graph pGraph, float fScreenX, float fScreenY, float* pWorldX, float* pWorldY);`
- 实现: src/xui_flow_graph.c:2137（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:72

## xuiFlowGraphAddDiagnostic
- 位置: xui.h:10371  已注释: 否
- 签名: `XUI_API int xuiFlowGraphAddDiagnostic(xui_flow_graph pGraph, const xui_flow_diagnostic_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_flow_graph.c:2153（体 35 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:225; test_xui/xui_flow_graph_widget_test.c:165; test_xui/xui_style_flow_graph_test.c:127

## xuiFlowGraphClearDiagnostics
- 位置: xui.h:10372  已注释: 否
- 签名: `XUI_API int xuiFlowGraphClearDiagnostics(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:2189（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_test.c:227

## xuiFlowGraphGetDiagnosticCount
- 位置: xui.h:10373  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetDiagnosticCount(xui_flow_graph pGraph);`
- 实现: src/xui_flow_graph.c:2200（体 4 行）
- 用法: test_xui/xui_flow_graph_test.c:226; test_xui/xui_flow_graph_test.c:228; test_xui/xui_workflow_schema_test.c:449

## xuiFlowGraphGetNodeDiagnosticCount
- 位置: xui.h:10374  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetNodeDiagnosticCount(xui_flow_graph pGraph, const char* sNodeId);`
- 实现: src/xui_flow_graph.c:2205（体 18 行）

## xuiFlowGraphGetEdgeDiagnosticCount
- 位置: xui.h:10375  已注释: 否
- 签名: `XUI_API int xuiFlowGraphGetEdgeDiagnosticCount(xui_flow_graph pGraph, const char* sEdgeId);`
- 实现: src/xui_flow_graph.c:2224（体 18 行）

## xuiFlowGraphGetType
- 位置: xui.h:10376  已注释: 否
- 签名: `XUI_API xui_widget_type xuiFlowGraphGetType(xui_context pContext);`
- 实现: src/xui_flow_graph_widget.c:1635（体 32 行）
- 返回码: NULL

## xuiFlowGraphWidgetCreate
- 位置: xui.h:10377  已注释: 否
- 签名: `XUI_API int xuiFlowGraphWidgetCreate(xui_context pContext, xui_widget* ppWidget, const xui_flow_graph_desc_t* pDesc);`
- 实现: src/xui_flow_graph_widget.c:1668（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch182_main1.c:45; examples/xui_flowgraph/main.c:285; test_xui/xui_flow_graph_widget_test.c:172

## xuiFlowGraphWidgetGetGraph
- 位置: xui.h:10378  已注释: 否
- 签名: `XUI_API xui_flow_graph xuiFlowGraphWidgetGetGraph(xui_widget pWidget);`
- 实现: src/xui_flow_graph_widget.c:1683（体 7 行）
- 用法: examples/xui_flowgraph/main.c:355; test_xui/xui_flow_graph_widget_test.c:174; test_xui/xui_workflow_widget_test.c:120

## xuiFlowGraphWidgetSetGraph
- 位置: xui.h:10379  已注释: 否
- 签名: `XUI_API int xuiFlowGraphWidgetSetGraph(xui_widget pWidget, xui_flow_graph pGraph, int bOwnGraph);`
- 实现: src/xui_flow_graph_widget.c:1691（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiFlowGraphWidgetSetContextMenu
- 位置: xui.h:10380  已注释: 否
- 签名: `XUI_API int xuiFlowGraphWidgetSetContextMenu(xui_widget pWidget, xui_flow_context_proc onContext, void* pUser);`
- 实现: src/xui_flow_graph_widget.c:1718（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_widget_test.c:179

## xuiFlowGraphWidgetHitTest
- 位置: xui.h:10381  已注释: 否
- 签名: `XUI_API int xuiFlowGraphWidgetHitTest(xui_widget pWidget, float fX, float fY, xui_flow_hit_t* pHit);`
- 实现: src/xui_flow_graph_widget.c:1832（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_workflow_editor/main.c:587; examples/xui_workflow_editor/main.c:732; test_xui/xui_flow_graph_widget_test.c:204

## xuiFlowGraphWidgetGetHoverHit
- 位置: xui.h:10382  已注释: 否
- 签名: `XUI_API int xuiFlowGraphWidgetGetHoverHit(xui_widget pWidget, xui_flow_hit_t* pHit);`
- 实现: src/xui_flow_graph_widget.c:1844（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_flow_graph_widget_test.c:228; test_xui/xui_flow_graph_widget_test.c:234; test_xui/xui_flow_graph_widget_test.c:240

## xuiFlowGraphWidgetSelectAt
- 位置: xui.h:10383  已注释: 否
- 签名: `XUI_API int xuiFlowGraphWidgetSelectAt(xui_widget pWidget, float fX, float fY, xui_flow_hit_t* pHit);`
- 实现: src/xui_flow_graph_widget.c:2458（体 4 行）
- 用法: test_xui/xui_flow_graph_widget_test.c:224; test_xui/xui_flow_graph_widget_test.c:226; test_xui/xui_workflow_interaction_test.c:170

