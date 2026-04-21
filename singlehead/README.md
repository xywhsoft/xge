# XGE single header

此目录用于放置发布用单头文件版本、生成器和单头文件测试代码。

仓库主源码不直接维护为单头文件。常规开发使用根目录 `xge.h` + `xge.c`，实现按功能模块放在 `src/`。发布时由本目录内的生成器把 `xge.h`、`xge.c` 和 `src/` 模块按固定顺序组装为单头文件版本。

计划文件：

- `gen_singlehead.*`：生成器脚本。
- `xge_single.h`：生成后的发布文件。
- `test_singlehead.c`：单头文件集成测试。

