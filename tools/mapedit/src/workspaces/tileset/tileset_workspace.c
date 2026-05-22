#include "tileset_workspace.h"

static const mapedit_window_desc_t g_arrTilesetWindows[MAPEDIT_TILESET_DOCK_WINDOW_COUNT] = {
	{ "tileset.make", "图块制作", "图块制作工具区占位", MAPEDIT_REGION_LEFT, 1, 1 },
	{ "tileset.passage", "通行编辑", "图块通行规则占位", MAPEDIT_REGION_LEFT, 1, 1 },
	{ "tileset.actor_overlay", "角色覆盖", "角色覆盖预览占位", MAPEDIT_REGION_LEFT, 1, 1 },
	{ "tileset.tags", "图块打标", "图块标签编辑占位", MAPEDIT_REGION_LEFT, 1, 1 },
	{ "tileset.materials", "图块素材", "图块素材列表占位", MAPEDIT_REGION_DOCUMENT, 0, 1 },
	{ "tileset.properties", "图块属性", "图块属性检查器占位", MAPEDIT_REGION_RIGHT, 0, 1 }
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
