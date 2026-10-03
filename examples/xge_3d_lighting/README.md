# 光照与阴影范例

两种金属度/粗糙度、共享网格、太阳级联阴影、天空与可选静态 IBL。应用每帧更新光向和曝光，展示昼夜参数的接入位置；库不包含天气或时间业务。

```bat
examples\xge_3d_lighting\build.bat
build\3d\xge_3d_lighting.exe --frames 3 --ibl --capture artifacts\xge-3d\p3-demo.png
```

构建脚本先用 C 工具生成白色环境验证数据，IBL 是低强度补光。取消 `--ibl` 可观察纯直接光与阴影。不指定帧数则持续运行，Escape 退出。预过滤数据加载函数展示图片复制与释放顺序。实际环境可采用更大尺寸的六面图片交给 `tools/xge3d_ibl.c` 预处理。
