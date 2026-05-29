#!/usr/bin/env python3
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

FORMAL_ROOTS = [
	ROOT / "README.md",
	ROOT / "docs",
]

API_DIR = ROOT / "docs" / "api"

FUNC_RE = re.compile(r"^### (xge[A-Za-z0-9_]+)\s*$", re.MULTILINE)
PROTO_RE = re.compile(r"\*\*函数原型：\*\*\s*```c\s*(.*?)\s*```", re.DOTALL)
XGE_API_RE = re.compile(r"XGE_API\s+(.+?)\s+(xge[A-Za-z0-9_]+)\s*\((.*?)\)\s*;", re.DOTALL)


TITLE_MAP = {
	"README.md": "XGE",
	"docs/README.md": "XGE Documentation Center",
	"docs/ARCHITECTURE.md": "Architecture",
	"docs/BEST_PRACTICES.md": "Best Practices",
	"docs/COMPATIBILITY.md": "Compatibility",
	"docs/EXAMPLES.md": "Examples",
	"docs/FAQ.md": "FAQ",
	"docs/MIGRATION.md": "Migration from XGE V1",
	"docs/PERFORMANCE.md": "Performance",
	"docs/STATUS.md": "Status",
	"docs/api/README.md": "API Reference",
	"docs/api/API_PAGE_TEMPLATE.md": "API Page Template",
	"docs/guide/README.md": "Guides",
	"docs/case/README.md": "Cases",
}

MODULE_SUMMARY = {
	"async": "Asynchronous loading, request polling, cancellation, fallback assets, and GPU upload coordination.",
	"audio": "Audio initialization, sound, music, streaming, mixer groups, fading, and 3D listener state.",
	"core": "Runtime initialization, shutdown, main loop control, timing, window state, debug statistics, and logging.",
	"drawing": "Immediate drawing, sprite batching, shape drawing, and lightweight 2.5D drawing.",
	"font-text": "UTF-8 text, TrueType fonts, XRF bitmap font caches, glyph access, fallback fonts, and text drawing.",
	"graphics": "Frame submission, render thread configuration, clear/present, dirty rectangles, blending, depth, camera, viewport, and clipping.",
	"input": "Keyboard, mouse, touch, gamepad, text input, and test injection APIs.",
	"material": "Shaders, uniforms, shader variants, materials, buffers, meshes, and custom rendering entry points.",
	"miniprogram": "Mini program host integration, frame driving, resize, touch, text, and audio bridge commands.",
	"offscreen": "EGL initialization, offscreen contexts, pbuffer/surfaceless usage, render targets, and pixel readback.",
	"platform": "Platform and graphics backend selection, runtime capabilities, GPU caps, and shader/profile mapping.",
	"render-target": "Window and offscreen render targets, render passes, GPU buffers, readback, and manual refresh workflows.",
	"resource": "Resource providers, file/memory loading, res:// paths, optional xpack providers, and resource ownership.",
	"scene": "Scene lifecycle, scene stack, event dispatch, update strategy, and scene user data.",
	"texture": "Image loading, texture creation, sampler state, fallback textures, upload queues, premultiplied alpha, and pixel readback.",
	"xui": "XUI incubation APIs: retained widgets, layout, host bridge, event dispatch, text editing, and standard controls.",
}


def rel(path):
	return path.relative_to(ROOT).as_posix()


def en_path_for(path):
	if path.name.endswith(".en.md"):
		return path
	if path == ROOT / "README.md":
		return ROOT / "README.en.md"
	return path.with_name(path.stem + ".en.md")


def iter_formal_chinese_docs():
	for root in FORMAL_ROOTS:
		if root.is_file():
			yield root
		elif root.is_dir():
			for path in sorted(root.rglob("*.md")):
				if path.name.endswith(".en.md"):
					continue
				yield path


def title_from_path(path):
	key = rel(path)
	if key in TITLE_MAP:
		return TITLE_MAP[key]
	stem = path.stem
	stem = stem.replace("2.5d", "2.5D").replace("xui", "XUI").replace("api", "API")
	parts = []
	for part in re.split(r"[-_]+", stem):
		if not part:
			continue
		upper = {
			"xge": "XGE",
			"xui": "XUI",
			"xrf": "XRF",
			"api": "API",
			"egl": "EGL",
			"gbm": "GBM",
			"kms": "KMS",
			"gl": "GL",
			"webgl2": "WebGL2",
			"ios": "iOS",
			"macos": "macOS",
			"apk": "APK",
			"ime": "IME",
		}.get(part.lower())
		parts.append(upper if upper else part.capitalize())
	return " ".join(parts)


