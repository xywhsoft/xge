// Full-scan search oracle from 92230b0, using the final pixel-edge coordinate contract.
// Independent raw-layout geometry coverage lives in xui_rich_edit_fractional_test.h.
static xui_rect_t auditReferenceCaret(xui_widget pWidget, xui_rich_edit_data_t* pData, int iOffset)
{
	xui_rect_t tContent = __xuiRichEditContentRect(pWidget, pData);
	xui_rect_t tRect = __xuiRichEditPixelRect((xui_rich_edges_t){0, 0, 1, 16},
		(double)tContent.fX - pData->fScrollX, (double)tContent.fY - pData->fScrollY);
	xui_rich_layout_fragment_t tFragment;
	xui_rich_block_layout_t* pBlock;
	int iBlock;
	int i;
	(void)__xuiRichEditEnsureLayout(pWidget, pData);
	if ( pData->iBlockCount <= 0 ) return tRect;
	iBlock = __xuiRichEditFindBlockByOffset(pData, iOffset);
	if ( iBlock < 0 ) return tRect;
	pBlock = &pData->pBlocks[iBlock];
	for ( i = 0; i < pBlock->iFragmentCount; i++ ) {
		xui_rich_layout_fragment_t* pFragment = &tFragment;
		__xuiRichEditGetBlockFragment(pBlock, i, pFragment);
		if ( iOffset <= pFragment->tPublic.iDocumentStart ) {
			tRect = __xuiRichEditFragmentRect(pWidget, pData, pFragment);
			tRect.fW = 1.0f;
			break;
		}
		if ( iOffset <= pFragment->tPublic.iDocumentEnd ) {
			tRect = __xuiRichEditFragmentRect(pWidget, pData, pFragment);
			tRect.fX += tRect.fW;
			tRect.fW = 1.0f;
			break;
		}
		tRect = __xuiRichEditFragmentRect(pWidget, pData, pFragment);
		tRect.fX += tRect.fW;
		tRect.fW = 1.0f;
	}
	return tRect;
}

