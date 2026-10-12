#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""One-shot site hygiene pass (M6). Idempotent; ASCII console output.
- move _preview_*.png out of wwwroot to ../previews
- add res/img/favicon.svg + inject <link rel=icon> into every html page
- remove maximum-scale=1 from viewport meta
- write robots.txt
Console output ASCII only (site files are UTF-8, preserved as-is).
"""
import glob
import os
import re
import shutil

SITE = "D:/GIT/home/host/xge/wwwroot"
PREV = "D:/GIT/home/host/xge/previews"

FAVICON = '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 34 34">
<rect width="34" height="34" fill="#081114"/>
<rect x="2" y="2" width="30" height="30" stroke="#4fd8c2" stroke-width="2" fill="none"/>
<path d="M9 9 L25 25 M25 9 L9 25" stroke="#ffb454" stroke-width="2.6" stroke-linecap="square"/>
<circle cx="17" cy="17" r="4" fill="#081114" stroke="#ff6b5e" stroke-width="2"/>
</svg>
'''

ROBOTS = "User-agent: *\nAllow: /\n"

os.makedirs(PREV, exist_ok=True)
moved = 0
for p in glob.glob(os.path.join(SITE, "_preview_*.png")):
    shutil.move(p, os.path.join(PREV, os.path.basename(p)))
    moved += 1
print("moved previews:", moved)

imgdir = os.path.join(SITE, "res", "img")
os.makedirs(imgdir, exist_ok=True)
with open(os.path.join(imgdir, "favicon.svg"), "w", encoding="utf-8", newline="\n") as f:
    f.write(FAVICON)
with open(os.path.join(SITE, "robots.txt"), "w", encoding="utf-8", newline="\n") as f:
    f.write(ROBOTS)
print("favicon + robots written")

pages = glob.glob(os.path.join(SITE, "*.html"))
pages += glob.glob(os.path.join(SITE, "docs", "*.html"))
pages += glob.glob(os.path.join(SITE, "download", "*.html"))
pages += glob.glob(os.path.join(SITE, "examples", "*.html"))
pages += glob.glob(os.path.join(SITE, "tutorial", "*.html"))

n_view = n_fav = 0
for p in pages:
    with open(p, "r", encoding="utf-8", newline="") as f:
        t = f.read()
    orig = t
    t2 = t.replace(", maximum-scale=1", "")
    if t2 != t:
        n_view += 1
        t = t2
    if "favicon.svg" not in t:
        depth = os.path.relpath(p, SITE).count(os.sep)
        prefix = "../" * depth if depth else ""
        eol = "\r\n" if "\r\n" in t else "\n"
        link = '%s<link rel="icon" type="image/svg+xml" href="%sres/img/favicon.svg">%s' % (
            ("\t" if eol == "\n" else ""), prefix, eol)
        if eol == "\n":
            t = t.replace("</head>", "\t" + link + "\n</head>", 1)
        else:
            t = t.replace("</head>", "\t" + link + "\r\n</head>", 1)
        n_fav += 1
    if t != orig:
        with open(p, "w", encoding="utf-8", newline="") as f:
            f.write(t)
print("pages:", len(pages), "viewport-fixed:", n_view, "favicon-injected:", n_fav)
