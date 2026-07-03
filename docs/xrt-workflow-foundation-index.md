# XRT Foundation Index For XUI Workflow

This document indexes XRT capabilities that should be reused by the XUI
Workflow / FlowGraph implementation. The goal is to avoid rebuilding basic
runtime infrastructure inside XUI2.

Source inspected:

- `dev/xui2/lib/xrt/xrt.h`

The current XUI2 public header already includes XRT:

- `dev/xui2/xui.h` includes `lib/xrt/xrt.h`

## Rule

Workflow implementation must prefer XRT primitives for foundational work:

- memory ownership
- temporary allocations
- strings and string formatting
- hash keys and stable lookup
- arrays, dictionaries, lists, stacks, buffers
- typed config values
- XSON and JSON persistence
- file/path helpers
- time and profiling
- logging
- random ids and XID ids
- thread/future helpers where asynchronous work is explicitly needed

Only add a workflow-local primitive when XRT lacks the required behavior or
when graph-specific performance requires a specialized structure.

## Module Availability And Cut Flags

XRT is a single-header runtime. It supports module cut flags such as:

- `XRT_NO_FILE`
- `XRT_NO_THREAD`
- `XRT_NO_ARRAY`
- `XRT_NO_DICT`
- `XRT_NO_LIST`
- `XRT_NO_VALUE`
- `XRT_NO_JSON`
- `XRT_NO_XSON`
- `XRT_NO_LOGGER`

Important dependency rules seen in `xrt.h`:

- JSON requires Value, JNUM, Stack, and File.
- XSON depends on File, Value, Time, JNUM, and JSON.
- Value requires Dict/List-related infrastructure.
- Logger requires Time.

Workflow XSON support therefore assumes XRT Value, JSON, XSON, File, Time, and
container modules are available.

## Memory

Use XRT memory APIs for allocations owned by workflow internals:

- `xrtMalloc`
- `xrtCalloc`
- `xrtRealloc`
- `xrtFree`
- `xrtTempMemory`
- `xrtFreeTempMemory`

Workflow usage:

- graph, node, port, edge, command, and diagnostic arrays.
- copied strings for ids, labels, titles, config paths, and diagnostics.
- temporary path/string building during validation or export.

Guidelines:

- All memory returned by XRT helpers that says it must be freed should be freed
  with `xrtFree`.
- Do not mix CRT `malloc/free` with XRT-owned data.
- Avoid long-lived pointers to `xrtTempMemory` results.
- When a model stores a string, copy it into owned XRT memory unless the API
  explicitly documents borrowed lifetime.

## Errors

Useful APIs:

- `xrtSetError`
- `xrtGetError`

Workflow usage:

- Low-level load/save helpers can set XRT error text.
- Public workflow validation should still return structured diagnostics rather
  than only using the thread error string.

Guidelines:

- Use structured workflow diagnostics for user-visible model errors.
- Use XRT error state for unexpected internal/runtime failure details.

## Strings And Paths

Useful string APIs:

- `xrtStrDup`
- `xrtStrComp`
- `xrtStrLike`
- `xrtStrSim`
- `xrtStrApprox`
- `xrtStrFormat`
- `xrtStrReplace`
- `xrtStrSplit`
- `xrtRandStr`
- `xrtHash32`
- `xrtHash64`

Useful path APIs:

- `xrtPathGetNameExt`
- `xrtPathGetName`
- `xrtPathGetExt`
- `xrtPathGetDir`
- `xrtPathIsAbs`
- `xrtPathJoin`
- `xrtPathRandom`

Workflow usage:

- id, type id, port id, variable id, and config path comparisons.
- fuzzy search in the node library.
- command palette or node search matching.
- export/import path handling.
- diagnostic message formatting.
- hash-based spatial buckets or id lookup keys.

Guidelines:

- Use exact string compare for ids.
- Use `xrtStrLike`/`xrtStrApprox` only for search/filter UI.
- Use `xrtPathJoin` for file examples and workflow save/load paths.

## Containers

### Dynamic Arrays

Useful APIs:

- `xrtArrayCreate`
- `xrtArrayDestroy`
- `xrtArrayInit`
- `xrtArrayUnit`
- `xrtArrayAlloc`
- `xrtArrayAppend`
- `xrtArrayInsert`
- `xrtArrayRemove`
- `xrtArrayGet`
- `xrtArraySort`

