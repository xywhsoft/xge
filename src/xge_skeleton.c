static void __xgeSkeletonCopyName(char* sDst, const char* sSrc)
{
	if ( sDst == NULL ) {
		return;
	}
	if ( sSrc == NULL ) {
		sDst[0] = 0;
		return;
	}
	strncpy(sDst, sSrc, XGE_SKELETON_NAME_CAPACITY - 1u);
	sDst[XGE_SKELETON_NAME_CAPACITY - 1u] = 0;
}

#define XGE_XSKEL_BONE_CAPACITY 256
#define XGE_XSKEL_SLOT_CAPACITY 256
#define XGE_XSKEL_ATTACHMENT_CAPACITY 256
#define XGE_XSKEL_ANIMATION_CAPACITY 64
#define XGE_XSKEL_KEYFRAME_CAPACITY 2048

typedef struct xge_xskel_bone_t {
	int iId;
	int iParent;
	char sName[XGE_SKELETON_NAME_CAPACITY];
	float fLocalX;
	float fLocalY;
	float fRotation;
	float fLength;
	int bConnected;
	int bVisible;
} xge_xskel_bone_t;

typedef struct xge_xskel_slot_t {
	int iId;
	int iBone;
	char sName[XGE_SKELETON_NAME_CAPACITY];
	int iAttachment;
	int iDrawOrder;
	uint32_t iColor;
	int bVisible;
} xge_xskel_slot_t;

typedef struct xge_xskel_attachment_t {
	int iId;
	int iBone;
	char sName[XGE_SKELETON_NAME_CAPACITY];
	char sImage[XGE_SKELETON_NAME_CAPACITY];
	float fOffsetX;
	float fOffsetY;
	float fRotation;
	float fScaleX;
	float fScaleY;
	float fPivotX;
	float fPivotY;
	int iDrawOrder;
} xge_xskel_attachment_t;

typedef struct xge_xskel_animation_t {
	int iId;
	char sName[XGE_SKELETON_NAME_CAPACITY];
	int iFrameStart;
	int iFrameEnd;
	int bLoop;
} xge_xskel_animation_t;

typedef struct xge_xskel_keyframe_t {
	int iId;
	int iAnimation;
	int iBone;
	int iFrame;
	float fLocalX;
	float fLocalY;
	float fRotation;
	float fLength;
} xge_xskel_keyframe_t;

typedef struct xge_xskel_document_t {
	char sName[XGE_SKELETON_NAME_CAPACITY];
	int iFrameRate;
	xge_xskel_bone_t arrBones[XGE_XSKEL_BONE_CAPACITY];
	int iBoneCount;
	xge_xskel_slot_t arrSlots[XGE_XSKEL_SLOT_CAPACITY];
	int iSlotCount;
	xge_xskel_attachment_t arrAttachments[XGE_XSKEL_ATTACHMENT_CAPACITY];
	int iAttachmentCount;
	xge_xskel_animation_t arrAnimations[XGE_XSKEL_ANIMATION_CAPACITY];
	int iAnimationCount;
	xge_xskel_keyframe_t arrKeyframes[XGE_XSKEL_KEYFRAME_CAPACITY];
	int iKeyframeCount;
} xge_xskel_document_t;

static void __xgeSkeletonStripLineEnd(char* sText)
{
	size_t iLen;

	if ( sText == NULL ) {
		return;
	}
	iLen = strlen(sText);
	while ( iLen > 0u && (sText[iLen - 1u] == '\n' || sText[iLen - 1u] == '\r') ) {
		sText[iLen - 1u] = 0;
		iLen--;
	}
}

static int __xgeSkeletonPathIsAbsolute(const char* sPath)
{
	if ( (sPath == NULL) || (sPath[0] == 0) ) {
		return 0;
	}
	if ( sPath[0] == '/' || sPath[0] == '\\' ) {
		return 1;
	}
	return ((sPath[0] >= 'A' && sPath[0] <= 'Z') || (sPath[0] >= 'a' && sPath[0] <= 'z')) && sPath[1] == ':';
}

static void __xgeSkeletonPathJoin(char* sOut, size_t iOutSize, const char* sBaseFile, const char* sRel)
{
	const char* sSlashA;
	const char* sSlashB;
	const char* sSlash;
	size_t iPrefix;

	if ( (sOut == NULL) || (iOutSize == 0u) ) {
		return;
	}
	sOut[0] = 0;
	if ( (sRel == NULL) || (sRel[0] == 0) ) {
		return;
	}
	if ( __xgeSkeletonPathIsAbsolute(sRel) || sBaseFile == NULL ) {
		snprintf(sOut, iOutSize, "%s", sRel);
		return;
	}
	sSlashA = strrchr(sBaseFile, '/');
	sSlashB = strrchr(sBaseFile, '\\');
	sSlash = sSlashA > sSlashB ? sSlashA : sSlashB;
	if ( sSlash == NULL ) {
		snprintf(sOut, iOutSize, "%s", sRel);
		return;
	}
	iPrefix = (size_t)(sSlash - sBaseFile) + 1u;
	if ( iPrefix >= iOutSize ) {
		iPrefix = iOutSize - 1u;
	}
	memcpy(sOut, sBaseFile, iPrefix);
	sOut[iPrefix] = 0;
	strncat(sOut, sRel, iOutSize - strlen(sOut) - 1u);
}

