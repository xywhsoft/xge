#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#include <assert.h>
#include <stdio.h>

typedef struct worker_inputs {
    xge_shape_path_snapshot* path;
    xge_shape_gradient_snapshot* gradient;
    xge_shape_dash_snapshot* dash;
} worker_inputs;

static int check_thread(void* raw)
{
    worker_inputs* snapshots = raw;
    const uint8_t* data = NULL;
    size_t count = 99;
    xge_shape_gradient_info_t info;
    float offset = 0;
    assert(xgeShapePathSnapshotCommands(snapshots->path,&data,&count) == XGE_ERROR_INVALID_STATE);
    assert(!data && count == 0);
    assert(xgeShapePathSnapshotAddRef(snapshots->path) == XGE_ERROR_INVALID_STATE);
    assert(xgeShapeGradientSnapshotInfo(snapshots->gradient,&info) == XGE_ERROR_INVALID_STATE);
    assert(xgeShapeGradientSnapshotReleaseChecked(snapshots->gradient) == XGE_ERROR_INVALID_STATE);
    assert(xgeShapeDashSnapshotOffset(snapshots->dash,&offset) == XGE_ERROR_INVALID_STATE);
    assert(xgeShapeDashSnapshotReleaseChecked(snapshots->dash) == XGE_ERROR_INVALID_STATE);
    return 0;
}

