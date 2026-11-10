# Sound reference boundaries

`xgeSoundAddRef` returns a positive acquired reference count, or zero without
changing the record for NULL, empty/dead counts and `INT_MAX`. It never revives a
zero/negative count or performs signed overflow. The previous implementation
blindly incremented every non-NULL record; the generated CPU-only boundary
fixture reproduces those failures on both Windows and Linux release builds.

The correction matches the existing Font reference contract and changes no C
signature, record layout or positive-count release behavior. It adds count
comparisons, without allocating, locking, copying, changing the ambient error or
calling a forwarding function. A 1,000-cycle AddRef/Free check with allocation
forbidden returns to the original count/live heap and preserves the caller's
diagnostic. This is a counter boundary test; it does not initialize an engine,
sound backend or audio device, and does not qualify playback lifetime.

The matched host/static O0/O2 fixtures run together with the valid PCM OOM/span
regressions. The public SDK is unchanged; strict dependency-checked reuse
rebuilds the production aggregate TU and the static build's generated contract
witness. Other objects come from the original complete r44 build, without
modifying its inputs. Exact source, old failure and positive report pins are in
`docs/validation/audio-references-20261111.json`.

Owning audio engine/group/voice lifetimes, complete language bindings, the
immutable clip cross-thread contract and final matched SDK delivery remain
separate unfinished work. This correction does not turn the legacy caller-owned
Sound/Group records or their borrowed parent links into owning language types.