static int __xgeSkeletonLoadXskelDocument(xge_xskel_document_t* pDoc, const char* sPath)
{
	FILE* fp;
	char sLine[768];
	char sTag[32];
	int iVersion;
	int i;

	if ( (pDoc == NULL) || (sPath == NULL) || (sPath[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pDoc, 0, sizeof(*pDoc));
	pDoc->iFrameRate = 24;
	fp = fopen(sPath, "rb");
	if ( fp == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL ||
	     sscanf(sLine, "%31s\t%d", sTag, &iVersion) != 2 ||
	     strcmp(sTag, "XGE_SKEL_TEXT") != 0 ||
	     iVersion != 1 ) {
		fclose(fp);
		return XGE_ERROR;
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL ||
	     sscanf(sLine, "document\t%63[^\t]\t%d", pDoc->sName, &pDoc->iFrameRate) != 2 ) {
		fclose(fp);
		return XGE_ERROR;
	}
	if ( pDoc->iFrameRate <= 0 ) {
		pDoc->iFrameRate = 24;
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "bones\t%d", &pDoc->iBoneCount) != 1 ||
	     pDoc->iBoneCount <= 0 || pDoc->iBoneCount > XGE_XSKEL_BONE_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iBoneCount; i++ ) {
		xge_xskel_bone_t* pBone = &pDoc->arrBones[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		__xgeSkeletonStripLineEnd(sLine);
		if ( sscanf(sLine, "bone\t%d\t%d\t%63[^\t]\t%f\t%f\t%f\t%f\t%d\t%d",
		     &pBone->iId, &pBone->iParent, pBone->sName, &pBone->fLocalX, &pBone->fLocalY,
		     &pBone->fRotation, &pBone->fLength, &pBone->bConnected, &pBone->bVisible) != 9 ) {
			fclose(fp);
			return XGE_ERROR;
		}
		if ( pBone->iParent >= i || pBone->iParent < -1 ) {
			fclose(fp);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "slots\t%d", &pDoc->iSlotCount) != 1 ||
	     pDoc->iSlotCount < 0 || pDoc->iSlotCount > XGE_XSKEL_SLOT_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iSlotCount; i++ ) {
		xge_xskel_slot_t* pSlot = &pDoc->arrSlots[i];
		unsigned int iColor;
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		__xgeSkeletonStripLineEnd(sLine);
		if ( sscanf(sLine, "slot\t%d\t%d\t%63[^\t]\t%d\t%d\t%u\t%d",
		     &pSlot->iId, &pSlot->iBone, pSlot->sName, &pSlot->iAttachment, &pSlot->iDrawOrder, &iColor, &pSlot->bVisible) != 7 ) {
			fclose(fp);
			return XGE_ERROR;
		}
		pSlot->iColor = (uint32_t)iColor;
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "attachments\t%d", &pDoc->iAttachmentCount) != 1 ||
	     pDoc->iAttachmentCount < 0 || pDoc->iAttachmentCount > XGE_XSKEL_ATTACHMENT_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iAttachmentCount; i++ ) {
		xge_xskel_attachment_t* pAttachment = &pDoc->arrAttachments[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		__xgeSkeletonStripLineEnd(sLine);
		if ( sscanf(sLine, "attachment\t%d\t%d\t%63[^\t]\t%63[^\t]\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d",
		     &pAttachment->iId, &pAttachment->iBone, pAttachment->sName, pAttachment->sImage,
		     &pAttachment->fOffsetX, &pAttachment->fOffsetY, &pAttachment->fRotation,
		     &pAttachment->fScaleX, &pAttachment->fScaleY, &pAttachment->fPivotX, &pAttachment->fPivotY,
		     &pAttachment->iDrawOrder) != 12 ) {
			fclose(fp);
			return XGE_ERROR;
		}
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "animations\t%d", &pDoc->iAnimationCount) != 1 ||
	     pDoc->iAnimationCount < 0 || pDoc->iAnimationCount > XGE_XSKEL_ANIMATION_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iAnimationCount; i++ ) {
		xge_xskel_animation_t* pAnim = &pDoc->arrAnimations[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		__xgeSkeletonStripLineEnd(sLine);
		if ( sscanf(sLine, "animation\t%d\t%63[^\t]\t%d\t%d\t%d",
		     &pAnim->iId, pAnim->sName, &pAnim->iFrameStart, &pAnim->iFrameEnd, &pAnim->bLoop) != 5 ) {
			fclose(fp);
			return XGE_ERROR;
		}
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "keyframes\t%d", &pDoc->iKeyframeCount) != 1 ||
	     pDoc->iKeyframeCount < 0 || pDoc->iKeyframeCount > XGE_XSKEL_KEYFRAME_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iKeyframeCount; i++ ) {
		xge_xskel_keyframe_t* pKey = &pDoc->arrKeyframes[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		__xgeSkeletonStripLineEnd(sLine);
		if ( sscanf(sLine, "keyframe\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f",
		     &pKey->iId, &pKey->iAnimation, &pKey->iBone, &pKey->iFrame,
		     &pKey->fLocalX, &pKey->fLocalY, &pKey->fRotation, &pKey->fLength) != 8 ) {
			fclose(fp);
			return XGE_ERROR;
		}
	}
	fclose(fp);
	return XGE_OK;
}

static float __xgeSkeletonLerp(float fA, float fB, float fT)
{
	return fA + ((fB - fA) * fT);
}

static void __xgeSkeletonAssetClear(xge_skeleton_asset pAsset)
{
	int i;

	if ( pAsset == NULL ) {
		return;
	}
	if ( pAsset->arrOwnedTextures != NULL ) {
		for ( i = 0; i < pAsset->iOwnedTextureCount; i++ ) {
			xgeTextureFree(&pAsset->arrOwnedTextures[i]);
		}
		xrtFree(pAsset->arrOwnedTextures);
	}
	if ( pAsset->arrAnimations != NULL ) {
		for ( i = 0; i < pAsset->iAnimationCount; i++ ) {
			int j;
			xge_skeleton_animation_t* pAnim = &pAsset->arrAnimations[i];
			if ( pAnim->arrBoneTracks != NULL ) {
				for ( j = 0; j < pAnim->iBoneTrackCount; j++ ) {
					if ( pAnim->arrBoneTracks[j].arrKeys != NULL ) {
						xrtFree(pAnim->arrBoneTracks[j].arrKeys);
					}
				}
				xrtFree(pAnim->arrBoneTracks);
			}
		}
		xrtFree(pAsset->arrAnimations);
	}
	if ( pAsset->arrBones != NULL ) {
		xrtFree(pAsset->arrBones);
	}
	if ( pAsset->arrSlots != NULL ) {
		xrtFree(pAsset->arrSlots);
	}
	if ( pAsset->arrRegions != NULL ) {
		xrtFree(pAsset->arrRegions);
	}
	memset(pAsset, 0, sizeof(*pAsset));
}

static int __xgeSkeletonValidateDesc(const xge_skeleton_asset_desc_t* pDesc)
{
	int i;
	int j;

	if ( (pDesc == NULL) || (pDesc->arrBones == NULL) || (pDesc->iBoneCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDesc->iBoneCount > 4096 || pDesc->iSlotCount < 0 || pDesc->iRegionCount < 0 || pDesc->iAnimationCount < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pDesc->iBoneCount; i++ ) {
		if ( pDesc->arrBones[i].iParent >= i ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	for ( i = 0; i < pDesc->iSlotCount; i++ ) {
		if ( (pDesc->arrSlots == NULL) || (pDesc->arrSlots[i].iBone < 0) || (pDesc->arrSlots[i].iBone >= pDesc->iBoneCount) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( (pDesc->arrSlots[i].iAttachment < 0) || (pDesc->arrSlots[i].iAttachment >= pDesc->iRegionCount) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( (pDesc->iRegionCount > 0) && (pDesc->arrRegions == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pDesc->iAnimationCount > 0) && (pDesc->arrAnimations == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pDesc->iAnimationCount; i++ ) {
		const xge_skeleton_animation_desc_t* pAnim = &pDesc->arrAnimations[i];
		if ( (pAnim->iBoneTrackCount < 0) || ((pAnim->iBoneTrackCount > 0) && (pAnim->arrBoneTracks == NULL)) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		for ( j = 0; j < pAnim->iBoneTrackCount; j++ ) {
			const xge_skeleton_bone_track_desc_t* pTrack = &pAnim->arrBoneTracks[j];
			if ( (pTrack->iBone < 0) || (pTrack->iBone >= pDesc->iBoneCount) || (pTrack->iKeyCount <= 0) || (pTrack->arrKeys == NULL) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		}
	}
	return XGE_OK;
}

int xgeSkeletonAssetInit(xge_skeleton_asset pAsset, const xge_skeleton_asset_desc_t* pDesc)
{
	int i;
	int j;
	int iRet;

	if ( pAsset == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeSkeletonValidateDesc(pDesc);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memset(pAsset, 0, sizeof(*pAsset));
	pAsset->arrBones = (xge_skeleton_bone_t*)xrtMalloc((size_t)pDesc->iBoneCount * sizeof(xge_skeleton_bone_t));
	if ( pAsset->arrBones == NULL ) {
		__xgeSkeletonAssetClear(pAsset);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pAsset->arrBones, 0, (size_t)pDesc->iBoneCount * sizeof(xge_skeleton_bone_t));
	pAsset->iBoneCount = pDesc->iBoneCount;
	for ( i = 0; i < pDesc->iBoneCount; i++ ) {
		xge_skeleton_bone_t* pBone = &pAsset->arrBones[i];
		const xge_skeleton_bone_desc_t* pSrc = &pDesc->arrBones[i];
		__xgeSkeletonCopyName(pBone->sName, pSrc->sName);
		pBone->iParent = pSrc->iParent;
		pBone->fX = pSrc->fX;
		pBone->fY = pSrc->fY;
		pBone->fRotation = pSrc->fRotation;
		pBone->fScaleX = (pSrc->fScaleX == 0.0f) ? 1.0f : pSrc->fScaleX;
		pBone->fScaleY = (pSrc->fScaleY == 0.0f) ? 1.0f : pSrc->fScaleY;
		pBone->fLength = pSrc->fLength;
	}
	if ( pDesc->iSlotCount > 0 ) {
		pAsset->arrSlots = (xge_skeleton_slot_t*)xrtMalloc((size_t)pDesc->iSlotCount * sizeof(xge_skeleton_slot_t));
		if ( pAsset->arrSlots == NULL ) {
			__xgeSkeletonAssetClear(pAsset);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pAsset->arrSlots, 0, (size_t)pDesc->iSlotCount * sizeof(xge_skeleton_slot_t));
		pAsset->iSlotCount = pDesc->iSlotCount;
		for ( i = 0; i < pDesc->iSlotCount; i++ ) {
			__xgeSkeletonCopyName(pAsset->arrSlots[i].sName, pDesc->arrSlots[i].sName);
			pAsset->arrSlots[i].iBone = pDesc->arrSlots[i].iBone;
			pAsset->arrSlots[i].iAttachment = pDesc->arrSlots[i].iAttachment;
			pAsset->arrSlots[i].iColor = pDesc->arrSlots[i].iColor == 0 ? XGE_COLOR_RGBA(255, 255, 255, 255) : pDesc->arrSlots[i].iColor;
		}
	}
	if ( pDesc->iRegionCount > 0 ) {
		pAsset->arrRegions = (xge_skeleton_region_t*)xrtMalloc((size_t)pDesc->iRegionCount * sizeof(xge_skeleton_region_t));
		if ( pAsset->arrRegions == NULL ) {
			__xgeSkeletonAssetClear(pAsset);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pAsset->arrRegions, 0, (size_t)pDesc->iRegionCount * sizeof(xge_skeleton_region_t));
		pAsset->iRegionCount = pDesc->iRegionCount;
		for ( i = 0; i < pDesc->iRegionCount; i++ ) {
			pAsset->arrRegions[i].pTexture = pDesc->arrRegions[i].pTexture;
			__xgeSkeletonCopyName(pAsset->arrRegions[i].sName, pDesc->arrRegions[i].sName);
			pAsset->arrRegions[i].tSrc = pDesc->arrRegions[i].tSrc;
			pAsset->arrRegions[i].tSize = pDesc->arrRegions[i].tSize;
			pAsset->arrRegions[i].tPivot = pDesc->arrRegions[i].tPivot;
			pAsset->arrRegions[i].tOffset = pDesc->arrRegions[i].tOffset;
			pAsset->arrRegions[i].fRotation = pDesc->arrRegions[i].fRotation;
			pAsset->arrRegions[i].fScaleX = pDesc->arrRegions[i].fScaleX == 0.0f ? 1.0f : pDesc->arrRegions[i].fScaleX;
			pAsset->arrRegions[i].fScaleY = pDesc->arrRegions[i].fScaleY == 0.0f ? 1.0f : pDesc->arrRegions[i].fScaleY;
			pAsset->arrRegions[i].iColor = pDesc->arrRegions[i].iColor == 0 ? XGE_COLOR_RGBA(255, 255, 255, 255) : pDesc->arrRegions[i].iColor;
		}
	}
	if ( pDesc->iAnimationCount > 0 ) {
		pAsset->arrAnimations = (xge_skeleton_animation_t*)xrtMalloc((size_t)pDesc->iAnimationCount * sizeof(xge_skeleton_animation_t));
		if ( pAsset->arrAnimations == NULL ) {
			__xgeSkeletonAssetClear(pAsset);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pAsset->arrAnimations, 0, (size_t)pDesc->iAnimationCount * sizeof(xge_skeleton_animation_t));
		pAsset->iAnimationCount = pDesc->iAnimationCount;
		for ( i = 0; i < pDesc->iAnimationCount; i++ ) {
			xge_skeleton_animation_t* pAnim = &pAsset->arrAnimations[i];
			const xge_skeleton_animation_desc_t* pSrc = &pDesc->arrAnimations[i];
			__xgeSkeletonCopyName(pAnim->sName, pSrc->sName);
			pAnim->fDuration = pSrc->fDuration;
			pAnim->iBoneTrackCount = pSrc->iBoneTrackCount;
			if ( pAnim->iBoneTrackCount > 0 ) {
				pAnim->arrBoneTracks = (xge_skeleton_bone_track_t*)xrtMalloc((size_t)pAnim->iBoneTrackCount * sizeof(xge_skeleton_bone_track_t));
				if ( pAnim->arrBoneTracks == NULL ) {
					__xgeSkeletonAssetClear(pAsset);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				memset(pAnim->arrBoneTracks, 0, (size_t)pAnim->iBoneTrackCount * sizeof(xge_skeleton_bone_track_t));
				for ( j = 0; j < pAnim->iBoneTrackCount; j++ ) {
					const xge_skeleton_bone_track_desc_t* pTrackSrc = &pSrc->arrBoneTracks[j];
					xge_skeleton_bone_track_t* pTrack = &pAnim->arrBoneTracks[j];
					pTrack->iBone = pTrackSrc->iBone;
					pTrack->iKeyCount = pTrackSrc->iKeyCount;
					pTrack->arrKeys = (xge_skeleton_bone_key_t*)xrtMalloc((size_t)pTrack->iKeyCount * sizeof(xge_skeleton_bone_key_t));
					if ( pTrack->arrKeys == NULL ) {
						__xgeSkeletonAssetClear(pAsset);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					memcpy(pTrack->arrKeys, pTrackSrc->arrKeys, (size_t)pTrack->iKeyCount * sizeof(xge_skeleton_bone_key_t));
				}
			}
		}
	}
	return XGE_OK;
}

static void __xgeSkeletonFreeTextureArray(xge_texture_t* arrTextures, int iCount)
{
	int i;

	if ( arrTextures == NULL ) {
		return;
	}
	for ( i = 0; i < iCount; i++ ) {
		xgeTextureFree(&arrTextures[i]);
	}
	xrtFree(arrTextures);
}

static int __xgeSkeletonLoadTextureWithSize(xge_texture pTexture, const char* sPath, float* pWidth, float* pHeight)
{
	xge_image_t tImage;
	FILE* fp;
	unsigned char* pData;
	long iSize;
	int iRet;

	if ( (pTexture == NULL) || (sPath == NULL) || (pWidth == NULL) || (pHeight == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tImage, 0, sizeof(tImage));
	fp = fopen(sPath, "rb");
	if ( fp == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( fseek(fp, 0, SEEK_END) != 0 ) {
		fclose(fp);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iSize = ftell(fp);
	if ( iSize <= 0 ) {
		fclose(fp);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( fseek(fp, 0, SEEK_SET) != 0 ) {
		fclose(fp);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pData = (unsigned char*)xrtMalloc((size_t)iSize);
	if ( pData == NULL ) {
		fclose(fp);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( fread(pData, 1, (size_t)iSize, fp) != (size_t)iSize ) {
		xrtFree(pData);
		fclose(fp);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	fclose(fp);
	iRet = xgeImageLoadMemoryEx(&tImage, pData, (int)iSize, XGE_IMAGE_PREMULTIPLIED);
	xrtFree(pData);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeTextureCreateFromImage(pTexture, &tImage);
	if ( iRet == XGE_OK ) {
		*pWidth = (float)tImage.iWidth;
		*pHeight = (float)tImage.iHeight;
	}
	xgeImageFree(&tImage);
	return iRet;
}

static int __xgeSkeletonSlotLess(const xge_xskel_slot_t* pA, const xge_xskel_slot_t* pB)
{
	if ( pA->iDrawOrder != pB->iDrawOrder ) {
		return pA->iDrawOrder < pB->iDrawOrder;
	}
	return pA->iId < pB->iId;
}

int xgeSkeletonAssetLoadXskel(xge_skeleton_asset pAsset, const char* sPath)
{
	xge_xskel_document_t tDoc;
	xge_skeleton_bone_desc_t arrBones[XGE_XSKEL_BONE_CAPACITY];
	xge_skeleton_slot_desc_t arrSlots[XGE_XSKEL_SLOT_CAPACITY];
	xge_skeleton_region_desc_t arrRegions[XGE_XSKEL_ATTACHMENT_CAPACITY];
	xge_skeleton_bone_key_t arrKeys[XGE_XSKEL_KEYFRAME_CAPACITY];
	xge_skeleton_bone_track_desc_t arrTracks[XGE_XSKEL_KEYFRAME_CAPACITY];
	xge_skeleton_animation_desc_t arrAnimations[XGE_XSKEL_ANIMATION_CAPACITY];
	int arrSlotOrder[XGE_XSKEL_SLOT_CAPACITY];
	xge_texture_t* arrTextures;
	xge_skeleton_asset_desc_t tDesc;
	int iTextureCount;
	int iTrackCount;
	int iKeyCount;
	int iSlotCount;
	int i;
	int j;
	int iRet;

	if ( (pAsset == NULL) || (sPath == NULL) || (sPath[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeSkeletonLoadXskelDocument(&tDoc, sPath);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( tDoc.iAttachmentCount > 0 ) {
		arrTextures = (xge_texture_t*)xrtMalloc((size_t)tDoc.iAttachmentCount * sizeof(xge_texture_t));
		if ( arrTextures == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(arrTextures, 0, (size_t)tDoc.iAttachmentCount * sizeof(xge_texture_t));
	} else {
		arrTextures = NULL;
	}
	iTextureCount = 0;
	memset(arrBones, 0, sizeof(arrBones));
	for ( i = 0; i < tDoc.iBoneCount; i++ ) {
		const xge_xskel_bone_t* pSrc = &tDoc.arrBones[i];
		arrBones[i].sName = pSrc->sName;
		arrBones[i].iParent = pSrc->iParent;
		if ( pSrc->bConnected && pSrc->iParent >= 0 ) {
			arrBones[i].fX = tDoc.arrBones[pSrc->iParent].fLength;
			arrBones[i].fY = 0.0f;
		} else {
			arrBones[i].fX = pSrc->fLocalX;
			arrBones[i].fY = pSrc->fLocalY;
		}
		arrBones[i].fRotation = pSrc->fRotation;
		arrBones[i].fScaleX = 1.0f;
		arrBones[i].fScaleY = 1.0f;
		arrBones[i].fLength = pSrc->fLength;
	}
	memset(arrRegions, 0, sizeof(arrRegions));
	for ( i = 0; i < tDoc.iAttachmentCount; i++ ) {
		const xge_xskel_attachment_t* pAttachment = &tDoc.arrAttachments[i];
		char sImagePath[512];
		float fWidth = 0.0f;
		float fHeight = 0.0f;

		if ( (pAttachment->iBone < 0) || (pAttachment->iBone >= tDoc.iBoneCount) || pAttachment->sImage[0] == 0 ) {
			__xgeSkeletonFreeTextureArray(arrTextures, iTextureCount);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		__xgeSkeletonPathJoin(sImagePath, sizeof(sImagePath), sPath, pAttachment->sImage);
		iRet = __xgeSkeletonLoadTextureWithSize(&arrTextures[iTextureCount], sImagePath, &fWidth, &fHeight);
		if ( iRet != XGE_OK ) {
			__xgeSkeletonFreeTextureArray(arrTextures, iTextureCount);
			return iRet;
		}
		arrRegions[i].sName = pAttachment->sName;
		arrRegions[i].pTexture = &arrTextures[iTextureCount];
		arrRegions[i].tSrc = (xge_rect_t){ 0.0f, 0.0f, fWidth, fHeight };
		arrRegions[i].tSize = (xge_vec2_t){ fWidth, fHeight };
		arrRegions[i].tPivot = (xge_vec2_t){ pAttachment->fPivotX, pAttachment->fPivotY };
		arrRegions[i].tOffset = (xge_vec2_t){ pAttachment->fOffsetX, pAttachment->fOffsetY };
		arrRegions[i].fRotation = pAttachment->fRotation;
		arrRegions[i].fScaleX = (pAttachment->fScaleX == 0.0f) ? 1.0f : pAttachment->fScaleX;
		arrRegions[i].fScaleY = (pAttachment->fScaleY == 0.0f) ? 1.0f : pAttachment->fScaleY;
		arrRegions[i].iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		iTextureCount++;
	}
	for ( i = 0; i < tDoc.iSlotCount; i++ ) {
		arrSlotOrder[i] = i;
	}
	for ( i = 1; i < tDoc.iSlotCount; i++ ) {
		int iSlot = arrSlotOrder[i];
		j = i - 1;
		while ( j >= 0 && __xgeSkeletonSlotLess(&tDoc.arrSlots[iSlot], &tDoc.arrSlots[arrSlotOrder[j]]) ) {
			arrSlotOrder[j + 1] = arrSlotOrder[j];
			j--;
		}
		arrSlotOrder[j + 1] = iSlot;
	}
	memset(arrSlots, 0, sizeof(arrSlots));
	iSlotCount = 0;
	for ( i = 0; i < tDoc.iSlotCount; i++ ) {
		const xge_xskel_slot_t* pSlot = &tDoc.arrSlots[arrSlotOrder[i]];
		if ( !pSlot->bVisible ) {
			continue;
		}
		if ( (pSlot->iBone < 0) || (pSlot->iBone >= tDoc.iBoneCount) ||
		     (pSlot->iAttachment < 0) || (pSlot->iAttachment >= tDoc.iAttachmentCount) ) {
			__xgeSkeletonFreeTextureArray(arrTextures, iTextureCount);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrSlots[iSlotCount].sName = pSlot->sName;
		arrSlots[iSlotCount].iBone = pSlot->iBone;
		arrSlots[iSlotCount].iAttachment = pSlot->iAttachment;
		arrSlots[iSlotCount].iColor = pSlot->iColor == 0 ? XGE_COLOR_RGBA(255, 255, 255, 255) : pSlot->iColor;
		iSlotCount++;
	}
	memset(arrKeys, 0, sizeof(arrKeys));
	memset(arrTracks, 0, sizeof(arrTracks));
	memset(arrAnimations, 0, sizeof(arrAnimations));
	iTrackCount = 0;
	iKeyCount = 0;
	for ( i = 0; i < tDoc.iAnimationCount; i++ ) {
		int iAnimTrackStart = iTrackCount;
		int iFrameStart = tDoc.arrAnimations[i].iFrameStart;
		int iFrameEnd = tDoc.arrAnimations[i].iFrameEnd;
		float fFrameRate = (float)((tDoc.iFrameRate > 0) ? tDoc.iFrameRate : 24);

		for ( j = 0; j < tDoc.iBoneCount; j++ ) {
			int iTrackKeyStart = iKeyCount;
			int k;
			for ( k = 0; k < tDoc.iKeyframeCount; k++ ) {
				const xge_xskel_keyframe_t* pSrcKey = &tDoc.arrKeyframes[k];
				xge_skeleton_bone_key_t* pDstKey;
				const xge_xskel_bone_t* pBone;
				float fBaseLength;

				if ( pSrcKey->iAnimation != i || pSrcKey->iBone != j ) {
					continue;
				}
				if ( iKeyCount >= XGE_XSKEL_KEYFRAME_CAPACITY ) {
					__xgeSkeletonFreeTextureArray(arrTextures, iTextureCount);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				pBone = &tDoc.arrBones[j];
				fBaseLength = pBone->fLength == 0.0f ? 1.0f : pBone->fLength;
				pDstKey = &arrKeys[iKeyCount++];
				pDstKey->fTime = (float)(pSrcKey->iFrame - iFrameStart) / fFrameRate;
				if ( pDstKey->fTime < 0.0f ) {
					pDstKey->fTime = 0.0f;
				}
				pDstKey->iFields = XGE_SKELETON_KEY_X | XGE_SKELETON_KEY_Y | XGE_SKELETON_KEY_ROTATION | XGE_SKELETON_KEY_SCALE_X;
				if ( pBone->bConnected && pBone->iParent >= 0 ) {
					pDstKey->fX = tDoc.arrBones[pBone->iParent].fLength;
					pDstKey->fY = 0.0f;
				} else {
					pDstKey->fX = pSrcKey->fLocalX;
					pDstKey->fY = pSrcKey->fLocalY;
				}
				pDstKey->fRotation = pSrcKey->fRotation;
				pDstKey->fScaleX = pSrcKey->fLength / fBaseLength;
				if ( pDstKey->fScaleX == 0.0f ) {
					pDstKey->fScaleX = 1.0f;
				}
				pDstKey->fScaleY = 1.0f;
			}
			if ( iKeyCount > iTrackKeyStart ) {
				arrTracks[iTrackCount].iBone = j;
				arrTracks[iTrackCount].arrKeys = &arrKeys[iTrackKeyStart];
				arrTracks[iTrackCount].iKeyCount = iKeyCount - iTrackKeyStart;
				iTrackCount++;
			}
		}
		arrAnimations[i].sName = tDoc.arrAnimations[i].sName;
		arrAnimations[i].fDuration = (float)(iFrameEnd - iFrameStart) / fFrameRate;
		if ( arrAnimations[i].fDuration < 0.0f ) {
			arrAnimations[i].fDuration = 0.0f;
		}
		arrAnimations[i].arrBoneTracks = &arrTracks[iAnimTrackStart];
		arrAnimations[i].iBoneTrackCount = iTrackCount - iAnimTrackStart;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.arrBones = arrBones;
	tDesc.iBoneCount = tDoc.iBoneCount;
	tDesc.arrSlots = arrSlots;
	tDesc.iSlotCount = iSlotCount;
	tDesc.arrRegions = arrRegions;
	tDesc.iRegionCount = tDoc.iAttachmentCount;
	tDesc.arrAnimations = arrAnimations;
	tDesc.iAnimationCount = tDoc.iAnimationCount;
	iRet = xgeSkeletonAssetInit(pAsset, &tDesc);
	if ( iRet != XGE_OK ) {
		__xgeSkeletonFreeTextureArray(arrTextures, iTextureCount);
		return iRet;
	}
	pAsset->arrOwnedTextures = arrTextures;
	pAsset->iOwnedTextureCount = iTextureCount;
	return XGE_OK;
}

void xgeSkeletonAssetFree(xge_skeleton_asset pAsset)
{
	__xgeSkeletonAssetClear(pAsset);
}

static void __xgeSkeletonLocalFromBone(const xge_skeleton_bone_t* pBone, xge_skeleton_pose_t* pPose)
{
	pPose->fX = pBone->fX;
	pPose->fY = pBone->fY;
	pPose->fRotation = pBone->fRotation;
	pPose->fScaleX = pBone->fScaleX == 0.0f ? 1.0f : pBone->fScaleX;
	pPose->fScaleY = pBone->fScaleY == 0.0f ? 1.0f : pBone->fScaleY;
}

static void __xgeSkeletonComputeWorld(xge_skeleton pSkeleton)
{
	int i;

	for ( i = 0; i < pSkeleton->iBoneCount; i++ ) {
		xge_skeleton_pose_t* pPose = &pSkeleton->arrPose[i];
		xge_skeleton_bone_t* pBone = &pSkeleton->pAsset->arrBones[i];
		float fCos = cosf(pPose->fRotation);
		float fSin = sinf(pPose->fRotation);
		float fA = fCos * pPose->fScaleX;
		float fB = -fSin * pPose->fScaleY;
		float fC = fSin * pPose->fScaleX;
		float fD = fCos * pPose->fScaleY;
		if ( pBone->iParent >= 0 ) {
			xge_skeleton_pose_t* pParent = &pSkeleton->arrPose[pBone->iParent];
			pPose->fWorldX = pParent->fWorldX + (pParent->fWorldA * pPose->fX) + (pParent->fWorldB * pPose->fY);
			pPose->fWorldY = pParent->fWorldY + (pParent->fWorldC * pPose->fX) + (pParent->fWorldD * pPose->fY);
			pPose->fWorldA = (pParent->fWorldA * fA) + (pParent->fWorldB * fC);
			pPose->fWorldB = (pParent->fWorldA * fB) + (pParent->fWorldB * fD);
			pPose->fWorldC = (pParent->fWorldC * fA) + (pParent->fWorldD * fC);
			pPose->fWorldD = (pParent->fWorldC * fB) + (pParent->fWorldD * fD);
		} else {
			pPose->fWorldX = pPose->fX;
			pPose->fWorldY = pPose->fY;
			pPose->fWorldA = fA;
			pPose->fWorldB = fB;
			pPose->fWorldC = fC;
			pPose->fWorldD = fD;
		}
	}
}

int xgeSkeletonInit(xge_skeleton pSkeleton, xge_skeleton_asset pAsset)
{
	int i;

	if ( (pSkeleton == NULL) || (pAsset == NULL) || (pAsset->arrBones == NULL) || (pAsset->iBoneCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSkeleton, 0, sizeof(*pSkeleton));
	pSkeleton->arrSetupPose = (xge_skeleton_pose_t*)xrtMalloc((size_t)pAsset->iBoneCount * sizeof(xge_skeleton_pose_t));
	pSkeleton->arrPose = (xge_skeleton_pose_t*)xrtMalloc((size_t)pAsset->iBoneCount * sizeof(xge_skeleton_pose_t));
	if ( (pSkeleton->arrSetupPose == NULL) || (pSkeleton->arrPose == NULL) ) {
		xgeSkeletonFree(pSkeleton);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pSkeleton->arrSetupPose, 0, (size_t)pAsset->iBoneCount * sizeof(xge_skeleton_pose_t));
	memset(pSkeleton->arrPose, 0, (size_t)pAsset->iBoneCount * sizeof(xge_skeleton_pose_t));
	pSkeleton->pAsset = pAsset;
	pSkeleton->iBoneCount = pAsset->iBoneCount;
	pSkeleton->iAnimation = -1;
	for ( i = 0; i < pSkeleton->iBoneCount; i++ ) {
		__xgeSkeletonLocalFromBone(&pAsset->arrBones[i], &pSkeleton->arrSetupPose[i]);
		pSkeleton->arrPose[i] = pSkeleton->arrSetupPose[i];
	}
	__xgeSkeletonComputeWorld(pSkeleton);
	return XGE_OK;
}

void xgeSkeletonFree(xge_skeleton pSkeleton)
{
	if ( pSkeleton == NULL ) {
		return;
	}
	if ( pSkeleton->arrSetupPose != NULL ) {
		xrtFree(pSkeleton->arrSetupPose);
	}
	if ( pSkeleton->arrPose != NULL ) {
		xrtFree(pSkeleton->arrPose);
	}
	memset(pSkeleton, 0, sizeof(*pSkeleton));
}

int xgeSkeletonFindAnimation(xge_skeleton_asset pAsset, const char* sName)
{
	int i;

	if ( (pAsset == NULL) || (sName == NULL) ) {
		return -1;
	}
	for ( i = 0; i < pAsset->iAnimationCount; i++ ) {
		if ( strcmp(pAsset->arrAnimations[i].sName, sName) == 0 ) {
			return i;
		}
	}
	return -1;
}

int xgeSkeletonFindBone(xge_skeleton pSkeleton, const char* sName)
{
	int i;

	if ( (pSkeleton == NULL) || (pSkeleton->pAsset == NULL) || (sName == NULL) ) {
		return -1;
	}
	for ( i = 0; i < pSkeleton->pAsset->iBoneCount; i++ ) {
		if ( strcmp(pSkeleton->pAsset->arrBones[i].sName, sName) == 0 ) {
			return i;
		}
	}
	return -1;
}

int xgeSkeletonSetAnimation(xge_skeleton pSkeleton, const char* sName, int bLoop)
{
	int iAnim;

	if ( (pSkeleton == NULL) || (pSkeleton->pAsset == NULL) || (sName == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iAnim = xgeSkeletonFindAnimation(pSkeleton->pAsset, sName);
	if ( iAnim < 0 ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	pSkeleton->iAnimation = iAnim;
	pSkeleton->fTime = 0.0f;
	pSkeleton->bLoop = bLoop != 0;
	return xgeSkeletonSetTime(pSkeleton, 0.0f);
}

static float __xgeSkeletonWrapTime(float fTime, float fDuration, int bLoop)
{
	if ( fDuration <= 0.0f ) {
		return 0.0f;
	}
	if ( bLoop ) {
		while ( fTime < 0.0f ) {
			fTime += fDuration;
		}
		while ( fTime > fDuration ) {
			fTime -= fDuration;
		}
		return fTime;
	}
	if ( fTime < 0.0f ) {
		return 0.0f;
	}
	if ( fTime > fDuration ) {
		return fDuration;
	}
	return fTime;
}

static float __xgeSkeletonKeyValue(const xge_skeleton_bone_key_t* pKey, uint32_t iField, const xge_skeleton_pose_t* pSetup)
{
	if ( (pKey->iFields & iField) == 0 ) {
		switch ( iField ) {
			case XGE_SKELETON_KEY_X: return pSetup->fX;
			case XGE_SKELETON_KEY_Y: return pSetup->fY;
			case XGE_SKELETON_KEY_ROTATION: return pSetup->fRotation;
			case XGE_SKELETON_KEY_SCALE_X: return pSetup->fScaleX;
			case XGE_SKELETON_KEY_SCALE_Y: return pSetup->fScaleY;
			default: break;
		}
	}
	switch ( iField ) {
		case XGE_SKELETON_KEY_X: return pKey->fX;
		case XGE_SKELETON_KEY_Y: return pKey->fY;
		case XGE_SKELETON_KEY_ROTATION: return pKey->fRotation;
		case XGE_SKELETON_KEY_SCALE_X: return pKey->fScaleX;
		case XGE_SKELETON_KEY_SCALE_Y: return pKey->fScaleY;
		default: break;
	}
	return 0.0f;
}

static void __xgeSkeletonApplyTrack(xge_skeleton pSkeleton, const xge_skeleton_bone_track_t* pTrack, float fTime)
{
	int i;
	float fT;
	xge_skeleton_pose_t* pPose;
	const xge_skeleton_pose_t* pSetup;
	const xge_skeleton_bone_key_t* pA;
	const xge_skeleton_bone_key_t* pB;

	if ( (pTrack == NULL) || (pTrack->iKeyCount <= 0) || (pTrack->iBone < 0) || (pTrack->iBone >= pSkeleton->iBoneCount) ) {
		return;
	}
	pPose = &pSkeleton->arrPose[pTrack->iBone];
	pSetup = &pSkeleton->arrSetupPose[pTrack->iBone];
	if ( pTrack->iKeyCount == 1 || fTime <= pTrack->arrKeys[0].fTime ) {
		pA = &pTrack->arrKeys[0];
		pPose->fX = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_X, pSetup);
		pPose->fY = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_Y, pSetup);
		pPose->fRotation = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_ROTATION, pSetup);
		pPose->fScaleX = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_SCALE_X, pSetup);
		pPose->fScaleY = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_SCALE_Y, pSetup);
		return;
	}
	for ( i = 0; i < pTrack->iKeyCount - 1; i++ ) {
		if ( fTime <= pTrack->arrKeys[i + 1].fTime ) {
			pA = &pTrack->arrKeys[i];
			pB = &pTrack->arrKeys[i + 1];
			fT = (pB->fTime > pA->fTime) ? ((fTime - pA->fTime) / (pB->fTime - pA->fTime)) : 0.0f;
			pPose->fX = __xgeSkeletonLerp(__xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_X, pSetup), __xgeSkeletonKeyValue(pB, XGE_SKELETON_KEY_X, pSetup), fT);
			pPose->fY = __xgeSkeletonLerp(__xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_Y, pSetup), __xgeSkeletonKeyValue(pB, XGE_SKELETON_KEY_Y, pSetup), fT);
			pPose->fRotation = __xgeSkeletonLerp(__xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_ROTATION, pSetup), __xgeSkeletonKeyValue(pB, XGE_SKELETON_KEY_ROTATION, pSetup), fT);
			pPose->fScaleX = __xgeSkeletonLerp(__xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_SCALE_X, pSetup), __xgeSkeletonKeyValue(pB, XGE_SKELETON_KEY_SCALE_X, pSetup), fT);
			pPose->fScaleY = __xgeSkeletonLerp(__xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_SCALE_Y, pSetup), __xgeSkeletonKeyValue(pB, XGE_SKELETON_KEY_SCALE_Y, pSetup), fT);
			return;
		}
	}
	pA = &pTrack->arrKeys[pTrack->iKeyCount - 1];
	pPose->fX = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_X, pSetup);
	pPose->fY = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_Y, pSetup);
	pPose->fRotation = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_ROTATION, pSetup);
	pPose->fScaleX = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_SCALE_X, pSetup);
	pPose->fScaleY = __xgeSkeletonKeyValue(pA, XGE_SKELETON_KEY_SCALE_Y, pSetup);
}

int xgeSkeletonSetTime(xge_skeleton pSkeleton, float fTime)
{
	int i;
	xge_skeleton_animation_t* pAnim;

	if ( (pSkeleton == NULL) || (pSkeleton->pAsset == NULL) || (pSkeleton->arrPose == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pSkeleton->iBoneCount; i++ ) {
		pSkeleton->arrPose[i] = pSkeleton->arrSetupPose[i];
	}
	if ( (pSkeleton->iAnimation >= 0) && (pSkeleton->iAnimation < pSkeleton->pAsset->iAnimationCount) ) {
		pAnim = &pSkeleton->pAsset->arrAnimations[pSkeleton->iAnimation];
		pSkeleton->fTime = __xgeSkeletonWrapTime(fTime, pAnim->fDuration, pSkeleton->bLoop);
		for ( i = 0; i < pAnim->iBoneTrackCount; i++ ) {
			__xgeSkeletonApplyTrack(pSkeleton, &pAnim->arrBoneTracks[i], pSkeleton->fTime);
		}
	} else {
		pSkeleton->fTime = fTime;
	}
	__xgeSkeletonComputeWorld(pSkeleton);
	return XGE_OK;
}

int xgeSkeletonUpdate(xge_skeleton pSkeleton, float fDelta)
{
	if ( pSkeleton == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return xgeSkeletonSetTime(pSkeleton, pSkeleton->fTime + fDelta);
}

int xgeSkeletonPoseGet(xge_skeleton pSkeleton, int iBone, xge_skeleton_pose_t* pPose)
{
	if ( (pSkeleton == NULL) || (pPose == NULL) || (iBone < 0) || (iBone >= pSkeleton->iBoneCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pPose = pSkeleton->arrPose[iBone];
	return XGE_OK;
}

static uint32_t __xgeSkeletonColorMul(uint32_t iA, uint32_t iB)
{
	int aR = (int)((iA >> 24) & 0xFFu);
	int aG = (int)((iA >> 16) & 0xFFu);
	int aB = (int)((iA >> 8) & 0xFFu);
	int aA = (int)(iA & 0xFFu);
	int bR = (int)((iB >> 24) & 0xFFu);
	int bG = (int)((iB >> 16) & 0xFFu);
	int bB = (int)((iB >> 8) & 0xFFu);
	int bA = (int)(iB & 0xFFu);
	return XGE_COLOR_RGBA((aR * bR) / 255, (aG * bG) / 255, (aB * bB) / 255, (aA * bA) / 255);
}

static void __xgeSkeletonTransformPoint(const xge_skeleton_pose_t* pPose, float fLocalX, float fLocalY, float fBaseX, float fBaseY, float fScale, float* pX, float* pY)
{
	*pX = fBaseX + ((pPose->fWorldX + (pPose->fWorldA * fLocalX) + (pPose->fWorldB * fLocalY)) * fScale);
	*pY = fBaseY + ((pPose->fWorldY + (pPose->fWorldC * fLocalX) + (pPose->fWorldD * fLocalY)) * fScale);
}

void xgeSkeletonDraw(xge_skeleton pSkeleton, const xge_skeleton_draw_desc_t* pDesc)
{
	int i;
	float fBaseX;
	float fBaseY;
	float fScale;
	uint32_t iColor;
	uint32_t iFlags;

	if ( (pSkeleton == NULL) || (pSkeleton->pAsset == NULL) || (pSkeleton->arrPose == NULL) ) {
		return;
	}
	fBaseX = pDesc != NULL ? pDesc->fX : 0.0f;
	fBaseY = pDesc != NULL ? pDesc->fY : 0.0f;
	fScale = (pDesc != NULL && pDesc->fScale != 0.0f) ? pDesc->fScale : 1.0f;
	iColor = (pDesc != NULL && pDesc->iColor != 0) ? pDesc->iColor : XGE_COLOR_RGBA(255, 255, 255, 255);
	iFlags = pDesc != NULL ? pDesc->iFlags : 0;
	for ( i = 0; i < pSkeleton->pAsset->iSlotCount; i++ ) {
		xge_skeleton_slot_t* pSlot = &pSkeleton->pAsset->arrSlots[i];
		xge_skeleton_region_t* pRegion;
		xge_skeleton_pose_t* pPose;
		xge_vertex_t arrVertices[4];
		float fLocalX[4];
		float fLocalY[4];
		float fU0;
		float fV0;
		float fU1;
		float fV1;
		float fCos;
		float fSin;
		int j;
		uint32_t iFinalColor;

		if ( (pSlot->iBone < 0) || (pSlot->iBone >= pSkeleton->iBoneCount) || (pSlot->iAttachment < 0) || (pSlot->iAttachment >= pSkeleton->pAsset->iRegionCount) ) {
			continue;
		}
		pRegion = &pSkeleton->pAsset->arrRegions[pSlot->iAttachment];
		if ( (pRegion->pTexture == NULL) || (pRegion->pTexture->iWidth <= 0) || (pRegion->pTexture->iHeight <= 0) ) {
			continue;
		}
		pPose = &pSkeleton->arrPose[pSlot->iBone];
		fLocalX[0] = -pRegion->tPivot.fX;
		fLocalY[0] = -pRegion->tPivot.fY;
		fLocalX[1] = pRegion->tSize.fX - pRegion->tPivot.fX;
		fLocalY[1] = -pRegion->tPivot.fY;
		fLocalX[2] = -pRegion->tPivot.fX;
		fLocalY[2] = pRegion->tSize.fY - pRegion->tPivot.fY;
		fLocalX[3] = pRegion->tSize.fX - pRegion->tPivot.fX;
		fLocalY[3] = pRegion->tSize.fY - pRegion->tPivot.fY;
		fCos = cosf(pRegion->fRotation);
		fSin = sinf(pRegion->fRotation);
		for ( j = 0; j < 4; j++ ) {
			float fX = fLocalX[j] * pRegion->fScaleX;
			float fY = fLocalY[j] * pRegion->fScaleY;
			float fRX = (fX * fCos) - (fY * fSin) + pRegion->tOffset.fX;
			float fRY = (fX * fSin) + (fY * fCos) + pRegion->tOffset.fY;
			memset(&arrVertices[j], 0, sizeof(arrVertices[j]));
			__xgeSkeletonTransformPoint(pPose, fRX, fRY, fBaseX, fBaseY, fScale, &arrVertices[j].fX, &arrVertices[j].fY);
			arrVertices[j].fW = 1.0f;
		}
		fU0 = pRegion->tSrc.fX / (float)pRegion->pTexture->iWidth;
		fV0 = pRegion->tSrc.fY / (float)pRegion->pTexture->iHeight;
		fU1 = (pRegion->tSrc.fX + pRegion->tSrc.fW) / (float)pRegion->pTexture->iWidth;
		fV1 = (pRegion->tSrc.fY + pRegion->tSrc.fH) / (float)pRegion->pTexture->iHeight;
		arrVertices[0].fU = fU0; arrVertices[0].fV = fV0;
		arrVertices[1].fU = fU1; arrVertices[1].fV = fV0;
		arrVertices[2].fU = fU0; arrVertices[2].fV = fV1;
		arrVertices[3].fU = fU1; arrVertices[3].fV = fV1;
		iFinalColor = __xgeSkeletonColorMul(__xgeSkeletonColorMul(iColor, pSlot->iColor), pRegion->iColor);
		for ( j = 0; j < 4; j++ ) {
			arrVertices[j].iColor = iFinalColor;
		}
		xgeDrawQuad3D(pRegion->pTexture, arrVertices, iFlags);
	}
}
