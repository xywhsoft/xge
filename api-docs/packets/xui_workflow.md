# 草稿包：xui.h / workflow（41 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiWorkflowGetType
- 位置: xui.h:12196  已注释: 否
- 签名: `XUI_API xui_widget_type xuiWorkflowGetType(xui_context pContext);`
- 实现: src/xui_workflow_widget.c:142（体 25 行）
- 返回码: NULL
- 用法: test_xui/xui_workflow_widget_test.c:84; test_xui/xui_workflow_widget_test.c:86

## xuiWorkflowWidgetCreate
- 位置: xui.h:12197  已注释: 否
- 签名: `XUI_API int xuiWorkflowWidgetCreate(xui_context pContext, xui_widget* ppWidget, const xui_workflow_desc_t* pDesc);`
- 实现: src/xui_workflow_widget.c:168（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/xui_workflow_editor/main.c:644; test_xui/xui_workflow_widget_test.c:116

## xuiWorkflowWidgetGetWorkflow
- 位置: xui.h:12198  已注释: 否
- 签名: `XUI_API xui_workflow xuiWorkflowWidgetGetWorkflow(xui_widget pWidget);`
- 实现: src/xui_workflow_widget.c:183（体 7 行）
- 用法: examples/xui_workflow_editor/main.c:849; test_xui/xui_workflow_widget_test.c:118; test_xui/xui_workflow_widget_test.c:146

## xuiWorkflowWidgetSetWorkflow
- 位置: xui.h:12199  已注释: 否
- 签名: `XUI_API int xuiWorkflowWidgetSetWorkflow(xui_widget pWidget, xui_workflow pWorkflow, int bOwnWorkflow);`
- 实现: src/xui_workflow_widget.c:191（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_workflow_editor/main.c:387; test_xui/xui_workflow_widget_test.c:145

## xuiWorkflowWidgetSetContextMenu
- 位置: xui.h:12200  已注释: 否
- 签名: `XUI_API int xuiWorkflowWidgetSetContextMenu(xui_widget pWidget, xui_flow_context_proc onContext, void* pUser);`
- 实现: src/xui_workflow_widget.c:212（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:646; test_xui/xui_workflow_widget_test.c:125

## xuiWorkflowWidgetGetCanvas
- 位置: xui.h:12201  已注释: 否
- 签名: `XUI_API xui_widget xuiWorkflowWidgetGetCanvas(xui_widget pWidget);`
- 实现: src/xui_workflow_widget.c:221（体 7 行）
- 用法: examples/xui_workflow_editor/main.c:510; examples/xui_workflow_editor/main.c:578; examples/xui_workflow_editor/main.c:722

## xuiWorkflowCreate
- 位置: xui.h:12203  已注释: 否
- 签名: `XUI_API int xuiWorkflowCreate(xui_workflow* ppWorkflow);`
- 实现: src/xui_workflow.c:507（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:261; examples/xui_workflow_editor/main.c:270; examples/xui_workflow_editor/main.c:377

## xuiWorkflowDestroy
- 位置: xui.h:12204  已注释: 否
- 签名: `XUI_API void xuiWorkflowDestroy(xui_workflow pWorkflow);`
- 实现: src/xui_workflow.c:536（体 32 行）
- 用法: examples/xui_workflow_editor/main.c:283; examples/xui_workflow_editor/main.c:284; examples/xui_workflow_editor/main.c:390

## xuiWorkflowGetGraph
- 位置: xui.h:12205  已注释: 否
- 签名: `XUI_API xui_flow_graph xuiWorkflowGetGraph(xui_workflow pWorkflow);`
- 实现: src/xui_workflow.c:569（体 4 行）
- 用法: examples/xui_workflow_editor/main.c:206; examples/xui_workflow_editor/main.c:227; examples/xui_workflow_editor/main.c:228

## xuiWorkflowConfigSchemaCreate
- 位置: xui.h:12206  已注释: 否
- 签名: `XUI_API int xuiWorkflowConfigSchemaCreate(xvalue** ppSchema);`
- 实现: src/xui_workflow.c:574（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:163

## xuiWorkflowConfigSchemaAddField
- 位置: xui.h:12207  已注释: 否
- 签名: `XUI_API int xuiWorkflowConfigSchemaAddField(xvalue* pSchema, const xui_workflow_config_field_desc_t* pDesc);`
- 实现: src/xui_workflow.c:601（体 100 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:175; test_xui/xui_workflow_schema_test.c:192; test_xui/xui_workflow_schema_test.c:200

## xuiWorkflowRegisterNodeType
- 位置: xui.h:12208  已注释: 否
- 签名: `XUI_API int xuiWorkflowRegisterNodeType(xui_workflow pWorkflow, const xui_workflow_node_type_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_workflow.c:702（体 66 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:158; test_xui/xui_workflow_schema_test.c:157; test_xui/xui_workflow_schema_test.c:319

## xuiWorkflowFindNodeType
- 位置: xui.h:12209  已注释: 否
- 签名: `XUI_API int xuiWorkflowFindNodeType(xui_workflow pWorkflow, const char* sId);`
- 实现: src/xui_workflow.c:769（体 7 行）
- 用法: test_xui/xui_workflow_schema_test.c:159

## xuiWorkflowGetNodeTypeCount
- 位置: xui.h:12210  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetNodeTypeCount(xui_workflow pWorkflow);`
- 实现: src/xui_workflow.c:777（体 4 行）
- 用法: test_xui/xui_workflow_schema_test.c:145; test_xui/xui_workflow_schema_test.c:404

