/* Deterministic whole-grapheme font metrics, shared by renderer/editor tests. */
static xui_text_shape_proc unit_font_base_shape;
static xui_font unit_fonts[2];
static unsigned unit_sample, unit_fail;
static const char* const unit_stems[] = {"e\xcc\x81",
    "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb",
    "\xf0\x9f\x87\xa8\xf0\x9f\x87\xb3"};
static const unsigned unit_prefix[] = {1, 4, 4};
/* Entity-leading tails allow a strong close after an emoji symbol. */
static const char* const unit_md_tails[] = {"&#769;XY", "&#8205;\xf0\x9f\x92\xbbXY", "&#127475;XY"};
static xui_font unit_font(xui_context context, const char* family,
    uint32_t marks, float size, void* user)
{
    (void)context; (void)family; (void)size; (void)user;
    return unit_fonts[!!(marks & XUI_DOC_BOLD)];
}
static int unit_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    xui_font font = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    unsigned length = (unsigned)strlen(unit_stems[unit_sample]);
    int result = unit_font_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK && bytes >= (int)length && !memcmp(text, unit_stems[unit_sample], length) &&
        unit_fail && !--unit_fail) return XUI_ERROR_OUT_OF_MEMORY;
    if (result == XUI_OK) {
        int i, count = 0; double width = 0;
        for (i = 0; i < shape->iClusterCount; i++) {
            xui_text_cluster_t cluster = shape->pClusters[i];
            if (bytes - cluster.iTextStart >= (int)length &&
                !memcmp(text + cluster.iTextStart, unit_stems[unit_sample], length)) {
                cluster.iTextEnd = cluster.iTextStart + (int)length;
                cluster.fAdvance = font == unit_fonts[1] ? 22 : 11;
                while (i + 1 < shape->iClusterCount && shape->pClusters[i + 1].iTextStart < cluster.iTextEnd) i++;
            }
            shape->pClusters[count++] = cluster; width += cluster.fAdvance;
        }
        shape->iClusterCount = count; shape->fWidth = (float)width;
    }
    return result;
}
