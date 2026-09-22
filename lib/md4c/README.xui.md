# MD4C in XUI Document

Source: https://github.com/mity/md4c

Pinned revision: `b3c6223903c1df483cef926ba347e531248f0b92`.
The upstream files are unmodified. See `LICENSE.md` for the MIT license.

`src/xui_document_md4c.c` compiles the parser with allocation functions routed
to Document's allocator. It restores thread-local state after each parse.
The Document allocation-failure tests include parser allocation failures.
Do not also compile `md4c.c` directly into the XUI source target.

`entity.c` supplies the named-entity table. `md4c-html.c` is retained as an
upstream reference and is not part of the XUI target; Document exports HTML
from its shared semantic tree. Parsing math or Mermaid source does not itself
provide mathematical typesetting or diagram rendering.
