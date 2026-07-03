#include "map_workspace.h"

static const mapedit_window_desc_t g_arrMapWindows[MAPEDIT_MAP_DOCK_WINDOW_COUNT] = {
	{ "map.list", "地图管理", "地图列表与当前地图切换", MAPEDIT_REGION_LEFT, 0, 1 },
	{ "map.tile_select", "图块选择", "当前地图图集的图块选择区域", MAPEDIT_REGION_LEFT, 1, 1 },
	{ "map.edit", "地图编辑", "地图画布占位", MAPEDIT_REGION_DOCUMENT, 0, 1 },
	{ "map.passage_adjust", "通行调整", "地图通行调整占位", MAPEDIT_REGION_DOCUMENT, 1, 1 },
	{ "map.object_edit", "对象编辑", "对象列表与对象编辑占位", MAPEDIT_REGION_DOCUMENT, 1, 1 },
	{ "map.event_edit", "事件编辑", "事件页与触发条件占位", MAPEDIT_REGION_DOCUMENT, 1, 1 },
	{ "map.tags", "地图打标", "地图标签编辑占位", MAPEDIT_REGION_DOCUMENT, 1, 1 },
	{ "map.properties", "地图属性", "地图属性检查器占位", MAPEDIT_REGION_RIGHT, 0, 1 }
};

static const mapedit_workspace_def_t g_tMapWorkspaceDef = {
	MAPEDIT_WORKSPACE_MAP,
	"地图编辑器",
	"option/layout_map.xson",
	"option/default_layout_map.xson",
	g_arrMapWindows,
	MAPEDIT_MAP_DOCK_WINDOW_COUNT
};

const mapedit_workspace_def_t* MapEditMapWorkspaceDef(void)
{
	return &g_tMapWorkspaceDef;
}
