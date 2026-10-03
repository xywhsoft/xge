# XUI Document browser rendering assets

This folder is a local-only static renderer for Document math and Mermaid nodes.
The page accepts JSON over the generic WebView message API and produces a measured
DOM result for later PNG capture. It does not own Document content or history.
It is not a raw-HTML execution host.
Requests carry an opaque foreground/background palette and generation. The page
applies the colors to KaTeX and Mermaid; the native provider discards old
generation results when the palette changes.

- KaTeX 0.18.7, MIT: `katex.min.js`, `katex.min.css`, `fonts/`, `LICENSE.katex`.
  npm tarball: `https://registry.npmjs.org/katex/-/katex-0.18.7.tgz`;
  SHA-512 (base64): `h+UCwkZ+4Jz8WQ7MLGfj7UVFrRCizGb912fwF4luGdYsC5paYG1vx+jy+KRcC/XkpjGva/P7nAWuxNnPzRvzHw==`.
- Mermaid 10.9.3, MIT: `mermaid.min.js`, `LICENSE.mermaid`.
  npm tarball: `https://registry.npmjs.org/mermaid/-/mermaid-10.9.3.tgz`;
  SHA-512 (base64): `V80X1isSEvAewIL3xhmz/rVmc27CVljcsbWxkxlWJWY/1kQa4XOABqpDl2qQLGKzpKm6WbTfUEKImBlUfFYArw==`.

The tarballs were checked against their npm `dist.integrity` values before
extracting the above files. The WebView maps this directory to a dedicated
virtual HTTPS host; no CDN or network request is needed for rendering.
