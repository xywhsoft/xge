# Material Mesh Buffer Lab

`examples/material_mesh_buffer_lab` 覆盖 buffer、mesh 和 material 三组底层渲染 API。范例创建一张程序纹理，先显式创建、更新并上传 vertex/index buffer，再用 material 绘制一个旋转 quad，用 mesh 绘制另一个 indexed quad。

## 覆盖 API

- `xgeBufferCreate`
- `xgeBufferUpdate`
- `xgeBufferUpload`
- `xgeBufferFree`
- `xgeMeshCreate`
- `xgeMeshUpdate`
- `xgeMeshDraw`
- `xgeMeshFree`
- `xgeMaterialInit`
- `xgeMaterialSetShader`
- `xgeMaterialSetTexture`
- `xgeMaterialSetColor`
- `xgeMaterialSetBlend`
- `xgeMaterialDraw`
- `xgeMaterialFree`

## 构建和运行

```bat
examples\material_mesh_buffer_lab\build.bat
build\xge_material_mesh_buffer_lab.exe --frames 120
```

## 验证点

- 控制台输出 `material-mesh-buffer-lab init`。
- `buffer=1/1/1` 表示 create/update/upload 都成功。
- `mesh=1/1/1` 表示 mesh create/update/draw 都进入成功路径。
- `material=1/1/1/1/1/1` 表示 material 初始化、shader、texture、color、blend 和 draw 都进入成功路径。
- 窗口中左侧可见 material 绘制的旋转纹理，右侧可见 mesh indexed quad。
