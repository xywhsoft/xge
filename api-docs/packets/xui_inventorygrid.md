# 草稿包：xui.h / inventorygrid（63 条 API）

> 生成 2026-09-10 03:00 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiInventoryGridGetType
- 位置: xui.h:8307  已注释: 否
- 签名: `XUI_API xui_widget_type xuiInventoryGridGetType(xui_context pContext);`
- 实现: src/xui_inventory_grid.c:2971（体 32 行）
- 返回码: NULL

## xuiInventoryGridCreate
- 位置: xui.h:8308  已注释: 否
- 签名: `XUI_API int xuiInventoryGridCreate(xui_context pContext, xui_widget* ppWidget, const xui_inventory_grid_desc_t* pDesc);`
- 实现: src/xui_inventory_grid.c:3004（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch186_main1.c:14; examples/tutorial_capture/ch187_main1.c:14; examples/xui_inventory_grid/main.c:379

## xuiInventoryGridSetSlotCount
- 位置: xui.h:8309  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetSlotCount(xui_widget pWidget, int iSlotCount);`
- 实现: src/xui_inventory_grid.c:3015（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_inventory_grid_test.c:654; test_xui/xui_style_chrome_test.c:381

## xuiInventoryGridGetSlotCount
- 位置: xui.h:8310  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetSlotCount(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3022（体 5 行）
- 用法: examples/xui_inventory_grid/main.c:526; examples/xui_inventory_grid/main.c:542; examples/xui_inventory_grid/main.c:664

## xuiInventoryGridSetSlot
- 位置: xui.h:8311  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetSlot(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot);`
- 实现: src/xui_inventory_grid.c:3028（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:198; examples/xui_inventory_grid/main.c:423; examples/xui_inventory_grid/main.c:426

