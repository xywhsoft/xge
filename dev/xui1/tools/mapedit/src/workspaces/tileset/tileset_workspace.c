#include "tileset_workspace.h"

static const mapedit_window_desc_t g_arrTilesetWindows[MAPEDIT_TILESET_DOCK_WINDOW_COUNT] = {
	{ "tileset.materials", "素材管理", "素材管理占位", MAPEDIT_REGION_LEFT, 1, 1 },
	{ "tileset.set", "图集管理", "TileSet、特殊图块槽位和静态图块集管理", MAPEDIT_REGION_LEFT, 1, 1 },
	{ "tileset.set_layout", "图集编排", "最终图集 ID、橡皮和独立素材槽位编排占位", MAPEDIT_REGION_DOCUMENT, 0, 1 },
	{ "tileset.passage", "通行编辑", "简单模式、专家模式和八方向通行工具占位", MAPEDIT_REGION_DOCUMENT, 0, 1 },
	{ "tileset.tags", "图块打标", "customData 通道选择和默认数据打标占位", MAPEDIT_REGION_DOCUMENT, 0, 1 },
	{ "tileset.actor_overlay", "角色覆盖", "图块遮盖状态和草木繁茂处配置占位", MAPEDIT_REGION_DOCUMENT, 0, 1 },
	{ "tileset.properties", "图块属性", "当前素材、图块、图集和通行数据属性占位", MAPEDIT_REGION_RIGHT, 0, 1 }
};

static const mapedit_workspace_def_t g_tTilesetWorkspaceDef = {
	MAPEDIT_WORKSPACE_TILESET,
	"图块编辑器",
	"option/layout_tileset.xson",
	"option/default_layout_tileset.xson",
	g_arrTilesetWindows,
	MAPEDIT_TILESET_DOCK_WINDOW_COUNT
};

const mapedit_workspace_def_t* MapEditTilesetWorkspaceDef(void)
{
	return &g_tTilesetWorkspaceDef;
}
