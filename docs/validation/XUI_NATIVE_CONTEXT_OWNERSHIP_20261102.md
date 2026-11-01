# XUI 原生 context 拥有权检查

2026-11-02，xlang 声明式绑定前置修复。此项仅检查 XUI 的原生生命周期，尚未接入语言 owner／失效视图。

修复前的独立复现：创建两个未挂树的自定义控件，调用 `xuiDestroy`，析构计数为 0，预期为 2。原实现只遍历 root／overlay，未登记其余仍属于 context 的实例。

修复采用 context 内的侵入式实例登记，不复制控件树。成功分配布局后、执行任意类型初始化回调前即登记实例并保护类型；物理销毁前摘除登记。context 清理 root／overlay 后继续回收剩余实例。已成功初始化的类型链与失败初始化的回滚分开，避免自毁队列中的实例被提前释放，或父级析构重复执行。初始化期间请求关闭 context 后，创建调用不会返回已释放的指针；关闭期间重新创建会拒绝。

`tools/test/check_xui_lifetime.py` 将新专项和既有拖放重入测试按同一构建配置执行：

```text
python tools/test/check_xui_lifetime.py --out <fresh-directory> --cache <object-cache>
```

Linux 可追加 `--sanitize`，启用 ASan／UBSan／泄漏检查。测试仅使用无窗口的 CPU 路径，不创建网络连接。

- Windows GCC O0/O2：全部通过。
- Linux GCC O0/O2 + ASan/UBSan：全部通过，构建输入核对一致。
- 未挂树、混合树与脱离实例、初始化期间销毁自身／context、初始化失败的父级回滚均通过。
- 每种配置覆盖 6 个逻辑分配失败位置，活动分配与字节数回到起点，无无效或重复释放。
- 既有拖放重入每种配置 47 项全部通过。

证据：Windows `F:/xlang-validation/xge-lifetime-20261102/registry-gates-r1/results.json`；Linux `/home/ubuntu/xlang-validation/xge-lifetime-20261102-r1/registry-asan-r2/results.json`。失败复现记录位于同批 `baseline/`。

Windows x64 默认配置实测：widget 由 1960 增至 1976 字节，context 由 143328 增至 143336 字节。登记不分配额外堆对象；插入／摘除为 O(1)，context 收尾为 O(n)。这些是原生实例管理成本，不应宣称整套绑定已实现零开销。
