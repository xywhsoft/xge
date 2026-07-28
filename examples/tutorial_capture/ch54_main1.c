/* ch54 — 混合模式（ShapeEx 级）*/
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pBase = NULL;
	xge_shape_ex pOverlay = NULL;

	/* 底色圆形 */
	xgeShapeExCreate(&pBase);
	xgeShapeExAppendCircle(pBase, 200, 150, 90, 90, 1);
	xgeShapeExFillColor(pBase, XGE_COLOR_RGBA(54, 162, 235, 255));
	xgeShapeExDraw(pBase, 0.5f);
	xgeShapeExDestroy(pBase);

	/* Add 混合 */
	xgeShapeExCreate(&pOverlay);
	xgeShapeExAppendCircle(pOverlay, 260, 150, 90, 90, 1);
	xgeShapeExFillColor(pOverlay, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeExBlend(pOverlay, XGE_BLEND_ADD);
	xgeShapeExDraw(pOverlay, 0.5f);
	xgeShapeExDestroy(pOverlay);

	/* Multiply 混合 */
	xgeShapeExCreate(&pBase);
	xgeShapeExAppendCircle(pBase, 200, 380, 90, 90, 1);
	xgeShapeExFillColor(pBase, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeExDraw(pBase, 0.5f);
	xgeShapeExDestroy(pBase);

	xgeShapeExCreate(&pOverlay);
	xgeShapeExAppendCircle(pOverlay, 260, 380, 90, 90, 1);
	xgeShapeExFillColor(pOverlay, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExBlend(pOverlay, XGE_BLEND_MULTIPLY);
	xgeShapeExDraw(pOverlay, 0.5f);
	xgeShapeExDestroy(pOverlay);

	/* Screen 混合 */
	xgeShapeExCreate(&pBase);
	xgeShapeExAppendCircle(pBase, 550, 150, 90, 90, 1);
	xgeShapeExFillColor(pBase, XGE_COLOR_RGBA(153, 102, 255, 255));
	xgeShapeExDraw(pBase, 0.5f);
	xgeShapeExDestroy(pBase);

	xgeShapeExCreate(&pOverlay);
	xgeShapeExAppendCircle(pOverlay, 610, 150, 90, 90, 1);
	xgeShapeExFillColor(pOverlay, XGE_COLOR_RGBA(255, 206, 86, 200));
	xgeShapeExBlend(pOverlay, XGE_BLEND_SCREEN);
	xgeShapeExDraw(pOverlay, 0.5f);
	xgeShapeExDestroy(pOverlay);

	/* Overlay 混合 */
	xgeShapeExCreate(&pBase);
	xgeShapeExAppendCircle(pBase, 550, 380, 90, 90, 1);
	xgeShapeExFillColor(pBase, XGE_COLOR_RGBA(255, 99, 132, 255));
	xgeShapeExDraw(pBase, 0.5f);
	xgeShapeExDestroy(pBase);

	xgeShapeExCreate(&pOverlay);
	xgeShapeExAppendCircle(pOverlay, 610, 380, 90, 90, 1);
	xgeShapeExFillColor(pOverlay, XGE_COLOR_RGBA(75, 192, 192, 200));
	xgeShapeExBlend(pOverlay, XGE_BLEND_OVERLAY);
	xgeShapeExDraw(pOverlay, 0.5f);
	xgeShapeExDestroy(pOverlay);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch54", argc, argv);
}
