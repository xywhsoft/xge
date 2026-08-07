# XGE built-in Emoji pack

`generate_builtin.js` converts the UTF-8 sequences and SVG files listed in
`res/emoji/twemoji/manifest.txt` into `src/xge_builtin_emoji.inc`.

Run from the repository root:

```powershell
node tools/emoji_pack/generate_builtin.js
```

The generated include stores exact SVG bytes. Layout metrics remain separate
from the SVG view box so text measurement never parses vector data.
