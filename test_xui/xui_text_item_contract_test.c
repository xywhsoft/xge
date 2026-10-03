#include "../src/xui_text_item_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr,"text item contract line %d: %s\n",__LINE__,#e); exit(1); } } while (0)

static void rejected(const xui_text_item_t* item)
{
    xui_text_item_t output, before;
    memset(&output,0xa5,sizeof(output));before=output;
    CHECK(__xuiTextItemNormalize(item,&output)==XUI_ERROR_INVALID_ARGUMENT);
    CHECK(!memcmp(&output,&before,sizeof(output)));
}

int main(void)
{
    const char context[]="x\xd8\xa8y";
    const char bounded[]={'\xd8','\xa8'}; /* No NUL or adjacent readable sentinel. */
    xui_text_item_t item={0},normalized,before,bad;
    uint32_t* short_header=malloc(sizeof(*short_header));
    CHECK(short_header!=NULL);*short_header=sizeof(*short_header);
    rejected((const xui_text_item_t*)short_header);free(short_header);
    rejected(NULL);
    item.iSize=sizeof(item);item.pFont=(xui_font)(uintptr_t)1;
    item.sText=bounded;item.iTextSize=sizeof(bounded);item.iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL;
    item.sContext=context;item.iContextSize=-1;item.iContextOffset=1;
    item.iScript=UINT32_C(0x41726162);item.sLanguage="ar-EG";
    before=item;
    CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK);
    CHECK(!memcmp(&item,&before,sizeof(item)));
    CHECK(normalized.sText==bounded && normalized.iTextSize==2 && normalized.sContext==context &&
        normalized.iContextSize==4 && normalized.iContextOffset==1 && normalized.iFlags==item.iFlags &&
        normalized.iScript==item.iScript && normalized.sLanguage==item.sLanguage && normalized.pFont==item.pFont);
    CHECK(__xuiTextItemNormalize(&normalized,&normalized)==XUI_OK && normalized.iContextSize==4);
    bad=item;bad.iContextOffset=2;rejected(&bad);
    item.sText=context;item.iTextSize=4;item.sContext=NULL;item.iContextSize=item.iContextOffset=0;
    item.iFlags|=XUI_TEXT_SHAPE_RANGE;item.iRangeStart=1;item.iRangeEnd=3;
    CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK && normalized.iRangeStart==1 && normalized.iRangeEnd==3);
    CHECK(__xuiTextItemNormalize(&normalized,&normalized)==XUI_OK);
    {
        xui_text_paint_span_t spans[2]={{sizeof(*spans),0,1,~0u},{sizeof(*spans),1,2,~0u}};
        item.pPaintSpans=spans;item.iPaintSpanCount=2;
        CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK && normalized.pPaintSpans==spans);
        bad=item;bad.iPaintSpanCount=-1;rejected(&bad);
        bad=item;bad.pPaintSpans=NULL;rejected(&bad);
        spans[1].iStart=0;rejected(&item);spans[1].iStart=1;
        spans[1].iEnd=3;rejected(&item);spans[1].iEnd=2;
        spans[1].iSize=4;rejected(&item);item.pPaintSpans=NULL;item.iPaintSpanCount=0;
    }
    bad=item;bad.iRangeStart=2;rejected(&bad);
    bad=item;bad.iRangeEnd=2;rejected(&bad);
    bad=item;bad.iRangeEnd=5;rejected(&bad);
    bad=item;bad.iRangeStart=-1;rejected(&bad);
    bad=item;bad.iRangeStart=4;rejected(&bad);
    bad=item;bad.iFlags &= ~XUI_TEXT_SHAPE_RANGE;rejected(&bad);
    {
        xui_text_shape_t shape={.iSize=sizeof(shape),.iTextSize=4};
        item.pShape=&shape;CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK && normalized.pShape==&shape);
        shape.iTextSize=3;rejected(&item);shape.iTextSize=4;
        shape.iFlags=XUI_TEXT_SHAPE_RANGE;rejected(&item);shape.iFlags=0;
        shape.iSize=4;rejected(&item);item.pShape=NULL;
    }
    item.iRangeStart=item.iRangeEnd=0;
    CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK);
    item=before;
    bad=item;bad.iTextSize=1;rejected(&bad);
    bad=item;bad.iContextSize=-2;rejected(&bad);
    bad=item;bad.iContextOffset=INT_MAX;rejected(&bad);
    bad=item;bad.iTextSize=-2;rejected(&bad);
    bad=item;bad.pFont=NULL;rejected(&bad);
    bad=item;bad.sText=NULL;rejected(&bad);
    bad=item;bad.sLanguage="en_US";rejected(&bad);
    bad=item;bad.iScript=UINT32_C(0x41726130);rejected(&bad);
    bad=item;bad.fDrawOffsetX=NAN;rejected(&bad);
    bad=item;bad.fDrawOffsetX=INFINITY;rejected(&bad);
    bad=item;bad.fDrawOffsetX=.5001f;rejected(&bad);
    bad=item;bad.fDrawOffsetX=-.5001f;rejected(&bad);
    item.fDrawOffsetX=-.375f;
    CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK && normalized.fDrawOffsetX==item.fDrawOffsetX);
    item.sText="xy";item.iTextSize=-1;item.sContext=NULL;item.iContextSize=item.iContextOffset=0;
    CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK && normalized.iTextSize==2 &&
        !normalized.sContext && !normalized.iContextSize);
    bad=item;bad.iContextSize=1;rejected(&bad);
    bad=item;bad.iContextOffset=1;rejected(&bad);
    item.iTextSize=0;
    CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK && !normalized.iTextSize && !normalized.iContextSize);
    item.sContext=context;item.iContextSize=4;item.iContextOffset=4;
    CHECK(__xuiTextItemNormalize(&item,&normalized)==XUI_OK && !normalized.iTextSize && normalized.iContextOffset==4);
    bad=item;bad.iContextOffset=2;rejected(&bad);
    puts("XUI text item C contract: short descriptor, exact non-NUL buffer, alias/idempotence, immutable borrowed properties, empty range and invalid-input atomicity passed");
    return 0;
}
