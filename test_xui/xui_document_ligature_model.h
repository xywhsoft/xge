/* A deterministic ligature shaper. Glyph clusters may cover several Unicode
 * graphemes; the Document's caret units must still follow the source. */
#ifndef XUI_DOCUMENT_LIGATURE_MODEL_H
#define XUI_DOCUMENT_LIGATURE_MODEL_H
#include <math.h>
static xui_text_shape_proc ligature_base_shape;
static unsigned ligature_sample;
static unsigned ligature_caret_mode, ligature_bad_stop, ligature_oom_shape;
static const char* const ligature_patterns[] = {"fi", "ffi", "e\xcc\x81i", "\xce\xb1\xce\xb2", "fi"};
static const char* const ligature_sources[] = {"fiX", "ffiX", "e\xcc\x81iX", "\xce\xb1\xce\xb2X", "f\xe2\x81\xa0iX"};
static const unsigned ligature_middle[] = {1, 1, 3, 2, 1};
static int ligature_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int result = ligature_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK) {
        int i, count = 0, original_count = shape->iClusterCount;
        int length = (int)strlen(ligature_patterns[ligature_sample]);
        double total = 0;
        for (i = 0; i < shape->iClusterCount; i++) {
            xui_text_cluster_t cluster = shape->pClusters[i];
            if (bytes - cluster.iTextStart >= length &&
                !memcmp(text + cluster.iTextStart, ligature_patterns[ligature_sample], (size_t)length)) {
                cluster.iTextEnd = cluster.iTextStart + length;
                cluster.fAdvance = ligature_sample == 1 ? 18 : 12;
                if (ligature_caret_mode) {
                    xui_text_caret_t* stop;
                    if (!shape->pCarets) {
                        shape->pCarets = xrtCalloc((size_t)original_count, sizeof(*shape->pCarets));
                        if (!shape->pCarets) return XUI_ERROR_OUT_OF_MEMORY;
                    }
                    stop = &shape->pCarets[shape->iCaretCount++];
                    stop->iSize = sizeof(*stop); stop->iTextOffset = cluster.iTextStart + (int)ligature_middle[ligature_sample];
                    stop->fAdvance = ligature_sample == 1 ? 4 : 8;
                    if (ligature_sample == 1) {
                        stop = &shape->pCarets[shape->iCaretCount++];
                        stop->iSize = sizeof(*stop); stop->iTextOffset = cluster.iTextStart + 2; stop->fAdvance = 11;
                    }
                }
                while (i + 1 < shape->iClusterCount && shape->pClusters[i + 1].iTextStart < cluster.iTextEnd) i++;
            }
            shape->pClusters[count++] = cluster; total += cluster.fAdvance;
        }
        shape->iClusterCount = count; shape->fWidth = (float)total;
        if (shape->iCaretCount) {
            switch (ligature_bad_stop) {
            case 1: shape->pCarets[0].fAdvance = NAN; break;
            case 2: shape->pCarets[0].iTextOffset = 0; break;
            case 3: shape->iCaretCount--; break;
            case 4: shape->pCarets[0].iTextOffset = 1; break;
            case 5: shape->pCarets[1].fAdvance = 3; break;
            case 6: shape->pCarets[1].iTextOffset = shape->pCarets[0].iTextOffset; break;
            case 7: shape->pCarets[0].fAdvance = 99; break;
            }
            if (ligature_oom_shape && !--ligature_oom_shape) return XUI_ERROR_OUT_OF_MEMORY;
        }
    }
    return result;
}
#endif
