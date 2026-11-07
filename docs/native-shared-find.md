# XGE 原生共享查找窗口

`xui_code_find_scope` 既是多编辑器查找域，也是可选的共享查找会话。成员和调用必须位于同一个 XUI context／UI 线程。

- 用 `xuiCodeFindScopeAddEditorNamed` 登记文档。`xuiCodeEditOpenFind`／`OpenReplace` 自动使用该编辑器所属查找域的同一个内建窗口；无查找域时仍使用独立窗口。
- 宿主切换活动文档时调用 `xuiCodeFindScopeSetActiveEditor(scope, editor)`，无活动文档传 NULL。它不会打开窗口、改变查询或抢焦点。`GetActiveEditor`／`GetFindWindow` 可查询目标和窗口。
- Scope 勾选框只改变搜索范围。替换当前匹配及全部替换始终作用于活动编辑器，保留原生撤销。只读／NULL 目标不能替换；NULL 目标仍可搜索所有成员。
- `xuiCodeFindScopeFindNext(scope, previous)` 在已打开的内建窗口中导航；宿主通过原有 `SetActivate` 回调切换文档标签。回调中的结果是调用期快照，回调内销毁查找域仍可读取它。
- `xuiCodeFindScopeRefresh` 立即刷新窗口当前查询；表达式错误通过原生状态栏交付。输入变化采用 80ms 合并，正常事件循环需要调用 `xuiUpdate`。宿主不需实现搜索、结果表或替换逻辑。
- 查询、替换、选项和窗口由独立的原生复合控件管理。销毁成员会自动退订并清除过期结果，不销毁共享会话。查找域或 context 销毁时清理窗口；可按任意先后顺序清理成员、域和 context。
- 窗口使用默认界面字体和 XUI 布局；内容单独滚动，分隔线和操作按钮固定，支持调整尺寸和最小尺寸。窗口／控件指针只供查询或受支持的属性设置，生命周期属于 XGE，不可由宿主单独销毁内部控件。

专项检查：`tests/xui/build_code_find_session_test.bat`。IDE 集成、真实输入、字体／DPI 和跨标签场景由 xlang 的 `find_follow_view_test` 覆盖。
