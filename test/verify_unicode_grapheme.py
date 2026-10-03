"""Compare every C property result to an independent official-UCD expansion."""
from pathlib import Path
from hashlib import sha256
import sys

root = Path(__file__).resolve().parent.parent
directory = Path(sys.argv[2]) if len(sys.argv) > 2 else root / "build/document/deps/unicode-17"
channels = [bytearray(0x110000) for _ in range(3)]
gcb_names = "Other CR LF Control Extend ZWJ Regional_Indicator Prepend SpacingMark L V T LV LVT".split()
incb_names = {"Consonant": 1, "Extend": 2, "Linker": 3}
core = directory / "DerivedCoreProperties.txt"
if not core.exists():
    core = root / "build/document/deps/SheenBidi-3.0.0/Tools/Unicode/DerivedCoreProperties.txt"
paths = [directory / "GraphemeBreakProperty.txt", core, directory / "emoji-data.txt"]
for channel, path in enumerate(paths):
    for line in path.read_text(encoding="utf-8").splitlines():
        record = line.split("#", 1)[0].strip()
        if not record:
            continue
        fields = [field.strip() for field in record.split(";")]
        if channel == 1 and fields[1] != "InCB":
            continue
        if channel == 2 and fields[1] != "Extended_Pictographic":
            continue
        ends = fields[0].split("..")
        first, last = int(ends[0], 16), int(ends[-1], 16)
        value = gcb_names.index(fields[1]) if channel == 0 else incb_names[fields[2]] if channel == 1 else 1
        channels[channel][first:last + 1] = bytes([value]) * (last + 1 - first)
expected = bytes(gcb + 16 * incb + 64 * ep for gcb, incb, ep in zip(*channels))
actual = Path(sys.argv[1]).read_bytes()
assert len(actual) == len(expected) == 0x110000
for cp, (a, b) in enumerate(zip(actual, expected)):
    assert a == b, f"U+{cp:04X}: actual={a} expected={b}"
print(f"Actual C GCB/InCB/Extended_Pictographic: all {len(actual)} codepoints match official UCD 17; SHA256 {sha256(actual).hexdigest()}")
