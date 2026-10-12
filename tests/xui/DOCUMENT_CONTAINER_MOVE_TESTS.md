# Document 容器与通用块移动验证

此入口为统一 Document 内核构建 29 个全新编译单元、零复用，执行 13 个程序、56 条实际编译/归档/运行命令。

- 既有容器、嵌套、首槽、脚注、相邻移动 396 个来源黄金与 60 组完整 OOM/取消扫描，15 个原 C/H 文件字节不变。
- 新增标题、围栏代码、完整列表、表格、空引用/空列表项/空脚注、裸空标记以及无段落前缀样本：27 个独立前后源码黄金，LF/CRLF/CR × BOM × EOF 共 324 个组合。
- 新增 54 组逐点 OOM/取消扫描：状态、来源、修订、保存点、通知、事务毒化、重试、所有旧节点身份、快照、撤销/重做、共享来源 Blob 和零泄漏。
- 合计 720 个来源组合和 114 组故障扫描；两类覆盖分别报告，不与外部 336 个块移动组合重复相加。
- 保留原六个 1/10MiB 持久来源大正文验证；不据此声称完整 UI、固定机器 P95 或 RSS 达标。
- 保留被移动内容的原标记、实体及字面代码；只有解析器确认的祖先前缀及必要分隔发生变化。空容器从自己的语法/引用记录取得前缀。

```powershell
python tests/xui/build_document_container_move_test.py --out-dir build/document-container-normal
python tests/xui/build_document_container_move_test.py --out-dir build/document-container-collision --variant collision
```

Linux 默认启用 ASan/UBSan。每次必须选择新输出目录；report.json 记录真实子命令、日志散列、源码输入与运行产物，失败不覆盖既有记录。完整 Core、旧块移动与旧 56 组故障由联合控制器对同一新归档继续验证。

黄金修正记录保留：不同列表必须使用能被解析器分开的标记；目标未修改的尾部空白在移动后成为 EOF 时仍保留。语义、来源和历史断言未放宽。

## Editor 对外部 Document 移动事务的观察

新增入口使用调用方指定的实际共享库、构建配置和导入库，运行27个结构场景×3初始模式×3选区方向，共243个用例。验证外部根事务发布、精确源码/HTML/所有旧节点身份、有效选区、Editor的共同Undo/Redo与三模式显示；使用proxy后端，不代替原生窗口或物理IME验收。

```powershell
python tests/xui/build_document_block_kind_move_editor_test.py --build-dir build --out-dir build/document-block-kind-editor
```

该入口不重新编译产品库，也不将已有运行库视作新构建；report.json记录实际编译/运行退出码、运行库、配置、输入、产物及日志散列。


## Sole-child container moves

The same fresh standalone kernel entry also runs 1,260 independently declared
source/HTML/all-NodeId/snapshot/Undo/Redo variants, including paragraphs, headings,
fences, lists and tables moved from a quote, list item or footnote to root or any
other such container, before and after its original physical location. It keeps
the source container's empty opener and shares moved source payload blobs.

66 exhaustive OOM/cancellation scans cover all 15 root fixtures plus the 18
paragraph cross-container direction fixtures. The original fault invariants,
notification/history/savepoint/retry and zero-leak checks remain unchanged.
Other subtrees are covered by the complete 1,260 source matrix; this does not
claim that all 105 fixtures separately underwent allocation-failure sweeps.

The entry builds 29 units without reuse and executes 15 programs /60 actual
compiler/archive/runtime commands. It retains the original 720 source variants,
114 fault scans and six 1/10MiB cases; totals are 1,980 variants and180 scans.


## Sole-child Editor observer

`build_document_sole_child_move_editor_test.py --source-root ROOT --build-dir
MATCHING_SHARED_LIBRARY_DIR --out-dir NEW_DIR` compiles and runs 945 cases:
105 source fixtures times three initial modes and three selection directions.
The transaction uses the public Document move API; an existing Editor observes
the notification, valid mapped selection and common Undo/Redo, then switches
SOURCE/LIVE/VISUAL without content or revision drift. The backend is a proxy;
this is not physical IME/accessibility or a new arbitrary-reparent GUI command.

The new entry preserves the separate old243 observer and252 Query/Execute
command tests. Its compile/run/input/runtime hashes are reported. The core C/H
and core builder are identical to the23-file package already under fresh tests.


Task host and Tab phase cases add216 independent source variants and36 exhaustive fault scans. Empty task items retain a canonical paragraph with a stable new identity across Redo. Code moves use parser column anchors across footnote dedentation; only required indent changes, and literal payload remains shared. The entry now runs68 real compile/archive/test commands with29 freshly compiled kernel units, zero reuse,2196 source cases and216 fault scans; full Document acceptance still requires repository/UI/platform gates.


Permanent Editor regression: `python tests/xui/build_document_move_editor_suite.py --source-root . --build-dir <matching-build> --out-dir <new-directory>` supports Windows and Linux. All five entries share one runtime and retain their original assertions:252 block,243 block-kind,945 sole-child,81 task/Tab and81 code-Tab cases (1602 total),ten actual compile/run checks. `build_document_suite.bat` runs all five entries. These proxy checks do not replace native windows, physical IME, readers or DPI acceptance.
