#ifndef XUI_TEXT_ITEM_INTERNAL_H
#define XUI_TEXT_ITEM_INTERNAL_H
#include "../xui.h"
#include "xge_text_input.h"
/* Both proxy calls and public shaping use the same validation contract.
 * No XGE font/runtime dependency; only plain C input validation is shared. */
static inline int __xuiTextItemNormalize(const xui_text_item_t* item, xui_text_item_t* output)
{
    int bytes,context_bytes;
    if (!item || item->iSize < sizeof(*item) || !item->pFont || !item->sText || item->iTextSize < -1)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (!(item->fDrawOffsetX>=-.5f && item->fDrawOffsetX<=.5f))return XUI_ERROR_INVALID_ARGUMENT;
    bytes=item->iTextSize;
    if (bytes<0) {size_t n=strlen(item->sText);if(n>INT_MAX)return XUI_ERROR_INVALID_ARGUMENT;bytes=(int)n;}
    if (!__xgeTextLanguageValid(item->sLanguage) || !__xgeTextScriptValid(item->iScript) ||
        !__xgeTextInputRange(item->sText,bytes,item->sContext,item->iContextSize,item->iContextOffset,&context_bytes))
        return XUI_ERROR_INVALID_ARGUMENT;
    if(item->iFlags & XUI_TEXT_SHAPE_RANGE){
        if(item->iRangeStart<0 || item->iRangeEnd<item->iRangeStart || item->iRangeEnd>bytes ||
            (item->iRangeStart<bytes && ((unsigned char)item->sText[item->iRangeStart]&0xc0)==0x80) ||
            (item->iRangeEnd<bytes && ((unsigned char)item->sText[item->iRangeEnd]&0xc0)==0x80))
            return XUI_ERROR_INVALID_ARGUMENT;
        if(item->iPaintSpanCount<0 || (item->iPaintSpanCount && !item->pPaintSpans))return XUI_ERROR_INVALID_ARGUMENT;
        for(int i=0;i<item->iPaintSpanCount;i++)if(item->pPaintSpans[i].iSize<sizeof(*item->pPaintSpans) ||
            item->pPaintSpans[i].iStart<0 || item->pPaintSpans[i].iEnd<item->pPaintSpans[i].iStart ||
            item->pPaintSpans[i].iEnd>item->iRangeEnd-item->iRangeStart ||
            (i && item->pPaintSpans[i].iStart<item->pPaintSpans[i-1].iEnd))return XUI_ERROR_INVALID_ARGUMENT;
        if(item->pShape && (item->pShape->iSize<sizeof(*item->pShape) ||
            item->pShape->iTextSize!=bytes || (item->pShape->iFlags & XUI_TEXT_SHAPE_RANGE)))
            return XUI_ERROR_INVALID_ARGUMENT;
    }else if(item->iRangeStart || item->iRangeEnd || item->pShape || item->pPaintSpans || item->iPaintSpanCount)return XUI_ERROR_INVALID_ARGUMENT;
    *output=*item;output->iTextSize=bytes;
    /* Keep implicit context implicit. A reduced native backend can accept
     * ordinary text while correctly rejecting explicit contextual requests. */
    output->iContextSize=item->sContext ? context_bytes : 0;
    return XUI_OK;
}
#endif
