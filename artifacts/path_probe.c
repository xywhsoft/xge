#include "xge.h"
#include <stdio.h>
#include <string.h>

static void probe(const char* s) {
    xge_shape_ex p = NULL;
    const uint8_t* cmds = NULL;
    const xge_vec2_t* pts = NULL;
    int cc = 0, pc = 0;
    int ret = xgeShapeExCreate(&p);
    if (ret == XGE_OK) ret = xgeShapeExAppendSvgPath(p, s);
    if (ret == XGE_OK) xgeShapeExGetPath(p, &cmds, &cc, &pts, &pc);
    printf("ret=%d commands=%d points=%d path=%s\n", ret, cc, pc, s);
    xgeShapeExDestroy(p);
}

int main(void) {
    probe("M4 9 A5 5 0 0114 9");
    probe("M4 9 A5 5 0 0 1 14 9");
    probe("M4 9 A5 5 0 2 1 14 9");
    probe("M4 9 A5 5 0 0 2 14 9");
    probe("M4 9 A5 5 0 0 1 nan 9");
    probe("M4 9 A5 5 0 0 1 1e500 9");
    return 0;
}
