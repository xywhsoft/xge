# Windows IME Contract v1

Status: merged into the current XGE/XUI mainline.

This document is the compatibility contract between the Windows platform
backend, XGE ordered input, XUI input dispatch, and editable XUI controls. It
is versioned because changes to ownership or commit semantics can otherwise
reintroduce duplicate input and application-specific key forwarding.

## Platform boundary

The TSF/IMM implementation is compiled only for `_WIN32` or `_WIN64`. Other
platforms keep their existing text-input backend and may support only
`XGE_IME_MODE_NATIVE`.

Windows text input has one native owner at a time. An application must not
infer ownership from a list of Backspace, Enter, Space, arrow, or other keys.
In TSF modes, `ITfKeystrokeMgr::TestKeyDown/KeyDown` and
`TestKeyUp/KeyUp` are the authority. A key accepted by TSF is not published to
XUI, global hotkeys, or the XGE application fallback.

## Four implementation stages

### Stage 1: native ownership and ordered transport

- Platform events are normalized into one growable, same-thread FIFO.
- Every event receives a monotonically increasing sequence and a timestamp.
- Keyboard, text, IME, pointer, and touch records keep native arrival order.
- Short UTF-8 payloads use inline storage. Heap allocation is used only when a
  payload exceeds the inline capacity.
- There is no worker thread, mutex, timer, or extra-frame delay in this path.
- Queue allocation failure increments the dropped-event counter and does not
  corrupt existing events.

For a key not owned by TSF, XUI dispatch order is:

1. focused control target;
2. global hotkey table;
3. focused control ancestor bubble;
4. XUI built-in defaults such as focus traversal and default actions;
5. optional XGE application fallback after the XUI pump.

`XUI_EVENT_DISPATCH_STOP` is the consume signal. Error and consume are
different concepts; consuming input must not terminate the render loop.

### Stage 2: XUI marked text

`xui_ime_composition_t` represents marked text independently from document
storage:

- `bActive` distinguishes an empty active composition from composition end.
- Text, cursor, and selection offsets are UTF-8 byte offsets.
- START/UPDATE never modify the real document.
- The displayed document is a temporary view: prefix + marked text + suffix.
- COMMIT inserts the result exactly once through the control's normal edit
  path and creates one undo record.
- END, cancellation, blur, and destruction remove only transient marked state.
- Input, TextEdit, and CodeEdit implement the same semantics.

### Stage 3: TSF text store

The focused XUI text control exposes a bounded UTF-8 surrounding-text window
and selection through `xge_ime_text_client_t`. The window is approximately 64 KiB,
is aligned to UTF-8 boundaries, and includes its document offset, full document
size, and revision. The native backend copies the window during the callback;
caller-owned pointers do not escape the callback. CodeEdit reads the window
through its document range API and never materializes the full document.

The Windows backend converts the snapshot to UTF-16 ACP and exposes it through
`ITextStoreACP`, including selection, text access, lock/sink behavior, and
screen geometry. TSF edits are converted back to UTF-8 marked-text events.
Only the active composition range is published to XUI, while surrounding text
remains available to the text service.

### Stage 4: presentation and candidate UI

| Mode | Marked text | Candidate list | Ownership |
| --- | --- | --- | --- |
| Native | Windows | Windows | DefWindowProc/Sokol; committed text enters as ordered TEXT |
| Composition | XUI | Windows | TSF |
| Full | XUI | XUI | TSF UI-less candidate path |

Full mode hides a native candidate element only after it can read that element,
control selection through `ITfCandidateListUIElementBehavior`, and XUI has
reported a ready candidate presenter. Unsupported or non-interactive TIP UI
remains visible as a native fallback. The XUI popup displays at most ten rows
and uses the TSF candidate selection/finalization APIs.

When TSF initialization is unavailable, Composition and Full fall back to IMM
marked-text events with the native candidate window. Native mode leaves the
complete IME lifecycle to Windows; Sokol converts the committed `WM_CHAR` into
an ordinary ordered text event.

## Lifecycle

- `xgeImeSetMode` may be called after `xgeInit` and before the native window is
  created. The desired mode is applied when the Win32 backend installs.
- Focusing a text control binds its snapshot provider and caret geometry.
- Focus loss cancels transient composition state.
- Destroying an XUI context detaches its IME client before the context memory
  becomes invalid.
- XUI candidate/composition rectangles are transformed from context-local
  coordinates through the active input viewport into framebuffer coordinates;
  the Win32 backend then converts them to client and screen coordinates using
  current DPI.

## Performance contract

The ordered queue adds one copy and one FIFO insertion per native event. It
does not add a scheduling turn. Ordinary key and short composition records do
not allocate individually after initial queue capacity is available. Document
snapshot conversion occurs only for the focused text client when TSF requests
or refreshes state, and its cost is bounded by the surrounding-text window.

## Prohibited shortcuts

- No application-maintained editing-key forwarding table.
- No `bComposing` key blacklist.
- No duplicate dispatch through both polling getters and ordered input.
- No direct document mutation for preedit updates.
- No hiding a native candidate UI unless XGE can render and control it.
- No render-loop termination for a consumed event or empty layout geometry.

## Verification matrix

Automated tests cover FIFO growth/order, TSF key ownership, text-store
composition, candidate UI extraction, Native mode isolation, marked-text
rendering, selection replacement, one-step undo, input routing, and empty
geometry rendering.

Manual release validation should cover Microsoft Pinyin and at least one
third-party TIP at 100%, 125%, 150%, and 200% DPI:

- composition at the beginning, middle, and end of text;
- replacement of a selection;
- held Backspace during composition and outside composition;
- Enter, Escape, Space, Tab, arrows, Home/End, and global hotkeys;
- focus changes between Input, TextEdit, and CodeEdit;
- native, composition, and full presentation modes;
- candidate paging, selection, commit, cancellation, and unsupported-TIP
  fallback;
- dock/split resizing while the candidate or composition is visible.