static int auditReferenceHit(xui_widget pWidget, xui_rich_edit_data_t* pData, float fLocalX, float fLocalY)
{
	float fX = fLocalX;
	float fY = fLocalY;
	int iLineStart;
	int iLineEnd;
	int iNext;
	int i;
	int iBest;
	float fBest;
	float fDocumentTop;
	float fDocumentBottom;
	xui_rich_layout_fragment_t tFragment;
	xui_rich_layout_fragment_t tNextFragment;
	(void)__xuiRichEditEnsureLayout(pWidget, pData);
	if ( pData->iTotalFragmentCount == 0 || __xuiRichEditGetLayoutFragment(pData, 0, &tFragment) != XUI_OK ) return 0;

	/* Outside the document vertically, editors conventionally clamp to its ends. */
	fDocumentTop = __xuiRichEditFragmentRect(pWidget, pData, &tFragment).fY;
	fDocumentBottom = fDocumentTop + __xuiRichEditFragmentRect(pWidget, pData, &tFragment).fH;
	for ( i = 1; i < pData->iTotalFragmentCount; i++ ) {
		xui_rect_t tRect;
		(void)__xuiRichEditGetLayoutFragment(pData, i, &tFragment);
		tRect = __xuiRichEditFragmentRect(pWidget, pData, &tFragment);
		if ( tRect.fY < fDocumentTop ) fDocumentTop = tRect.fY;
		if ( tRect.fY + tRect.fH > fDocumentBottom ) fDocumentBottom = tRect.fY + tRect.fH;
	}
	if ( fY < fDocumentTop ) return 0;
	if ( fY > fDocumentBottom ) return xuiRichDocumentGetLength(pData->pDocument);

	/* Pick a visual line first. Horizontal distance must never select another line. */
	iLineStart = 0;
	for (;;) {
		int iLine;
		float fBottom;
		float fNextTop;
		(void)__xuiRichEditGetLayoutFragment(pData, iLineStart, &tFragment);
		iLine = tFragment.tPublic.iLine;
		fBottom = __xuiRichEditFragmentRect(pWidget, pData, &tFragment).fY + __xuiRichEditFragmentRect(pWidget, pData, &tFragment).fH;
		iLineEnd = iLineStart + 1;
		while ( iLineEnd < pData->iTotalFragmentCount ) {
			xui_rect_t tRect;
			(void)__xuiRichEditGetLayoutFragment(pData, iLineEnd, &tNextFragment);
			if ( tNextFragment.tPublic.iLine != iLine ) break;
			tRect = __xuiRichEditFragmentRect(pWidget, pData, &tNextFragment);
			if ( tRect.fY + tRect.fH > fBottom ) fBottom = tRect.fY + tRect.fH;
			iLineEnd++;
		}
		if ( iLineEnd >= pData->iTotalFragmentCount ) break;
		iNext = iLineEnd;
		(void)__xuiRichEditGetLayoutFragment(pData, iLineEnd, &tNextFragment);
		fNextTop = __xuiRichEditFragmentRect(pWidget, pData, &tNextFragment).fY;
		while ( iNext < pData->iTotalFragmentCount ) {
			(void)__xuiRichEditGetLayoutFragment(pData, iNext, &tFragment);
			if ( tFragment.tPublic.iLine != tNextFragment.tPublic.iLine ) break;
			if ( __xuiRichEditFragmentRect(pWidget, pData, &tFragment).fY < fNextTop ) fNextTop = __xuiRichEditFragmentRect(pWidget, pData, &tFragment).fY;
			iNext++;
		}
		if ( fY < (fBottom + fNextTop) * 0.5f ) break;
		iLineStart = iLineEnd;
	}

	/* Every fragment contributes a leading and trailing caret stop. */
	(void)__xuiRichEditGetLayoutFragment(pData, iLineStart, &tFragment);
	iBest = tFragment.tPublic.iDocumentStart;
	fBest = 3.402823466e+38F;
	for ( i = iLineStart; i < iLineEnd; i++ ) {
		xui_rect_t tRect;
		(void)__xuiRichEditGetLayoutFragment(pData, i, &tFragment);
		tRect = __xuiRichEditFragmentRect(pWidget, pData, &tFragment);
		float fDistance = fX >= tRect.fX ? fX - tRect.fX : tRect.fX - fX;
		if ( fDistance < fBest ) {
			fBest = fDistance;
			iBest = tFragment.tPublic.iDocumentStart;
		}
		fDistance = fX >= tRect.fX + tRect.fW ? fX - tRect.fX - tRect.fW : tRect.fX + tRect.fW - fX;
		if ( fDistance <= fBest ) {
			fBest = fDistance;
			iBest = tFragment.tPublic.iDocumentEnd;
		}
	}
	return iBest;
}

static xui_rich_node auditReferenceLinkAt(xui_widget pWidget, xui_rich_edit_data_t* pData,
	float fLocalX, float fLocalY)
{
	float fX = fLocalX;
	float fY = fLocalY;
	xui_rich_layout_fragment_t tFragment;
	int i;
	(void)__xuiRichEditEnsureLayout(pWidget, pData);
	for ( i = 0; i < pData->iTotalFragmentCount; i++ ) {
		xui_rich_layout_fragment_t* pFragment = &tFragment;
		(void)__xuiRichEditGetLayoutFragment(pData, i, pFragment);
		xui_rect_t tRect = __xuiRichEditFragmentRect(pWidget, pData, pFragment);
		if ( pFragment->tPublic.iNodeType == XUI_RICH_NODE_LINK && fX >= tRect.fX &&
		     fX <= tRect.fX + tRect.fW && fY >= tRect.fY && fY <= tRect.fY + tRect.fH ) return pFragment->pNode;
	}
	return NULL;
}

