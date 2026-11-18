#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#include <assert.h>
#include <float.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void set_decimal_comma(void)
{
    const char* candidates[] = {"de_DE.UTF-8","German_Germany.1252","German_Germany"};
    for (size_t i = 0; i < sizeof(candidates)/sizeof(*candidates); ++i) {
        if (setlocale(LC_NUMERIC,candidates[i]) && strcmp(localeconv()->decimal_point,",")==0)
            return;
    }
    assert(!"a decimal-comma test locale must be available");
}

int main(void)
{
    const char svg_path[] = "M1.5 2.25 Q3 4 5.5 6 C2 3 4 5 6 7 L1.5 6 Z";
    const char svg_doc[] = "<svg width='8' height='8'><path d='M1.5 2.25 L4.5 2.25 L4.5 6.25 Z'/></svg>";
    xge_shape_ex shape = NULL, copy = NULL;
    char* text = NULL;
    size_t bytes = 0;
    int required = -1;
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    set_decimal_comma();
    assert(xgeShapeExCreate(&shape) == XGE_OK);
    assert(xgeShapeExCreate(&copy) == XGE_OK);
    assert(xgeShapeExExportSvgPathData(shape,&text,&bytes) == XGE_OK && text && !bytes && !*text);
    xrtFree(text);
    assert(xgeShapeExAppendSvgPath(shape,svg_path) == XGE_OK);
    assert(xgeShapeExGetSvgPathData(shape,NULL,0,&required) == XGE_OK && required == sizeof(svg_path));
    char small[4];
    assert(xgeShapeExGetSvgPathData(shape,small,sizeof(small),&required) == XGE_ERROR_BUFFER_TOO_SMALL);
    assert(strcmp(small,"M1.") == 0 && required == sizeof(svg_path));
    char no_space = '@';
    assert(xgeShapeExGetSvgPathData(shape,&no_space,0,&required) == XGE_ERROR_BUFFER_TOO_SMALL);
    assert(no_space == '@' && required == sizeof(svg_path));
    char direct[sizeof(svg_path)];
    assert(xgeShapeExGetSvgPathData(shape,direct,sizeof(direct),NULL) == XGE_OK);
    assert(strcmp(direct,svg_path) == 0);
    assert(xgeShapeExExportSvgPathData(shape,&text,&bytes) == XGE_OK);
    assert(bytes == sizeof(svg_path)-1 && strcmp(text,svg_path)==0);
    assert(xgeShapeExAppendSvgPath(copy,text) == XGE_OK);
    const xge_vec2_t *a, *b;
    int na, nb;
    assert(xgeShapeExGetPath(shape,NULL,NULL,&a,&na) == XGE_OK);
    assert(xgeShapeExGetPath(copy,NULL,NULL,&b,&nb) == XGE_OK);
    assert(na == 7 && nb == na && memcmp(a,b,(size_t)na*sizeof(*a)) == 0);

    /* The SVG document loader uses the same path parser as the Shape API. */
    xge_svg svg;
    xge_rect_t bounds;
    assert(xgeSvgCreate(&svg) == XGE_OK);
    assert(xgeSvgLoadMemory(svg,svg_doc,sizeof(svg_doc)-1) == XGE_OK);
    assert(xgeSvgGetBounds(svg,0.25f,&bounds) == XGE_OK);
    assert(bounds.fX == 1.5f && bounds.fY == 2.25f && bounds.fW == 3 && bounds.fH == 4);
    xgeSvgDestroy(svg);

    /* A bad first command fails transactionally. A valid prefix still commits. */
    assert(xgeShapeExAppendSvgPath(shape,"M1e400 3") == XGE_ERROR_INVALID_ARGUMENT);
    assert(!xrtGetError());
    assert(xgeShapeExGetSvgPathData(shape,direct,sizeof(direct),NULL) == XGE_OK && strcmp(direct,svg_path)==0);
    assert(xgeShapeExReset(copy) == XGE_OK);
    assert(xgeShapeExAppendSvgPath(copy,"M1 2 L1e400 3") == XGE_OK);
    char prefix[8];
    assert(xgeShapeExGetSvgPathData(copy,prefix,sizeof(prefix),NULL) == XGE_OK && strcmp(prefix,"M1 2")==0);

    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    char* rejected = text;
    size_t rejected_size = 9;
    assert(xrtMemDebugFailAfter(0));
    assert(xgeShapeExExportSvgPathData(shape,&rejected,&rejected_size) == XGE_ERROR_OUT_OF_MEMORY);
    assert(rejected == NULL && rejected_size == 0);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);

    assert(xgeShapeExReset(shape) == XGE_OK && xgeShapeExReset(copy) == XGE_OK);
    assert(strcmp(text,svg_path) == 0);
    xrtFree(text);
    assert(xgeShapeExMoveTo(shape,FLT_MAX,-FLT_MAX) == XGE_OK);
    assert(xgeShapeExLineTo(shape,FLT_TRUE_MIN,-0.0f) == XGE_OK);
    assert(xgeShapeExExportSvgPathData(shape,&text,&bytes) == XGE_OK);
    assert(!strchr(text,',') && text[bytes] == '\0');
    assert(xgeShapeExAppendSvgPath(copy,text) == XGE_OK);
    assert(xgeShapeExGetPath(copy,NULL,NULL,&b,&nb) == XGE_OK && nb == 2);
    assert(b[0].fX == FLT_MAX && b[0].fY == -FLT_MAX);
    assert(b[1].fX == FLT_TRUE_MIN && b[1].fY == 0 && signbit(b[1].fY));
    xrtFree(text);
    assert(setlocale(LC_NUMERIC,"C"));
    assert(xgeShapeExReset(copy) == XGE_OK);
    assert(xgeShapeExExportSvgPathData(shape,&text,&bytes) == XGE_OK);
    assert(xgeShapeExAppendSvgPath(copy,text) == XGE_OK);
    assert(xgeShapeExGetPath(copy,NULL,NULL,&b,&nb) == XGE_OK && nb == 2);
    assert(b[0].fX == FLT_MAX && b[0].fY == -FLT_MAX);
    assert(b[1].fX == FLT_TRUE_MIN && b[1].fY == 0 && signbit(b[1].fY));
    xgeShapeExDestroy(shape);
    xgeShapeExDestroy(copy);
    xrtFree(text);
    puts("SHAPE_SVG_PATH_EXPORT_NATIVE_OK");
    return 0;
}
