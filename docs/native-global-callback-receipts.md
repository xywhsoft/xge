# Global callback owner outcomes

The shared Error/Quit/Frame/Drag publisher kernel treats ownership and checked
success as two independent facts. Hold returning true acquires one reference
even when it also sets a diagnostic. Such a Hold rejects publication/dispatch
and must roll back exactly that reference. A false Hold never authorizes Drop.
Drop consumes its acquired reference on every return; a true result with a
fresh diagnostic is still a checked failure.

Hold and Drop isolate the older TLS diagnostic. Snapshot cleanup captures the
callback's error before calling arbitrary owner cleanup. Failed status is the
explicit first-failure bit, so a later diagnostic cannot replace an earlier
failure which had no error object. Node-pin release, rollback, shutdown refusal
and normal snapshot exit use the same rule. Owner calls remain outside publisher
locks, registration pins protect reentrant slot replacement, and callback activity
remains held until the native tail and owner cleanup have returned.

The focused CPU gate includes the actual production header and links one exact
XRT host. It covers Hold's four result/diagnostic combinations, OOM, warm
allocation-free snapshots, reentrant unpublication, shutdown refusal, a cleanup
which clears the business error, and NULL first-failure precedence. Its new
fixture rejects the old header at O0/O2 on both platforms. It does not load a
full engine or qualify the actual OS, GUI, language callback or shipped SDK.
Those full-engine public dispatch paths remain a separate matching-build gate.

The fix adds diagnostic isolation/checks. It does not allocate on the successful
snapshot path or add a second owner node. It is not a zero-total-cost claim.
