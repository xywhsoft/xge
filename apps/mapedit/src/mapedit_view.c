#include "mapedit_view.h"
#include <math.h>
float mapeditViewZoom(float zoom)
{
	if (!isfinite(zoom) || zoom <= 0.0f) return 1.0f;
	return zoom < 0.25f ? 0.25f : (zoom > 8.0f ? 8.0f : zoom);
}
int mapeditViewCell(int tileSize, float zoom)
{
	int cell = (int)floorf((float)tileSize * mapeditViewZoom(zoom) + 0.5f);
	return cell > 0 ? cell : 1;
}
float mapeditViewAnchor(float scroll, float anchor, int oldCell, int newCell)
{
	float result = (scroll + anchor) * (float)newCell / (float)oldCell - anchor;
	return result > 0.0f ? result : 0.0f;
}
int mapeditViewHit(int position, float scroll, int cell, int count)
{
	int pixel = position + (int)floorf(scroll);
	int index;
	if (pixel < 0 || cell <= 0) return -1;
	index = pixel / cell;
	return index < count ? index : -1;
}
