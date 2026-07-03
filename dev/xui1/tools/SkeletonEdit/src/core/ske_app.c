#include "ske_app.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void SkeletonEditDocumentAddBone(ske_document_t* pDoc, const char* sName, int iParent, float fRotation, float fLength)
{
	ske_bone_t* pBone;

	if ( (pDoc == NULL) || (pDoc->iBoneCount >= SKE_BONE_CAPACITY) ) {
		return;
	}
	pBone = &pDoc->arrBones[pDoc->iBoneCount];
	memset(pBone, 0, sizeof(*pBone));
	pBone->iId = pDoc->iNextBoneId++;
	snprintf(pBone->sName, sizeof(pBone->sName), "%s", (sName != NULL) ? sName : "bone");
	pBone->iParent = iParent;
	pBone->fRotation = fRotation;
	pBone->fLength = fLength;
	pBone->bConnected = (iParent >= 0) ? 1 : 0;
	pBone->bVisible = 1;
	pDoc->iBoneCount++;
}

static void SkeletonEditDocumentAddAnimation(ske_document_t* pDoc, const char* sName, int iFrameStart, int iFrameEnd)
{
	ske_animation_t* pAnim;

	if ( (pDoc == NULL) || (pDoc->iAnimationCount >= SKE_ANIMATION_CAPACITY) ) {
		return;
	}
	pAnim = &pDoc->arrAnimations[pDoc->iAnimationCount];
	memset(pAnim, 0, sizeof(*pAnim));
	pAnim->iId = pDoc->iNextAnimationId++;
	snprintf(pAnim->sName, sizeof(pAnim->sName), "%s", (sName != NULL) ? sName : "anim");
	pAnim->iFrameStart = iFrameStart;
	pAnim->iFrameEnd = iFrameEnd;
	pAnim->bLoop = 1;
	pDoc->iAnimationCount++;
}

static void SkeletonEditDocumentInitDefault(ske_document_t* pDoc)
{
	if ( pDoc == NULL ) {
		return;
	}
	memset(pDoc, 0, sizeof(*pDoc));
	snprintf(pDoc->sName, sizeof(pDoc->sName), "hero.xskel");
	pDoc->iFrameRate = 24;
	pDoc->iSelectedType = SKE_SELECTION_BONE;
	pDoc->iSelectedIndex = 0;
	pDoc->iSelectedImageNode = 111;
	pDoc->iActiveBoneIndex = 0;
	pDoc->iActiveSlotIndex = -1;
	pDoc->iActiveAnimationIndex = 0;
	pDoc->iNextBoneId = 1;
	pDoc->iNextSlotId = 1;
	pDoc->iNextAttachmentId = 1;
	pDoc->iNextAnimationId = 1;
	pDoc->iNextKeyframeId = 1;
	SkeletonEditDocumentAddBone(pDoc, "root", -1, -1.5707963f, 76.0f);
	SkeletonEditDocumentAddBone(pDoc, "body", 0, 0.0f, 86.0f);
	SkeletonEditDocumentAddBone(pDoc, "head", 1, 0.0f, 34.0f);
	SkeletonEditDocumentAddBone(pDoc, "arm.L", 1, -2.3800000f, 62.0f);
	SkeletonEditDocumentAddBone(pDoc, "arm.R", 1, 2.3800000f, 62.0f);
	SkeletonEditDocumentAddAnimation(pDoc, "idle", 0, 48);
}

void SkeletonEditDocumentResetDefault(ske_document_t* pDoc)
{
	SkeletonEditDocumentInitDefault(pDoc);
}

static void SkeletonEditStripLineEnd(char* sText)
{
	size_t n;

	if ( sText == NULL ) {
		return;
	}
	n = strlen(sText);
	while ( n > 0 && (sText[n - 1] == '\n' || sText[n - 1] == '\r') ) {
		sText[n - 1] = 0;
		n--;
	}
}

static int SkeletonEditFloatEqual(float a, float b)
{
	return fabsf(a - b) <= 0.0001f;
}