Workflow usage:

- ordered node list.
- ordered edge list.
- port list per node.
- selection list.
- diagnostics list.
- command history.
- visible render item list.

Guidelines:

- Prefer `xrtArray` for dense ordered lists where ids are not the primary key.
- Use `xrtArraySort` for render order or diagnostic order when needed.

### Dictionaries

Useful APIs:

- `xrtDictCreate`
- `xrtDictDestroy`
- `xrtDictSet`
- `xrtDictSetPtr`
- `xrtDictGet`
- `xrtDictGetPtr`
- `xrtDictRemove`
- `xrtDictExists`
- `xrtDictCount`
- `xrtDictWalk`
- `xrtDictIterBegin`
- `xrtDictIterNext`
- `xrtDictIterEnd`

Workflow usage:

- node id to node index/pointer.
- edge id to edge index/pointer.
- node type id to registered type.
- variable id to variable metadata.
- command id or quick-fix id lookup.
- spatial bucket map if sparse buckets are used.

Guidelines:

- Use dictionaries for id lookups instead of repeated linear scans.
- Store ids as stable UTF-8 byte keys.
- For pointer values, use `xrtDictSetPtr` / `xrtDictGetPtr` when it fits.

### Lists

Useful APIs:

- `xrtListCreate`
- `xrtListDestroy`
- `xrtListSet`
- `xrtListGet`
- `xrtListRemove`
- `xrtListExists`
- `xrtListCount`
- `xrtListWalk`

Workflow usage:

- sparse integer keyed maps.
- optional z-order or command sequence maps if stable integer keys matter.

Guidelines:

- Prefer `xrtArray` for dense sequences.
- Prefer `xrtList` only when sparse integer keys are a real requirement.

### Stack

Useful APIs:

- `xrtStackCreate`
- `xrtStackPush`
- `xrtStackPushData`
- `xrtStackPushPtr`
- `xrtStackPop`
- `xrtStackPopPtr`
- `xrtStackTop`
- `xrtStackGetPos`

Workflow usage:

- DFS cycle detection.
- graph traversal.
- nested validation path stack.
- undo/redo helper if a stack is simpler than command arrays.

## Buffer

Useful APIs:

- `xrtBufferCreate`
- `xrtBufferDestroy`
- `xrtBufferInit`
- `xrtBufferUnit`
- `xrtBufferMalloc`
- `xrtBufferInsert`
- `xrtBufferAppend`

Workflow usage:

- building export strings when not using XSON stringify directly.
- assembling diagnostic text.
- building generated labels or copied subgraph text.

Guidelines:

- Prefer XSON stringify for full workflow persistence.
- Use buffer for incremental generated text.

## Typed Values

XRT Value (`xvalue`) is the best base for workflow config and XSON data.

Creation APIs:

- `xvoCreateNull`
- `xvoCreateBool`
- `xvoCreateInt`
- `xvoCreateFloat`
- `xvoCreateText`
- `xvoCreateTime`
- `xvoCreateArray`
- `xvoCreateList`
- `xvoCreateColl`
- `xvoCreateTable`
- `xvoCreateCustom`

Reference APIs:

- `xvoAddRef`
- `xvoUnref`
- `xvoCopy`
- `xvoDeepCopy`

Read APIs:

- `xvoGetBool`
- `xvoGetInt`
- `xvoGetFloat`
- `xvoGetText`
- `xvoGetTime`
- `xvoGetArray`
- `xvoGetList`
- `xvoGetTable`
- `xvoType`
- `xvoTypeName`
- `xvoIsNull`
- `xvoIsNumber`
- `xvoIsContainer`

Array APIs:

- `xvoArrayGetValue`
- `xvoArrayAppendValue`
- `xvoArraySetValue`
- `xvoArrayRemove`
- `xvoArrayItemCount`
- `xvoArrayClear`

Table APIs:

- `xvoTableGetValue`
- `xvoTableSetValue`
- `xvoTableExists`
- `xvoTableRemove`
- `xvoTableItemCount`
- `xvoTableClear`

Workflow usage:

- node config storage.
- config schema storage when schema-only node types are loaded from XSON.
- workflow variables.
- runtime result preview metadata.
- imported/exported workflow documents.
- expression and mapping builder structured values.

Guidelines:

- Store node config as `xvalue` table.
- Use `xvoDeepCopy` when duplicating nodes or paste subgraphs.
- Use table paths for config diagnostics.
- Convert to typed workflow model before rendering or validation where repeated
  access would otherwise become fragile.

## JSON And XSON

JSON APIs:

- `xrtParseJSON`
- `xrtParseJSON_File`
- `xrtStringifyJSON`
- `xrtStringifyJSON_File`
- `xrtJsonParseSAX`
- `xrtJsonPrintStart`
- `xrtJsonPrintValue`
- `xrtJsonPrintFinish`

XSON APIs:

- `xrtParseXSON`
- `xrtParseXSONEx`
- `xrtParseXSON_File`
- `xrtParseXSON_FileEx`
- `xrtStringifyXSON`
- `xrtStringifyXSON_File`

XSON flags:

- `XSON_F_IGNORE_UNSUPPORTED_ENCODE`
- `XSON_F_IGNORE_UNSUPPORTED_DECODE`

Workflow usage:

- primary workflow persistence format.
- node type definition persistence.
- config schema persistence.
- round-trip tests.
- optional JSON interoperability for host tooling.

Guidelines:

- Use XRT XSON directly for workflow save/load.
- Do not write a custom parser for workflow XSON.
- Keep a workflow-local adapter around XRT Value so public code does not
  depend on XRT internals beyond `xvalue`.
- Preserve invalid references as diagnostics where possible instead of
  deleting malformed nodes or edges during load.

## File And Directory

Useful APIs:

- `xrtFileReadAll`
- `xrtFileWriteAll`
- `xrtFileGetAll`
- `xrtFilePutAll`
- `xrtPathExists`
- `xrtFileExists`
- `xrtFileGetSize`
- `xrtFileCopy`
- `xrtFileMove`
- `xrtFileDelete`
- `xrtDirCreate`
- `xrtDirCreateAll`

Async file APIs exist through `xfuture`, for example:

- `xrtFileReadAllAsync`
- `xrtFileWriteAllAsync`
- `xrtFileGetAllAsync`
- `xrtFilePutAllAsync`

Workflow usage:

- load workflow files.
- save workflow files.
- load node type libraries.
- import/export example workflows.

Guidelines:

- Start with synchronous file save/load for editor examples.
- Use async file helpers only when host UI needs non-blocking large file work.

## Time And Profiling

Useful APIs:

- `xrtTimer`
- `xrtTime`
- `xrtTimeStr`
- `xrtTimeToStr`
- `xrtStrToTime`
- `xrtTimeFormat`
- `xrtTimeParse`
- `xrtTimezoneOffset`

Workflow usage:

- profiling validation and layout passes.
- mock run-state playback.
- diagnostic timestamps.
- node runtime duration display.
- saved metadata such as last modified or last run time.

Guidelines:

- Use `xrtTimer` for durations.
- Use `xtime`/time helpers for persisted timestamps if needed.

## Logging

Useful APIs and macros:

- `xlogCreate`
- `xlogDestroy`
- `xlogDefault`
- `xlogSetDefault`
- `xlogSetLevel`
- `xlogAddConsole`
- `xlogAddFile`
- `xlogAddRollingFile`
- `xlogWrite`
- `xloggerTrace`
- `xloggerDebug`
- `xloggerInfo`
- `xloggerWarn`
- `xloggerError`
- `xloggerFatal`

Workflow usage:

- developer diagnostics during load/save/validation.
- optional host-facing editor logs.
- performance measurement logs.

Guidelines:

- User-visible workflow problems should be diagnostics, not only log lines.
- Logs are for development and host integration.

## Random And IDs

Useful APIs:

- `xrtRandSeed`
- `xrtRand32`
- `xrtRand64`
- `xrtRandRange`
- `xrtRandStr`
- `xrtRandomBytes`
- `xrtMakeXID`
- `xrtMakeXIDS`
- `xrtEncodeXID`
- `xrtDecodeXID`
- `xrtCompXID`

Workflow usage:

- generated node ids.
- generated edge ids.
- generated workflow ids.
- pasted subgraph id remapping.

Guidelines:

- Prefer deterministic `node_N` / `edge_N` ids inside tests.
- Prefer XID or random string helpers for host-created workflow ids.
- Keep ids stable after save/load.

## Threads, Futures, And Async

