"""Verify the vendored source against the pinned, extracted upstream archive.

Only relative project includes and the two reviewed fixes are permitted.
The original archive is deliberately not tracked; see the vendor README.
"""
import os
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
SOURCE = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "build/document/deps/SheenBidi-3.0.0"
TARGET = ROOT / "lib/sheenbidi"
INCLUDE = re.compile(r"#include <(SheenBidi|API|Core|Data|Script|Text|UBA)/([^>]+)>")
PREFIX = "/* XGE portability: project includes use relative paths. */\n"

def project_includes(text, relative):
    def replace(match):
        destination = (TARGET / "Headers/SheenBidi" if match[1] == "SheenBidi"
                       else TARGET / "Source" / match[1]) / match[2]
        path = os.path.relpath(destination, (TARGET / relative).parent).replace("\\", "/")
        return f'#include "{path}"'
    result = INCLUDE.sub(replace, text)
    return PREFIX + result if result != text else result

def expected(text, relative):
    result = project_includes(text, relative)
    if relative.as_posix() == "Source/API/SBParagraph.c":
        before = "    if (paragraph) {\n        paragraph->fixedLevels = pointers[LEVELS];"
        after = """    if (paragraph) {
        /* XGE: failure during ProcessParagraph releases this object before
         * CreateParagraph assigns its retained algorithm. */
        paragraph->_algorithm = NULL;
        paragraph->fixedLevels = pointers[LEVELS];"""
        assert result.count(before) == 1
        result = result.replace(before, after)
    if relative.as_posix() == "Source/API/SBLine.c":
        before = "                context->runCount += 1;\n            }\n            break;"
        after = """                context->runCount += 1;
            } else {
                /* XGE: a non-trailing isolate/space terminates the pending
                 * BN chain. UTF-8 continuation bytes are also encoded as BN;
                 * carrying this length across the character can reset only
                 * part of it at an earlier S/B and split a scalar in L1. */
                length = 0;
            }
            break;"""
        assert result.count(before) == 1
        result = result.replace(before, after)
    return result

files = [SOURCE / "LICENSE"]
for folder in ("Headers", "Source"):
    files.extend(sorted((SOURCE / folder).rglob("*")))
count = 0
for original in files:
    if not original.is_file():
        continue
    relative = original.relative_to(SOURCE)
    actual = (TARGET / relative).read_text(encoding="utf-8")
    assert actual == expected(original.read_text(encoding="utf-8"), relative), str(relative)
    count += 1
actual_files = {f.relative_to(TARGET).as_posix() for folder in ("Headers", "Source")
                for f in (TARGET / folder).rglob("*") if f.is_file()}
original_files = {f.relative_to(SOURCE).as_posix() for folder in ("Headers", "Source")
                  for f in (SOURCE / folder).rglob("*") if f.is_file()}
assert actual_files == original_files
print(f"SheenBidi {count} files: pinned upstream + relative includes + paragraph OOM and UTF-8 L1 fixes verified")