## xuiInventoryGridGetSlot
- 位置: xui.h:8312  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetSlot(xui_widget pWidget, int iSlot, xui_inventory_slot_t* pSlot);`
- 实现: src/xui_inventory_grid.c:3049（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:236; examples/xui_inventory_grid/main.c:267; examples/xui_inventory_grid/main.c:268

## xuiInventoryGridClearSlot
- 位置: xui.h:8313  已注释: 否
- 签名: `XUI_API int xuiInventoryGridClearSlot(xui_widget pWidget, int iSlot);`
- 实现: src/xui_inventory_grid.c:3057（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:196

## xuiInventoryGridClearAll
- 位置: xui.h:8314  已注释: 否
- 签名: `XUI_API int xuiInventoryGridClearAll(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3072（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInventoryGridSetCurrent
- 位置: xui.h:8315  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetCurrent(xui_widget pWidget, int iSlot, int bNotify);`
- 实现: src/xui_inventory_grid.c:3092（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:442; examples/xui_inventory_grid/main.c:551; test_xui/xui_inventory_grid_test.c:496

## xuiInventoryGridGetCurrent
- 位置: xui.h:8316  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetCurrent(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3099（体 5 行）
- 用法: examples/xui_inventory_grid/main.c:553; test_xui/xui_inventory_grid_test.c:466; test_xui/xui_inventory_grid_test.c:483

## xuiInventoryGridSetSelected
- 位置: xui.h:8317  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetSelected(xui_widget pWidget, int iSlot, int bSelected, int bNotify);`
- 实现: src/xui_inventory_grid.c:3105（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:441; examples/xui_inventory_grid/main.c:527

## xuiInventoryGridGetSelected
- 位置: xui.h:8318  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetSelected(xui_widget pWidget, int iSlot);`
- 实现: src/xui_inventory_grid.c:3129（体 6 行）
- 用法: test_xui/xui_inventory_grid_test.c:466; test_xui/xui_inventory_grid_test.c:479; test_xui/xui_inventory_grid_test.c:479

## xuiInventoryGridClearSelection
- 位置: xui.h:8319  已注释: 否
- 签名: `XUI_API int xuiInventoryGridClearSelection(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3136（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInventoryGridSetLayout
- 位置: xui.h:8320  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetLayout(xui_widget pWidget, const xui_inventory_grid_layout_t* pLayout);`
- 实现: src/xui_inventory_grid.c:3146（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_inventory_grid_test.c:400; test_xui/xui_inventory_grid_test.c:410

## xuiInventoryGridGetLayout
- 位置: xui.h:8321  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetLayout(xui_widget pWidget, xui_inventory_grid_layout_t* pLayout);`
- 实现: src/xui_inventory_grid.c:3155（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_inventory_grid_test.c:381; test_xui/xui_inventory_grid_test.c:397; test_xui/xui_inventory_grid_test.c:405

## xuiInventoryGridSetFont
- 位置: xui.h:8322  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_inventory_grid.c:3163（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInventoryGridGetFont
- 位置: xui.h:8323  已注释: 否
- 签名: `XUI_API xui_font xuiInventoryGridGetFont(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3175（体 5 行）

## xuiInventoryGridSetMetrics
- 位置: xui.h:8324  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetMetrics(xui_widget pWidget, float fSlotSize, float fSlotGap, float fPadding, float fIconPadding, float fBorderWidth);`
- 实现: src/xui_inventory_grid.c:3181（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInventoryGridSetColors
- 位置: xui.h:8325  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetColors(xui_widget pWidget, const xui_inventory_grid_colors_t* pColors);`
- 实现: src/xui_inventory_grid.c:3193（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInventoryGridGetColors
- 位置: xui.h:8326  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetColors(xui_widget pWidget, xui_inventory_grid_colors_t* pColors);`
- 实现: src/xui_inventory_grid.c:3202（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_inventory_grid_test.c:383

## xuiInventoryGridGetScrollModel
- 位置: xui.h:8327  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiInventoryGridGetScrollModel(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3210（体 5 行）
- 用法: examples/xui_inventory_grid/main.c:557; test_xui/xui_inventory_grid_test.c:587

## xuiInventoryGridGetSlotRect
- 位置: xui.h:8328  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetSlotRect(xui_widget pWidget, int iSlot, xui_rect_t* pRect);`
- 实现: src/xui_inventory_grid.c:3216（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_inventory_grid_test.c:388; test_xui/xui_inventory_grid_test.c:390; test_xui/xui_inventory_grid_test.c:392

## xuiInventoryGridHitTest
- 位置: xui.h:8329  已注释: 否
- 签名: `XUI_API int xuiInventoryGridHitTest(xui_widget pWidget, float fX, float fY, xui_inventory_hit_t* pHit);`
- 实现: src/xui_inventory_grid.c:3229（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_inventory_grid_test.c:442

## xuiInventoryGridEnsureSlotVisible
- 位置: xui.h:8330  已注释: 否
- 签名: `XUI_API int xuiInventoryGridEnsureSlotVisible(xui_widget pWidget, int iSlot);`
- 实现: src/xui_inventory_grid.c:3236（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:531; test_xui/xui_inventory_grid_test.c:585; test_xui/xui_inventory_grid_test.c:670

## xuiInventoryGridGetVisibleRange
- 位置: xui.h:8331  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetVisibleRange(xui_widget pWidget, xui_inventory_visible_range_t* pRange);`
- 实现: src/xui_inventory_grid.c:3265（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:541; test_xui/xui_inventory_grid_test.c:658; test_xui/xui_inventory_grid_test.c:672

## xuiInventoryGridGetLastPaintRange
- 位置: xui.h:8332  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetLastPaintRange(xui_widget pWidget, xui_inventory_visible_range_t* pRange);`
- 实现: src/xui_inventory_grid.c:3277（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:494; test_xui/xui_inventory_grid_test.c:665

## xuiInventoryGridGetLastPaintSlotCount
- 位置: xui.h:8333  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetLastPaintSlotCount(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3285（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:667

## xuiInventoryGridQuerySlots
- 位置: xui.h:8334  已注释: 否
- 签名: `XUI_API int xuiInventoryGridQuerySlots(xui_widget pWidget, const xui_inventory_slot_query_t* pQuery, int* arrSlots, int iSlotCapacity, int* pSlotCount);`
- 实现: src/xui_inventory_grid.c:3291（体 55 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_inventory_grid/main.c:503; examples/xui_inventory_grid/main.c:547; test_xui/xui_inventory_grid_test.c:599

## xuiInventoryGridSortSlots
- 位置: xui.h:8335  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSortSlots(xui_widget pWidget, int* arrSlots, int iSlotCount, int iSortMode, uint32_t iFlags, xui_inventory_compare_proc onCompare, void* pUser);`
- 实现: src/xui_inventory_grid.c:3347（体 34 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_inventory_grid_test.c:625

## xuiInventoryGridSetGamepadProfile
- 位置: xui.h:8336  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetGamepadProfile(xui_widget pWidget, const xui_inventory_gamepad_profile_t* pProfile);`
- 实现: src/xui_inventory_grid.c:3382（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_inventory_grid_test.c:508; test_xui/xui_inventory_grid_test.c:516

## xuiInventoryGridGetGamepadProfile
- 位置: xui.h:8337  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetGamepadProfile(xui_widget pWidget, xui_inventory_gamepad_profile_t* pProfile);`
- 实现: src/xui_inventory_grid.c:3403（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:549; test_xui/xui_inventory_grid_test.c:494; test_xui/xui_inventory_grid_test.c:518

## xuiInventoryGridGamepadButton
- 位置: xui.h:8338  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGamepadButton(xui_widget pWidget, int iButton, int bPressed, uint32_t iModifiers);`
- 实现: src/xui_inventory_grid.c:3414（体 68 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:552; examples/xui_inventory_grid/main.c:554; test_xui/xui_inventory_grid_test.c:498

## xuiInventoryGridSetSelectCallback
- 位置: xui.h:8339  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetSelectCallback(xui_widget pWidget, xui_inventory_select_proc onSelect, void* pUser);`
- 实现: src/xui_inventory_grid.c:3483（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:338; test_xui/xui_inventory_grid_test.c:334

## xuiInventoryGridSetActivateCallback
- 位置: xui.h:8340  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetActivateCallback(xui_widget pWidget, xui_inventory_activate_proc onActivate, void* pUser);`
- 实现: src/xui_inventory_grid.c:3492（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:339; test_xui/xui_inventory_grid_test.c:336

## xuiInventoryGridSetContextCallback
- 位置: xui.h:8341  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetContextCallback(xui_widget pWidget, xui_inventory_context_proc onContext, void* pUser);`
- 实现: src/xui_inventory_grid.c:3501（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:340; test_xui/xui_inventory_grid_test.c:338

## xuiInventoryGridSetDragCallback
- 位置: xui.h:8342  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetDragCallback(xui_widget pWidget, xui_inventory_drag_proc onDrag, void* pUser);`
- 实现: src/xui_inventory_grid.c:3510（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:341; test_xui/xui_inventory_grid_test.c:340

## xuiInventoryGridSetDropCallback
- 位置: xui.h:8343  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetDropCallback(xui_widget pWidget, xui_inventory_drop_proc onDrop, void* pUser);`
- 实现: src/xui_inventory_grid.c:3519（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:342; test_xui/xui_inventory_grid_test.c:342

## xuiInventoryGridSetSplitCallback
- 位置: xui.h:8344  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetSplitCallback(xui_widget pWidget, xui_inventory_split_proc onSplit, void* pUser);`
- 实现: src/xui_inventory_grid.c:3528（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:343; test_xui/xui_inventory_grid_test.c:344

## xuiInventoryGridSetTooltipVisible
- 位置: xui.h:8345  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetTooltipVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_inventory_grid.c:3537（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:538; examples/xui_inventory_grid/main.c:540; test_xui/xui_inventory_grid_test.c:459

## xuiInventoryGridGetTooltipVisible
- 位置: xui.h:8346  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetTooltipVisible(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3554（体 5 行）
- 用法: examples/xui_inventory_grid/main.c:537; examples/xui_inventory_grid/main.c:539; test_xui/xui_inventory_grid_test.c:452

## xuiInventoryGridSetTooltipCallback
- 位置: xui.h:8347  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetTooltipCallback(xui_widget pWidget, xui_inventory_tooltip_proc onTooltip, void* pUser);`
- 实现: src/xui_inventory_grid.c:3560（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:344; test_xui/xui_inventory_grid_test.c:346

## xuiInventoryGridSetRenderCallback
- 位置: xui.h:8348  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetRenderCallback(xui_widget pWidget, xui_inventory_slot_render_proc onRender, void* pUser);`
- 实现: src/xui_inventory_grid.c:3575（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_inventory_grid_test.c:348

## xuiInventoryGridSetAnimationRenderCallback
- 位置: xui.h:8349  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetAnimationRenderCallback(xui_widget pWidget, xui_inventory_animation_render_proc onRender, void* pUser);`
- 实现: src/xui_inventory_grid.c:3584（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:345; test_xui/xui_inventory_grid_test.c:350

## xuiInventoryGridOpenSplitPopup
- 位置: xui.h:8350  已注释: 否
- 签名: `XUI_API int xuiInventoryGridOpenSplitPopup(xui_widget pWidget, int iSlot, float fX, float fY);`
- 实现: src/xui_inventory_grid.c:3593（体 61 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:239; examples/xui_inventory_grid/main.c:533; test_xui/xui_inventory_grid_test.c:521

## xuiInventoryGridCommitSplitPopup
- 位置: xui.h:8351  已注释: 否
- 签名: `XUI_API int xuiInventoryGridCommitSplitPopup(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3655（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:535; test_xui/xui_inventory_grid_test.c:531

## xuiInventoryGridCloseSplitPopup
- 位置: xui.h:8352  已注释: 否
- 签名: `XUI_API int xuiInventoryGridCloseSplitPopup(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3662（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInventoryGridIsSplitPopupOpen
- 位置: xui.h:8353  已注释: 否
- 签名: `XUI_API int xuiInventoryGridIsSplitPopupOpen(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3669（体 5 行）
- 用法: examples/xui_inventory_grid/main.c:534; test_xui/xui_inventory_grid_test.c:522; test_xui/xui_inventory_grid_test.c:524

## xuiInventoryGridGetSplitPopupWidget
- 位置: xui.h:8354  已注释: 否
- 签名: `XUI_API xui_widget xuiInventoryGridGetSplitPopupWidget(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3675（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:526

## xuiInventoryGridGetSplitInputWidget
- 位置: xui.h:8355  已注释: 否
- 签名: `XUI_API xui_widget xuiInventoryGridGetSplitInputWidget(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3681（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:527

## xuiInventoryGridGetSplitSlot
- 位置: xui.h:8356  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetSplitSlot(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3687（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:525

## xuiInventoryGridGetSplitCount
- 位置: xui.h:8357  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetSplitCount(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3693（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:525

## xuiInventoryGridSetSlotAnimation
- 位置: xui.h:8358  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSetSlotAnimation(xui_widget pWidget, int iSlot, xui_animation_object_t* pAnimation, uint32_t iFlags, float fScale, uint32_t iTint);`
- 实现: src/xui_inventory_grid.c:3699（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_inventory_grid/main.c:528; examples/xui_inventory_grid/main.c:532; test_xui/xui_inventory_grid_test.c:562

## xuiInventoryGridGetSlotAnimation
- 位置: xui.h:8359  已注释: 否
- 签名: `XUI_API xui_animation_object_t* xuiInventoryGridGetSlotAnimation(xui_widget pWidget, int iSlot, uint32_t* pFlags, float* pScale, uint32_t* pTint);`
- 实现: src/xui_inventory_grid.c:3713（体 9 行）
- 返回码: NULL
- 用法: examples/xui_inventory_grid/main.c:529; test_xui/xui_inventory_grid_test.c:564; test_xui/xui_inventory_grid_test.c:583

## xuiInventoryGridClearSlotAnimation
- 位置: xui.h:8360  已注释: 否
- 签名: `XUI_API int xuiInventoryGridClearSlotAnimation(xui_widget pWidget, int iSlot);`
- 实现: src/xui_inventory_grid.c:3723（体 4 行）
- 用法: examples/xui_inventory_grid/main.c:530; test_xui/xui_inventory_grid_test.c:582

## xuiInventoryGridGetTooltipSlot
- 位置: xui.h:8361  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetTooltipSlot(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3728（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:457; test_xui/xui_inventory_grid_test.c:460

## xuiInventoryGridGetHoverSlot
- 位置: xui.h:8362  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetHoverSlot(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3734（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:451

## xuiInventoryGridGetActiveSlot
- 位置: xui.h:8363  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetActiveSlot(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3740（体 5 行）

## xuiInventoryGridGetDragSource
- 位置: xui.h:8364  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetDragSource(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3746（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:544

## xuiInventoryGridGetDropTarget
- 位置: xui.h:8365  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetDropTarget(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3752（体 5 行）
- 用法: test_xui/xui_inventory_grid_test.c:544

## xuiInventoryGridGetChangeCount
- 位置: xui.h:8366  已注释: 否
- 签名: `XUI_API int xuiInventoryGridGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_inventory_grid.c:3758（体 5 行）
- 用法: examples/xui_inventory_grid/main.c:691

## xuiInventoryGridToXValue
- 位置: xui.h:8367  已注释: 否
- 签名: `XUI_API int xuiInventoryGridToXValue(xui_widget pWidget, xvalue** ppValue);`
- 实现: src/xui_inventory_grid.c:3764（体 141 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_inventory_grid_test.c:566

## xuiInventoryGridExportXSON
- 位置: xui.h:8368  已注释: 否
- 签名: `XUI_API int xuiInventoryGridExportXSON(xui_widget pWidget, char* sBuffer, int iCapacity);`
- 实现: src/xui_inventory_grid.c:3906（体 37 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_RESOURCE_FAILED, XUI_ERROR_BUFFER_TOO_SMALL
- 用法: test_xui/xui_inventory_grid_test.c:572

## xuiInventoryGridSaveXSONFile
- 位置: xui.h:8369  已注释: 否
- 签名: `XUI_API int xuiInventoryGridSaveXSONFile(xui_widget pWidget, const char* sPath);`
- 实现: src/xui_inventory_grid.c:3944（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_inventory_grid_test.c:574

