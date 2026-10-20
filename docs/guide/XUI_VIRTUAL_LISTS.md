# Borrowed data for XUI list views

`xuiListViewSetItems` retains its existing contract: the control copies every
string and owns the copy. Its drawing is virtualized, but replacing a large
array still allocates and frees storage proportional to its size.

Use `xuiListViewSetVirtualItems(list, count, onText, user)` when the application
already owns an indexed data snapshot. The single-selection path stores only
the count and provider, without allocating per-row strings or state arrays.
Binding does not enumerate the provider. Default drawing requests text only for
visible rows; `xuiListViewGetItemText` requests exactly the specified row.
`onText == NULL` is supported for applications that draw all rows themselves.

The provider runs on the UI thread. It must not perform I/O, mutate or destroy
the control. Return UTF-8 text borrowed until the next provider call; NULL is
treated as an empty string. Keep `user` and its data alive until replacement or
widget destruction. Rebind to publish a changed snapshot, including when its
row count stays the same. The engine does not free the application's snapshot.

```c
static const char *row_text(xui_widget list, int index, void *user)
{
    const MySnapshot *snapshot = user;
    (void)list;
    return snapshot->rows[index].name;
}

xuiListViewSetVirtualItems(list, snapshot->count, row_text, snapshot);
/* Replace the borrowed binding before releasing the old snapshot. */
xuiListViewSetVirtualItems(list, next->count, row_text, next);
release_snapshot(snapshot);
```

Single selection, keyboard navigation, scrollbars, custom rendering and context
menus continue to use the normal list APIs. Enabling multi/range selection or
explicitly supplying enabled flags allocates the corresponding state on demand;
those explicit operations and multi-selection ranges can still take linear
time. Caller-provided selection buffers retain their existing size contract.
Switching away from an owned string list releases its old copies synchronously.
This API does not promise constant time for allocator/OS calls or those mode
transitions. Very large logical extents exceeding `XUI_LAYOUT_UNBOUNDED` are
rejected without replacing the current binding.

Programmatic `xuiListViewSetScroll` and `xuiListViewEnsureVisible` now invalidate
row drawing when the actual offset changes, including after a viewport cache
has already been rendered. They do not synthesize selection notifications.
Setting the same offset remains a no-op.

The existing descriptor layout and owned-string APIs are unchanged. Rebuild the
matching XGE library to obtain the additional exported function; an older DLL
does not contain it. The workshop candidate uses an isolated full-dev profile,
leaving previously published binaries and DLLs available.

`tests/xui/build_list_virtual_test.bat` accepts `CC` and `XGE_PROFILE`. Its C test
uses the actual library and CPU test proxy, a tracking backing allocator,
50,000 and 1,000,000 synthetic rows, primed-cache scrolling, keyboard selection,
disabled/range and external selection, same-count replacement, shrinking and
owned-string round trips. These rows are not game assets or GPU model previews.