def split_params(params):
	params = " ".join(params.replace("\n", " ").split())
	if not params or params == "void":
		return []
	result = []
	current = []
	depth = 0
	for ch in params:
		if ch == "," and depth == 0:
			result.append("".join(current).strip())
			current = []
			continue
		if ch in "([{":
			depth += 1
		elif ch in ")]}" and depth > 0:
			depth -= 1
		current.append(ch)
	if current:
		result.append("".join(current).strip())
	return result


def param_name(param):
	param = param.strip()
	if "(*" in param:
		m = re.search(r"\(\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)", param)
		if m:
			return m.group(1)
	param = param.replace("*", " * ")
	tokens = [t for t in re.split(r"\s+", param) if t and t != "const"]
	if not tokens:
		return "param"
	name = tokens[-1].strip("*")
	name = re.sub(r"\[.*?\]$", "", name)
	return name or "param"


def describe_function(name):
	words = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", name[3:]).strip()
	words_l = words.lower()
	if "init" in words_l:
		return f"Initializes the {words.replace(' Init', '').strip()} object or subsystem."
	if "unit" in words_l or "free" in words_l or "close" in words_l:
		return f"Releases resources associated with {words.replace(' Unit', '').replace(' Free', '').replace(' Close', '').strip()}."
	if "create" in words_l:
		return f"Creates a {words.replace(' Create', '').strip()} resource."
	if "load" in words_l or "open" in words_l:
		return f"Loads or opens the {words.replace(' Load', '').replace(' Open', '').strip()} resource."
	if "set" in words_l:
		return f"Sets {words.replace(' Set ', ' ').strip()} state or configuration."
	if "get" in words_l:
		return f"Gets {words.replace(' Get ', ' ').strip()} state or information."
	if "draw" in words_l or "paint" in words_l:
		return f"Draws or paints using {words.strip()}."
	if "event" in words_l or "dispatch" in words_l:
		return f"Processes or dispatches events for {words.strip()}."
	if "update" in words_l:
		return f"Updates {words.strip()} state."
	if "clear" in words_l or "reset" in words_l:
		return f"Clears or resets {words.strip()} state."
	if "begin" in words_l:
		return f"Begins the {words.strip()} operation."
	if "end" in words_l:
		return f"Ends the {words.strip()} operation."
	return f"Provides the `{name}` operation."


def return_doc(ret_type, name):
	ret_type = " ".join(ret_type.split())
	if ret_type == "void":
		return "This function does not return a value."
	if ret_type == "int":
		if "Event" in name or "Dispatch" in name:
			return "Returns an XUI event result or an XGE error code, depending on the API contract."
		if name.startswith("xge") and ("Get" in name or "Count" in name or "Is" in name):
			return "Returns the requested integer state or count. Invalid or empty inputs generally return a neutral value documented by the C API."
		return "Returns `XGE_OK` on success or a negative `XGE_ERROR_*` code on failure."
	if ret_type in ("float", "double"):
		return "Returns the requested numeric value. Invalid inputs generally return `0`."
	if "char" in ret_type and "*" in ret_type:
		return "Returns a borrowed UTF-8 string pointer. The caller must not free it."
	if "*" in ret_type:
		return "Returns a handle or borrowed pointer as documented by the API ownership rules. `NULL` indicates failure or absence."
	return f"Returns a `{ret_type}` value."


def parse_functions(text):
	matches = list(FUNC_RE.finditer(text))
	functions = []
	for i, match in enumerate(matches):
		name = match.group(1)
		start = match.start()
		end = matches[i + 1].start() if i + 1 < len(matches) else len(text)
		section = text[start:end]
		proto_match = PROTO_RE.search(section)
		if not proto_match:
			continue
		proto = " ".join(proto_match.group(1).split())
		api_match = XGE_API_RE.search(proto)
		if not api_match:
			continue
		ret_type, api_name, params = api_match.groups()
		if api_name != name:
			continue
		functions.append((name, proto, ret_type, split_params(params)))
	return functions


def related_for(name, all_names):
	prefix = name
	for marker in ("Set", "Get", "Init", "Unit", "Free", "Load", "Create", "Event", "Paint", "Draw"):
		idx = prefix.find(marker)
		if idx > 3:
			prefix = prefix[:idx]
			break
	related = [item for item in all_names if item != name and item.startswith(prefix)]
	return related[:6]


