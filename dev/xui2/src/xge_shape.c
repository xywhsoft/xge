#define XGE_SHAPE_AUTO_VERTEX_FLOATS 6
#define XGE_SHAPE_CIRCLE_SEGMENTS 48

static float g_arrShapeCircleCos[XGE_SHAPE_CIRCLE_SEGMENTS];
static float g_arrShapeCircleSin[XGE_SHAPE_CIRCLE_SEGMENTS];
static int g_bShapeCircleTableReady;

static void __xgeShapeCircleTableInit(void)
{
	float fStep;
	int i;

	if ( g_bShapeCircleTableReady != 0 ) {
		return;
	}
	fStep = 6.28318530718f / (float)XGE_SHAPE_CIRCLE_SEGMENTS;
	for ( i = 0; i < XGE_SHAPE_CIRCLE_SEGMENTS; i++ ) {
		g_arrShapeCircleCos[i] = cosf(fStep * (float)i);
		g_arrShapeCircleSin[i] = sinf(fStep * (float)i);
	}
	g_bShapeCircleTableReady = 1;
}

static void __xgeShapeAutoBatchReset(void)
{
	g_xge.iShapeAutoVertexCount = 0;
	g_xge.iShapeAutoIndexCount = 0;
	if ( (g_xge.bInitialized == 0) && (g_xge.pShapeAutoVertices != NULL) ) {
		xrtFree(g_xge.pShapeAutoVertices);
		g_xge.pShapeAutoVertices = NULL;
		g_xge.iShapeAutoVertexCapacity = 0;
	}
	if ( (g_xge.bInitialized == 0) && (g_xge.pShapeAutoIndices != NULL) ) {
		xrtFree(g_xge.pShapeAutoIndices);
		g_xge.pShapeAutoIndices = NULL;
		g_xge.iShapeAutoIndexCapacity = 0;
	}
}

