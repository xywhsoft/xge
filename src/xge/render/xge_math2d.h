#ifndef XGE_MATH2D_INTERNAL_H
#define XGE_MATH2D_INTERNAL_H

/* Allocation-free helpers shared by vector drawing and particle simulation. */
static xge_vec2_t __xgeMath2DPoint(xge_shape_ex_matrix_t m, xge_vec2_t p)
{
	return (xge_vec2_t){m.fA * p.fX + m.fC * p.fY + m.fE, m.fB * p.fX + m.fD * p.fY + m.fF};
}

static uint32_t __xgeMathColorLerp(uint32_t a, uint32_t b, float t)
{
	int r, g, blue, alpha;
	if (t < 0)
		t = 0;
	if (t > 1)
		t = 1;
	r = (int)(XGE_COLOR_GET_R(a) + ((float)XGE_COLOR_GET_R(b) - XGE_COLOR_GET_R(a)) * t + .5f);
	g = (int)(XGE_COLOR_GET_G(a) + ((float)XGE_COLOR_GET_G(b) - XGE_COLOR_GET_G(a)) * t + .5f);
	blue = (int)(XGE_COLOR_GET_B(a) + ((float)XGE_COLOR_GET_B(b) - XGE_COLOR_GET_B(a)) * t + .5f);
	alpha = (int)(XGE_COLOR_GET_A(a) + ((float)XGE_COLOR_GET_A(b) - XGE_COLOR_GET_A(a)) * t + .5f);
	return XGE_COLOR_RGBA(r, g, blue, alpha);
}
#endif