## xuiWorkflowAddNode
- 位置: xui.h:12211  已注释: 否
- 签名: `XUI_API int xuiWorkflowAddNode(xui_workflow pWorkflow, const char* sType, const char* sId, const char* sTitle, float fX, float fY, int* pIndex);`
- 实现: src/xui_workflow.c:835（体 74 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:209; examples/xui_workflow_editor/main.c:211; examples/xui_workflow_editor/main.c:213

## xuiWorkflowConnect
- 位置: xui.h:12212  已注释: 否
- 签名: `XUI_API int xuiWorkflowConnect(xui_workflow pWorkflow, const char* sEdgeId, const char* sFromNode, const char* sFromPort, const char* sToNode, const char* sToPort, int* pIndex);`
- 实现: src/xui_workflow.c:910（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_workflow_editor/main.c:219; examples/xui_workflow_editor/main.c:221; examples/xui_workflow_editor/main.c:223

## xuiWorkflowRemoveNode
- 位置: xui.h:12213  已注释: 否
- 签名: `XUI_API int xuiWorkflowRemoveNode(xui_workflow pWorkflow, const char* sId);`
- 实现: src/xui_workflow.c:929（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_workflow_schema_test.c:470; test_xui/xui_workflow_schema_test.c:504; test_xui/xui_workflow_schema_test.c:506

## xuiWorkflowSelectNode
- 位置: xui.h:12214  已注释: 否
- 签名: `XUI_API int xuiWorkflowSelectNode(xui_workflow pWorkflow, const char* sId, int bSelected);`
- 实现: src/xui_workflow.c:937（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_workflow_editor/main.c:231; test_xui/xui_workflow_schema_test.c:430

## xuiWorkflowGetSelectedNode
- 位置: xui.h:12215  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetSelectedNode(xui_workflow pWorkflow, xui_flow_node_info_t* pInfo);`
- 实现: src/xui_workflow.c:945（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:433

## xuiWorkflowGetNodeCount
- 位置: xui.h:12216  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetNodeCount(xui_workflow pWorkflow);`
- 实现: src/xui_workflow.c:969（体 4 行）
- 用法: examples/xui_workflow_editor/main.c:278; examples/xui_workflow_editor/main.c:393; examples/xui_workflow_editor/main.c:850

## xuiWorkflowGetNodeLibraryCount
- 位置: xui.h:12217  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetNodeLibraryCount(xui_workflow pWorkflow);`
- 实现: src/xui_workflow.c:974（体 4 行）
- 用法: test_xui/xui_workflow_schema_test.c:405

## xuiWorkflowGetNodeLibraryItem
- 位置: xui.h:12218  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetNodeLibraryItem(xui_workflow pWorkflow, int iIndex, xui_workflow_node_library_item_t* pItem);`
- 实现: src/xui_workflow.c:979（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:407

## xuiWorkflowCreateDefaultConfig
- 位置: xui.h:12219  已注释: 否
- 签名: `XUI_API int xuiWorkflowCreateDefaultConfig(xui_workflow pWorkflow, const char* sType, xvalue** ppConfig);`
- 实现: src/xui_workflow.c:1000（体 65 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:321

## xuiWorkflowValidateConfig
- 位置: xui.h:12220  已注释: 否
- 签名: `XUI_API int xuiWorkflowValidateConfig(xui_workflow pWorkflow, const char* sType, xvalue* pConfig, int* pDiagnosticCount);`
- 实现: src/xui_workflow.c:1066（体 4 行）
- 用法: test_xui/xui_workflow_schema_test.c:337; test_xui/xui_workflow_schema_test.c:349; test_xui/xui_workflow_schema_test.c:354

## xuiWorkflowValidateConfigEx
- 位置: xui.h:12221  已注释: 否
- 签名: `XUI_API int xuiWorkflowValidateConfigEx(xui_workflow pWorkflow, const char* sType, xvalue* pConfig, xui_workflow_config_diagnostic_t* pDiagnostics, int iDiagnosticCapacity, int* pDiagnosticCount);`
- 实现: src/xui_workflow.c:1071（体 73 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:343

## xuiWorkflowValidateGraph
- 位置: xui.h:12222  已注释: 否
- 签名: `XUI_API int xuiWorkflowValidateGraph(xui_workflow pWorkflow, int* pDiagnosticCount);`
- 实现: src/xui_workflow.c:1145（体 334 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:246; examples/xui_workflow_editor/main.c:359; test_xui/xui_workflow_schema_test.c:448

## xuiWorkflowSetNodeConfig
- 位置: xui.h:12223  已注释: 否
- 签名: `XUI_API int xuiWorkflowSetNodeConfig(xui_workflow pWorkflow, const char* sId, xvalue* pConfig);`
- 实现: src/xui_workflow.c:1480（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_workflow_schema_test.c:426; test_xui/xui_workflow_schema_test.c:428; test_xui/xui_workflow_schema_test.c:483

