/* Static object renderer. The native host owns document content and history. */
(() => {
  'use strict';
  const output = document.getElementById('content');
  let sequence = Promise.resolve();

  const reply = (value) => window.chrome.webview.postMessage(value);
  const settlePaint = () => new Promise((resolve) =>
    requestAnimationFrame(() => requestAnimationFrame(resolve)));
  const color = (value, fallback) =>
    typeof value === 'string' && /^#[0-9a-fA-F]{6}$/.test(value) ? value : fallback;
  const rgb = (value) => [1, 3, 5].map((offset) =>
    parseInt(value.slice(offset, offset + 2), 16));
  const blend = (background, foreground, weight) => {
    const back = rgb(background);
    const front = rgb(foreground);
    return `#${back.map((value, index) =>
      Math.round(value * (1 - weight) + front[index] * weight)
        .toString(16).padStart(2, '0')).join('')}`;
  };

  function loadStaticHtml(source, available, fontSize, foreground, background) {
    const safe = DOMPurify.sanitize(source, {
      USE_PROFILES: { html: true },
      FORBID_TAGS: ['script', 'style', 'iframe', 'object', 'embed',
        'form', 'input', 'button', 'textarea', 'select', 'option',
        'meta', 'link', 'base'],
      FORBID_ATTR: ['srcdoc', 'srcset', 'formaction', 'target'],
      ALLOW_DATA_ATTR: false
    });
    const frame = document.createElement('iframe');
    frame.setAttribute('sandbox', 'allow-same-origin');
    frame.setAttribute('tabindex', '-1');
    frame.setAttribute('aria-hidden', 'true');
    frame.style.width = `${available}px`;
    frame.style.height = '1px';
    output.appendChild(frame);
    const page = `<!doctype html><html><head><meta charset="utf-8">` +
      `<meta http-equiv="Content-Security-Policy" content="` +
      `default-src 'none'; script-src 'none'; style-src 'unsafe-inline'; ` +
      `img-src data:; font-src 'none'; connect-src 'none'; ` +
      `frame-src 'none'; form-action 'none'; object-src 'none'; base-uri 'none'">` +
      `<style>html,body{margin:0;padding:0;overflow:hidden}` +
      `body{background:${background};color:${foreground};` +
      `font: ${fontSize}px/1.4 Arial,sans-serif}</style>` +
      `</head><body data-xui-static="1">${safe}</body></html>`;
    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => reject(new Error('HTML load timed out')), 3000);
      frame.addEventListener('load', () => {
        const body = frame.contentDocument && frame.contentDocument.body;
        if (!body || body.dataset.xuiStatic !== '1') return;
        clearTimeout(timeout);
        resolve(frame);
      });
      frame.srcdoc = page;
    });
  }

  async function render(request) {
    const id = request.id;
    if (!Number.isSafeInteger(id) || id < 0 ||
        typeof request.source !== 'string' || request.source.length > 262144 ||
        (request.type !== 'math' && request.type !== 'diagram' &&
          request.type !== 'html')) {
      reply({ kind: 'error', id, generation: request.generation,
        error: 'invalid request' });
      return;
    }
    output.replaceChildren();
    const fontSize = Math.max(8, Math.min(72, Number(request.fontSize) || 16));
    const available = Math.max(1, Math.min(4096, Number(request.available) || 512));
    output.style.fontSize = `${fontSize}px`;
    output.style.width = request.type === 'html' ? `${available}px` : 'auto';
    output.style.maxWidth = request.type === 'diagram' ?
      `${available}px` : 'none';
    const background = color(request.background, '#ffffff');
    const foreground = color(request.foreground, '#111111');
    const channels = rgb(background);
    const dark = channels[0] * 0.2126 + channels[1] * 0.7152 +
      channels[2] * 0.0722 < 128;
    document.body.style.backgroundColor = background;
    output.style.color = foreground;
    try {
      let baseline = 0;
      if (request.type === 'math') {
        katex.render(request.source, output, {
          displayMode: !!request.display,
          throwOnError: true,
          trust: false,
          strict: 'error',
          output: 'htmlAndMathml'
        });
        const marker = document.createElement('span');
        marker.style.cssText = 'display:inline-block;width:0;height:0;padding:0;margin:0';
        output.appendChild(marker);
        await document.fonts.ready;
        await settlePaint();
        baseline = marker.getBoundingClientRect().top - output.getBoundingClientRect().top;
      } else if (request.type === 'diagram') {
        mermaid.initialize({
          startOnLoad: false,
          securityLevel: 'strict',
          theme: 'base',
          themeVariables: {
            darkMode: dark,
            background,
            primaryColor: blend(background, foreground, dark ? 0.12 : 0.07),
            primaryTextColor: foreground,
            primaryBorderColor: blend(background, foreground, 0.32),
            secondaryColor: blend(background, foreground, dark ? 0.18 : 0.12),
            tertiaryColor: background,
            lineColor: foreground,
            textColor: foreground,
            noteBkgColor: blend(background, foreground, 0.14),
            noteTextColor: foreground
          },
          flowchart: { htmlLabels: false },
          deterministicIds: true,
          deterministicIDSeed: 'xui-document'
        });
        const rendered = await mermaid.render(`xui-object-${id}`, request.source);
        output.innerHTML = rendered.svg;
        await document.fonts.ready;
        await settlePaint();
      } else {
        const frame = await loadStaticHtml(request.source, available,
          fontSize, foreground, background);
        const doc = frame.contentDocument;
        if (!doc || !doc.body || doc.body.dataset.xuiStatic !== '1')
          throw new Error('HTML sandbox is unavailable');
        await doc.fonts.ready;
        await settlePaint();
        const contentHeight = Math.ceil(Math.max(doc.documentElement.scrollHeight,
          doc.body.scrollHeight));
        if (contentHeight < 1 || contentHeight > 4096 ||
            available * contentHeight > 16000000)
          throw new Error('HTML size exceeds limit');
        frame.style.height = `${contentHeight}px`;
        await settlePaint();
        baseline = contentHeight;
      }
      const bounds = output.getBoundingClientRect();
      const width = Math.ceil(bounds.width);
      const height = Math.ceil(bounds.height);
      if (width < 1 || height < 1 || width > 4096 || height > 4096 ||
          width * height > 16000000) throw new Error('rendered size exceeds limit');
      reply({ kind: 'rendered', id, generation: request.generation,
        type: request.type,
        width, height, baseline: Math.max(0, Math.min(height, baseline)) });
    } catch (error) {
      output.replaceChildren();
      reply({ kind: 'error', id, generation: request.generation,
        error: String(error && error.message || error).slice(0, 256) });
    }
  }

  window.chrome.webview.addEventListener('message', (event) => {
    const request = event.data;
    if (!request || request.kind !== 'render') return;
    sequence = sequence.then(() => render(request));
  });
  reply({ kind: 'ready', katex: typeof katex.render === 'function',
    mermaid: typeof mermaid.render === 'function',
    dompurify: typeof DOMPurify.sanitize === 'function' });
})();