int main(void)
{
    const xge_shape_ex_color_stop_t stops[] = {{0.75f,0xff0000ff},{-0.25f,0x00ff00ff}};
    const float dash_values[] = {3,-2,5};
    const xge_shape_ex_matrix_t transform = {1,0,0,1,11,12};
    xge_shape_ex shape = NULL;
    xge_shape_path_snapshot* path = NULL;
    xge_shape_gradient_snapshot* fill = NULL;
    xge_shape_gradient_snapshot* stroke = NULL;
    xge_shape_dash_snapshot* dash = NULL;
    const uint8_t* commands;
    const xge_vec2_t* points;
    const xge_shape_ex_color_stop_t* colors;
    const float* pattern;
    size_t count;
    float offset;

    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xgeShapeExCreate(&shape) == XGE_OK);
    assert(xgeShapeExCapturePath(shape,&path) == XGE_OK);
    assert(xgeShapePathSnapshotPoints(path,&points,&count) == XGE_OK && !points && !count);
    assert(xgeShapePathSnapshotCommands(path,&commands,&count) == XGE_OK && !commands && !count);
    assert(xgeShapePathSnapshotReleaseChecked(path) == XGE_OK);
    assert(xgeShapeExCaptureFillGradient(shape,&fill) == XGE_ERROR_NOT_FOUND && !fill);
    assert(xgeShapeExCaptureStrokeGradient(shape,&stroke) == XGE_ERROR_NOT_FOUND && !stroke);
    assert(xgeShapeExCaptureStrokeDash(shape,&dash) == XGE_OK);
    assert(xgeShapeDashSnapshotPattern(dash,&pattern,&count) == XGE_OK && !pattern && !count);
    assert(xgeShapeDashSnapshotOffset(dash,&offset) == XGE_OK && offset == 0);
    assert(xgeShapeDashSnapshotReleaseChecked(dash) == XGE_OK);

    assert(xgeShapeExAppendSvgPath(shape,"M1 2 L3 4 Q5 6 7 8 Z") == XGE_OK);
    assert(xgeShapeExFillLinearGradient(shape,0,0,10,20,XGE_SHAPE_EX_GRADIENT_USER_SPACE,stops,2) == XGE_OK);
    assert(xgeShapeExFillGradientSpread(shape,XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT) == XGE_OK);
    assert(xgeShapeExFillGradientTransformSet(shape,&transform) == XGE_OK);
    assert(xgeShapeExStrokeRadialGradientEx(shape,4,5,9,6,7,2,XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX,stops,2) == XGE_OK);
    assert(xgeShapeExStrokeDash(shape,dash_values,3,1.5f) == XGE_OK);
    const float* old_pattern;
    int old_count;
    assert(xgeShapeExStrokeDashGet(shape,&old_pattern,&old_count,&offset) == XGE_OK);
    assert(old_count == 3 && offset == 1.5f && old_pattern[1] == 0);

    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeShapeExStrokeDash(shape,dash_values,3,2) == XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeShapeExCapturePath(shape,&path) == XGE_ERROR_OUT_OF_MEMORY && !path);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeShapeExCaptureFillGradient(shape,&fill) == XGE_ERROR_OUT_OF_MEMORY && !fill);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeShapeExCaptureStrokeGradient(shape,&stroke) == XGE_ERROR_OUT_OF_MEMORY && !stroke);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeShapeExCaptureStrokeDash(shape,&dash) == XGE_ERROR_OUT_OF_MEMORY && !dash);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    assert(xgeShapeExStrokeDashGet(shape,&pattern,&old_count,&offset) == XGE_OK);
    assert(pattern == old_pattern && old_count == 3 && offset == 1.5f);
    assert(xgeShapeExStrokeDash(shape,pattern,old_count,2) == XGE_OK);

    assert(xgeShapeExCapturePath(shape,&path) == XGE_OK);
    assert(xgeShapeExCaptureFillGradient(shape,&fill) == XGE_OK);
    assert(xgeShapeExCaptureStrokeGradient(shape,&stroke) == XGE_OK);
    assert(xgeShapeExCaptureStrokeDash(shape,&dash) == XGE_OK);
    worker_inputs inputs = {path,fill,dash};
    xthread* worker = xrtThreadCreate(check_thread,&inputs,0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(xgeShapeExReset(shape) == XGE_OK);
    xgeShapeExDestroy(shape);

    assert(xgeShapePathSnapshotAddRef(path) == XGE_OK);
    assert(xgeShapePathSnapshotReleaseChecked(path) == XGE_OK);
    assert(xgeShapePathSnapshotCommands(path,&commands,&count) == XGE_OK && count == 4);
    assert(commands[0] == XGE_SHAPE_EX_CMD_MOVE_TO && commands[2] == XGE_SHAPE_EX_CMD_QUAD_TO && commands[3] == XGE_SHAPE_EX_CMD_CLOSE);
    assert(xgeShapePathSnapshotPoints(path,&points,&count) == XGE_OK && count == 4);
    assert(points[2].fX == 5 && points[3].fY == 8);
    xge_shape_gradient_info_t info;
    assert(xgeShapeGradientSnapshotInfo(fill,&info) == XGE_OK);
    assert(info.iType == XGE_SHAPE_EX_PAINT_LINEAR_GRADIENT && info.iSpread == XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT);
    assert(info.fX2 == 10 && info.fY2 == 20 && info.tTransform.fE == 11 && info.fFocalRadius == 0);
    assert(xgeShapeGradientSnapshotStops(fill,&colors,&count) == XGE_OK && count == 2);
    assert(colors[0].iColor == stops[0].iColor && colors[1].fOffset == stops[1].fOffset);
    assert(xgeShapeGradientSnapshotInfo(stroke,&info) == XGE_OK);
    assert(info.iType == XGE_SHAPE_EX_PAINT_RADIAL_GRADIENT && info.iUnits == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX);
    assert(info.fX1 == 4 && info.fY1 == 5 && info.fRadius == 9 && info.fFocalX == 6 && info.fFocalY == 7 && info.fFocalRadius == 2);
    assert(xgeShapeDashSnapshotOffset(dash,&offset) == XGE_OK && offset == 2);
    assert(xgeShapeDashSnapshotPattern(dash,&pattern,&count) == XGE_OK && count == 3);
    assert(pattern[0] == 3 && pattern[1] == 0 && pattern[2] == 5);
    assert(xgeShapePathSnapshotReleaseChecked(path) == XGE_OK);
    assert(xgeShapeGradientSnapshotReleaseChecked(fill) == XGE_OK);
    assert(xgeShapeGradientSnapshotReleaseChecked(stroke) == XGE_OK);
    assert(xgeShapeDashSnapshotReleaseChecked(dash) == XGE_OK);
    puts("SHAPE_SNAPSHOTS_NATIVE_OK");
    return 0;
}
