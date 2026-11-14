# Stop pending events after an error observer closes Context

An error observer can close its Context while `xuiDispatchPendingEvents` reports
an invalid event. The pending pump used to check logical close only before the
error observer. It then polled and dispatched the following queued command.
That contradicts the existing rule that logical close stops business dispatch.

The native pump now checks `xuiInternalContextDestroyPending` immediately after
error reporting and returns success before polling another event. The outer
native operation continues to protect storage until its final teardown.

The standalone CPU test uses the original public raw observer API: it queues an
interior-NUL IME event followed by a command, and closes Context in its observer.
The unchanged SDK delivers the command once (distinct exit76); the patched SDK
delivers none. This has no language closure, window or network dependency.

`tools/test/check_error_close_pump.py` tests explicit matched SDKs at O0/O2.
The baseline covers hosted/static Windows/Linux; minimal fixed static SDKs reuse
the same prior objects and compile only the production input unit with this one
close check. The larger callback-ownership SDK separately qualifies the same
fix in hosted and static modes. Runtime archives and detailed reports remain
external; the small index is `docs/validation/error-observer-close-20261114.json`.
