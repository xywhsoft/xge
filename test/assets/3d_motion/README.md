# 外部动作验证来源

素材作者：Kay Lousberg / KayKit，Adventurers 1.0；许可证为 CC0 1.0。原始 [项目](https://github.com/KayKit-Game-Assets/KayKit-Character-Pack-Adventures-1.0) 与 [许可证](LICENSE-KayKit.txt)；固定 commit、下载地址、大小及 SHA-256 见 [kaykit.json](kaykit.json)。

`python test/fetch_3d_motion_assets.py` 验证缓存，缺少时下载 Knight.fbx 与许可证到 `artifacts/xge-3d/external/kaykit/`。下载与工具依赖不进入运行库构建；没有付费、登录或云转换步骤。固定源文件含 76 个动作片段；当前验收选用 Idle、Walking_A、Running_A、Jump_Full_Long、1H_Melee_Attack_Slice_Horizontal、PickUp，分别对应待机、行走、奔跑、跳跃、挥砍与拾取采集。

`test/make_3d_rigs.py` 生成项目自己的两种测试角色及 JSON 映射。它们有不同骨长、T/A 参考姿态和局部轴，使用真正的网格与 inverse bind 数据；不是复制模型的换色验证。转换及重定向由 C 工具执行，Python 只负责下载、构建、测试资源生成与验收调度。

ufbx v0.23.1 只用于离线工具，固定来源和 MIT/Unlicense 许可见 `lib/ufbx/SOURCE.json` 与 `lib/ufbx/LICENSE`。glTF 解析沿用运行库已有的 cgltf v1.15。工具自己的 xrt 配置只启用所需容器、文件、Base64 与 JSON，未扩大 DLL 的 xrt 配置。
