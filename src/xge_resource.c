int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider)
{
	if ( (pProvider == NULL) || (pProvider->sScheme == NULL) || (pProvider->load == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.iResourceProviderCount >= XGE_RESOURCE_PROVIDER_MAX ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	g_xge.arrResourceProviders[g_xge.iResourceProviderCount] = *pProvider;
	g_xge.iResourceProviderCount++;
	return XGE_OK;
}

void xgeResourceProviderClear(void)
{
	memset(g_xge.arrResourceProviders, 0, sizeof(g_xge.arrResourceProviders));
	g_xge.iResourceProviderCount = 0;
}

int xgeResourceLoad(const char* sURI, xge_resource_t* pResource)
{
	int iSchemeLen;
	int iProvider;
	int iRet;
	size_t iSize;
	void* pData;

	if ( (sURI == NULL) || (pResource == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pResource, 0, sizeof(*pResource));

	iSchemeLen = __xgeUriSchemeLen(sURI);
	if ( iSchemeLen > 0 ) {
		iProvider = __xgeResourceFindProvider(sURI, iSchemeLen);
		if ( iProvider >= 0 ) {
			iRet = g_xge.arrResourceProviders[iProvider].load(sURI, &pResource->pData, &pResource->iSize, g_xge.arrResourceProviders[iProvider].pUser);
			if ( iRet != XGE_OK ) {
				memset(pResource, 0, sizeof(*pResource));
				return iRet;
			}
			pResource->iProvider = iProvider + 1;
			pResource->free = g_xge.arrResourceProviders[iProvider].free;
			pResource->pUser = g_xge.arrResourceProviders[iProvider].pUser;
			return XGE_OK;
		}
		if ( __xgeSchemeEqual(sURI, iSchemeLen, "res") ) {
			sURI += iSchemeLen + 3;
		} else if ( __xgeSchemeEqual(sURI, iSchemeLen, "file") ) {
			sURI += iSchemeLen + 3;
		} else {
			return XGE_ERROR_UNSUPPORTED;
		}
	}

	pData = __xgeFileGetAll(sURI, &iSize);
	if ( pData == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( iSize > (size_t)INT32_MAX ) {
		xrtFree(pData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pResource->pData = pData;
	pResource->iSize = (int)iSize;
	pResource->iProvider = 0;
	return XGE_OK;
}

int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource)
{
	void* pCopy;

	if ( (pData == NULL) || (iSize < 0) || (pResource == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pResource, 0, sizeof(*pResource));
	if ( iSize == 0 ) {
		return XGE_OK;
	}
	pCopy = xrtMalloc((size_t)iSize);
	if ( pCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pCopy, pData, (size_t)iSize);
	pResource->pData = pCopy;
	pResource->iSize = iSize;
	pResource->iProvider = 0;
	return XGE_OK;
}

void xgeResourceFree(xge_resource_t* pResource)
{
	if ( (pResource == NULL) || (pResource->pData == NULL) ) {
		return;
	}
	if ( pResource->free != NULL ) {
		pResource->free(pResource->pData, pResource->pUser);
	} else {
		xrtFree(pResource->pData);
	}
	memset(pResource, 0, sizeof(*pResource));
}
