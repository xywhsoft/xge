/* ch104 — Material 与 Mesh */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_material_t material;
	xge_mesh_t mesh;
	xge_vertex_t vertices[4];
	uint16_t indices[6] = {0, 1, 2, 2, 3, 0};
	unsigned char pixels[32 * 32 * 4];
	xge_texture_t tex;
	int x, y, ret;

	/* 生成纹理 */
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 32; x++) {
			int idx = (y * 32 + x) * 4;
			pixels[idx + 0] = (unsigned char)(x * 8);
			pixels[idx + 1] = (unsigned char)(y * 8);
			pixels[idx + 2] = 180;
			pixels[idx + 3] = 255;
		}
	}
	memset(&tex, 0, sizeof(tex));
	xgeTextureCreateRGBA(&tex, 32, 32, pixels);

	/* 初始化 Material */
	xgeMaterialInit(&material);
	xgeMaterialSetTexture(&material, &tex);
	xgeMaterialSetColor(&material, XGE_COLOR_RGBA(255, 255, 255, 255));

	/* 创建 Mesh（四边形） */
	memset(vertices, 0, sizeof(vertices));
	vertices[0].fX = 80.0f;  vertices[0].fY = 60.0f;  vertices[0].fU = 0.0f; vertices[0].fV = 0.0f; vertices[0].iColor = 0xFFFFFFFF;
	vertices[1].fX = 320.0f; vertices[1].fY = 60.0f;  vertices[1].fU = 1.0f; vertices[1].fV = 0.0f; vertices[1].iColor = 0xFFFFFFFF;
	vertices[2].fX = 320.0f; vertices[2].fY = 260.0f; vertices[2].fU = 1.0f; vertices[2].fV = 1.0f; vertices[2].iColor = 0xFFFFFFFF;
	vertices[3].fX = 80.0f;  vertices[3].fY = 260.0f; vertices[3].fU = 0.0f; vertices[3].fV = 1.0f; vertices[3].iColor = 0xFFFFFFFF;

	ret = xgeMeshCreate(&mesh, vertices, 4, indices, 6, 0);
	if (ret == XGE_OK) {
		xgeMeshDraw(&mesh, &tex, 0);
		xgeMeshFree(&mesh);
	}

	/* Material Draw 方式 */
	{
		xge_draw_t draw;
		memset(&draw, 0, sizeof(draw));
		draw.pTexture = &tex;
		draw.tSrc.fW = 32.0f; draw.tSrc.fH = 32.0f;
		draw.tDst.fX = 420.0f; draw.tDst.fY = 60.0f;
		draw.tDst.fW = 200.0f; draw.tDst.fH = 200.0f;
		draw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeMaterialDraw(&material, &draw);
	}

	xgeMaterialFree(&material);
	xgeTextureFree(&tex);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch104", argc, argv); }
