/* ch31 — 多边形与 Mesh：五角星 + 顶点色渐变 */
#include "tut_capture.h"
#include <math.h>

#ifndef XGE_PI
#define XGE_PI 3.14159265358979323846f
#endif

static void draw_scene(void)
{
	xge_vec2_t pts[10];
	xge_shape_vertex_t verts[4];
	uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
	int i;

	/* 正五边形 */
	for ( i = 0; i < 5; i++ ) {
		float a = -XGE_PI / 2.0f + i * 2.0f * XGE_PI / 5.0f;
		pts[i].fX = 200.0f + 80.0f * (float)cos(a);
		pts[i].fY = 200.0f + 80.0f * (float)sin(a);
	}
	xgeShapePolygonFill(pts, 5, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 五角星 */
	for ( i = 0; i < 10; i++ ) {
		float a = -XGE_PI / 2.0f + i * XGE_PI / 5.0f;
		float rad = (i % 2 == 0) ? 120.0f : 50.0f;
		pts[i].fX = 550.0f + rad * (float)cos(a);
		pts[i].fY = 250.0f + rad * (float)sin(a);
	}
	xgeShapePolygonFill(pts, 10, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 顶点色渐变 Mesh */
	verts[0].fX = 150; verts[0].fY = 400; verts[0].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
	verts[1].fX = 450; verts[1].fY = 400; verts[1].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);
	verts[2].fX = 450; verts[2].fY = 530; verts[2].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);
	verts[3].fX = 150; verts[3].fY = 530; verts[3].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
	xgeShapeMeshFill(verts, 4, indices, 6);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch31", argc, argv);
}