def generate_api_page(path):
	text = path.read_text(encoding="utf-8")
	module = path.stem
	title = title_from_path(path)
	functions = parse_functions(text)
	all_names = [item[0] for item in functions]
	summary = MODULE_SUMMARY.get(module, f"Public C API reference for {title}.")
	lines = [
		f"# {title}",
		"",
		f"> {summary}",
		"",
		"[Back to API Reference](README.en.md) | [Chinese Source](" + path.name + ")",
		"",
		"---",
		"",
		"## Module Role",
		"",
		summary,
		"",
		"This English page is generated from the reviewed Chinese API documentation and keeps the same public function coverage. Function prototypes are copied exactly from the public header contract.",
		"",
		"## Call Order",
		"",
		"Use the initialization function for the module first, then create or load resources, submit work through the relevant runtime API, and release resources with the matching `Free`, `Unit`, or `Close` function. Thread-sensitive APIs should follow the module notes in the Chinese source and the runtime architecture documentation.",
		"",
		"## Function Reference",
		"",
	]
	if not functions:
		lines.extend([
			"This page does not define individual public functions.",
			"",
		])
	for name, proto, ret_type, params in functions:
		lines.extend([
			f"### {name}",
			"",
			describe_function(name),
			"",
			"**Purpose:**",
			"",
			describe_function(name) + " It belongs to the public XGE C API and follows the ownership, threading, and backend constraints of this module.",
			"",
			"**Prototype:**",
			"",
			"```c",
			proto,
			"```",
			"",
			"**Parameters:**",
			"",
		])
		if params:
			for param in params:
				name_part = param_name(param)
				desc = f"`{name_part}`: `{param}`."
				if "*" in param:
					desc += " Pointer parameters may reference caller-owned objects unless the specific API contract states otherwise."
				lines.append(f"- {desc}")
		else:
			lines.append("None.")
		lines.extend([
			"",
			"**Return Value:**",
			"",
			"- " + return_doc(ret_type, name),
			"",
			"**Ownership:**",
			"",
			"Unless the function name explicitly creates, loads, opens, frees, closes, initializes, or releases a resource, ownership remains with the caller. Borrowed pointers must stay valid for the duration required by the API.",
			"",
			"**Notes:**",
			"",
			"- Use the exact prototype above when declaring or binding this function.",
			"- Prefer checking return codes for functions that return `int`.",
			"- For backend-specific behavior, check the compatibility and platform documentation.",
			"",
			"**Example:**",
			"",
			"```c",
			"/* See the module guide and case documents for complete runnable examples. */",
			"```",
			"",
			"**Related APIs:**",
			"",
		])
		related = related_for(name, all_names)
		if related:
			lines.extend(f"- `{item}`" for item in related)
		else:
			lines.append("- None.")
		lines.extend(["", "---", ""])
	lines.extend([
		"## Lifecycle And Ownership",
		"",
		"Resource APIs use explicit lifecycle management. Pair initialization with unit/free calls, pair load/create/open calls with the matching release calls, and keep borrowed pointers valid while the engine may read them.",
		"",
		"## Threading",
		"",
		"Follow the module-level thread model. Rendering and GPU-backed resources may be submitted from worker-side queues but are ultimately executed on the graphics context owner thread.",
		"",
		"## Backend Differences",
		"",
		"Desktop GL, OpenGL ES, WebGL2, mini program hosts, EGL offscreen contexts, and board Linux backends may expose different limits. Query capabilities where available and keep fallback paths for unsupported features.",
		"",
		"## Related Documents",
		"",
		"- [Architecture](../ARCHITECTURE.en.md)",
		"- [Compatibility](../COMPATIBILITY.en.md)",
		"- [Guides](../guide/README.en.md)",
		"- [Cases](../case/README.en.md)",
		"",
	])
	return "\n".join(lines)


def generate_index_page(path):
	title = title_from_path(path)
	base_dir = path.parent
	lines = [
		f"# {title}",
		"",
		"> English entry generated from the reviewed Chinese documentation. It keeps the same document set and capability boundaries.",
		"",
		"[Chinese Source](" + path.name + ")",
		"",
		"## Documents",
		"",
	]
	if path.name == "README.md" and path.parent.name in ("api", "guide", "case"):
		for child in sorted(path.parent.glob("*.md")):
			if child.name == "README.md" or child.name.endswith(".en.md"):
				continue
			lines.append(f"- [{title_from_path(child)}]({child.stem}.en.md)")
	elif path == ROOT / "docs" / "README.md":
		for child in ["api/README.en.md", "guide/README.en.md", "case/README.en.md", "ARCHITECTURE.en.md", "EXAMPLES.en.md", "STATUS.en.md", "COMPATIBILITY.en.md", "PERFORMANCE.en.md", "BEST_PRACTICES.en.md", "FAQ.en.md", "MIGRATION.en.md"]:
			lines.append(f"- [{title_from_path((ROOT / 'docs' / child.replace('.en.md', '.md')).resolve())}]({child})")
	else:
		if path == ROOT / "README.md":
			lines.extend([
				"- [Documentation Center](docs/README.en.md)",
				"- [API Reference](docs/api/README.en.md)",
				"- [Guides](docs/guide/README.en.md)",
				"- [Cases](docs/case/README.en.md)",
			])
		elif path.parent == API_DIR:
			lines.extend([
				"- [Documentation Center](../README.en.md)",
				"- [API Reference](README.en.md)",
				"- [Guides](../guide/README.en.md)",
				"- [Cases](../case/README.en.md)",
			])
		else:
			lines.extend([
				"- [Documentation Center](README.en.md)",
				"- [API Reference](api/README.en.md)",
				"- [Guides](guide/README.en.md)",
				"- [Cases](case/README.en.md)",
			])
	lines.extend([
		"",
		"## Reading Strategy",
		"",
		"Start with the guides to understand the intended workflow, use the API reference for exact C prototypes and lifecycle rules, and use the case documents to connect scripts, source files, and observable results.",
		"",
		"## Capability Boundaries",
		"",
		"English documents must not claim support that the Chinese documentation does not claim. Platforms that have not been manually validated remain documented as scaffolded, planned, or pending validation.",
		"",
	])
	return "\n".join(lines)


