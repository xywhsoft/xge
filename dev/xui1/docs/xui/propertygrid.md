# XUI PropertyGrid

`PropertyGrid` 是属性检查器控件，本质上是 `TableGrid` 的专用封装：一级分类行负责折叠展开，属性行固定为“属性名 / 属性值”两列，属性值列复用 `TableGrid` 的全部显示和编辑能力。

它不再保留旧版私有滚动、私有输入框、私有枚举弹窗和旧 editor 枚举。属性表的核心价值是更方便地描述属性模型、分类、默认值、状态和回调。

## 设计边界

- 只支持一级分类，不支持分类嵌套。
- 不显示表头，分类行自身承担分组标题职责。
- 不显示横向滚动条；控件宽度变化时两列会压缩到当前视口内。
- 默认采用单击编辑模式。
- 属性名列不可编辑，用风格化背景表达只读含义。
- 属性值列可编辑，编辑类型使用 `XGE_XUI_TABLE_CELL_TYPE_*`。
- 分类行是合并单元格，点击整行折叠或展开。
- 描述信息默认通过 widget 内置 tooltip 展示。
- `picker` / `file` / `image` / `custom` 不内建弹窗，只触发回调，由业务层决定如何编辑和写回。

## 属性模型

```c
typedef struct xge_xui_property_desc_t {
    const char* sId;
    const char* sName;
    const char* sDescription;
    int iType;
    const char* sValue;
    const char* sDefaultValue;
    int iFlags;
} xge_xui_property_desc_t;
```

`sId` 必须唯一。`sName` 是显示文本，允许重复。值按字符串保存，类型由 `iType` 决定，常用类型可以通过 bool/int/float/color helper 读写。

## 类型

支持全部 TableGrid 单元格类型：

- `TEXT`
- `INT`
- `FLOAT`
- `BOOL`
- `TEXTAREA`
- `DATE`
- `TIME`
- `DATETIME`
- `ENUM`
- `COLOR`
- `PICKER`
- `CUSTOM`
- `FILE`
- `IMAGE`

## 状态

```c
#define XGE_XUI_PROPERTY_FLAG_READONLY  0x0001
#define XGE_XUI_PROPERTY_FLAG_DISABLED  0x0002
#define XGE_XUI_PROPERTY_FLAG_DIRTY     0x0004
#define XGE_XUI_PROPERTY_FLAG_INVALID   0x0008
#define XGE_XUI_PROPERTY_FLAG_HIDDEN    0x0010
```

如果设置了 `sDefaultValue`，控件会自动比较当前值和默认值来显示 dirty 标记。也可以显式设置 `DIRTY`。

## C API

```c
int xgeXuiPropertyGridInit(xge_xui_property_grid grid, xge_xui_context context, xge_xui_widget widget);
void xgeXuiPropertyGridUnit(xge_xui_property_grid grid);
void xgeXuiPropertyGridClear(xge_xui_property_grid grid);

int xgeXuiPropertyGridAddCategory(xge_xui_property_grid grid, const char* id, const char* name, int expanded);
int xgeXuiPropertyGridAddProperty(xge_xui_property_grid grid, int category, const xge_xui_property_desc_t* desc);

int xgeXuiPropertyGridFindCategory(xge_xui_property_grid grid, const char* id);
int xgeXuiPropertyGridFindProperty(xge_xui_property_grid grid, const char* id);

int xgeXuiPropertyGridSetValue(xge_xui_property_grid grid, int property, const char* value);
const char* xgeXuiPropertyGridGetValue(xge_xui_property_grid grid, int property);

int xgeXuiPropertyGridSetBool(xge_xui_property_grid grid, int property, int value);
int xgeXuiPropertyGridGetBool(xge_xui_property_grid grid, int property, int defaultValue);
int xgeXuiPropertyGridSetInt(xge_xui_property_grid grid, int property, int value);
int xgeXuiPropertyGridGetInt(xge_xui_property_grid grid, int property, int defaultValue);
int xgeXuiPropertyGridSetFloat(xge_xui_property_grid grid, int property, float value);
float xgeXuiPropertyGridGetFloat(xge_xui_property_grid grid, int property, float defaultValue);
int xgeXuiPropertyGridSetColor(xge_xui_property_grid grid, int property, uint32_t color);
uint32_t xgeXuiPropertyGridGetColor(xge_xui_property_grid grid, int property, uint32_t defaultValue);
```

编辑器配置复用 `xge_xui_table_grid_editor_config_t`：

```c
void xgeXuiPropertyGridSetEditorConfig(
    xge_xui_property_grid grid,
    int property,
    const xge_xui_table_grid_editor_config_t* config);
```

## 回调

```c
void xgeXuiPropertyGridSetSelect(xge_xui_property_grid grid, xge_xui_property_grid_select_proc proc, void* user);
void xgeXuiPropertyGridSetValidate(xge_xui_property_grid grid, xge_xui_property_grid_validate_proc proc, void* user);
void xgeXuiPropertyGridSetChange(xge_xui_property_grid grid, xge_xui_property_grid_change_proc proc, void* user);
void xgeXuiPropertyGridSetGlobalAction(xge_xui_property_grid grid, xge_xui_property_grid_action_proc proc, void* user);
void xgeXuiPropertyGridSetAction(xge_xui_property_grid grid, int property, xge_xui_property_grid_action_proc proc, void* user);
void xgeXuiPropertyGridSetRenderer(xge_xui_property_grid grid, int property, xge_xui_property_grid_render_proc proc, void* user);
```

提交流程：

1. TableGrid 打开编辑器。
2. 提交前调用 `validate`。
3. 校验失败时编辑器保持打开，并显示错误状态。
4. 校验通过后写回属性值。
5. 值变化时触发 `change`。

## XSON

```json
{
  "type": "propertyGrid",
  "nameWidth": 165,
  "rowHeight": 25,
  "categoryHeight": 27,
  "descriptionMode": "tooltip",
  "categories": [
    {
      "id": "appearance",
      "name": "Appearance",
      "expanded": true,
      "properties": [
        {
          "id": "appearance.accent",
          "name": "Accent Color",
          "description": "Color uses ColorPicker editor",
          "type": "color",
          "value": "#2E7CD6",
          "defaultValue": "#2E7CD6"
        },
        {
          "id": "appearance.theme",
          "name": "Theme",
          "type": "enum",
          "value": "Blue",
          "options": ["Blue", "Green", "Orange", "Purple"]
        }
      ]
    }
  ]
}
```

## 范例

- `examples/xui_propertygrid`
- `examples/xui_propertygrid_xson`

范例覆盖：分类折叠、属性名只读风格、bool、int、float、enum、color、textarea、picker、custom renderer、readonly、disabled、dirty、invalid、XSON 加载。
