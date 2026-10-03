"""Independent official-data oracle for every actual C lookup."""
from hashlib import sha256
from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parent.parent
data = (root / "test_xui/data/unicode-17/DerivedJoiningType.txt").read_bytes()
assert sha256(data).hexdigest() == "f39ebe974825d6736aee15582250307aa532b2cfab3caf3f86bd23fddc9c5c4d"
expected = bytearray(b"." * 0x110000)
for match in re.finditer(rb"^([0-9A-F]+)(?:\.\.([0-9A-F]+))?\s*;\s*T\s*#", data, re.M):
    first = int(match[1], 16)
    last = int(match[2] or match[1], 16)
    expected[first:last+1] = b"T" * (last-first+1)
actual = subprocess.check_output([sys.argv[1], "--dump"], cwd=root)
assert len(actual) == len(expected)
assert actual == expected, next(hex(i) for i, pair in enumerate(zip(actual, expected)) if pair[0] != pair[1])
print("All 1114112 actual C joining queries match independent Unicode 17 DerivedJoiningType; generator not invoked")
