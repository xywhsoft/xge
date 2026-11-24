#include "xui.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    xui_table_view_cell_output_t* cell;
    xui_table_cell_reference_t* reference;
} thread_input;

static int wrong_thread(void* raw)
{
    const thread_input* input = raw;
    xui_table_cell_reference_t* reference = input->reference;
    assert(xuiTableViewCellOutputWatchReference(input->cell, &reference) == XUI_ERROR_BUSY);
    assert(!reference && !xuiTableCellReferenceIsLive(input->reference));
    xui_table_view_cell_output_t* cell = input->cell;
    assert(xuiTableCellReferenceLockOwned(input->reference, &cell) == XUI_ERROR_BUSY && !cell);
    bool consumed = true;
    assert(xuiTableCellReferenceReleaseOwnerChecked(input->reference, &consumed) == XUI_ERROR_BUSY);
    assert(!consumed);
    return 0;
}

typedef struct {
    xui_table_cell_reference_t* reference;
    bool invalidated;
} retirement_probe;

static void check_retirement(void* raw)
{
    retirement_probe* probe = raw;
    xui_table_view_cell_output_t* cell = NULL;
    assert(!xuiTableCellReferenceIsLive(probe->reference));
    assert(xuiTableCellReferenceLockOwned(probe->reference, &cell) == XUI_OK && !cell);
    probe->invalidated = true;
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xui_table_view_cell_output_t* cell = NULL;
    assert(xuiTableViewCellOutputCreate(&cell) == XUI_OK && cell);
    assert(xuiTableViewCellOutputWatchReference(cell, NULL) == XUI_ERROR_INVALID_ARGUMENT);
    xui_table_cell_reference_t* reference = NULL;
    assert(xrtMemDebugFailAfter(0));
    assert(xuiTableViewCellOutputWatchReference(cell, &reference) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(!reference && xuiTableViewCellOutputCanRetire(cell));

    xmemdebugsnapshot before = {0}, after = {0};
    xrtMemDebugSnapshot(&before);
    assert(xuiTableViewCellOutputWatchReference(cell, &reference) == XUI_OK && reference);
    assert(xuiTableCellReferenceRelease(reference) == XUI_OK);
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);

    assert(xuiTableViewCellOutputWatchReference(cell, &reference) == XUI_OK && reference);
    xui_table_cell_reference_t* other = NULL;
    assert(xuiTableViewCellOutputWatchReference(cell, &other) == XUI_OK && other);
    const thread_input input = {cell, reference};
    xthread* worker = xrtThreadCreate(wrong_thread, (void*)&input, 0);
    assert(worker && xrtThreadWaitFor(worker, 5000) == XWAIT_OK);
    xrtThreadDestroy(worker);

    xui_table_view_cell_output_t* owner = NULL;
    assert(xuiTableCellReferenceLockOwned(reference, &owner) == XUI_OK && owner == cell);
    retirement_probe probe = {reference, false};
    xui_lifetime_link_t link = {0};
    assert(xuiTableViewCellOutputLifetimeWatch(cell, &link, check_retirement, &probe) == XUI_OK);
    assert(xuiTableViewCellOutputReleaseChecked(cell) == XUI_OK);
    assert(xuiTableCellReferenceIsLive(reference) && !probe.invalidated);
    assert(xuiTableViewCellOutputReleaseChecked(owner) == XUI_OK && probe.invalidated);
    assert(!xuiTableCellReferenceIsLive(reference) && !xuiTableCellReferenceIsLive(other));
    owner = (xui_table_view_cell_output_t*)(uintptr_t)1;
    assert(xuiTableCellReferenceLockOwned(other, &owner) == XUI_OK && !owner);
    assert(xuiTableCellReferenceRelease(other) == XUI_OK);
    bool consumed = false;
    assert(xuiTableCellReferenceReleaseOwnerChecked(reference, &consumed) == XUI_OK && consumed);
    puts("XUI_TABLE_CELL_REFERENCE_NATIVE_OK");
    return 0;
}
