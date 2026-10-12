/*
 * xanim_render.c - Render tree construction and ShapeEx drawing
 *
 * Builds the render tree from evaluated frame state and
 * draws using XGE ShapeEx API.
 */

#include "xanim.h"
#include "xge.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Render a single GRAPHIC symbol's path data                         */
/* ------------------------------------------------------------------ */

void xanimRenderGraphicSymbol(const uint8_t* pPathData, uint32_t iDataSize,
                              float fTx, float fTy, float fScaleX, float fScaleY,
                              float fRotation, float fOpacity)
{
	xge_shape_ex pShape;
	const uint8_t* p = pPathData;
	const uint8_t* pEnd = pPathData + iDataSize;
	xge_shape_ex_matrix_t matrix;

	if ( xgeShapeExCreate(&pShape) != XGE_OK )
		return;

	/* Parse command stream */
	while ( p < pEnd ) {
		uint8_t opcode = *p++;
		switch ( opcode ) {
		case XANIM_PATH_MOVE_TO:
			if ( p + 8 <= pEnd ) {
				float x, y;
				memcpy(&x, p, 4); p += 4;
				memcpy(&y, p, 4); p += 4;
				xgeShapeExMoveTo(pShape, x, y);
			}
			break;
		case XANIM_PATH_LINE_TO:
			if ( p + 8 <= pEnd ) {
				float x, y;
				memcpy(&x, p, 4); p += 4;
				memcpy(&y, p, 4); p += 4;
				xgeShapeExLineTo(pShape, x, y);
			}
			break;
		case XANIM_PATH_QUAD_TO:
			if ( p + 16 <= pEnd ) {
				float cx, cy, x, y;
				memcpy(&cx, p, 4); p += 4;
				memcpy(&cy, p, 4); p += 4;
				memcpy(&x, p, 4); p += 4;
				memcpy(&y, p, 4); p += 4;
				xgeShapeExQuadTo(pShape, cx, cy, x, y);
			}
			break;
		case XANIM_PATH_CUBIC_TO:
			if ( p + 24 <= pEnd ) {
				float c1x, c1y, c2x, c2y, x, y;
				memcpy(&c1x, p, 4); p += 4;
				memcpy(&c1y, p, 4); p += 4;
				memcpy(&c2x, p, 4); p += 4;
				memcpy(&c2y, p, 4); p += 4;
				memcpy(&x, p, 4); p += 4;
				memcpy(&y, p, 4); p += 4;
				xgeShapeExCubicTo(pShape, c1x, c1y, c2x, c2y, x, y);
			}
			break;
		case XANIM_PATH_CLOSE:
			xgeShapeExClose(pShape);
			break;
		default:
			/* Unknown opcode or paint data - skip */
			p = pEnd;
			break;
		}
	}

	/* Apply transform */
	xgeShapeExMatrixIdentity(&matrix);
	xgeShapeExMatrixTranslate(&matrix, fTx, fTy);
	/* Scale and rotation would be composed here */
	xgeShapeExTransformSet(pShape, &matrix);

	/* Set opacity via color alpha */
	{
		uint32_t alpha = (uint32_t)(fOpacity * 255.0f);
		if ( alpha > 255 ) alpha = 255;
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 255, 255, alpha));
	}

	xgeShapeExDraw(pShape);
	xgeShapeExDestroy(pShape);

	(void)fScaleX; (void)fScaleY; (void)fRotation;
}

/* ------------------------------------------------------------------ */
/* Render a sprite (texture quad)                                     */
/* ------------------------------------------------------------------ */

void xanimRenderSprite(uint32_t iTextureId, float fX, float fY,
                       float fW, float fH, float fOpacity)
{
	/* Sprite rendering uses XGE texture/sprite batch.
	 * Placeholder: draw colored rectangle via ShapeEx. */
	xge_shape_ex pShape;
	uint32_t alpha = (uint32_t)(fOpacity * 255.0f);

	if ( xgeShapeExCreate(&pShape) != XGE_OK )
		return;

	xgeShapeExMoveTo(pShape, fX, fY);
	xgeShapeExLineTo(pShape, fX + fW, fY);
	xgeShapeExLineTo(pShape, fX + fW, fY + fH);
	xgeShapeExLineTo(pShape, fX, fY + fH);
	xgeShapeExClose(pShape);

	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(180, 200, 220, alpha));
	xgeShapeExDraw(pShape);
	xgeShapeExDestroy(pShape);

	(void)iTextureId;
}

/* ------------------------------------------------------------------ */
/* Render full frame (all layers, all elements)                       */
/* ------------------------------------------------------------------ */

void xanimRenderFrame(xanim_player pPlayer, float fX, float fY)
{
	/* This is called by xanimPlayerRender.
	 * The actual implementation iterates layers and elements,
	 * evaluates transforms from curves, and draws each symbol.
	 * Full implementation is in xanim_player.c's Render function. */
	(void)pPlayer; (void)fX; (void)fY;
}