static int auditReferenceAtomicAt(xui_widget pWidget, xui_rich_edit_data_t* pData,
	float fLocalX, float fLocalY, int* pStart, int* pEnd, xui_rich_node* ppNode)
{
	float fX = fLocalX;
	float fY = fLocalY;
	xui_rich_layout_fragment_t tFragment;
	int i;
	(void)__xuiRichEditEnsureLayout(pWidget, pData);
	for ( i = 0; i < pData->iTotalFragmentCount; i++ ) {
		xui_rect_t tRect;
		(void)__xuiRichEditGetLayoutFragment(pData, i, &tFragment);
		if ( tFragment.tPublic.iNodeType == XUI_RICH_NODE_TEXT ||
		     tFragment.tPublic.iNodeType == XUI_RICH_NODE_LINK ) continue;
		tRect = __xuiRichEditFragmentRect(pWidget, pData, &tFragment);
		if ( fX >= tRect.fX && fX <= tRect.fX + tRect.fW &&
		     fY >= tRect.fY && fY <= tRect.fY + tRect.fH ) {
			if ( pStart != NULL ) *pStart = tFragment.tPublic.iDocumentStart;
			if ( pEnd != NULL ) *pEnd = tFragment.tPublic.iDocumentEnd;
			if ( ppNode != NULL ) *ppNode = tFragment.pNode;
			return 1;
		}
	}
	return 0;
}

