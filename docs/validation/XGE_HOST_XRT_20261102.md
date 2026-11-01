# XGE 共用宿主 XRT

独立 C 程序继续使用默认的内嵌 XRT 构建。xlang 集成使用显式的宿主模式：

```text
python tools/build/build.py default <output-directory> --host-xrt
```

宿主模式不定义 `XRT_IMPLEMENTATION`，不导出另一套 XRT。工具从固定版本的权威 `xrt.h` 声明生成精确类型的函数指针槽和函数 designator 宏；不是转发函数、动态操作码或 xvalue 调用。只为实际目标文件依赖的符号建立存储。未知声明、未知函数宏、遗漏的普通 XRT 引用和链接产物中的 XRT 实现符号均拒绝。

宿主必须在任何其他 XGE/XUI 调用之前，完成 `xgeHostXrtBind`。先核对版本化合同、规范化头文件 SHA256、目标数据模型和核心公开布局，再解析全部所需地址；缺少任一项不会发布部分槽。绑定后只允许完全相同的重复绑定，不提供换宿主或解绑。绑定是启动协议，不能与引擎调用并行。resolver 必须正常返回，不能调用引擎；回调和 userdata 不会被保留。宿主 XRT 的代码必须比引擎活得久。`include/xge_host_xrt.h` 记录完整调用合同。

生成的 `xge_host_xrt_contract.h` 必须在**宿主自己的匹配 XRT 声明**后编译，不能复制消费者所报告的值来冒充宿主合同。当前验证目标为 Windows x86-64 和 Linux x86-64，使用默认 C ABI／打包选项；不承诺任意编译器选项、其他架构或不同 XRT 配置布局可互换。

注册时保留 XRT 原有内存调试文件／行宏。开启这些宏时，取被包装的裸 allocator 函数地址尚不支持，会在编译或闭合符号检查中拒绝；普通调用仍走匹配的 `*At` 导入。宿主地址只有运行时才确定，因此不接受要求 C 常量地址的静态初始化；必须改为显式初始化。实际默认引擎已完整编译通过，没有为绕过限制添加地址转发 thunk。

调用成本：XGE 调用 XRT 时经过一个精确函数指针槽，与手写相同宿主链接方式的 C 一致。相对把 XRT 和调用者编入同一编译单元，这会失去相关内联机会并采用间接调用；不能把它宣称为相对旧单体实现的零成本。绑定只在启动时解析、检查地址；普通 XRT 调用不做名称查找或初始化检查。

验收：

- Windows GCC O0/O2，双模块独立测试：合同／缺符号／重入绑定、地址身份、拒绝另一个运行时、跨模块分配释放、共同内存计数与线程错误状态，及 XUI 生命周期全部通过。原生模块中 XRT 实现符号为零。
- Linux GCC O0/O2 + ASan/UBSan/泄漏检查，相同双模块测试全部通过。
- 完整默认引擎 O2 构建通过，实际需要 174 个 XRT 函数。与 xlang 发布目录的 `xl.dll`、`xlc.dll` 分别在独立进程绑定；每次三轮直接调用及一轮嵌入式 TCC JIT 调用均完成 context／控件创建和销毁，宿主分配计数恢复，没有缺失符号。
- 同一份源代码、相同功能开关、相同 GCC O2，默认独立引擎为 9,224,256 字节；宿主模式为 8,661,099 字节，净减少 **563,157 字节**（已计入导入槽、绑定代码和元数据）。此数是这一构建配置的实测，不是所有产品的固定收益。

证据入口：`tools/test/check_xge_host_xrt.py`；xlang 实际 DLL 入口为 `tools/check_xge_host_runtime.py`。报告在 `F:/xlang-validation/xge-host-xrt-20261102/` 的 `native-win-r3/`、`profile-win-r1/`、`profile-embedded-win-r1/`、`actual-xl-r2/`、`actual-xlc-r2/`；Linux 在 `/home/ubuntu/xlang-validation/xge-host-xrt-20261102-r1/host-asan-r1/`。

这是原生链接前置。xlang 包的自动初始化、受管控件／回调、SDK 和 IDE 的最终迁移仍按其各自生命周期合同继续实施；本次没有替换发布目录中的引擎 DLL。
