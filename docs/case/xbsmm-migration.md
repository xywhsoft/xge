# xBsmm Migration

`902.xBsmm` in XGE_V1 is a tiny teaching sample for a fixed-size struct block container. It appends structs, reads them back by index, deletes a batch of entries, and verifies that the remaining items stay compact.

XGE V2 does not expose `xBsmm` as a public learning surface, so this migration note is intentionally a document instead of a fake runtime example. In V2, this concern is treated as an internal implementation detail, while public-facing sample code focuses on resource loading, providers, scene flow, and XUI behavior.

## V1 Source

- `XGE_V1/release/examples/freebasic/902.xBsmm/01_Test1.bas`

Core behaviors shown by the V1 sample:

- allocate a block manager for `sizeof(MyStruct)`
- append fixed-size records
- get a writable pointer for each appended record
- iterate by logical index
- delete a set of records in place
- verify the container compacts remaining records

## V2 Mapping

There is no one-to-one public `xBsmm` API in V2. The closest migration guidance is:

- use normal caller-owned arrays or pools for fixed-size gameplay/editor data
- use the resource/provider path for externally loaded blobs and package-backed assets
- keep low-level storage helpers behind module boundaries instead of teaching them as top-level engine API

Relevant V2 references:

- [Resource Provider Lab](resource-provider-lab.md)
- [Resource API](../api/resource.md)
- [资源加载说明](../../dev/docs/资源加载说明.md)

## Suggested Rewrite Pattern

When migrating old code that used `xBsmm`, prefer rewriting the call site instead of searching for a hidden V2 replacement:

1. Replace append/get-pointer logic with a typed array, vector, or pool owned by the feature module.
2. Replace delete-and-compact logic with explicit remove/compact code at the module layer.
3. If the old code was really serving packaged file data rather than gameplay structs, move that path to the V2 resource/provider API instead.

## Why There Is No Standalone Lab

`902.xBsmm` is useful as a historical note, but in V2 it would make the public sample gallery noisier without teaching the way users should build new code today. For that reason, the recommended learning path is:

- learn package and provider loading from [Resource Provider Lab](resource-provider-lab.md)
- learn asset lifetime and async loading from [Async Assets Lab](async-assets-lab.md)
- keep custom fixed-size containers in project code, not in the engine tutorial surface