int SkeletonEditDocumentSave(const ske_document_t* pDoc, const char* sPath)
{
	FILE* fp;
	const ske_bone_t* pBone;
	const ske_slot_t* pSlot;
	const ske_attachment_t* pAttachment;
	const ske_animation_t* pAnim;
	const ske_keyframe_t* pKey;
	int i;

	if ( (pDoc == NULL) || (sPath == NULL) || (sPath[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fp = fopen(sPath, "wb");
	if ( fp == NULL ) {
		return XGE_ERROR;
	}
	fprintf(fp, "XGE_SKEL_TEXT\t1\n");
	fprintf(fp, "document\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
		pDoc->sName,
		pDoc->iFrameRate,
		pDoc->bDirty,
		pDoc->iSelectedType,
		pDoc->iSelectedIndex,
		pDoc->iSelectedImageNode,
		pDoc->iActiveBoneIndex,
		pDoc->iActiveSlotIndex,
		pDoc->iActiveAnimationIndex,
		pDoc->iNextBoneId,
		pDoc->iNextSlotId,
		pDoc->iNextAttachmentId,
		pDoc->iNextAnimationId,
		pDoc->iNextKeyframeId);
	fprintf(fp, "bones\t%d\n", pDoc->iBoneCount);
	for ( i = 0; i < pDoc->iBoneCount; i++ ) {
		pBone = &pDoc->arrBones[i];
		fprintf(fp, "bone\t%d\t%d\t%s\t%.9g\t%.9g\t%.9g\t%.9g\t%d\t%d\n",
			pBone->iId,
			pBone->iParent,
			pBone->sName,
			pBone->fLocalX,
			pBone->fLocalY,
			pBone->fRotation,
			pBone->fLength,
			pBone->bConnected,
			pBone->bVisible);
	}
	fprintf(fp, "slots\t%d\n", pDoc->iSlotCount);
	for ( i = 0; i < pDoc->iSlotCount; i++ ) {
		pSlot = &pDoc->arrSlots[i];
		fprintf(fp, "slot\t%d\t%d\t%s\t%d\t%d\t%u\t%d\n",
			pSlot->iId,
			pSlot->iBone,
			pSlot->sName,
			pSlot->iAttachment,
			pSlot->iDrawOrder,
			(unsigned int)pSlot->iColor,
			pSlot->bVisible);
	}
	fprintf(fp, "attachments\t%d\n", pDoc->iAttachmentCount);
	for ( i = 0; i < pDoc->iAttachmentCount; i++ ) {
		pAttachment = &pDoc->arrAttachments[i];
		fprintf(fp, "attachment\t%d\t%d\t%s\t%s\t%.9g\t%.9g\t%.9g\t%.9g\t%.9g\t%.9g\t%.9g\t%d\n",
			pAttachment->iId,
			pAttachment->iBone,
			pAttachment->sName,
			pAttachment->sImage,
			pAttachment->fOffsetX,
			pAttachment->fOffsetY,
			pAttachment->fRotation,
			pAttachment->fScaleX,
			pAttachment->fScaleY,
			pAttachment->fPivotX,
			pAttachment->fPivotY,
			pAttachment->iDrawOrder);
	}
	fprintf(fp, "animations\t%d\n", pDoc->iAnimationCount);
	for ( i = 0; i < pDoc->iAnimationCount; i++ ) {
		pAnim = &pDoc->arrAnimations[i];
		fprintf(fp, "animation\t%d\t%s\t%d\t%d\t%d\n",
			pAnim->iId,
			pAnim->sName,
			pAnim->iFrameStart,
			pAnim->iFrameEnd,
			pAnim->bLoop);
	}
	fprintf(fp, "keyframes\t%d\n", pDoc->iKeyframeCount);
	for ( i = 0; i < pDoc->iKeyframeCount; i++ ) {
		pKey = &pDoc->arrKeyframes[i];
		fprintf(fp, "keyframe\t%d\t%d\t%d\t%d\t%.9g\t%.9g\t%.9g\t%.9g\n",
			pKey->iId,
			pKey->iAnimation,
			pKey->iBone,
			pKey->iFrame,
			pKey->fLocalX,
			pKey->fLocalY,
			pKey->fRotation,
			pKey->fLength);
	}
	fprintf(fp, "end\n");
	fclose(fp);
	return XGE_OK;
}

int SkeletonEditDocumentLoad(ske_document_t* pDoc, const char* sPath)
{
	FILE* fp;
	char sLine[512];
	char sTag[32];
	int iVersion;
	int i;

	if ( (pDoc == NULL) || (sPath == NULL) || (sPath[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fp = fopen(sPath, "rb");
	if ( fp == NULL ) {
		return XGE_ERROR;
	}
	memset(pDoc, 0, sizeof(*pDoc));
	if ( fgets(sLine, sizeof(sLine), fp) == NULL ||
	     sscanf(sLine, "%31s\t%d", sTag, &iVersion) != 2 ||
	     strcmp(sTag, "XGE_SKEL_TEXT") != 0 ||
	     iVersion != 1 ) {
		fclose(fp);
		return XGE_ERROR;
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL ||
	     sscanf(sLine, "document\t%63[^\t]\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
		     pDoc->sName,
		     &pDoc->iFrameRate,
		     &pDoc->bDirty,
		     &pDoc->iSelectedType,
		     &pDoc->iSelectedIndex,
		     &pDoc->iSelectedImageNode,
		     &pDoc->iActiveBoneIndex,
		     &pDoc->iActiveSlotIndex,
		     &pDoc->iActiveAnimationIndex,
		     &pDoc->iNextBoneId,
		     &pDoc->iNextSlotId,
		     &pDoc->iNextAttachmentId,
		     &pDoc->iNextAnimationId,
		     &pDoc->iNextKeyframeId) != 14 ) {
		fclose(fp);
		return XGE_ERROR;
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "bones\t%d", &pDoc->iBoneCount) != 1 ||
	     pDoc->iBoneCount < 0 || pDoc->iBoneCount > SKE_BONE_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iBoneCount; i++ ) {
		ske_bone_t* pBone = &pDoc->arrBones[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		SkeletonEditStripLineEnd(sLine);
		if ( sscanf(sLine, "bone\t%d\t%d\t%63[^\t]\t%f\t%f\t%f\t%f\t%d\t%d",
			     &pBone->iId,
			     &pBone->iParent,
			     pBone->sName,
			     &pBone->fLocalX,
			     &pBone->fLocalY,
			     &pBone->fRotation,
			     &pBone->fLength,
			     &pBone->bConnected,
			     &pBone->bVisible) != 9 ) {
			fclose(fp);
			return XGE_ERROR;
		}
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "slots\t%d", &pDoc->iSlotCount) != 1 ||
	     pDoc->iSlotCount < 0 || pDoc->iSlotCount > SKE_SLOT_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iSlotCount; i++ ) {
		ske_slot_t* pSlot = &pDoc->arrSlots[i];
		unsigned int iColor;
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		SkeletonEditStripLineEnd(sLine);
		if ( sscanf(sLine, "slot\t%d\t%d\t%63[^\t]\t%d\t%d\t%u\t%d",
			     &pSlot->iId,
			     &pSlot->iBone,
			     pSlot->sName,
			     &pSlot->iAttachment,
			     &pSlot->iDrawOrder,
			     &iColor,
			     &pSlot->bVisible) != 7 ) {
			fclose(fp);
			return XGE_ERROR;
		}
		pSlot->iColor = (uint32_t)iColor;
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "attachments\t%d", &pDoc->iAttachmentCount) != 1 ||
	     pDoc->iAttachmentCount < 0 || pDoc->iAttachmentCount > SKE_ATTACHMENT_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iAttachmentCount; i++ ) {
		ske_attachment_t* pAttachment = &pDoc->arrAttachments[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		SkeletonEditStripLineEnd(sLine);
		if ( sscanf(sLine, "attachment\t%d\t%d\t%63[^\t]\t%63[^\t]\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d",
			     &pAttachment->iId,
			     &pAttachment->iBone,
			     pAttachment->sName,
			     pAttachment->sImage,
			     &pAttachment->fOffsetX,
			     &pAttachment->fOffsetY,
			     &pAttachment->fRotation,
			     &pAttachment->fScaleX,
			     &pAttachment->fScaleY,
			     &pAttachment->fPivotX,
			     &pAttachment->fPivotY,
			     &pAttachment->iDrawOrder) != 12 ) {
			fclose(fp);
			return XGE_ERROR;
		}
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "animations\t%d", &pDoc->iAnimationCount) != 1 ||
	     pDoc->iAnimationCount < 0 || pDoc->iAnimationCount > SKE_ANIMATION_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iAnimationCount; i++ ) {
		ske_animation_t* pAnim = &pDoc->arrAnimations[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		SkeletonEditStripLineEnd(sLine);
		if ( sscanf(sLine, "animation\t%d\t%63[^\t]\t%d\t%d\t%d",
			     &pAnim->iId,
			     pAnim->sName,
			     &pAnim->iFrameStart,
			     &pAnim->iFrameEnd,
			     &pAnim->bLoop) != 5 ) {
			fclose(fp);
			return XGE_ERROR;
		}
	}
	if ( fgets(sLine, sizeof(sLine), fp) == NULL || sscanf(sLine, "keyframes\t%d", &pDoc->iKeyframeCount) != 1 ||
	     pDoc->iKeyframeCount < 0 || pDoc->iKeyframeCount > SKE_KEYFRAME_CAPACITY ) {
		fclose(fp);
		return XGE_ERROR;
	}
	for ( i = 0; i < pDoc->iKeyframeCount; i++ ) {
		ske_keyframe_t* pKey = &pDoc->arrKeyframes[i];
		if ( fgets(sLine, sizeof(sLine), fp) == NULL ) {
			fclose(fp);
			return XGE_ERROR;
		}
		SkeletonEditStripLineEnd(sLine);
		if ( sscanf(sLine, "keyframe\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f",
			     &pKey->iId,
			     &pKey->iAnimation,
			     &pKey->iBone,
			     &pKey->iFrame,
			     &pKey->fLocalX,
			     &pKey->fLocalY,
			     &pKey->fRotation,
			     &pKey->fLength) != 8 ) {
			fclose(fp);
			return XGE_ERROR;
		}
	}
	fclose(fp);
	return XGE_OK;
}

int SkeletonEditDocumentEquals(const ske_document_t* pA, const ske_document_t* pB)
{
	int i;

	if ( pA == NULL || pB == NULL ) {
		return 0;
	}
	if ( strcmp(pA->sName, pB->sName) != 0 ||
	     pA->iFrameRate != pB->iFrameRate ||
	     pA->bDirty != pB->bDirty ||
	     pA->iBoneCount != pB->iBoneCount ||
	     pA->iSlotCount != pB->iSlotCount ||
	     pA->iAttachmentCount != pB->iAttachmentCount ||
	     pA->iAnimationCount != pB->iAnimationCount ||
	     pA->iKeyframeCount != pB->iKeyframeCount ||
	     pA->iSelectedType != pB->iSelectedType ||
	     pA->iSelectedIndex != pB->iSelectedIndex ||
	     pA->iSelectedImageNode != pB->iSelectedImageNode ||
	     pA->iActiveBoneIndex != pB->iActiveBoneIndex ||
	     pA->iActiveSlotIndex != pB->iActiveSlotIndex ||
	     pA->iActiveAnimationIndex != pB->iActiveAnimationIndex ||
	     pA->iNextBoneId != pB->iNextBoneId ||
	     pA->iNextSlotId != pB->iNextSlotId ||
	     pA->iNextAttachmentId != pB->iNextAttachmentId ||
	     pA->iNextAnimationId != pB->iNextAnimationId ||
	     pA->iNextKeyframeId != pB->iNextKeyframeId ) {
		return 0;
	}
	for ( i = 0; i < pA->iBoneCount; i++ ) {
		const ske_bone_t* a = &pA->arrBones[i];
		const ske_bone_t* b = &pB->arrBones[i];
		if ( a->iId != b->iId || a->iParent != b->iParent || strcmp(a->sName, b->sName) != 0 ||
		     !SkeletonEditFloatEqual(a->fLocalX, b->fLocalX) ||
		     !SkeletonEditFloatEqual(a->fLocalY, b->fLocalY) ||
		     !SkeletonEditFloatEqual(a->fRotation, b->fRotation) ||
		     !SkeletonEditFloatEqual(a->fLength, b->fLength) ||
		     a->bConnected != b->bConnected ||
		     a->bVisible != b->bVisible ) {
			return 0;
		}
	}
	for ( i = 0; i < pA->iSlotCount; i++ ) {
		const ske_slot_t* a = &pA->arrSlots[i];
		const ske_slot_t* b = &pB->arrSlots[i];
		if ( a->iId != b->iId || a->iBone != b->iBone || strcmp(a->sName, b->sName) != 0 ||
		     a->iAttachment != b->iAttachment ||
		     a->iDrawOrder != b->iDrawOrder ||
		     a->iColor != b->iColor ||
		     a->bVisible != b->bVisible ) {
			return 0;
		}
	}
	for ( i = 0; i < pA->iAttachmentCount; i++ ) {
		const ske_attachment_t* a = &pA->arrAttachments[i];
		const ske_attachment_t* b = &pB->arrAttachments[i];
		if ( a->iId != b->iId || a->iBone != b->iBone || strcmp(a->sName, b->sName) != 0 ||
		     strcmp(a->sImage, b->sImage) != 0 ||
		     !SkeletonEditFloatEqual(a->fOffsetX, b->fOffsetX) ||
		     !SkeletonEditFloatEqual(a->fOffsetY, b->fOffsetY) ||
		     !SkeletonEditFloatEqual(a->fRotation, b->fRotation) ||
		     !SkeletonEditFloatEqual(a->fScaleX, b->fScaleX) ||
		     !SkeletonEditFloatEqual(a->fScaleY, b->fScaleY) ||
		     !SkeletonEditFloatEqual(a->fPivotX, b->fPivotX) ||
		     !SkeletonEditFloatEqual(a->fPivotY, b->fPivotY) ||
		     a->iDrawOrder != b->iDrawOrder ) {
			return 0;
		}
	}
	for ( i = 0; i < pA->iAnimationCount; i++ ) {
		const ske_animation_t* a = &pA->arrAnimations[i];
		const ske_animation_t* b = &pB->arrAnimations[i];
		if ( a->iId != b->iId || strcmp(a->sName, b->sName) != 0 ||
		     a->iFrameStart != b->iFrameStart ||
		     a->iFrameEnd != b->iFrameEnd ||
		     a->bLoop != b->bLoop ) {
			return 0;
		}
	}
	for ( i = 0; i < pA->iKeyframeCount; i++ ) {
		const ske_keyframe_t* a = &pA->arrKeyframes[i];
		const ske_keyframe_t* b = &pB->arrKeyframes[i];
		if ( a->iId != b->iId ||
		     a->iAnimation != b->iAnimation ||
		     a->iBone != b->iBone ||
		     a->iFrame != b->iFrame ||
		     !SkeletonEditFloatEqual(a->fLocalX, b->fLocalX) ||
		     !SkeletonEditFloatEqual(a->fLocalY, b->fLocalY) ||
		     !SkeletonEditFloatEqual(a->fRotation, b->fRotation) ||
		     !SkeletonEditFloatEqual(a->fLength, b->fLength) ) {
			return 0;
		}
	}
	return 1;
}

static void SkeletonEditRootResize(ske_app_t* pApp)
{
	xge_rect_t tRect;

	if ( (pApp == NULL) || (pApp->pRoot == NULL) ) {
		return;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = (float)xgeGetWidth();
	tRect.fH = (float)xgeGetHeight();
	xgeXuiWidgetSetRect(pApp->pRoot, tRect);
}

int SkeletonEditAppInit(ske_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pApp, 0, sizeof(*pApp));
	pApp->iMode = SKE_MODE_SELECT;
	pApp->iCurrentFrame = 0;
	pApp->fPlaybackAccum = 0.0f;
	pApp->iSelectedTreeNode = 101;
	pApp->fCanvasZoom = 1.0f;
	SkeletonEditDocumentInitDefault(&pApp->tDocument);
	return XGE_OK;
}

void SkeletonEditAppUnit(ske_app_t* pApp)
{
	(void)pApp;
}

int SkeletonEditAppEnter(xge_scene pScene)
{
	ske_app_t* pApp;
	ske_document_t tRoundTripDoc;

	pApp = (ske_app_t*)pScene->pUser;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		printf("skeleton_edit enter failed: xgeXuiInit\n");
		return XGE_ERROR;
	}
	pApp->pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pApp->pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetBackground(pApp->pRoot, XGE_COLOR_RGBA(224, 229, 235, 255));
	SkeletonEditRootResize(pApp);
	if ( SkeletonEditShellBuild(pApp) != XGE_OK ) {
		printf("skeleton_edit enter failed: shell build\n");
		return XGE_ERROR;
	}
	if ( pApp->bSmoke ) {
		if ( SkeletonEditCommandExecute(pApp, SKE_CMD_NEW) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_RESET_LAYOUT) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_ABOUT) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_MODE_SELECT) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_MODE_BONE) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_MODE_SLOT) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_MODE_ATTACHMENT) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_MODE_KEY) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_ADD_BONE) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_ADD_ATTACHMENT) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_UNDO) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_REDO) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_ADD_SLOT) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_ADD_ANIMATION) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_ADD_KEY) != XGE_OK ) {
			printf("skeleton_edit enter failed: static edit smoke\n");
			return XGE_ERROR;
		}
		if ( getenv("XGE_SKELETONEDIT_IMPORT_IMAGE") != NULL &&
		     SkeletonEditCommandExecute(pApp, SKE_CMD_IMPORT_IMAGE) != XGE_OK ) {
			printf("skeleton_edit enter failed: image import smoke\n");
			return XGE_ERROR;
		}
		pApp->fCanvasPanX = 42.0f;
		pApp->fCanvasPanY = -18.0f;
		pApp->fCanvasZoom = 1.5f;
		if ( SkeletonEditCommandExecute(pApp, SKE_CMD_FRAME_ALL) != XGE_OK ) {
			printf("skeleton_edit enter failed: view smoke\n");
			return XGE_ERROR;
		}
		if ( SkeletonEditCommandExecute(pApp, SKE_CMD_PLAY) != XGE_OK ||
		     SkeletonEditCommandExecute(pApp, SKE_CMD_STOP) != XGE_OK ) {
			printf("skeleton_edit enter failed: playback command smoke\n");
			return XGE_ERROR;
		}
		if ( SkeletonEditDocumentSave(&pApp->tDocument, "projects/smoke_roundtrip.xskel") != XGE_OK ||
		     SkeletonEditDocumentLoad(&tRoundTripDoc, "projects/smoke_roundtrip.xskel") != XGE_OK ||
		     !SkeletonEditDocumentEquals(&pApp->tDocument, &tRoundTripDoc) ) {
			printf("skeleton_edit enter failed: document roundtrip\n");
			return XGE_ERROR;
		}
		pApp->bRoundTripOK = 1;
	}
	pApp->bCreateOK = 1;
	xgeXuiRefreshRequest(&pApp->tXui);
	return XGE_OK;
}

