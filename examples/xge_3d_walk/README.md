# 天空、随机岛屿与角色行走

按经典 3D 教学范例的方式组织：初始化 → 创建场景 → 读取输入 → 更新角色和相机 → 绘制。新增实现使用 C，数组、随机数和路径复用 xrt，运行时只调用公开 XGE API。

## 运行

Windows 下从仓库根目录执行，或直接双击 `run.bat`：

```bat
examples\xge_3d_walk\run.bat
```

需要 Python 3 与 GCC，和现有构建入口相同。首次构建后，也可以直接打开 `build/xge-3d-walk/xge_3d_walk.exe`；资源从 EXE 所在目录读取，不依赖工作目录。不需要联网或下载模型。

| 操作 | 按键 |
| --- | --- |
| 相对相机方向行走 | WASD，或 ↑/↓ |
| 跑步 / 跳跃 | 左右 Shift / Space |
| 旋转第三人称相机 | 按住鼠标右键拖动，或 Q/E |
| 相机远近 | 鼠标滚轮 |
| 切换第一/第三人称 | F |
| 返回出生点 | R |
| 生成下一颗种子的岛屿 | G |
| 退出 | Esc |

自动演示、截图与复现：

```bat
examples\xge_3d_walk\run.bat --seed 20261003 --autopilot --frames 600 --capture artifacts/xge-3d/island-walk.png
examples\xge_3d_walk\run.bat --first-person --seed 42
```

默认种子固定，方便教学复现；不同种子会生成不同地形。截图保存完整 3D 场景，窗口另有轻量操作提示。默认持续运行，只有指定 `--frames` 才自动退出。

## 读源码的顺序

1. `make_terrain`：xrt 随机数生成五层平滑噪声，形成 257×257、384 米见方的高度场；岛屿边缘降到海平面之下。按高度生成沙滩、草地和岩石颜色贴图，再调用 `xge3dTerrainCreate/Instantiate`。64 个块、四级 LOD；角色和相机附近保留精细网格。
2. `make_sky`、`make_sea`：六张方向一致的天空盒图像由代码生成。普通网格组成 y=0 的水平海面，远处与天空交接，形成可见海天线。
3. `make_actor`：加载自带 `assets/explorer.gltf`，实例化骨骼角色，取得待机、行走、跑步和跳跃四个动作。资源由 `generate_assets.py` 生成，可重复生成；没有外部素材许可依赖。
4. `update_player`：归一化移动输入，按时间计算速度；以最多 1/120 秒的步长更新位置。通过 `xge3dTerrainSample` 查询真实三角形高度与坡度，限制陡坡和海岸，应用跳跃重力，然后切换动作、写回角色变换。
5. `camera`：第三人称相机沿视线采样地形，缩短被遮挡的相机臂，并保持相机高于地表；第一人称隐藏角色模型。
6. `frame`：太阳级联阴影、天空颜色的补光、天空盒和库中的线性距离雾共同绘制；随后使用基础 2D 显示操作提示。

## 库与范例的边界

本次补充通用的 `xge3d_fog_settings_t`、`xge3dFogDefault` 与 `render_desc.fog`。雾按相机前向深度混合线性颜色，再进行曝光和 sRGB 转换；天空盒保持自身颜色。`XGE3D_ENABLE_FOG=0` 可以移除接口与着色器实现，使用者与库必须采用同一配置。同时补充公开的左右 Shift 键常量。

岛屿形状、角色速度、坡度阈值、跳跃和海岸规则属于本范例。这里的海面是静态普通平面，没有波浪模拟、反射或游泳；角色贴地与跳跃仅针对高度场，没有通用碰撞解算或导航。相机检测也是范例规则。

构建使用独立的 `build/xge-3d-walk/`，配置为 3D + 基础 2D，关闭文字、ShapeEx、XUI、粒子、IBL 和异步加载。提示字形在应用中绘制，不引入字体库；不修改其他程序的 profile。

## 验证记录

2026-10-03，Windows x64、GCC 16.2.0：本范例 DLL 为 915,721 B，约 894 KiB，自带角色文件 78,957 B。完整纯 3D DLL 为 874,249 B；本次距离雾相对上一版纯 3D 增加 512 B。

已运行 600 帧自动演示：行走 35.20 米，三次跳跃与落地，一次地形重新生成，三张太阳阴影图。另一个种子的第一人称运行 240 帧通过。测试直接调用本范例的实际控制代码，验证种子复现、重建后角色保留、斜向速度归一化、跑步倍率、帧率独立、贴地、跳跃、海岸限制和相机避开地表。

`3d` 与 `full-dev` 各 19 项 3D 验证通过；距离雾关闭、光照关闭和全部可选子模块关闭的配置通过声明、导出、着色器字符串及实际 GPU 检查。旧 default 保留 5,232 个旧导出，17 个结构布局与原基线一致，DLL 体积未增加。证据和截图保存在 `artifacts/xge-3d/walk-evidence.json`。

```bat
python test/build_3d_suite.py fog 3d
python test/build_3d_suite.py all 3d
python test/build_3d_suite.py all full-dev
python test/check_3d_trimming.py FOG LIGHTING BARE --evidence-prefix walk-trim
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL -I. -include build/xge-3d-walk/xge_build_config.h test/test_3d_walk.c build/xge-3d-walk/xge.lib -lm -o build/xge-3d-walk/test_3d_walk.exe
build\xge-3d-walk\test_3d_walk.exe
```
