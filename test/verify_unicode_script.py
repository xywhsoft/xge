"""Independently expand official UCD values and compare every C lookup."""
from array import array
from hashlib import sha256
from pathlib import Path
import struct
import sys

root = Path(__file__).resolve().parent.parent
directory = Path(sys.argv[2]) if len(sys.argv) > 2 else root / "build/document/deps/unicode-17"
fallback = root / "build/document/deps/SheenBidi-3.0.0/Tools/Unicode"
def records(name):
    path = directory / name
    if not path.exists():
        path = fallback / name
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.partition("#")[0].strip()
        if line:
            yield [x.strip() for x in line.split(";")]
aliases = {}
for f in records("PropertyValueAliases.txt"):
    if f[0] == "sc":
        aliases.update((alias, f[1]) for alias in f[1:])
primary = array("I", [int.from_bytes(b"Zzzz", "big")]) * 0x110000
extensions, brackets = {}, {}
for f in records("Scripts.txt"):
    endpoints = f[0].split("..")
    for cp in range(int(endpoints[0], 16), int(endpoints[-1], 16) + 1):
        primary[cp] = int.from_bytes(aliases[f[1]].encode("ascii"), "big")
for f in records("ScriptExtensions.txt"):
    endpoints = f[0].split("..")
    for cp in range(int(endpoints[0], 16), int(endpoints[-1], 16) + 1):
        extensions[cp] = frozenset(int.from_bytes(tag.encode("ascii"), "big") for tag in f[1].split())
for f in records("BidiBrackets.txt"):
    brackets[int(f[0], 16)] = (int(f[1], 16), 2 if f[2] == "o" else 0)
# UAX #9 canonical equivalent bracket matching, as used by the resolver.
brackets[0x2329] = (0x3009, 2)
brackets[0x232a] = (0x3008, 0)
data = Path(sys.argv[1]).read_bytes()
count = struct.unpack_from("<I", data)[0]
tags = struct.unpack_from(f"<{count}I", data, 4)
assert len(set(tags)) == count and set(tags) == set(primary) | {int.from_bytes(t.encode('ascii'), 'big') for t in aliases.values()}
offset = 4 + 4 * count
assert len(data) == offset + 30 * 0x110000
unique_sets = {}
for cp in range(0x110000):
    p = data[offset]
    raw = data[offset + 1:offset + 25]
    if raw not in unique_sets:
        bits = int.from_bytes(raw, "little")
        assert bits >> count == 0
        unique_sets[raw] = frozenset(tags[i] for i in range(count) if bits & (1 << i))
    actual = unique_sets[raw] or frozenset([tags[p]])
    assert tags[p] == primary[cp], (hex(cp), "Script")
    assert actual == extensions.get(cp, frozenset([primary[cp]])), (hex(cp), "Script_Extensions")
    pair, kind = struct.unpack_from("<IB", data, offset + 25)
    assert (pair, kind) == brackets.get(cp, (0, 1)), (hex(cp), "Bracket")
    offset += 30
print(f"All 1,114,112 actual C Script/Script_Extensions/bracket lookups match official Unicode 17.0.0; dump SHA256 {sha256(data).hexdigest()}")
