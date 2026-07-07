#include "xge.h"
#include <stdio.h>

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
    probe("M0x1 0 L4 4");
    probe("M0x1p2 0 L4 4");
    probe("M+ 0 L4 4");
    probe("M. 0 L4 4");
    probe("M.5 .25 L4 4");
    probe("M5. .25 L4 4");
    probe("M1e-2 0 L4 4");
    return 0;
}