static int __xgeShapeAutoBatchReserveVertices(int iNeeded)
{
	void* pVertices;
	int iCapacity;

	if ( iNeeded <= g_xge.iShapeAutoVertexCapacity ) {
		return XGE_OK;
	}
	iCapacity = (g_xge.iShapeAutoVertexCapacity > 0) ? g_xge.iShapeAutoVertexCapacity : 4096;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	if ( iCapacity > (INT32_MAX / ((int)sizeof(float) * XGE_SHAPE_AUTO_VERTEX_FLOATS)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pVertices = xrtRealloc(g_xge.pShapeAutoVertices, (size_t)iCapacity * XGE_SHAPE_AUTO_VERTEX_FLOATS * sizeof(float));
	if ( pVertices == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	g_xge.pShapeAutoVertices = pVertices;
	g_xge.iShapeAutoVertexCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeAutoBatchReserveIndices(int iNeeded)
{
	void* pIndices;
	int iCapacity;

	if ( iNeeded <= g_xge.iShapeAutoIndexCapacity ) {
		return XGE_OK;
	}
	iCapacity = (g_xge.iShapeAutoIndexCapacity > 0) ? g_xge.iShapeAutoIndexCapacity : 4096;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	if ( iCapacity > (INT32_MAX / (int)sizeof(uint32_t)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pIndices = xrtRealloc(g_xge.pShapeAutoIndices, (size_t)iCapacity * sizeof(uint32_t));
	if ( pIndices == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	g_xge.pShapeAutoIndices = pIndices;
	g_xge.iShapeAutoIndexCapacity = iCapacity;
	return XGE_OK;
}

static void __xgeShapeAutoBatchSetVertex(float* pVertices, int iIndex, float fX, float fY, uint32_t iColor, int bScreenSpace)
{
	float fSX;
	float fSY;

	__xgeCameraProjectVertex(fX, fY, 0.0f, bScreenSpace ? XGE_DRAW_SCREEN_SPACE : 0, &fSX, &fSY);
	pVertices[(iIndex * XGE_SHAPE_AUTO_VERTEX_FLOATS) + 0] = fSX;
	pVertices[(iIndex * XGE_SHAPE_AUTO_VERTEX_FLOATS) + 1] = fSY;
	pVertices[(iIndex * XGE_SHAPE_AUTO_VERTEX_FLOATS) + 2] = (float)XGE_COLOR_GET_R(iColor) / 255.0f;
	pVertices[(iIndex * XGE_SHAPE_AUTO_VERTEX_FLOATS) + 3] = (float)XGE_COLOR_GET_G(iColor) / 255.0f;
	pVertices[(iIndex * XGE_SHAPE_AUTO_VERTEX_FLOATS) + 4] = (float)XGE_COLOR_GET_B(iColor) / 255.0f;
	pVertices[(iIndex * XGE_SHAPE_AUTO_VERTEX_FLOATS) + 5] = (float)XGE_COLOR_GET_A(iColor) / 255.0f;
}

static int __xgeShapeAutoBatchFlush(void)
{
	if ( g_xge.iShapeAutoVertexCount <= 0 ) {
		return XGE_OK;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		g_xge.iShapeAutoVertexCount = 0;
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( __xgeShapeRendererInit() != XGE_OK ) {
		g_xge.iShapeAutoVertexCount = 0;
		g_xge.iShapeAutoIndexCount = 0;
		return XGE_ERROR_GPU_FAILED;
	}
	glUseProgram(g_xgeShapeRenderer.iProgram);
	glUniform2f(g_xgeShapeRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform4f(g_xgeShapeRenderer.iLocColor, 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1i(g_xgeShapeRenderer.iLocUseVertexColor, 1);
	glBindVertexArray(g_xgeShapeRenderer.iColorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeShapeRenderer.iColorVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_xgeShapeRenderer.iColorEBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, XGE_SHAPE_AUTO_VERTEX_FLOATS * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, XGE_SHAPE_AUTO_VERTEX_FLOATS * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, (size_t)g_xge.iShapeAutoVertexCount * XGE_SHAPE_AUTO_VERTEX_FLOATS * sizeof(float), g_xge.pShapeAutoVertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size_t)g_xge.iShapeAutoIndexCount * sizeof(uint32_t), g_xge.pShapeAutoIndices, GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, g_xge.iShapeAutoIndexCount, GL_UNSIGNED_INT, (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
	__xgeFrameStatsAddBatch();
	g_xge.iShapeAutoVertexCount = 0;
	g_xge.iShapeAutoIndexCount = 0;
	return XGE_OK;
}

static int __xgeShapeAutoBatchCircleFill(float fX, float fY, float fRadius, uint32_t iColor, int bScreenSpace)
{
	float* pVertices;
	uint32_t* pIndices;
	int iVertexBase;
	int iIndexBase;
	int i;
	int iRet;

	if ( (fRadius <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return XGE_OK;
	}
	iRet = __xgeShapeAutoBatchReserveVertices(g_xge.iShapeAutoVertexCount + XGE_SHAPE_CIRCLE_SEGMENTS + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeAutoBatchReserveIndices(g_xge.iShapeAutoIndexCount + (XGE_SHAPE_CIRCLE_SEGMENTS * 3));
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xgeShapeCircleTableInit();
	pVertices = (float*)g_xge.pShapeAutoVertices;
	pIndices = (uint32_t*)g_xge.pShapeAutoIndices;
	iVertexBase = g_xge.iShapeAutoVertexCount;
	iIndexBase = g_xge.iShapeAutoIndexCount;
	__xgeShapeAutoBatchSetVertex(pVertices, iVertexBase, fX, fY, iColor, bScreenSpace);
	for ( i = 0; i < XGE_SHAPE_CIRCLE_SEGMENTS; i++ ) {
		__xgeShapeAutoBatchSetVertex(pVertices, iVertexBase + 1 + i, fX + g_arrShapeCircleCos[i] * fRadius, fY + g_arrShapeCircleSin[i] * fRadius, iColor, bScreenSpace);
		pIndices[iIndexBase + (i * 3) + 0] = (uint32_t)iVertexBase;
		pIndices[iIndexBase + (i * 3) + 1] = (uint32_t)(iVertexBase + 1 + i);
		pIndices[iIndexBase + (i * 3) + 2] = (uint32_t)(iVertexBase + 1 + ((i + 1) % XGE_SHAPE_CIRCLE_SEGMENTS));
	}
	g_xge.iShapeAutoVertexCount += XGE_SHAPE_CIRCLE_SEGMENTS + 1;
	g_xge.iShapeAutoIndexCount += XGE_SHAPE_CIRCLE_SEGMENTS * 3;
	return XGE_OK;
}

static void __xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor, int bScreenSpace)
{
	float fR;
	float fG;
	float fB;
	float fA;
	float arrVertices[8];
	xge_vec2_t tPoint;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}
	if ( __xgeShapeRendererInit() != XGE_OK ) {
		return;
	}
	(void)__xgeShapeAutoBatchFlush();

	if ( bScreenSpace ) {
		arrVertices[0] = tRect.fX;
		arrVertices[1] = tRect.fY;
		arrVertices[2] = tRect.fX + tRect.fW;
		arrVertices[3] = tRect.fY;
		arrVertices[4] = tRect.fX;
		arrVertices[5] = tRect.fY + tRect.fH;
		arrVertices[6] = tRect.fX + tRect.fW;
		arrVertices[7] = tRect.fY + tRect.fH;
	} else {
		tPoint.fX = tRect.fX;
		tPoint.fY = tRect.fY;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[0] = tPoint.fX;
		arrVertices[1] = tPoint.fY;
		tPoint.fX = tRect.fX + tRect.fW;
		tPoint.fY = tRect.fY;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[2] = tPoint.fX;
		arrVertices[3] = tPoint.fY;
		tPoint.fX = tRect.fX;
		tPoint.fY = tRect.fY + tRect.fH;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[4] = tPoint.fX;
		arrVertices[5] = tPoint.fY;
		tPoint.fX = tRect.fX + tRect.fW;
		tPoint.fY = tRect.fY + tRect.fH;
		tPoint = xgeWorldToScreen(tPoint);
		arrVertices[6] = tPoint.fX;
		arrVertices[7] = tPoint.fY;
	}

	__xgeColorToFloat(iColor, &fR, &fG, &fB, &fA);
	glUseProgram(g_xgeShapeRenderer.iProgram);
	glUniform2f(g_xgeShapeRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform4f(g_xgeShapeRenderer.iLocColor, fR, fG, fB, fA);
	glUniform1i(g_xgeShapeRenderer.iLocUseVertexColor, 0);
	glBindVertexArray(g_xgeShapeRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeShapeRenderer.iVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(arrVertices), arrVertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
}

static void __xgeShapeQuadFill(const xge_vec2_t* pPoints, uint32_t iColor, int bScreenSpace)
{
	float fR;
	float fG;
	float fB;
	float fA;
	float arrVertices[8];
	xge_vec2_t tPoint;
	int i;

	if ( (pPoints == NULL) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}
	if ( __xgeShapeRendererInit() != XGE_OK ) {
		return;
	}
	(void)__xgeShapeAutoBatchFlush();
	for ( i = 0; i < 4; i++ ) {
		tPoint = pPoints[i];
		if ( bScreenSpace == 0 ) {
			tPoint = xgeWorldToScreen(tPoint);
		}
		arrVertices[(i * 2) + 0] = tPoint.fX;
		arrVertices[(i * 2) + 1] = tPoint.fY;
	}
	__xgeColorToFloat(iColor, &fR, &fG, &fB, &fA);
	glUseProgram(g_xgeShapeRenderer.iProgram);
	glUniform2f(g_xgeShapeRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform4f(g_xgeShapeRenderer.iLocColor, fR, fG, fB, fA);
	glUniform1i(g_xgeShapeRenderer.iLocUseVertexColor, 0);
	glBindVertexArray(g_xgeShapeRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeShapeRenderer.iVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(arrVertices), arrVertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
}

static void __xgeShapePointsFill(const xge_vec2_t* pPoints, int iCount, uint32_t iColor, int iMode, int bScreenSpace)
{
	float fR;
	float fG;
	float fB;
	float fA;
	float* pVertices;
	xge_vec2_t tPoint;
	int i;

	if ( (pPoints == NULL) || (iCount < 3) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}
	if ( __xgeShapeRendererInit() != XGE_OK ) {
		return;
	}
	(void)__xgeShapeAutoBatchFlush();
	pVertices = (float*)xrtMalloc(sizeof(float) * 2 * (size_t)iCount);
	if ( pVertices == NULL ) {
		return;
	}
	for ( i = 0; i < iCount; i++ ) {
		tPoint = pPoints[i];
		if ( bScreenSpace == 0 ) {
			tPoint = xgeWorldToScreen(tPoint);
		}
		pVertices[(i * 2) + 0] = tPoint.fX;
		pVertices[(i * 2) + 1] = tPoint.fY;
	}
	__xgeColorToFloat(iColor, &fR, &fG, &fB, &fA);
	glUseProgram(g_xgeShapeRenderer.iProgram);
	glUniform2f(g_xgeShapeRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform4f(g_xgeShapeRenderer.iLocColor, fR, fG, fB, fA);
	glUniform1i(g_xgeShapeRenderer.iLocUseVertexColor, 0);
	glBindVertexArray(g_xgeShapeRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeShapeRenderer.iVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * (size_t)iCount, pVertices, GL_DYNAMIC_DRAW);
	glDrawArrays((GLenum)iMode, 0, iCount);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	xrtFree(pVertices);
	__xgeFrameStatsAddDrawCall();
}

static void __xgeShapePoint(float fX, float fY, float fSize, uint32_t iColor, int bScreenSpace)
{
	xge_rect_t tRect;
	float fHalf;

	if ( fSize <= 0.0f ) {
		return;
	}
	fHalf = fSize * 0.5f;
	tRect.fX = fX - fHalf;
	tRect.fY = fY - fHalf;
	tRect.fW = fSize;
	tRect.fH = fSize;
	__xgeShapeRectFill(tRect, iColor, bScreenSpace);
}

void xgeShapePoint(float fX, float fY, float fSize, uint32_t iColor)
{
	__xgeShapePoint(fX, fY, fSize, iColor, 0);
}

void xgeShapePointPx(float fX, float fY, float fSize, uint32_t iColor)
{
	__xgeShapePoint(fX, fY, fSize, iColor, 1);
}

static void __xgeShapeLine(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor, int bScreenSpace)
{
	xge_vec2_t arrPoints[4];
	float fDX;
	float fDY;
	float fLen;
	float fNX;
	float fNY;
	float fHalf;

	if ( (fWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	fDX = fX1 - fX0;
	fDY = fY1 - fY0;
	fLen = sqrtf((fDX * fDX) + (fDY * fDY));
	if ( fLen <= 0.0f ) {
		return;
	}
	fHalf = fWidth * 0.5f;
	fNX = (-fDY / fLen) * fHalf;
	fNY = (fDX / fLen) * fHalf;
	arrPoints[0].fX = fX0 + fNX;
	arrPoints[0].fY = fY0 + fNY;
	arrPoints[1].fX = fX1 + fNX;
	arrPoints[1].fY = fY1 + fNY;
	arrPoints[2].fX = fX0 - fNX;
	arrPoints[2].fY = fY0 - fNY;
	arrPoints[3].fX = fX1 - fNX;
	arrPoints[3].fY = fY1 - fNY;
	__xgeShapeQuadFill(arrPoints, iColor, bScreenSpace);
}

void xgeShapeLine(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	__xgeShapeLine(fX0, fY0, fX1, fY1, fWidth, iColor, 0);
}

void xgeShapeLinePx(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	__xgeShapeLine(fX0, fY0, fX1, fY1, fWidth, iColor, 1);
}

void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor)
{
	__xgeShapeRectFill(tRect, iColor, 0);
}

void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor)
{
	__xgeShapeRectFill(tRect, iColor, 1);
}

static void __xgeShapeRectStroke(xge_rect_t tRect, float fWidth, uint32_t iColor, int bScreenSpace)
{
	xge_rect_t tEdge;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (fWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( fWidth > tRect.fW ) {
		fWidth = tRect.fW;
	}
	if ( fWidth > tRect.fH ) {
		fWidth = tRect.fH;
	}
	tEdge = tRect;
	tEdge.fH = fWidth;
	__xgeShapeRectFill(tEdge, iColor, bScreenSpace);
	tEdge.fY = tRect.fY + tRect.fH - fWidth;
	__xgeShapeRectFill(tEdge, iColor, bScreenSpace);
	tEdge.fX = tRect.fX;
	tEdge.fY = tRect.fY + fWidth;
	tEdge.fW = fWidth;
	tEdge.fH = tRect.fH - (fWidth * 2.0f);
	__xgeShapeRectFill(tEdge, iColor, bScreenSpace);
	tEdge.fX = tRect.fX + tRect.fW - fWidth;
	__xgeShapeRectFill(tEdge, iColor, bScreenSpace);
}

void xgeShapeRectStroke(xge_rect_t tRect, float fWidth, uint32_t iColor)
{
	__xgeShapeRectStroke(tRect, fWidth, iColor, 0);
}

void xgeShapeRectStrokePx(xge_rect_t tRect, float fWidth, uint32_t iColor)
{
	__xgeShapeRectStroke(tRect, fWidth, iColor, 1);
}

static void __xgeShapeCircleFill(float fX, float fY, float fRadius, uint32_t iColor, int bScreenSpace)
{
	xge_vec2_t arrPoints[XGE_SHAPE_CIRCLE_SEGMENTS + 2];
	float fStep;
	float fAngle;
	int i;

	if ( (fRadius <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( __xgeShapeAutoBatchCircleFill(fX, fY, fRadius, iColor, bScreenSpace) == XGE_OK ) {
		return;
	}
	(void)__xgeShapeAutoBatchFlush();
	arrPoints[0].fX = fX;
	arrPoints[0].fY = fY;
	fStep = 6.28318530718f / (float)XGE_SHAPE_CIRCLE_SEGMENTS;
	for ( i = 0; i <= XGE_SHAPE_CIRCLE_SEGMENTS; i++ ) {
		fAngle = fStep * (float)i;
		arrPoints[i + 1].fX = fX + cosf(fAngle) * fRadius;
		arrPoints[i + 1].fY = fY + sinf(fAngle) * fRadius;
	}
	__xgeShapePointsFill(arrPoints, XGE_SHAPE_CIRCLE_SEGMENTS + 2, iColor, GL_TRIANGLE_FAN, bScreenSpace);
}

void xgeShapeCircleFill(float fX, float fY, float fRadius, uint32_t iColor)
{
	__xgeShapeCircleFill(fX, fY, fRadius, iColor, 0);
}

void xgeShapeCircleFillPx(float fX, float fY, float fRadius, uint32_t iColor)
{
	__xgeShapeCircleFill(fX, fY, fRadius, iColor, 1);
}

static void __xgeShapeArc(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor, int bScreenSpace)
{
	float fSweep;
	float fStep;
	float fAngleA;
	float fAngleB;
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	int iSegments;
	int i;

	if ( (fRadius <= 0.0f) || (fWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	fSweep = fEndRadians - fStartRadians;
	if ( fSweep == 0.0f ) {
		return;
	}
	if ( fSweep < 0.0f ) {
		fSweep = -fSweep;
	}
	iSegments = (int)(fSweep / (6.28318530718f / (float)XGE_SHAPE_CIRCLE_SEGMENTS)) + 1;
	if ( iSegments < 1 ) {
		iSegments = 1;
	}
	if ( iSegments > XGE_SHAPE_CIRCLE_SEGMENTS ) {
		iSegments = XGE_SHAPE_CIRCLE_SEGMENTS;
	}
	fStep = (fEndRadians - fStartRadians) / (float)iSegments;
	for ( i = 0; i < iSegments; i++ ) {
		fAngleA = fStartRadians + fStep * (float)i;
		fAngleB = fStartRadians + fStep * (float)(i + 1);
		fX0 = fX + cosf(fAngleA) * fRadius;
		fY0 = fY + sinf(fAngleA) * fRadius;
		fX1 = fX + cosf(fAngleB) * fRadius;
		fY1 = fY + sinf(fAngleB) * fRadius;
		__xgeShapeLine(fX0, fY0, fX1, fY1, fWidth, iColor, bScreenSpace);
	}
}

void xgeShapeArc(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor)
{
	__xgeShapeArc(fX, fY, fRadius, fStartRadians, fEndRadians, fWidth, iColor, 0);
}

void xgeShapeArcPx(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor)
{
	__xgeShapeArc(fX, fY, fRadius, fStartRadians, fEndRadians, fWidth, iColor, 1);
}

void xgeShapeCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	__xgeShapeArc(fX, fY, fRadius, 0.0f, 6.28318530718f, fWidth, iColor, 0);
}

void xgeShapeCircleStrokePx(float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	__xgeShapeArc(fX, fY, fRadius, 0.0f, 6.28318530718f, fWidth, iColor, 1);
}

static void __xgeShapeTriangleFill(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor, int bScreenSpace)
{
	xge_vec2_t arrPoints[3];

	arrPoints[0] = tA;
	arrPoints[1] = tB;
	arrPoints[2] = tC;
	__xgeShapePointsFill(arrPoints, 3, iColor, GL_TRIANGLES, bScreenSpace);
}

void xgeShapeTriangleFill(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor)
{
	__xgeShapeTriangleFill(tA, tB, tC, iColor, 0);
}

void xgeShapeTriangleFillPx(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor)
{
	__xgeShapeTriangleFill(tA, tB, tC, iColor, 1);
}

void xgeShapePolygonFill(const xge_vec2_t* pPoints, int iCount, uint32_t iColor)
{
	__xgeShapePointsFill(pPoints, iCount, iColor, GL_TRIANGLE_FAN, 0);
}

void xgeShapePolygonFillPx(const xge_vec2_t* pPoints, int iCount, uint32_t iColor)
{
	__xgeShapePointsFill(pPoints, iCount, iColor, GL_TRIANGLE_FAN, 1);
}

static int __xgeShapeMeshFill(const xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, int bScreenSpace)
{
	float* pBatchVertices;
	uint32_t* pBatchIndices;
	int iVertexBase;
	int iIndexBase;
	int i;
	int iRet;

	if ( (pVertices == NULL) || (iVertexCount <= 0) || (pIndices == NULL) ||
	     (iIndexCount <= 0) || ((iIndexCount % 3) != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iVertexCount > (INT32_MAX - g_xge.iShapeAutoVertexCount)) ||
	     (iIndexCount > (INT32_MAX - g_xge.iShapeAutoIndexCount)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iIndexCount; i++ ) {
		if ( pIndices[i] >= (uint32_t)iVertexCount ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	iRet = __xgeShapeAutoBatchReserveVertices(g_xge.iShapeAutoVertexCount + iVertexCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeAutoBatchReserveIndices(g_xge.iShapeAutoIndexCount + iIndexCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pBatchVertices = (float*)g_xge.pShapeAutoVertices;
	pBatchIndices = (uint32_t*)g_xge.pShapeAutoIndices;
	iVertexBase = g_xge.iShapeAutoVertexCount;
	iIndexBase = g_xge.iShapeAutoIndexCount;
	for ( i = 0; i < iVertexCount; i++ ) {
		__xgeShapeAutoBatchSetVertex(pBatchVertices, iVertexBase + i, pVertices[i].fX, pVertices[i].fY, pVertices[i].iColor, bScreenSpace);
	}
	for ( i = 0; i < iIndexCount; i++ ) {
		pBatchIndices[iIndexBase + i] = (uint32_t)iVertexBase + pIndices[i];
	}
	g_xge.iShapeAutoVertexCount += iVertexCount;
	g_xge.iShapeAutoIndexCount += iIndexCount;
	return XGE_OK;
}

int xgeShapeMeshFill(const xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount)
{
	return __xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount, 0);
}

int xgeShapeMeshFillPx(const xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount)
{
	return __xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount, 1);
}

int xgeShapeBatchInit(xge_shape_batch pBatch, uint32_t iColor, int iTriangleCapacity, uint32_t iFlags)
{
	size_t iVertexSize;

	if ( (pBatch == NULL) || (iTriangleCapacity <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iTriangleCapacity > (INT32_MAX / (6 * (int)sizeof(float))) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pBatch, 0, sizeof(*pBatch));
	iVertexSize = (size_t)iTriangleCapacity * 3u * 2u * sizeof(float);
	pBatch->pVertices = xrtMalloc(iVertexSize);
	if ( pBatch->pVertices == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBatch->iColor = iColor;
	pBatch->iFlags = iFlags;
	pBatch->iTriangleCapacity = iTriangleCapacity;
	return XGE_OK;
}

void xgeShapeBatchFree(xge_shape_batch pBatch)
{
	if ( pBatch == NULL ) {
		return;
	}
	if ( pBatch->pVertices != NULL ) {
		xrtFree(pBatch->pVertices);
	}
	memset(pBatch, 0, sizeof(*pBatch));
}

void xgeShapeBatchClear(xge_shape_batch pBatch)
{
	if ( pBatch != NULL ) {
		pBatch->iTriangleCount = 0;
	}
}

static void __xgeShapeBatchSetVertex(float* pVertices, int iIndex, xge_vec2_t tPoint, uint32_t iFlags)
{
	float fX;
	float fY;

	__xgeCameraProjectVertex(tPoint.fX, tPoint.fY, 0.0f, iFlags, &fX, &fY);
	pVertices[(iIndex * 2) + 0] = fX;
	pVertices[(iIndex * 2) + 1] = fY;
}

int xgeShapeBatchTriangleFill(xge_shape_batch pBatch, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	float* pVertices;
	int iBase;

	if ( (pBatch == NULL) || (pBatch->pVertices == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pBatch->iTriangleCount >= pBatch->iTriangleCapacity ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pVertices = (float*)pBatch->pVertices;
	iBase = pBatch->iTriangleCount * 3;
	__xgeShapeBatchSetVertex(pVertices, iBase + 0, tA, pBatch->iFlags);
	__xgeShapeBatchSetVertex(pVertices, iBase + 1, tB, pBatch->iFlags);
	__xgeShapeBatchSetVertex(pVertices, iBase + 2, tC, pBatch->iFlags);
	pBatch->iTriangleCount++;
	return XGE_OK;
}

int xgeShapeBatchRectFill(xge_shape_batch pBatch, xge_rect_t tRect)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	xge_vec2_t tD;
	int iRet;

	if ( (pBatch == NULL) || (pBatch->pVertices == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pBatch->iTriangleCapacity - pBatch->iTriangleCount) < 2 ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	tA.fX = tRect.fX;
	tA.fY = tRect.fY;
	tB.fX = tRect.fX + tRect.fW;
	tB.fY = tRect.fY;
	tC.fX = tRect.fX;
	tC.fY = tRect.fY + tRect.fH;
	tD.fX = tRect.fX + tRect.fW;
	tD.fY = tRect.fY + tRect.fH;
	iRet = xgeShapeBatchTriangleFill(pBatch, tA, tB, tC);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return xgeShapeBatchTriangleFill(pBatch, tC, tB, tD);
}

int xgeShapeBatchFlush(xge_shape_batch pBatch)
{
	float fR;
	float fG;
	float fB;
	float fA;
	int iVertexCount;

	if ( (pBatch == NULL) || (pBatch->pVertices == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pBatch->iTriangleCount <= 0 ) {
		return XGE_OK;
	}
	if ( XGE_COLOR_GET_A(pBatch->iColor) == 0 ) {
		xgeShapeBatchClear(pBatch);
		return XGE_OK;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( __xgeShapeRendererInit() != XGE_OK ) {
		return XGE_ERROR_GPU_FAILED;
	}
	iVertexCount = pBatch->iTriangleCount * 3;
	__xgeColorToFloat(pBatch->iColor, &fR, &fG, &fB, &fA);
	glUseProgram(g_xgeShapeRenderer.iProgram);
	glUniform2f(g_xgeShapeRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform4f(g_xgeShapeRenderer.iLocColor, fR, fG, fB, fA);
	glUniform1i(g_xgeShapeRenderer.iLocUseVertexColor, 0);
	glBindVertexArray(g_xgeShapeRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeShapeRenderer.iVBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, (size_t)iVertexCount * 2u * sizeof(float), pBatch->pVertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, iVertexCount);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
	__xgeFrameStatsAddBatch();
	xgeShapeBatchClear(pBatch);
	return XGE_OK;
}
