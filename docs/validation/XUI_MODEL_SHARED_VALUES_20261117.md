# Explicit model value sharing

The existing value APIs keep their native DeepClone behavior. Five explicit
advanced APIs add reference identity sharing:

| API | Ownership |
| --- | --- |
| `xuiFlowGraphSetNodeConfigShared` | Model retains one reference to the input |
| `xuiFlowGraphGetNodeConfigShared` | Caller receives one owning reference to the stored value |
| `xuiFlowGraphCommandSetNodeConfigShared` | Initial resident configuration shares the input; command history still DeepClones both old and new values |
| `xuiWorkflowRegisterNodeTypeShared` | Copies descriptor metadata/ports/strings, retains the configuration schema |
| `xuiWorkflowAddVariableShared` | Copies descriptor metadata/strings, retains the default value |

These APIs expose mutation through aliases deliberately. They do not claim
independent snapshots. This also provides the native foundation for a language
boundary that first creates its own independent snapshot: the native store can
retain that result without immediately copying the entire data graph again.
No language conversion is implemented by these native functions.

The getters' returned value reference survives destruction of the model. It
does not itself keep the Graph alive. NULL configuration input clears the
configuration. NULL output for an absent configuration remains valid.
Set retains the incoming reference before publishing and releasing the previous
one, including repeated assignment of the same value. Workflow metadata is
still copied; only its xvalue schema/default ownership mode changes.

Shared commands preserve the original undo/redo snapshot contract. Mutating
the initial input can change the current configuration, but does not mutate
the recorded undo/redo payloads. Redo creates its usual independent resident
copy. History construction still has its original Clone and allocation costs.

All five APIs use the existing checked operation and creator-thread protocol.
Busy is tested before other arguments or outputs. Table/configuration Drop
retains the already-qualified same-Workflow/associated-Graph exclusion rules.
There are no new model fields or global policy switches.

The new test checks exact identities, Clone isolation, retained lifetimes,
command history isolation, failed command rollback, raw/shared-owned models,
and custom handles whose Clone must never run in Shared setters/getters.
Final custom Drop probes all integer model APIs for Busy. Over 1000 warm
Shared updates/reads, XRT heap allocation/reallocation/free counts are unchanged
even with allocation failure set at zero. Reference counting and the existing
operation protocol still cost CPU work.

`tools/test/check_model_shared_values.py` keeps the earlier schema/default,
Graph, ownership, XSON, widget and strict-header regressions. Its baseline test
excludes only the new APIs absent from the preceding native implementation.
The original 94 Busy probes remain active in that baseline. Current tests cover
all 99 integer model APIs, including the five additions.

The sibling xlang tool `tools/check_xui_model_shared_sdk.py` exercises the real
new hosted/static SDKs with O0/O2 C clients and one matched compiler-owned XRT;
it does not substitute production units. Public language snapshot declarations,
safe model promotion and final runtime delivery remain separate work. The small
JSON receipt records exact external build and acceptance evidence.
