# 等距视角深度排序

本案例说明 2D 等距视角中如何处理绘制顺序。

[返回范例解析](README.md) | [坐标教程](../guide/coordinate-intro.md) | [2.5D 教程](../guide/2.5d-intro.md)

## 问题

等距视角地图中，角色、建筑、树和道具需要按“屏幕靠下更靠前”的规则排序。多数情况下不需要真正 3D，只需要稳定排序。

## 排序键

```c
static float SortKey(float fTileX, float fTileY, float fZ)
{
	return fTileX + fTileY + fZ;
}
```

也可以直接使用对象脚底点的屏幕 y：

```c
object.fSortY = object.fFootY;
```

## 绘制顺序

```text
地面 tile
地面装饰
按 sort key 排序的对象
顶层 UI / debug
```

## 何时使用 2.5D

只有当对象确实需要透视倾斜、z buffer 或 3D quad 时才启用 2.5D。普通等距地图优先使用 2D 排序，批处理效率更高。

## 常见失败原因

对象脚底点不一致：排序应使用与视觉遮挡一致的参考点。

大对象遮挡错误：需要拆成底座和顶部两个层，或提供手工排序偏移。

频繁全量排序开销高：只在对象移动或层级变化时更新排序。
