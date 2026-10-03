# 3D 范例

当前演示相机、节点层级、共享网格、GPU 变换、深度遮挡与颜色。模型导入、材质、光照和动画将随对应工作包加入。

```bat
examples\xge_3d\build.bat 3d
build\3d\xge_3d.exe --frames 3 --capture artifacts\xge-3d\p1-demo.png
```

不指定帧数则持续运行，Escape 退出。使用公开 API，网格和场景可在创建窗口前准备；GPU 对象在帧回调内创建、使用和释放。两个节点持有同一网格，子节点随父节点旋转。
