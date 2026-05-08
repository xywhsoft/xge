# XUI Render Intro

> English translation for the XGE guide document `docs/guide/xui-render-intro.md`.

[Chinese Source](xui-render-intro.md)

> This guide describes the first render/host model. In Widget V2, clip is a PaintContext foundation feature instead of scattered per-control host calls; nested clips intersect with their parent clip, ordinary Control text and image drawing are automatically clipped to the content rect, and text, borders, children, scroll viewports, and overlay layers must obey the active clip stack. Sibling paint order uses `layer > z > treeOrder`, matching hit testing and point-event target selection. `hitTestVisible` removes a widget subtree from hit testing, while `inputTransparent` lets the widget itself pass input through without disabling child hits.

## Purpose

This document explains xui render intro for XGE users. It follows the same scope and support status as the Chinese source document.

## When To Use This Guide

Use this guide when integrating the related subsystem into an XGE application, game, tool, platform backend, or XUI-based interface.

## Typical Workflow

1. Initialize the XGE runtime or the relevant subsystem.
2. Create or load the resources required by the feature.
3. Submit work through the documented API in the expected frame or manual-refresh phase.
4. Release resources with the matching lifecycle function.

## Common Pitfalls

- Do not assume unvalidated platforms are production-ready.
- Keep resource ownership explicit.
- Check return codes for APIs that report `XGE_ERROR_*` values.
- Keep rendering work aligned with the active graphics context and render thread model.

## Next Steps

- [API Reference](../api/README.en.md)
- [Cases](../case/README.en.md)
