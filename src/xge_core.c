int xgeInit(const xge_desc_t* pDesc)
{
	xge_desc_t objDesc;

	if ( g_xge.bInitialized ) {
		return XGE_ERROR_ALREADY_INITIALIZED;
	}

	memset(&g_xge, 0, sizeof(g_xge));
	memset(&objDesc, 0, sizeof(objDesc));

	if ( pDesc != NULL ) {
		objDesc = *pDesc;
	}
	if ( objDesc.iWidth <= 0 ) {
		objDesc.iWidth = 800;
	}
	if ( objDesc.iHeight <= 0 ) {
		objDesc.iHeight = 600;
	}
	if ( objDesc.sTitle == NULL ) {
		objDesc.sTitle = "XGE";
	}

	if ( xrtInit() == NULL ) {
		return XGE_ERROR_BACKEND_FAILED;
	}

	g_xge.objDesc = objDesc;
	g_xge.iWidth = objDesc.iWidth;
	g_xge.iHeight = objDesc.iHeight;
	g_xge.iClearColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	g_xge.iBlend = XGE_BLEND_ALPHA;
	g_xge.fDelta = 1.0f / 60.0f;
	g_xge.tCamera = xgeCameraDefault((float)objDesc.iWidth, (float)objDesc.iHeight);
#ifndef XGE_NO_AUDIO
	g_xge.tAudioListener.tForward.fZ = -1.0f;
	g_xge.tAudioListener.tUp.fY = 1.0f;
#endif
	g_xge.bInitialized = 1;
	g_xge.bRunning = 1;
	return XGE_OK;
}

void xgeUnit(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return;
	}
	g_xge.bInitialized = 0;
	g_xge.bRunning = 0;
	xgeAudioUnit();
	xrtUnit();
}

int xgeRun(xge_scene_proc procFrame, void* pUser)
{
	sapp_desc objDesc;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}

	g_xge.procFrame = procFrame;
	g_xge.pFrameUser = pUser;
	objDesc = __xgeMakeSokolDesc();
	sapp_run(&objDesc);
	return XGE_OK;
}

void xgeQuit(void)
{
	g_xge.bRunning = 0;
	if ( g_xge.bSokolRunning ) {
		sapp_quit();
	}
}

int xgeFrame(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	return XGE_ERROR_UNSUPPORTED;
}

int xgeGetWidth(void)
{
	return g_xge.iWidth;
}

int xgeGetHeight(void)
{
	return g_xge.iHeight;
}

float xgeGetDelta(void)
{
	return g_xge.fDelta;
}

int xgeGetFPS(void)
{
	return g_xge.iFPS;
}

void xgeSetTitle(const char* sTitle)
{
	if ( sTitle != NULL ) {
		sapp_set_window_title(sTitle);
	}
}

void xgeClear(uint32_t iColor)
{
	float fR;
	float fG;
	float fB;
	float fA;

	g_xge.iClearColor = iColor;
	if ( g_xge.bSokolRunning ) {
		__xgeColorToFloat(iColor, &fR, &fG, &fB, &fA);
		glClearColor(fR, fG, fB, fA);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void xgePresent(void)
{
}

uint32_t xgeColorRGBA(int iR, int iG, int iB, int iA)
{
	return XGE_COLOR_RGBA(iR, iG, iB, iA);
}

xge_color_t xgeColorUnpack(uint32_t iColor)
{
	xge_color_t tColor;

	tColor.fR = (float)XGE_COLOR_GET_R(iColor) / 255.0f;
	tColor.fG = (float)XGE_COLOR_GET_G(iColor) / 255.0f;
	tColor.fB = (float)XGE_COLOR_GET_B(iColor) / 255.0f;
	tColor.fA = (float)XGE_COLOR_GET_A(iColor) / 255.0f;
	return tColor;
}

void xgeBlendSet(int iBlend)
{
	g_xge.iBlend = iBlend;
	__xgeBlendApply(iBlend);
}

int xgeBlendGet(void)
{
	return g_xge.iBlend;
}

xge_camera_t xgeCameraDefault(float fWidth, float fHeight)
{
	xge_camera_t tCamera;

	memset(&tCamera, 0, sizeof(tCamera));
	tCamera.tScale.fX = 1.0f;
	tCamera.tScale.fY = 1.0f;
	tCamera.tViewport.fX = 0.0f;
	tCamera.tViewport.fY = 0.0f;
	tCamera.tViewport.fW = fWidth;
	tCamera.tViewport.fH = fHeight;
	tCamera.iCoordinateMode = XGE_COORD_Y_DOWN;
	return tCamera;
}

void xgeCameraSet(const xge_camera_t* pCamera)
{
	if ( pCamera == NULL ) {
		return;
	}
	g_xge.tCamera = *pCamera;
	if ( g_xge.tCamera.tScale.fX == 0.0f ) {
		g_xge.tCamera.tScale.fX = 1.0f;
	}
	if ( g_xge.tCamera.tScale.fY == 0.0f ) {
		g_xge.tCamera.tScale.fY = 1.0f;
	}
}

xge_camera_t xgeCameraGet(void)
{
	return g_xge.tCamera;
}

xge_vec2_t xgeWorldToScreen(xge_vec2_t tPoint)
{
	xge_vec2_t tRet;
	xge_camera_t* pCamera;

	pCamera = &g_xge.tCamera;
	tRet.fX = (tPoint.fX - pCamera->tPosition.fX) * pCamera->tScale.fX;
	tRet.fY = (tPoint.fY - pCamera->tPosition.fY) * pCamera->tScale.fY;
	if ( pCamera->iCoordinateMode == XGE_COORD_CENTER ) {
		tRet.fX += pCamera->tViewport.fX + (pCamera->tViewport.fW * 0.5f);
		tRet.fY += pCamera->tViewport.fY + (pCamera->tViewport.fH * 0.5f);
	} else {
		tRet.fX += pCamera->tViewport.fX;
		tRet.fY += pCamera->tViewport.fY;
	}
	return tRet;
}

xge_vec2_t xgeScreenToWorld(xge_vec2_t tPoint)
{
	xge_vec2_t tRet;
	xge_camera_t* pCamera;
	float fX;
	float fY;

	pCamera = &g_xge.tCamera;
	fX = tPoint.fX - pCamera->tViewport.fX;
	fY = tPoint.fY - pCamera->tViewport.fY;
	if ( pCamera->iCoordinateMode == XGE_COORD_CENTER ) {
		fX -= pCamera->tViewport.fW * 0.5f;
		fY -= pCamera->tViewport.fH * 0.5f;
	}
	tRet.fX = (fX / pCamera->tScale.fX) + pCamera->tPosition.fX;
	tRet.fY = (fY / pCamera->tScale.fY) + pCamera->tPosition.fY;
	return tRet;
}

