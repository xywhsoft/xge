# Native style registration and cached type queries

Window and IconPicker return their registered type without changing style or
paint state. Each builtin publishes its property-initialization readiness only
after all definitions have registered. A failed initialization can retry the
same type; completed registration is not repeated.

Property registration compares effective fields and default-value content before
allocating. Identical definitions preserve the property ID, generation and
borrowed strings. Explicit changed definitions still cascade; float defaults use
bit identity so signed zero and repeated NaN bits retain their declared value.
A changed definition copies borrowed inputs before replacing its owned storage.

Window, IconPicker and MsgBox type initialization use balanced, nestable native
style transactions. MsgBox also owns a transaction for its full construction,
including first-use child-control registration. Every constructor exit closes
that transaction while its lifetime receipt is active, preserving the first
failure and rolling back an unsuccessful new manager.

The property registry retains its original owning list and iteration order.
Two intrusive 256-bucket indexes accelerate ID and UTF-8 name lookup. They add
4096 bytes per 64-bit Context and two pointers per property, without separate
index allocations. Collisions use exact comparison; explicit duplicate IDs keep
the previous newest-first lookup behavior. Unchanged queries add no allocation
or generation change. Explicit global style refresh keeps its existing eager
cascade semantics.

`tests/xui/xui_style_registration_test.c` exercises cached getters, user overrides,
nested initialization, first/reused MsgBox, borrowed defaults, inheritance,
1024 colliding IDs and floating-point bit identity. Run with the matching DLL
and build configuration; the ordinary style/incremental render/lifetime suites
remain required alongside host-level frame measurements.
