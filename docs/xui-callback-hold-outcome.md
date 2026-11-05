# Native callback Hold outcomes

A callback owner's `onHold` bool describes acquisition of one physical strong
reference. A successful acquisition and a checked business outcome are separate:

| Hold result | Fresh diagnostic | Candidate publication | Rollback Drop |
| --- | --- | --- | --- |
| false | NULL or non-NULL | rejected | none |
| true | NULL | may proceed after receiver revalidation | one if later rejected |
| true | non-NULL | rejected with CALLBACK_FAILED | exactly one |

Hold can reenter and change the same slot or request closure. Failure never
restores a pre-Hold slot over such legitimate nested changes. Group publication
rolls back every actually acquired candidate, including the candidate whose Hold
produced the first diagnostic. Drop runs outside the ownership mutation gate;
its failure does not replace the earlier nullable primary diagnostic.

Dispatch isolates its pre-existing error while determining whether Hold itself
failed. A pre-existing business error must not suppress an error notification.
Snapshot ownership survives the callback's physical return and all real output
consumers. A false Hold with no diagnostic remains an explicit failure and never
creates a Drop duty. This Native implementation does not query language-specific
error-scope state.

The private helper adds no allocation, public layout field or owner directory.
Owned publishing performs one additional diagnostic read. Owned snapshots isolate
and restore the Native diagnostic. Raw input-event dispatch without an owner
retains its direct call path. Other existing native scopes/RC checks remain and
this work does not claim that managed callbacks have zero lifetime bookkeeping.

CPU-only focused gates:

```
python -B tools/test/check_xui_owned_cache.py --source SOURCE --engine ENGINE --runtime XL_RUNTIME --out NEW_REPORT --case hold_outcome --case hold_renderer
```

The tests use real event primary/per-type slots, atomic Timeline groups, the
Context manager directory, Cell records and Renderer consumers. They cover fresh
Hold errors, a nested replacement that must survive rollback, false/NULL with no
extra Drop, inherited diagnostics, and callback-negative/NULL followed by a
secondary Drop diagnostic. No window, GPU, browser or network is initialized.