## xuiWorkflowGetNodeConfig
- 位置: xui.h:12224  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetNodeConfig(xui_workflow pWorkflow, const char* sId, xvalue** ppConfig);`
- 实现: src/xui_workflow.c:1512（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_workflow_schema_test.c:421

## xuiWorkflowMakeVariableId
- 位置: xui.h:12225  已注释: 否
- 签名: `XUI_API int xuiWorkflowMakeVariableId(xui_workflow pWorkflow, char* sBuffer, int iCapacity);`
- 实现: src/xui_workflow.c:1520（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_workflow_schema_test.c:513

## xuiWorkflowAddVariable
- 位置: xui.h:12226  已注释: 否
- 签名: `XUI_API int xuiWorkflowAddVariable(xui_workflow pWorkflow, const xui_workflow_variable_desc_t* pDesc, int* pIndex);`
- 实现: src/xui_workflow.c:1531（体 47 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:242; test_xui/xui_workflow_schema_test.c:524; test_xui/xui_workflow_schema_test.c:528

## xuiWorkflowFindVariable
- 位置: xui.h:12227  已注释: 否
- 签名: `XUI_API int xuiWorkflowFindVariable(xui_workflow pWorkflow, const char* sId);`
- 实现: src/xui_workflow.c:1579（体 7 行）
- 用法: test_xui/xui_workflow_schema_test.c:526

## xuiWorkflowGetVariableCount
- 位置: xui.h:12228  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetVariableCount(xui_workflow pWorkflow);`
- 实现: src/xui_workflow.c:1587（体 4 行）
- 用法: test_xui/xui_workflow_schema_test.c:527; test_xui/xui_workflow_xson_test.c:195

## xuiWorkflowGetVariable
- 位置: xui.h:12229  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetVariable(xui_workflow pWorkflow, int iIndex, xui_workflow_variable_desc_t* pDesc);`
- 实现: src/xui_workflow.c:1592（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWorkflowToXValue
- 位置: xui.h:12230  已注释: 否
- 签名: `XUI_API int xuiWorkflowToXValue(xui_workflow pWorkflow, xvalue** ppValue);`
- 实现: src/xui_workflow.c:1802（体 94 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK

## xuiWorkflowLoadXValue
- 位置: xui.h:12231  已注释: 否
- 签名: `XUI_API int xuiWorkflowLoadXValue(xui_workflow pWorkflow, xvalue* pValue);`
- 实现: src/xui_workflow.c:2019（体 66 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWorkflowSaveXSONFile
- 位置: xui.h:12232  已注释: 否
- 签名: `XUI_API int xuiWorkflowSaveXSONFile(xui_workflow pWorkflow, const char* sPath);`
- 实现: src/xui_workflow.c:2086（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_workflow_editor/main.c:325; examples/xui_workflow_editor/main.c:327; test_xui/xui_workflow_xson_test.c:185

## xuiWorkflowLoadXSONFile
- 位置: xui.h:12233  已注释: 否
- 签名: `XUI_API int xuiWorkflowLoadXSONFile(xui_workflow pWorkflow, const char* sPath);`
- 实现: src/xui_workflow.c:2103（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_RESOURCE_FAILED
- 用法: examples/xui_workflow_editor/main.c:274; examples/xui_workflow_editor/main.c:339; examples/xui_workflow_editor/main.c:341

## xuiWorkflowSetNodeRunState
- 位置: xui.h:12234  已注释: 否
- 签名: `XUI_API int xuiWorkflowSetNodeRunState(xui_workflow pWorkflow, const xui_workflow_node_run_state_t* pState);`
- 实现: src/xui_workflow.c:2165（体 43 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_workflow_editor/main.c:783; examples/xui_workflow_editor/main.c:787; examples/xui_workflow_editor/main.c:791

## xuiWorkflowGetNodeRunState
- 位置: xui.h:12235  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetNodeRunState(xui_workflow pWorkflow, const char* sNodeId, xui_workflow_node_run_state_t* pState);`
- 实现: src/xui_workflow.c:2209（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_workflow_editor/main.c:874; test_xui/xui_workflow_xson_test.c:172

## xuiWorkflowSetEdgeRunState
- 位置: xui.h:12236  已注释: 否
- 签名: `XUI_API int xuiWorkflowSetEdgeRunState(xui_workflow pWorkflow, const xui_workflow_edge_run_state_t* pState);`
- 实现: src/xui_workflow.c:2232（体 43 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_workflow_editor/main.c:806; examples/xui_workflow_editor/main.c:809; examples/xui_workflow_editor/main.c:813

## xuiWorkflowGetEdgeRunState
- 位置: xui.h:12237  已注释: 否
- 签名: `XUI_API int xuiWorkflowGetEdgeRunState(xui_workflow pWorkflow, const char* sEdgeId, xui_workflow_edge_run_state_t* pState);`
- 实现: src/xui_workflow.c:2276（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_workflow_xson_test.c:182

