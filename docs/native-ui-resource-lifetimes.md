# Native XUI resource lifetimes

The safe native resource protocol is real shared ownership, not a language
wrapper. Proxy capabilities, Font and Surface own native reference counts;
resources retain their originating capability and the backend owner. Native
methods still access the same payload directly. Raw by-value proxies remain an
explicit caller-managed C path. A context installed through SetProxyOwned only
admits resources from that exact capability.

| Native storage | Actual owner and admission | Trace and close rule |
| --- | --- | --- |
| Context copied proxy table | one owned capability ref, copied function table is a borrowed alias | trace one capability edge; release after widgets/caches/fonts |
| Context default font + theme font | two pointer aliases of one logical slot | retain/trace/release once, not twice |
| Context named font registry | one actual ref per registry entry | trace each entry, even when multiple names refer to one font |
| Fixed Widget TypeData font | one actual ref per declared fixed type slot | static offset metadata; retain final value after complete Init; release after type destructor |
| Widget children | Context owns native widget lifetime | borrowed internal parts cannot independently destroy/detach/reparent |
| Retained text shape paint | one actual Font ref in each owning paint | trace owning shape once; native checked paint cleanup consumes that ref |
| Input/CodeEdit/TextEdit cached shapes | unique owning records within widget/cache | unfold actual owning shape records from the unique Context owner |
| TextLayout | unique Create/Destroy object; borrows Context | internal owner unfolds real Font/shape edges; independent language view expires with Context |
| DocumentRenderer | unique Create/Release object; borrows Context; retains immutable snapshot/input | DocumentView/Editor own their renderer/projection separately; independent view expires with Context |
| Document renderer run/seed shapes | actual owning shape records only | shared_seed and source_run aliases must not duplicate edges |
| Document sized-font cache source | persistent key must retain actual Font identity | trace source ref to prevent address-reuse/ABA |
| Document sized-font cache output | owns backend-created Font | trace/release the created output independently of source |
| Generic Surface resource / Icon / render cache | real Surface owner when admitted to managed native storage | retain before publishing, release after last real consumer, trace each actual owner |
| Font/Surface returned borrowed from a Context | ordinary borrowed view of a shared resource | independent retention is explicit and holds a true native ref |

TextLayout and DocumentRenderer currently have no native AddRef. A renderer's
Release function is unique destruction, not a reference-count operation. Adding
an RC node to every uniquely owned layout would add bookkeeping without solving
their existing raw Context dependency. Their safe language presentation therefore
uses the Context owner period, checked invalidation and real native destruction;
Context closure forbids subsequent layout/registry/DPI access. Font and Surface
instead are intentionally shareable and retain their backend after Context close.

Construction, configuration and cleanup remain native transactions. Failed new
owner admission must not change the old slot. Field publication is under the
existing ownership mutation gate; arbitrary backend/user cleanup happens after
publication, outside that gate, with the actual Context call scope covering the
physical callback tail. Cleanup reports status separately from semantic owner
consumption, including failure with no allocated diagnostic. Historical raw C
objects do not become managed merely because their pointers share a typedef.

This table records the chosen ownership model. Each affected family requires its
own source mapping and matched native/language tests before being reported as
implemented; it is not a claim that every cache and surface path is completed.
