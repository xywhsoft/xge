/* Shape the same deletion projection used for drawing, then restore logical
 * source coordinates. The original UAX14 map still owns wrapping policy. */
static void __xuiTextProjectionGap(xui_text_layout pLayout, xui_text_cluster_t* pClusters,
    int* pCount, int* pSource, int iLimit)
{
    while ( *pSource < iLimit ) {
        int iStart = *pSource;
        XUI_TEXT_BREAK_TEST_COUNT(iMap, 1);
        if ( pLayout->pBreaks[iStart] & XUI_LB_INVISIBLE ) {
            size_t iNext = (size_t)iStart;
            xui_text_cluster_t* p = &pClusters[(*pCount)++];
            (void)__xuiTextBreakDecode(pLayout->sText, (size_t)pLayout->iTextSize, &iNext);
            p->iSize = sizeof(*p);
            p->iTextStart = iStart;
            p->iTextEnd = (int)iNext;
            *pSource = (int)iNext;
        } else {
            (*pSource)++;
        }
    }
}

static int __xuiTextShapeProjection(xui_text_layout pLayout)
{
    char* pWork = NULL;
    char* sDisplay;
    char* pGraphemes;
    int* pSource = NULL;
    xui_text_cluster_t* pClusters;
    int i, iDisplaySize = 0, iFormats = 0, iAt = 0, iCount = 0;
    int iRet, bOrdered = 1;
    for ( i = 0; i < pLayout->iTextSize; i++ ) {
        XUI_TEXT_BREAK_TEST_COUNT(iMap, 1);
        if ( !(pLayout->pBreaks[i] & XUI_LB_INVISIBLE) ) iDisplaySize++;
        else if ( ((unsigned char)pLayout->sText[i] & 0xc0u) != 0x80u ) iFormats++;
    }
    if ( iDisplaySize == pLayout->iTextSize )
        return xuiTextShape(pLayout->pContext, pLayout->tDesc.pFont, pLayout->sText,
            pLayout->iTextSize, XUI_TEXT_SHAPE_DEFAULT, &pLayout->tShape);
    if ( (size_t)iDisplaySize + 1u > SIZE_MAX / 2u ||
         (size_t)iDisplaySize + 1u > SIZE_MAX / sizeof(*pSource) ) return XUI_ERROR_OUT_OF_MEMORY;
    pWork = (char*)xrtMalloc(((size_t)iDisplaySize + 1u) * 2u);
    pSource = (int*)xrtMalloc(((size_t)iDisplaySize + 1u) * sizeof(*pSource));
    if ( pWork == NULL || pSource == NULL ) { iRet = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    sDisplay = pWork;
    pGraphemes = pWork + iDisplaySize + 1;
    for ( i = 0; i < pLayout->iTextSize; i++ ) {
        XUI_TEXT_BREAK_TEST_COUNT(iMap, 1);
        if ( !(pLayout->pBreaks[i] & XUI_LB_INVISIBLE) ) {
            pSource[iAt] = i;
            sDisplay[iAt++] = pLayout->sText[i];
        }
    }
    pSource[iDisplaySize] = pLayout->iTextSize;
    sDisplay[iDisplaySize] = 0;
    if ( iDisplaySize > 0 )
        set_graphemebreaks(sDisplay, (size_t)iDisplaySize, pGraphemes, __xuiTextBreakDecode);
    /* Deletion can join graphemes. Do not expose those new interiors in CHAR
     * mode even when a scalar-only shaping proxy reports separate clusters. */
    for ( i = 0, iAt = 0; i < pLayout->iTextSize; i++ ) {
        XUI_TEXT_BREAK_TEST_COUNT(iMap, 1);
        if ( iAt > 0 && iAt < iDisplaySize && pGraphemes[iAt - 1] != GRAPHEMEBREAK_BREAK )
            pLayout->pBreaks[i] &= (unsigned char)~XUI_LB_GRAPHEME;
        if ( !(pLayout->pBreaks[i] & XUI_LB_INVISIBLE) ) iAt++;
    }
    iRet = xuiTextShape(pLayout->pContext, pLayout->tDesc.pFont, sDisplay,
        iDisplaySize, XUI_TEXT_SHAPE_DEFAULT, &pLayout->tShape);
    if ( iRet != XUI_OK ) goto done;
    for ( i = 0; i < pLayout->tShape.iClusterCount; i++ ) {
        xui_text_cluster_t* p = &pLayout->tShape.pClusters[i];
        int iStart = p->iTextStart, iEnd = p->iTextEnd;
        XUI_TEXT_TEST_COUNT(iIndexBuildSteps, 1);
        if ( iStart < 0 || iEnd < iStart || iEnd > iDisplaySize ) { iRet = XUI_ERROR_INVALID_STATE; goto done; }
        p->iTextStart = pSource[iStart];
        p->iTextEnd = iEnd > iStart ? pSource[iEnd - 1] + 1 : p->iTextStart;
        if ( i && (p->iTextStart < pLayout->tShape.pClusters[i - 1].iTextStart ||
             p->iTextEnd < pLayout->tShape.pClusters[i - 1].iTextEnd) ) bOrdered = 0;
    }
    pLayout->tShape.iTextSize = pLayout->iTextSize;
    xrtFree(pSource);
    pSource = NULL;
    xrtFree(pWork);
    pWork = NULL;
    /* Keep the established scan fallback for unusual unordered custom shapes. */
    if ( !bOrdered ) return XUI_OK;
    if ( pLayout->tShape.iClusterCount > INT_MAX - iFormats ||
         (size_t)(pLayout->tShape.iClusterCount + iFormats) > SIZE_MAX / sizeof(*pClusters) ) return XUI_ERROR_OUT_OF_MEMORY;
    pClusters = (xui_text_cluster_t*)xrtCalloc((size_t)(pLayout->tShape.iClusterCount + iFormats), sizeof(*pClusters));
    if ( pClusters == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
    /* Zero-width gaps preserve raw SHY/ZWSP opportunities between displayed
     * clusters. Formats inside a newly formed cluster remain protected. */
    for ( i = 0, iAt = 0; i < pLayout->tShape.iClusterCount; i++ ) {
        const xui_text_cluster_t* p = &pLayout->tShape.pClusters[i];
        XUI_TEXT_TEST_COUNT(iIndexBuildSteps, 1);
        __xuiTextProjectionGap(pLayout, pClusters, &iCount, &iAt, p->iTextStart);
        pClusters[iCount++] = *p;
        if ( iAt < p->iTextEnd ) iAt = p->iTextEnd;
    }
    __xuiTextProjectionGap(pLayout, pClusters, &iCount, &iAt, pLayout->iTextSize);
    xrtFree(pLayout->tShape.pClusters);
    pLayout->tShape.pClusters = pClusters;
    pLayout->tShape.iClusterCount = iCount;
    return XUI_OK;
done:
    xrtFree(pSource);
    xrtFree(pWork);
    return iRet;
}
