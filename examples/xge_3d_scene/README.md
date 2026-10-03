# 模型场景范例

通过公开 C API 加载 GLB/glTF，创建两个共享资源的独立实例，放置在场景中，显示六面天空盒。左键通过屏幕射线拾取三角形并着色，输出所属实例和距离。应用自行决定对象摆放，不包含游戏业务。

```bat
examples\xge_3d_scene\build.bat 3d
build\3d\xge_3d_scene.exe --frames 3 --capture artifacts\xge-3d\p2-demo.png
build\3d\xge_3d_scene.exe --model path\model.glb
```

脚本生成小型确定性模型作为默认输入。其他模型保留源文件尺寸和层级，请按模型范围调整代码中的放置与相机位置。不指定帧数时持续运行，Escape 退出。GPU 对象在帧回调内释放。