static int auditReferenceRenderBlocks(xui_widget pWidget, xui_rich_edit_data_t* pData,
	xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tContent)
{
	int i;
	int iListNumber = 0;
	for ( i = 0; i < pData->iBlockCount; i++ ) {
		xui_rich_block_layout_t* pBlock = &pData->pBlocks[i];
		xui_rich_node_info_t tInfo;
		xui_rect_t tRect = __xuiRichEditBlockRect(pWidget, pData, pBlock);
		if ( tRect.fY + tRect.fH < tContent.fY || tRect.fY > tContent.fY + tContent.fH ) continue;
		memset(&tInfo, 0, sizeof(tInfo)); tInfo.iSize = sizeof(tInfo);
		if ( xuiRichNodeGetInfo(pBlock->pNode, &tInfo) != XUI_OK ) continue;
		if ( (tInfo.tParagraphStyle.iBackgroundColor & 0xffu) != 0 )
			(void)__xuiRichEditDrawRect(pProxy, pDraw, tRect, tInfo.tParagraphStyle.iBackgroundColor);
		if ( tInfo.iType == XUI_RICH_NODE_BLOCK_QUOTE && pProxy->drawLine != NULL )
			(void)pProxy->drawLine(pProxy, pDraw, tRect.fX + 4.0f * pData->fZoom, tRect.fY,
				tRect.fX + 4.0f * pData->fZoom, tRect.fY + tRect.fH, 3.0f * pData->fZoom,
				(tInfo.tParagraphStyle.iBorderColor & 0xffu) != 0 ?
				tInfo.tParagraphStyle.iBorderColor : XUI_COLOR_RGBA(90,120,160,255));
		if ( (tInfo.iType == XUI_RICH_NODE_LIST_ITEM || tInfo.tParagraphStyle.iListType != XUI_RICH_LIST_NONE) && pProxy->drawText != NULL ) {
			char sMarker[24];
			if ( tInfo.tParagraphStyle.iListType == XUI_RICH_LIST_NUMBER ) snprintf(sMarker, sizeof(sMarker), "%d.", ++iListNumber);
			else if ( tInfo.tParagraphStyle.iListType == XUI_RICH_LIST_CHECK )
				strcpy(sMarker, tInfo.tParagraphStyle.bListChecked ? "[x]" : "[ ]");
			else strcpy(sMarker, "\xe2\x80\xa2");
			(void)pProxy->drawText(pProxy, pDraw,
				__xuiRichEditSizedFont(pWidget, pData, pData->pFont, 0.0f), sMarker,
				__xuiRichEditMarkerRect(pWidget, pData, pBlock, &tInfo),
				pData->iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		} else iListNumber = 0;
		if ( tInfo.iType == XUI_RICH_NODE_IMAGE ) {
			if ( tInfo.pSurface != NULL && pProxy->drawSurface != NULL && pProxy->surfaceGetDesc != NULL ) {
				xui_surface_desc_t tDesc;
				memset(&tDesc, 0, sizeof(tDesc));
				if ( pProxy->surfaceGetDesc(pProxy, tInfo.pSurface, &tDesc) == XUI_OK )
					(void)pProxy->drawSurface(pProxy, pDraw, tInfo.pSurface,
						(xui_rect_t){0.0f, 0.0f, (float)tDesc.iWidth, (float)tDesc.iHeight}, tRect, 0xffffffffu, 0);
			} else {
				(void)__xuiRichEditDrawRect(pProxy, pDraw, tRect, XUI_COLOR_RGBA(238,241,245,255));
				if ( pProxy->drawRectStroke != NULL ) (void)pProxy->drawRectStroke(pProxy, pDraw, tRect, 1.0f, XUI_COLOR_RGBA(170,180,192,255));
				if ( pProxy->drawText != NULL ) (void)pProxy->drawText(pProxy, pDraw,
					__xuiRichEditSizedFont(pWidget, pData, pData->pFont, 0.0f),
					tInfo.sAltText != NULL ? tInfo.sAltText : "Image", tRect, XUI_COLOR_RGBA(90,100,112,255),
					XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			}
		} else if ( tInfo.iType == XUI_RICH_NODE_TABLE ) {
			uint32_t iBorder = (tInfo.iBorderColor & 0xffu) != 0 ? tInfo.iBorderColor : XUI_COLOR_RGBA(170,180,192,255);
			int r, c;
			for ( r = 0; r < tInfo.iRows; r++ ) for ( c = 0; c < tInfo.iColumns; c++ ) {
				xui_rect_t tCell = __xuiRichEditCellRect(pWidget, pData, pBlock, &tInfo, r, c, 0.0);
				uint32_t iCell = r == 0 ? tInfo.iHeaderColor : tInfo.iCellColor;
				xui_rich_text_style_t tCellStyle;
				const char* sCell = xuiRichTableGetCellText(pBlock->pNode, r, c);
				if ( (iCell & 0xffu) != 0 ) (void)__xuiRichEditDrawRect(pProxy, pDraw, tCell, iCell);
				if ( pProxy->drawRectStroke != NULL ) (void)pProxy->drawRectStroke(pProxy, pDraw, tCell,
					tInfo.fBorderWidth > 0.0f ? tInfo.fBorderWidth * pData->fZoom : 1.0f, iBorder);
				memset(&tCellStyle, 0, sizeof(tCellStyle)); tCellStyle.iSize = sizeof(tCellStyle);
				(void)xuiRichTableGetCellStyle(pBlock->pNode, r, c, &tCellStyle);
				if ( sCell != NULL && pProxy->drawText != NULL ) {
					xui_font pCellFont = __xuiRichEditSizedFont(pWidget, pData,
						tCellStyle.pFont != NULL ? tCellStyle.pFont : pData->pFont, tCellStyle.fFontSize);
					tCell = __xuiRichEditCellRect(pWidget, pData, pBlock, &tInfo, r, c, tInfo.fCellPadding * pData->fZoom);
					(void)pProxy->drawText(pProxy, pDraw,
						pCellFont != NULL ? pCellFont : pData->pFont, sCell, tCell,
						(tCellStyle.iTextColor & 0xffu) != 0 ? tCellStyle.iTextColor : pData->iTextColor,
						XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP |
						((tCellStyle.iFlags & XUI_RICH_STYLE_UNDERLINE) ? XUI_TEXT_UNDERLINE : 0));
				}
			}
		} else if ( tInfo.iType == XUI_RICH_NODE_HORIZONTAL_RULE && pProxy->drawLine != NULL ) {
			uint32_t iColor = (tInfo.tParagraphStyle.iBorderColor & 0xffu) != 0 ? tInfo.tParagraphStyle.iBorderColor : XUI_COLOR_RGBA(170,180,192,255);
			(void)pProxy->drawLine(pProxy, pDraw, tRect.fX, tRect.fY + tRect.fH * 0.5f,
				tRect.fX + tRect.fW, tRect.fY + tRect.fH * 0.5f, 1.0f, iColor);
		}
	}
	return XUI_OK;
}

static int auditReferenceRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_rich_edit_data_t* pData = __xuiRichEditData(pWidget);
	xui_proxy pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	xui_rect_t tBounds = xuiWidgetGetContentRect(pWidget);
	xui_rect_t tContent;
	xui_rect_t tRect;
	xui_rich_node_info_t tInfo;
	xui_rich_layout_fragment_t tFragment;
	int iStart;
	int iEnd;
	int i;
	int iRet;
	(void)iStateId; (void)pUser;
	if ( pData == NULL || pProxy == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiRichEditUpdateScrollModel(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiRichEditDrawRect(pProxy, pDraw, tBounds, pData->iBackgroundColor);
	tContent = __xuiRichEditContentRect(pWidget, pData);
	(void)auditReferenceRenderBlocks(pWidget, pData, pProxy, pDraw, tContent);
	__xuiRichEditSelection(pData, &iStart, &iEnd);
	for ( i = 0; i < pData->iTotalFragmentCount; i++ ) {
		xui_rich_layout_fragment_t* pFragment = &tFragment;
		(void)__xuiRichEditGetLayoutFragment(pData, i, pFragment);
		tRect = __xuiRichEditFragmentRect(pWidget, pData, pFragment);
		if ( tRect.fY + tRect.fH < tContent.fY || tRect.fY > tContent.fY + tContent.fH ||
		     tRect.fX + tRect.fW < tContent.fX || tRect.fX > tContent.fX + tContent.fW ) continue;
		(void)__xuiRichEditDrawRect(pProxy, pDraw, tRect, pFragment->iBackgroundColor);
		if ( pData->iFindResultCount > 0 ) {
			int iFind;
			for ( iFind = 0; iFind < pData->iFindResultCount; iFind++ ) {
				xui_find_result_t* pFind = &pData->pFindResults[iFind];
				if ( pFind->iEnd > pFragment->tPublic.iDocumentStart &&
				     pFind->iStart < pFragment->tPublic.iDocumentEnd ) {
					(void)__xuiRichEditDrawRect(pProxy, pDraw, tRect,
						iFind == pData->iFindActiveIndex ? pData->iFindActiveColor : pData->iFindResultColor);
					break;
				}
			}
		}
		if ( pFragment->tPublic.iNodeType == XUI_RICH_NODE_INLINE_IMAGE ) {
			memset(&tInfo, 0, sizeof(tInfo)); tInfo.iSize = sizeof(tInfo);
			if ( xuiRichNodeGetInfo(pFragment->pNode, &tInfo) == XUI_OK ) {
				if ( tInfo.pSurface != NULL && pProxy->drawSurface != NULL && pProxy->surfaceGetDesc != NULL ) {
					xui_surface_desc_t tSurfaceDesc;
					memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
					if ( pProxy->surfaceGetDesc(pProxy, tInfo.pSurface, &tSurfaceDesc) == XUI_OK )
						(void)pProxy->drawSurface(pProxy, pDraw, tInfo.pSurface,
							(xui_rect_t){0.0f, 0.0f, (float)tSurfaceDesc.iWidth, (float)tSurfaceDesc.iHeight},
							tRect, 0xffffffffu, 0);
				} else {
					(void)__xuiRichEditDrawRect(pProxy, pDraw, tRect, XUI_COLOR_RGBA(238,241,245,255));
					if ( pProxy->drawRectStroke != NULL ) (void)pProxy->drawRectStroke(pProxy, pDraw, tRect,
						1.0f, XUI_COLOR_RGBA(170,180,192,255));
				}
			}
		}
		if ( iEnd > pFragment->tPublic.iDocumentStart && iStart < pFragment->tPublic.iDocumentEnd )
			(void)__xuiRichEditDrawRect(pProxy, pDraw, tRect, pData->iSelectionColor);
	}
	for ( i = 0; i < pData->iTotalFragmentCount; ) {
		xui_rich_layout_fragment_t tNext;
		xui_rich_layout_fragment_t* pFragment = &tFragment;
		xui_rect_t tNextRect;
		int iRunEnd;
		int iTextEnd;
		(void)__xuiRichEditGetLayoutFragment(pData, i, pFragment);
		if ( pFragment->tPublic.iNodeType != XUI_RICH_NODE_TEXT && pFragment->tPublic.iNodeType != XUI_RICH_NODE_LINK ) { i++; continue; }
		tRect = __xuiRichEditFragmentRect(pWidget, pData, pFragment);
		iRunEnd = i + 1;
		iTextEnd = pFragment->tPublic.iEndOffset;
		while ( iRunEnd < pData->iTotalFragmentCount ) {
			(void)__xuiRichEditGetLayoutFragment(pData, iRunEnd, &tNext);
			tNextRect = __xuiRichEditFragmentRect(pWidget, pData, &tNext);
			if ( tNext.pNode != pFragment->pNode || tNext.pFont != pFragment->pFont ||
			     tNext.iTextColor != pFragment->iTextColor || tNext.iStyleFlags != pFragment->iStyleFlags ||
			     tNext.tPublic.iLine != pFragment->tPublic.iLine ||
			     tNext.tPublic.iStartOffset != iTextEnd ||
			     tNextRect.fX < tRect.fX + tRect.fW - 0.25f ||
			     tNextRect.fX > tRect.fX + tRect.fW + 0.25f ) break;
			iTextEnd = tNext.tPublic.iEndOffset;
			tRect.fW = tNextRect.fX + tNextRect.fW - tRect.fX;
			if ( tNextRect.fH > tRect.fH ) tRect.fH = tNextRect.fH;
			iRunEnd++;
		}
		if ( tRect.fY + tRect.fH < tContent.fY || tRect.fY > tContent.fY + tContent.fH ) { i = iRunEnd; continue; }
		memset(&tInfo, 0, sizeof(tInfo)); tInfo.iSize = sizeof(tInfo);
		if ( xuiRichNodeGetInfo(pFragment->pNode, &tInfo) != XUI_OK ) { i = iRunEnd; continue; }
		iRet = __xuiRichEditScratch(pData, iTextEnd - pFragment->tPublic.iStartOffset + 1);
		if ( iRet != XUI_OK ) return iRet;
		memcpy(pData->sScratch, tInfo.sText + pFragment->tPublic.iStartOffset,
			(size_t)(iTextEnd - pFragment->tPublic.iStartOffset));
		pData->sScratch[iTextEnd - pFragment->tPublic.iStartOffset] = 0;
		if ( pProxy->drawText != NULL ) (void)pProxy->drawText(pProxy, pDraw, pFragment->pFont, pData->sScratch,
			tRect, pFragment->iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP |
			((pFragment->iStyleFlags & XUI_RICH_STYLE_UNDERLINE) ? XUI_TEXT_UNDERLINE : 0));
		if ( (pFragment->iStyleFlags & XUI_RICH_STYLE_STRIKEOUT) != 0 && pProxy->drawLine != NULL )
			(void)pProxy->drawLine(pProxy, pDraw, tRect.fX, tRect.fY + tRect.fH * 0.52f,
				tRect.fX + tRect.fW, tRect.fY + tRect.fH * 0.52f, 1.0f, pFragment->iTextColor);
		i = iRunEnd;
	}
		if ( pData->bImeActive && pData->sImeText != NULL && pData->sImeText[0] != 0 && pProxy->drawText != NULL ) {
		tRect = __xuiRichEditCaret(pWidget, pData, pData->iImeStart);
		tRect.fW = tContent.fX + tContent.fW - tRect.fX;
		(void)pProxy->drawText(pProxy, pDraw,
			__xuiRichEditSizedFont(pWidget, pData, pData->pFont, 0.0f), pData->sImeText, tRect, pData->iTextColor,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_UNDERLINE | XUI_TEXT_CLIP);
	}
	if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget &&
	    xuiInternalCaretBlinkVisible(pWidget) && pProxy->drawRectFill != NULL ) {
		pData->tCursorRect = __xuiRichEditCaret(pWidget, pData, pData->bImeActive ? pData->iImeStart : pData->iCaret);
		(void)__xuiRichEditDrawRect(pProxy, pDraw, pData->tCursorRect, pData->iCursorColor);
	}
	if ( pData->fBorderWidth > 0.0f && pProxy->drawRectStroke != NULL ) {
		uint32_t iBorder = xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ? pData->iFocusBorderColor : pData->iBorderColor;
		tRect = xuiInternalStrokeCenterRectInside(tBounds, pData->fBorderWidth, NULL);
		(void)pProxy->drawRectStroke(pProxy, pDraw, tRect, pData->fBorderWidth, iBorder);
	}
	return XUI_OK;
}
