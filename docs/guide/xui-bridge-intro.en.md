# XUI Bridge Intro

> English translation for the XGE guide document `docs/guide/xui-bridge-intro.md`.

[Chinese Source](xui-bridge-intro.md)

> This guide describes the first bridge. XUI foundation design should follow `docs/xui/widget.md` and `docs/xui/scrollview.md`; splitting into an independent repository must wait until Widget clip, Z order, events, focus, scrolling, IME, and box model are mature.

## Purpose

This document explains xui bridge intro for XGE users. It follows the same scope and support status as the Chinese source document.

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