int SkeletonEditAppLeave(xge_scene pScene)
{
	ske_app_t* pApp;

	pApp = (ske_app_t*)pScene->pUser;
	if ( pApp != NULL ) {
		SkeletonEditShellUnit(pApp);
		xgeXuiUnit(&pApp->tXui);
	}
	return XGE_OK;
}

int SkeletonEditAppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	ske_app_t* pApp;
	int iResult;

	pApp = (ske_app_t*)pScene->pUser;
	if ( (pApp == NULL) || (pEvent == NULL) ) {
		return XGE_OK;
	}
	iResult = SkeletonEditCanvasHandleEvent(pApp, pEvent);
	if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && ((pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0) ) {
		if ( pEvent->iParam1 == 'N' || pEvent->iParam1 == 'n' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_NEW);
			return XGE_OK;
		}
		if ( pEvent->iParam1 == 'O' || pEvent->iParam1 == 'o' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_OPEN);
			return XGE_OK;
		}
		if ( pEvent->iParam1 == 'S' || pEvent->iParam1 == 's' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_SAVE);
			return XGE_OK;
		}
		if ( pEvent->iParam1 == 'Z' || pEvent->iParam1 == 'z' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_UNDO);
			return XGE_OK;
		}
		if ( pEvent->iParam1 == 'Y' || pEvent->iParam1 == 'y' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_REDO);
			return XGE_OK;
		}
	}
	iResult = xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( pEvent->iType == XGE_EVENT_RESIZE ) {
		SkeletonEditRootResize(pApp);
		xgeXuiRefreshRequest(&pApp->tXui);
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		if ( pEvent->iParam1 == 'B' || pEvent->iParam1 == 'b' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_ADD_BONE);
			return XGE_OK;
		}
		if ( pEvent->iParam1 == 'F' || pEvent->iParam1 == 'f' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_FRAME_ALL);
			return XGE_OK;
		}
		if ( pEvent->iParam1 == 'K' || pEvent->iParam1 == 'k' ) {
			SkeletonEditCommandExecute(pApp, SKE_CMD_ADD_KEY);
			return XGE_OK;
		}
		if ( pEvent->iParam1 == ' ' ) {
			SkeletonEditCommandExecute(pApp, (pApp->iMode == SKE_MODE_PLAY) ? SKE_CMD_STOP : SKE_CMD_PLAY);
			return XGE_OK;
		}
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_DELETE) ) {
		SkeletonEditCommandExecute(pApp, SKE_CMD_DELETE);
		return XGE_OK;
	}
	return XGE_OK;
}

