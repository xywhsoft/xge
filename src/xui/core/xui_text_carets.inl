/* Glyph clusters and editing units have different boundaries. Preserve the
 * actual cluster advance while exposing only Unicode grapheme subdivisions.
 * Backend stops are exact inputs; absent stops use a documented equal split. */
int xuiInternalTextShapeCaretFragments(const char* text, int bytes, xui_text_shape_t* shape)
{
    xui_text_cluster_t* normalized = NULL;
    char* graphemes = NULL;
    int i, j, previous = 0, needed = 0, total, out = 0, caret = 0;
    int result = XUI_OK;
    if (!text || bytes < 0 || !shape || shape->iClusterCount < 0 ||
        (shape->iClusterCount && !shape->pClusters) || shape->iCaretCount < 0 ||
        (shape->iCaretCount && !shape->pCarets)) return XUI_ERROR_INVALID_STATE;
    for (i = 0; i < shape->iClusterCount; i++) {
        const xui_text_cluster_t* cluster = &shape->pClusters[i];
        size_t next;
        if (cluster->iTextStart != previous || cluster->iTextEnd <= previous ||
            cluster->iTextEnd > bytes || !isfinite(cluster->fAdvance) || cluster->fAdvance < 0 ||
            (cluster->iTextEnd < bytes && ((unsigned char)text[cluster->iTextEnd] & 0xc0u) == 0x80u))
            return XUI_ERROR_INVALID_STATE;
        next = (size_t)cluster->iTextStart;
        (void)__xuiTextBreakDecode(text, (size_t)bytes, &next);
        if (next < (size_t)cluster->iTextEnd) needed = 1;
        previous = cluster->iTextEnd;
    }
    if (previous != bytes) return XUI_ERROR_INVALID_STATE;
    if (!needed && !shape->iCaretCount) return XUI_OK;
    graphemes = xrtMalloc((size_t)bytes + 1);
    if (!graphemes) return XUI_ERROR_OUT_OF_MEMORY;
    if (bytes) set_graphemebreaks(text, (size_t)bytes, graphemes, __xuiTextBreakDecode);
    total = shape->iClusterCount;
    for (i = 0; i < shape->iClusterCount; i++) {
        const xui_text_cluster_t* cluster = &shape->pClusters[i];
        for (j = cluster->iTextStart + 1; j < cluster->iTextEnd; j++)
            if (graphemes[j - 1] == GRAPHEMEBREAK_BREAK) {
                if (total == INT_MAX) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
                total++;
            }
    }
    /* Validate the complete metadata before changing even one cached width.
     * Stops at scalar interiors or at cluster endpoints cannot be used. */
    for (i = 0, j = 0; i < shape->iCaretCount; i++) {
        const xui_text_caret_t* stop = &shape->pCarets[i];
        const xui_text_cluster_t* cluster;
        if (stop->iSize < sizeof(*stop) || stop->iTextOffset <= 0 || stop->iTextOffset >= bytes ||
            (i && stop->iTextOffset <= shape->pCarets[i - 1].iTextOffset) ||
            graphemes[stop->iTextOffset - 1] != GRAPHEMEBREAK_BREAK ||
            !isfinite(stop->fAdvance) || stop->fAdvance < 0) { result = XUI_ERROR_INVALID_STATE; goto done; }
        while (j < shape->iClusterCount && shape->pClusters[j].iTextEnd <= stop->iTextOffset) j++;
        if (j == shape->iClusterCount) { result = XUI_ERROR_INVALID_STATE; goto done; }
        cluster = &shape->pClusters[j];
        if (stop->iTextOffset <= cluster->iTextStart || stop->fAdvance > cluster->fAdvance ||
            (i && shape->pCarets[i - 1].iTextOffset > cluster->iTextStart &&
             shape->pCarets[i - 1].fAdvance > stop->fAdvance)) { result = XUI_ERROR_INVALID_STATE; goto done; }
    }
    if ((size_t)total > SIZE_MAX / sizeof(*normalized)) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    if (total != shape->iClusterCount) {
        normalized = xrtCalloc((size_t)total, sizeof(*normalized));
        if (!normalized) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    }
    for (i = 0; i < shape->iClusterCount; i++) {
        const xui_text_cluster_t* cluster = &shape->pClusters[i];
        int begin = caret, pieces = 1, part = 0, start = cluster->iTextStart;
        double advance = 0;
        for (j = start + 1; j < cluster->iTextEnd; j++)
            if (graphemes[j - 1] == GRAPHEMEBREAK_BREAK) pieces++;
        while (caret < shape->iCaretCount && shape->pCarets[caret].iTextOffset < cluster->iTextEnd) caret++;
        if (caret != begin && caret - begin != pieces - 1) { result = XUI_ERROR_INVALID_STATE; goto done; }
        if (!normalized) continue;
        for (j = start + 1; j <= cluster->iTextEnd; j++) {
            double next;
            xui_text_cluster_t* fragment;
            if (j < cluster->iTextEnd && graphemes[j - 1] != GRAPHEMEBREAK_BREAK) continue;
            if (j == cluster->iTextEnd) next = cluster->fAdvance;
            else if (caret != begin) {
                if (shape->pCarets[begin + part].iTextOffset != j) { result = XUI_ERROR_INVALID_STATE; goto done; }
                next = shape->pCarets[begin + part].fAdvance;
            } else next = (float)((double)cluster->fAdvance * (part + 1) / pieces);
            fragment = &normalized[out++]; *fragment = *cluster;
            fragment->iTextStart = start; fragment->iTextEnd = j;
            fragment->fAdvance = (float)(next - advance);
            if (part) fragment->fOffsetX = fragment->fOffsetY = 0;
            if (j < cluster->iTextEnd) fragment->iFlags &= ~XUI_TEXT_CLUSTER_LINE_BREAK;
            start = j; advance = next; part++;
            if (j == cluster->iTextEnd) break;
        }
    }
    if (normalized) {
        xrtFree(shape->pClusters); shape->pClusters = normalized; normalized = NULL;
        shape->iClusterCount = total;
    }
    xrtFree(shape->pCarets); shape->pCarets = NULL; shape->iCaretCount = 0;
done:
    xrtFree(normalized); xrtFree(graphemes);
    return result;
}