Useful thread APIs:

- `xrtThreadCreate`
- `xrtThreadDestroy`
- `xrtThreadWait`
- `xrtThreadWaitTimeout`
- `xrtThreadStop`
- `xrtThreadShouldStop`
- `xrtThreadGetCurrentId`
- `xrtThreadYield`
- `xrtMutexCreate`
- `xrtMutexLock`
- `xrtMutexUnlock`

Useful future/task APIs:

- `xFutureCreate`
- `xFutureAddRef`
- `xFutureRelease`
- `xFutureState`
- `xFutureWait`
- `xFutureWaitTimeout`
- `xFutureRequestCancel`
- `xTaskRunThread`
- `xFutureThenInline`
- `xFutureWhenAny`
- `xFutureWhenAll`
- `xTaskGroupRunThread`
- `xTaskGroupJoinFuture`

Workflow usage:

- optional background validation for huge graphs.
- optional async load/save.
- optional mock execution playback.
- host execution bridge, if the host chooses XRT futures.

Guidelines:

- V1 editor operations should remain synchronous and deterministic.
- Async execution is host-owned. XUI may display state updates.
- Do not make the graph model thread-safe by default. Use clear ownership and
  explicit host synchronization if cross-thread updates are introduced.

## Regex And Expression Parsing

Useful APIs:

- `xrtRegexCreate`
- `xrtRegexDestroy`
- `xrtRegexIsMatch`
- `xrtRegexFind`
- `xrtRegexCaptures`
- `xrtRegexSetCreate`
- `xrtRegexSetMatches`

Workflow usage:

- extracting `${node.port}` and `${vars.name}` references from expression
  fields.
- validating simple id syntax.
- command/search filters where wildcard string helpers are not enough.

Guidelines:

- Keep V1 expression semantics simple.
- Prefer a small explicit scanner for `${...}` references if it is clearer than
  regex.
- Use XRT regex for richer validation only when the regex module is enabled.

## Template Engine

Useful APIs:

- `xteCreateEngine`
- `xteDestroyEngine`
- `xteRegisterFunction`
- `xteParseEx`
- `xteRenderEx`
- `xteResolvePath`

Workflow usage:

- possible future prompt/template node.
- possible future expression or mapping preview.

Guidelines:

- Do not make V1 Workflow depend on the template engine.
- Keep it as an optional future integration point for prompt nodes.

## Workflow Implementation Mapping

| Workflow Need | Prefer XRT Facility | Notes |
| --- | --- | --- |
| node/edge id lookup | `xrtDict` | id to pointer/index maps |
| ordered graph lists | `xrtArray` | nodes, edges, ports, diagnostics |
| config values | `xvalue` | tables/arrays/scalars |
| config copy | `xvoDeepCopy` | duplicate/paste node configs |
| XSON load/save | `xrtParseXSON`, `xrtStringifyXSON` | primary persistence |
| JSON interoperability | `xrtParseJSON`, `xrtStringifyJSON` | optional |
| diagnostics text | `xrtStrFormat`, `xrtBuffer` | short strings or builders |
| workflow files | `xrtFileReadAll`, `xrtFileWriteAll` | sync V1 |
| file paths | `xrtPathJoin`, `xrtPathIsAbs` | examples and tooling |
| duration/profiling | `xrtTimer` | validation/layout/render timings |
| timestamps | `xtime`, `xrtTime*` | metadata |
| logs | `xlog*`, `xlogger*` | developer diagnostics |
| generated ids | `xrtMakeXIDS`, `xrtRandStr` | host or pasted ids |
| DFS/cycle checks | `xrtStack`, `xrtArray` | graph validation |
| sparse maps | `xrtList` | only if integer keyed sparse data is needed |
| async load/save | `xFuture*`, async file APIs | later phase |

## Avoid Rebuilding

Do not add workflow-local versions of:

- generic dynamic array.
- generic string builder.
- generic dictionary/hash map.
- generic typed variant/value tree.
- XSON parser/printer.
- JSON parser/printer.
- file read/write helpers.
- logger.
- random id generator.
- generic thread/future wrapper.

Allowed workflow-local structures:

- graph item structs.
- port and edge route structs.
- graph command structs.
- spatial bucket records built on XRT arrays/dicts.
- validation diagnostic structs.
- typed adapters over `xvalue` for faster repeated access.