int SkeletonEditAppUpdate(xge_scene pScene, float fDelta)
{
	ske_app_t* pApp;

	pApp = (ske_app_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	SkeletonEditPropertiesFlushRefresh(pApp);
	if ( pApp->iMode == SKE_MODE_PLAY ) {
		float fFrameStep;
		int iRate;
		int iStart;
		int iEnd;

		iRate = (pApp->tDocument.iFrameRate > 0) ? pApp->tDocument.iFrameRate : 24;
		fFrameStep = 1.0f / (float)iRate;
		iStart = 0;
		iEnd = 95;
		if ( pApp->tDocument.iActiveAnimationIndex >= 0 && pApp->tDocument.iActiveAnimationIndex < pApp->tDocument.iAnimationCount ) {
			ske_animation_t* pAnim = &pApp->tDocument.arrAnimations[pApp->tDocument.iActiveAnimationIndex];
			iStart = pAnim->iFrameStart;
			iEnd = pAnim->iFrameEnd;
			if ( iEnd < iStart ) {
				iEnd = iStart;
			}
		}
		pApp->fPlaybackAccum += fDelta;
		while ( pApp->fPlaybackAccum >= fFrameStep ) {
			pApp->fPlaybackAccum -= fFrameStep;
			pApp->iCurrentFrame++;
			if ( pApp->iCurrentFrame > iEnd ) {
				pApp->iCurrentFrame = iStart;
			}
			if ( pApp->bTimelineReady ) {
				xgeXuiTimelineViewSetCurrentFrame(&pApp->tTimeline, pApp->iCurrentFrame);
			}
			if ( pApp->pCanvasWidget != NULL ) {
				xgeXuiWidgetMarkPaint(pApp->pCanvasWidget);
			}
		}
	}
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("skeleton_edit final-summary frames=%d create=%d dock=%d tree=%d props=%d timeline=%d mode=%d frame=%d selected=%d bones=%d slots=%d attachments=%d animations=%d keys=%d dirty=%d undo=%d redo=%d zoom=%.2f pan=%.1f,%.1f roundtrip=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bDockLayoutReady,
			pApp->bAssetTreeReady ? pApp->tAssetTree.iNodeCount : 0,
			pApp->bPropertyGridReady ? pApp->tPropertyGrid.iPropertyCount : 0,
			pApp->bTimelineReady ? xgeXuiTimelineViewGetLayerCount(&pApp->tTimeline) : 0,
			pApp->iMode,
			pApp->iCurrentFrame,
			pApp->iSelectedTreeNode,
			pApp->tDocument.iBoneCount,
			pApp->tDocument.iSlotCount,
			pApp->tDocument.iAttachmentCount,
			pApp->tDocument.iAnimationCount,
			pApp->tDocument.iKeyframeCount,
			pApp->tDocument.bDirty,
			pApp->iUndoCount,
			pApp->iRedoCount,
			pApp->fCanvasZoom,
			pApp->fCanvasPanX,
			pApp->fCanvasPanY,
			pApp->bRoundTripOK);
		xgeQuit();
	}
	return XGE_OK;
}

int SkeletonEditAppDraw(xge_scene pScene)
{
	ske_app_t* pApp;

	pApp = (ske_app_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(224, 229, 235, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}
