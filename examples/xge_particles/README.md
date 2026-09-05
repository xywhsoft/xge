# XGE 粒子范例：七个独立场景

每个场景有独立配置源码和可执行程序，公共 `common.c` 只负责窗口、资源绑定、输入、统计与截图。没有 XUI、布局系统或 Sprite/Animation 对象依赖；纹理和序列帧图集由程序生成，不需要下载图片。

## 构建与运行

Windows / MinGW-w64，在仓库根目录执行：

```bat
build_dll.bat
examples\xge_particles\build.bat
build\xge_particles_impact.exe
```

库在 `build/xge.dll`，范例也在 `build/`，不需要向系统目录复制 DLL。Linux 入口为 `sh build_dll.sh` 和 `sh examples/xge_particles/build.sh`；需现有 XGE 的 X11/OpenGL 依赖，尚未在本轮进行 Linux 运行验证。

## 场景与能力对应

| 程序后缀 / 源码 | 场景 | 重点观察 |
| --- | --- | --- |
| `impact` / [impact.c](impact.c) | 命中火花 | Cone 爆发、闪光叠加、速度朝向/拉伸、重力、阻力、目标矩形和地面反弹、碰撞子发射器。 |
| `fire_smoke` / [fire_smoke.c](fire_smoke.c) | 火焰与烟雾 | 三发射器组合、线段源、持续发射、预热、加色火焰与 alpha 烟雾、尺寸/透明度/颜色变化、噪声、图集帧。 |
| `explosion` / [explosion.c](explosion.c) | 爆炸 | 圆盘爆发、分层闪光/火球/延迟烟团、死亡子火星、阻力、生命周期序列帧、定时重叠播放和缓存复用。 |
| `weather` / [weather.c](weather.c) | 雨雪 | 两块矩形发射区；雨线拉伸、地面销毁/水花子发射；雪花噪声漂移与旋转；预热、较多粒子、视口剔除与裁剪。 |
| `dust` / [dust.c](dust.c) | 移动尾尘 | 沿移动路径按距离均匀出生、世界空间留在身后、速度继承、烟尘/碎屑分层。源静止时不继续按秒发射。 |
| `magic` / [magic.c](magic.c) | 魔法光环 | 圆环/圆盘、本地空间随根变换转动、径向/切向力、Hermite 曲线、自定义 Shader 材质批绘制与混合。 |
| `confetti` / [confetti.c](confetti.c) | UI 彩纸 | 屏幕空间四发射器、两侧多时点爆发、非正方形彩纸、随机颜色组/旋转/角速度、重力/阻力、后段淡出。底板只用 XGE 形状和文字绘制。 |

完整文件名均为 `build/xge_particles_<后缀>.exe`。系统支持的所有 API 不适合都塞进同一画面：暂停/清空/可见性/倍率通过交互演示，JSON/XSON 通过导出重载演示，容量/错误/缓存/重入保护通过测试程序验证。参见 [粒子系统文档](../../docs/PARTICLES.md)。

## 交互

- `Space`：暂停/继续整个场景，并对当前实例调用粒子暂停 API。
- `R`：释放当前效果，按同一种子从头播放；重做预热、恢复自动重复和可见性。
- `D`：停止自动重复，让已有效果排空。长寿命光环的排空可能很久，可用 C。
- `C`：停止自动重复并立即清空；R 恢复。
- `Enter`：命中/爆炸/彩纸场景额外触发一次效果，示范多实例并存。
- `B`：显示主要实例的实时包围盒。
- `V`：切换所有当前及后续实例的绘制可见性；默认仍然模拟。
- `+` / `-`：按秒/按距离密度倍率，范围 0.25–4；**不改变 Burst 数量**，因此纯爆发场景不受这个控制影响。
- `Esc`：退出。

HUD 显示存活/峰值粒子、活动实例、累计碰撞与最近 CPU Update 耗时。重放清除当前效果，不清零 World 的累计统计。字体使用 Windows Consolas 或 Linux DejaVu Sans；字体缺失不阻止粒子运行，但标题/HUD 可能不显示。

## 自动运行、截图与配置

```bat
build\xge_particles_weather.exe --frames 120 --capture artifacts\particles_weather.png
build\xge_particles_fire_smoke.exe --frames 120 --seed 7
build\xge_particles_explosion.exe --export artifacts\explosion.json
build\xge_particles_explosion.exe --export artifacts\explosion.xson
build\xge_particles_explosion.exe --config artifacts\explosion.json --frames 120
build\xge_particles_impact.exe --config examples\xge_particles\presets\sparks.json
```

先确保截图/导出文件的父目录存在。`--frames N` 使用固定 `1/60` 秒帧输入并忽略交互热键，避免自动运行被实时键盘输入干扰；不指定时使用真实帧时间并启用热键。`--capture` 单独使用会默认跑 180 帧后保存并退出，输出是实际离屏 pass 读回的 PNG。`--seed N` 控制模拟随机种子；截图不会逐像素稳定，因为 HUD 还显示实际耗时。

`--export PATH` 只导出定义然后退出，不创建窗口；`.xson` 路径使用 XSON，其余用 JSON。`--config PATH` 替换当前场景的发射器定义，仍使用该场景的背景、播放位置、重复间隔和移动行为，**不是完整关卡存档**。内置资源名为 `soft`、`sheet`；魔法场景另有材质名 `ring`。跨场景加载配置时要自行保证材质名可绑定。

建议截图时点：impact 120、fire_smoke 120、explosion 130（火球）或 150（烟团/火星）、weather 120、dust 150、magic 120、confetti 90 帧。爆炸与彩纸是短暂效果，不同时间截取会得到不同阶段。

## 回归测试

一键完整复验：`test\build_particle_suite.bat`。按需单独运行：

```bat
test\build_particle_test.bat
test\build_particle_dll_test.bat
test\build_particle_render_test.bat
```

第一项完全不依赖窗口/GPU；第二项验证 DLL 导出的同一 API；第三项创建短生命周期窗口并读取像素，验证透明度/排序/自定义材质批次/资源引用。
