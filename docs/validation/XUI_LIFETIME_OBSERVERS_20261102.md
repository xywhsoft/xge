# XUI 弱生命周期通知

`xuiContextLifetimeWatch` / `xuiWidgetLifetimeWatch` 供外部受管对象观察原生实例的逻辑关闭。`xuiLifetimeUnwatch` 注销监听，不需要读取原生对象；原生对象释放后重复注销仍安全。

监听记录由调用者提供，必须清零并保持地址稳定。监听是弱引用，不延长控件、context、回调代码或 userdata 的生命周期。操作和通知遵守原有 UI 线程约束。通知前先从链表摘除并清零记录，回调可以释放自身记录，或注销、释放尚未通知的记录。回调必须正常返回。

```c
struct widget_view {
    xui_lifetime_link_t lifetime;
    xui_widget native;
};
static void clear_view(void* user)
{
    ((struct widget_view*)user)->native = NULL;
}
/* view is zero-initialized, stable storage; widget is live. */
/* int status = xuiWidgetLifetimeWatch(widget, &view.lifetime, clear_view, &view); */
/* Before releasing view: xuiLifetimeUnwatch(&view.lifetime); */
```

context 开始关闭时，先通知 context 及全部登记控件（包含未挂树实例）；单独销毁控件时先标记完整子树，再通知。通知在用户析构和物理释放前完成。通知期间使用原生操作作用域防止重入释放正在遍历的对象；关闭中的实例拒绝新增监听。

登记和注销均为 O(1)，不分配内存；通知为 O(监听数)。每个 context / widget 增加一个链表头，每个外部监听记录在 64 位平台为四个指针。这是未来语言层控制记录的底座，尚不代表语言拥有图、跨线程退役或捕获回调已经接入。

验收（离线 CPU 路径，无窗口／网络）：

- Windows GCC O0/O2：生命周期、弱监听、既有拖放重入三组全部通过，构建输入一致。
- Linux GCC O0/O2 + ASan/UBSan/泄漏检查：上述三组全部通过，构建输入一致。
- 覆盖零分配登记／注销、非法与重复登记、显式关闭立即失效、回调重入销毁、回调释放自身和其他监听、构造期间关闭及失败回滚；无无效／重复释放，活动分配回到起点。

证据：`F:/xlang-validation/xge-lifetime-20261102/observer-win-r2/results.json` 和 `/home/ubuntu/xlang-validation/xge-lifetime-20261102-observer-r1/observer-asan-r1/results.json`。入口为 `tools/test/check_xui_lifetime.py`；Linux 追加 `--sanitize`。