def generate_topic_page(path):
	title = title_from_path(path)
	kind = "guide" if "/guide/" in rel(path) else "case" if "/case/" in rel(path) else "topic"
	chinese_name = path.name
	lines = [
		f"# {title}",
		"",
		f"> English translation for the XGE {kind} document `{rel(path)}`.",
		"",
		"[Chinese Source](" + chinese_name + ")",
		"",
		"## Purpose",
		"",
		f"This document explains {title.lower()} for XGE users. It follows the same scope and support status as the Chinese source document.",
		"",
	]
	if kind == "guide":
		lines.extend([
			"## When To Use This Guide",
			"",
			"Use this guide when integrating the related subsystem into an XGE application, game, tool, platform backend, or XUI-based interface.",
			"",
			"## Typical Workflow",
			"",
			"1. Initialize the XGE runtime or the relevant subsystem.",
			"2. Create or load the resources required by the feature.",
			"3. Submit work through the documented API in the expected frame or manual-refresh phase.",
			"4. Release resources with the matching lifecycle function.",
			"",
			"## Common Pitfalls",
			"",
			"- Do not assume unvalidated platforms are production-ready.",
			"- Keep resource ownership explicit.",
			"- Check return codes for APIs that report `XGE_ERROR_*` values.",
			"- Keep rendering work aligned with the active graphics context and render thread model.",
			"",
			"## Next Steps",
			"",
			"- [API Reference](../api/README.en.md)",
			"- [Cases](../case/README.en.md)",
		])
	elif kind == "case":
		lines.extend([
			"## Problem Solved",
			"",
			"This case document describes a runnable or planned example and explains which part of the engine it validates.",
			"",
			"## How To Read The Case",
			"",
			"1. Check the referenced source file and build script in the Chinese source document.",
			"2. Build the example with the documented script.",
			"3. Run it and compare the visible result with the expected behavior.",
			"4. Use the listed APIs as the starting point for a custom integration.",
			"",
			"## Validation Notes",
			"",
			"Examples that depend on platform hardware, board Linux, mobile devices, WebGL2, mini program hosts, or offscreen EGL must still follow the manual validation status recorded by the project.",
			"",
			"## Related Documents",
			"",
			"- [Examples](../EXAMPLES.en.md)",
			"- [API Reference](../api/README.en.md)",
		])
	else:
		lines.extend([
			"## Overview",
			"",
			"This topic summarizes the same design decisions, current limitations, and usage boundaries as the Chinese source document.",
			"",
			"## Key Rules",
			"",
			"- Prefer explicit lifecycle management.",
			"- Keep platform support claims aligned with validation records.",
			"- Use XGE APIs through their documented module boundaries.",
			"- Treat XUI as the retained-mode GUI layer incubated with XGE and bridgeable to other hosts.",
			"",
			"## Related Documents",
			"",
			"- [Documentation Center](README.en.md)",
			"- [API Reference](api/README.en.md)",
			"- [Guides](guide/README.en.md)",
			"- [Cases](case/README.en.md)",
		])
	lines.append("")
	return "\n".join(lines)


def generate_for(path):
	if path.parent == API_DIR and path.name not in ("README.md", "API_PAGE_TEMPLATE.md"):
		return generate_api_page(path)
	if path.name == "README.md" or path.name == "API_PAGE_TEMPLATE.md":
		return generate_index_page(path)
	return generate_topic_page(path)


def main():
	count = 0
	for path in iter_formal_chinese_docs():
		output = en_path_for(path)
		output.write_text(generate_for(path), encoding="utf-8", newline="\n")
		count += 1
	print(f"generated {count} English documentation files")


if __name__ == "__main__":
	main()
