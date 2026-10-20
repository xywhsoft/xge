void xgeDrawQuad3D(xge_texture pTexture, const xge_vertex_t* pVertices, uint32_t iFlags)
{
	float arrVertices[40];
	float fR;
	float fG;
	float fB;
	float fA;
	float fW;
	int i;
	float fX;
	float fY;

	if ( (pTexture == NULL) || (pVertices == NULL) || (pTexture->iBackendId == 0) ) {
		return;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}
	if ( __xgeQuad3DRendererInit() != XGE_OK ) {
		return;
	}

	for ( i = 0; i < 4; i++ ) {
		__xgeCameraProjectVertex(pVertices[i].fX, pVertices[i].fY, pVertices[i].fZ, iFlags, &fX, &fY);
		fW = pVertices[i].fW;
		if ( fW <= 0.0f ) {
			fW = 1.0f;
		}
		__xgeColorToFloat(pVertices[i].iColor, &fR, &fG, &fB, &fA);
		arrVertices[(i * 10) + 0] = fX;
		arrVertices[(i * 10) + 1] = fY;
		arrVertices[(i * 10) + 2] = pVertices[i].fZ;
		arrVertices[(i * 10) + 3] = fW;
		xge_vec2_t tUV = __xgeTextureDrawUV(pTexture, (xge_vec2_t){pVertices[i].fU, pVertices[i].fV}, iFlags);
		arrVertices[(i * 10) + 4] = tUV.fX;
		arrVertices[(i * 10) + 5] = tUV.fY;
		arrVertices[(i * 10) + 6] = fR;
		arrVertices[(i * 10) + 7] = fG;
		arrVertices[(i * 10) + 8] = fB;
		arrVertices[(i * 10) + 9] = fA;
	}

	xge_glUseProgram(g_xgeQuad3DRenderer.iProgram);
	xge_glUniform2f(g_xgeQuad3DRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	xge_glUniform1i(g_xgeQuad3DRenderer.iLocTexture, 0);
	xge_glActiveTexture(GL_TEXTURE0);
	xge_glBindTexture(GL_TEXTURE_2D, (GLuint)pTexture->iBackendId);
	xge_glBindVertexArray(g_xgeQuad3DRenderer.iVAO);
	xge_glBindBuffer(GL_ARRAY_BUFFER, g_xgeQuad3DRenderer.iVBO);
	xge_glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
	xge_glEnableVertexAttribArray(0);
	xge_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	xge_glEnableVertexAttribArray(1);
	xge_glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
	xge_glEnableVertexAttribArray(2);
	xge_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	xge_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	xge_glBindBuffer(GL_ARRAY_BUFFER, 0);
	xge_glBindVertexArray(0);
	xge_glBindTexture(GL_TEXTURE_2D, 0);
	xge_glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
}

static void __xgeMeshFreeCPU(xge_mesh pMesh)
{
	if ( pMesh->pVertices != NULL ) {
		xrtFree(pMesh->pVertices);
		pMesh->pVertices = NULL;
	}
	if ( pMesh->pIndices != NULL ) {
		xrtFree(pMesh->pIndices);
		pMesh->pIndices = NULL;
	}
}

static int __xgeMeshSetCPU(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount)
{
	size_t iVertexBytes;
	size_t iIndexBytes;

	if ( (pMesh == NULL) || (pVertices == NULL) || (iVertexCount <= 0) || (pIndices == NULL) || (iIndexCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iVertexCount > INT32_MAX / (int)sizeof(xge_vertex_t)) || (iIndexCount > INT32_MAX / (int)sizeof(uint16_t)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iVertexBytes = (size_t)iVertexCount * sizeof(xge_vertex_t);
	iIndexBytes = (size_t)iIndexCount * sizeof(uint16_t);
	__xgeMeshFreeCPU(pMesh);
	pMesh->pVertices = xrtMalloc(iVertexBytes);
	pMesh->pIndices = xrtMalloc(iIndexBytes);
	if ( (pMesh->pVertices == NULL) || (pMesh->pIndices == NULL) ) {
		__xgeMeshFreeCPU(pMesh);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pMesh->pVertices, pVertices, iVertexBytes);
	memcpy(pMesh->pIndices, pIndices, iIndexBytes);
	pMesh->iVertexCount = iVertexCount;
	pMesh->iIndexCount = iIndexCount;
	return XGE_OK;
}

static int __xgeMeshUpload(xge_mesh pMesh, xge_texture pTexture, uint32_t iDrawFlags)
{
	float* pPacked;
	xge_vertex_t* pVertices;
	float fR;
	float fG;
	float fB;
	float fA;
	int i;
	GLuint iBuffer;
	GLenum iUsage;
	float fX;
	float fY;

	if ( (pMesh == NULL) || (pMesh->pVertices == NULL) || (pMesh->pIndices == NULL) || (pMesh->iVertexCount <= 0) || (pMesh->iIndexCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	pPacked = (float*)xrtMalloc((size_t)pMesh->iVertexCount * 10u * sizeof(float));
	if ( pPacked == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pVertices = (xge_vertex_t*)pMesh->pVertices;
	for ( i = 0; i < pMesh->iVertexCount; i++ ) {
		__xgeCameraProjectVertex(pVertices[i].fX, pVertices[i].fY, pVertices[i].fZ, iDrawFlags, &fX, &fY);
		__xgeColorToFloat(pVertices[i].iColor, &fR, &fG, &fB, &fA);
		pPacked[(i * 10) + 0] = fX;
		pPacked[(i * 10) + 1] = fY;
		pPacked[(i * 10) + 2] = pVertices[i].fZ;
		pPacked[(i * 10) + 3] = pVertices[i].fW <= 0.0f ? 1.0f : pVertices[i].fW;
		xge_vec2_t tUV = __xgeTextureDrawUV(pTexture, (xge_vec2_t){pVertices[i].fU, pVertices[i].fV}, iDrawFlags);
		pPacked[(i * 10) + 4] = tUV.fX;
		pPacked[(i * 10) + 5] = tUV.fY;
		pPacked[(i * 10) + 6] = fR;
		pPacked[(i * 10) + 7] = fG;
		pPacked[(i * 10) + 8] = fB;
		pPacked[(i * 10) + 9] = fA;
	}
	iUsage = (pMesh->iFlags & XGE_MESH_DYNAMIC) != 0 ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	if ( pMesh->iVertexBufferId == 0 ) {
		xge_glGenBuffers(1, &iBuffer);
		pMesh->iVertexBufferId = iBuffer;
	}
	if ( pMesh->iIndexBufferId == 0 ) {
		xge_glGenBuffers(1, &iBuffer);
		pMesh->iIndexBufferId = iBuffer;
	}
	xge_glBindBuffer(GL_ARRAY_BUFFER, (GLuint)pMesh->iVertexBufferId);
	xge_glBufferData(GL_ARRAY_BUFFER, (size_t)pMesh->iVertexCount * 10u * sizeof(float), pPacked, iUsage);
	xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)pMesh->iIndexBufferId);
	xge_glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size_t)pMesh->iIndexCount * sizeof(uint16_t), pMesh->pIndices, iUsage);
	xge_glBindBuffer(GL_ARRAY_BUFFER, 0);
	xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	xrtFree(pPacked);
	return XGE_OK;
}

int xgeMeshCreate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount, uint32_t iFlags)
{
	int iRet;

	if ( pMesh == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pMesh, 0, sizeof(*pMesh));
	pMesh->iFlags = iFlags;
	iRet = __xgeMeshSetCPU(pMesh, pVertices, iVertexCount, pIndices, iIndexCount);
	if ( iRet != XGE_OK ) {
		memset(pMesh, 0, sizeof(*pMesh));
		return iRet;
	}
	return XGE_OK;
}

int xgeMeshUpdate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount)
{
	int iRet;
	uint32_t iFlags;
	uint32_t iVertexBufferId;
	uint32_t iIndexBufferId;

	if ( pMesh == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iFlags = pMesh->iFlags;
	iVertexBufferId = pMesh->iVertexBufferId;
	iIndexBufferId = pMesh->iIndexBufferId;
	iRet = __xgeMeshSetCPU(pMesh, pVertices, iVertexCount, pIndices, iIndexCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pMesh->iFlags = iFlags;
	pMesh->iVertexBufferId = iVertexBufferId;
	pMesh->iIndexBufferId = iIndexBufferId;
	return XGE_OK;
}

void xgeMeshFree(xge_mesh pMesh)
{
	GLuint iBuffer;

	if ( pMesh == NULL ) {
		return;
	}
	iBuffer = (GLuint)pMesh->iVertexBufferId;
	if ( (iBuffer != 0) && (g_xge.bSokolRunning != 0) && (xge_glDeleteBuffers != NULL) ) {
		xge_glDeleteBuffers(1, &iBuffer);
	}
	iBuffer = (GLuint)pMesh->iIndexBufferId;
	if ( (iBuffer != 0) && (g_xge.bSokolRunning != 0) && (xge_glDeleteBuffers != NULL) ) {
		xge_glDeleteBuffers(1, &iBuffer);
	}
	__xgeMeshFreeCPU(pMesh);
	memset(pMesh, 0, sizeof(*pMesh));
}

void xgeMeshDraw(xge_mesh pMesh, xge_texture pTexture, uint32_t iFlags)
{
	if ( (pMesh == NULL) || (pTexture == NULL) || (pTexture->iBackendId == 0) || (pMesh->iVertexCount <= 0) || (pMesh->iIndexCount <= 0) ) {
		return;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}
	if ( __xgeQuad3DRendererInit() != XGE_OK ) {
		return;
	}
	if ( __xgeMeshUpload(pMesh, pTexture, iFlags) != XGE_OK ) {
		return;
	}
	xge_glUseProgram(g_xgeQuad3DRenderer.iProgram);
	xge_glUniform2f(g_xgeQuad3DRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	xge_glUniform1i(g_xgeQuad3DRenderer.iLocTexture, 0);
	xge_glActiveTexture(GL_TEXTURE0);
	xge_glBindTexture(GL_TEXTURE_2D, (GLuint)pTexture->iBackendId);
	xge_glBindVertexArray(g_xgeQuad3DRenderer.iVAO);
	xge_glBindBuffer(GL_ARRAY_BUFFER, (GLuint)pMesh->iVertexBufferId);
	xge_glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
	xge_glEnableVertexAttribArray(0);
	xge_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	xge_glEnableVertexAttribArray(1);
	xge_glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
	xge_glEnableVertexAttribArray(2);
	xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)pMesh->iIndexBufferId);
	xge_glDrawElements(GL_TRIANGLES, pMesh->iIndexCount, GL_UNSIGNED_SHORT, (void*)0);
	xge_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	xge_glBindBuffer(GL_ARRAY_BUFFER, 0);
	xge_glBindVertexArray(0);
	xge_glBindTexture(GL_TEXTURE_2D, 0);
	xge_glUseProgram(0);
	__xgeFrameStatsAddDrawCall();
}
