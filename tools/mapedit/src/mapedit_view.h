#ifndef MAPEDIT_VIEW_H
#define MAPEDIT_VIEW_H
/* Raster grids use one rounded cell size for drawing, hit testing and scrolling.
 * Keeping scroll offsets in display pixels avoids fractional XUI rectangles. */
int mapeditViewCell(int tileSize, float zoom);
float mapeditViewZoom(float zoom);
float mapeditViewAnchor(float scroll, float anchor, int oldCell, int newCell);
int mapeditViewHit(int position, float scroll, int cell, int count);
#endif
