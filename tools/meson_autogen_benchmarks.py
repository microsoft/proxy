#!/usr/bin/env python3
# pyright: strict

import re
import sys
from pathlib import Path

from mesonbuild import mformat

PATTERN = re.compile(r"(?=[A-Z])")

benchmarkfile = Path(sys.argv[1])
mesonfile = Path(sys.argv[2])

with benchmarkfile.open(encoding="utf-8") as f:
    bench = [line.rstrip() for line in f.readlines()]

with mesonfile.open(encoding="utf-8") as f:
    build = f.readlines()

begin = -1
end = -1

for i, line in enumerate(build):
    if begin == -1 and line.strip() == "#pragma autogen push":
        begin = i + 1
    elif end == -1 and line.strip() == "#pragma autogen pop":
        end = i

if begin == -1 or end == -1:
    raise ValueError

benchmarks: "dict[str, list[str]]" = {}

for name in bench:
    suite = "".join(PATTERN.split(name.split('_', 1)[1])[2:4])
    if suite not in benchmarks:
        benchmarks[suite] = []
    else:
        benchmarks[suite].append(name)

config = mformat.get_meson_format([mesonfile])
formatter = mformat.Formatter(config, False, False)
pretty = formatter.format(f"{benchmarks = }", mesonfile)

if pretty == "".join(build[begin:end]):
    exit()

with mesonfile.open("w", encoding="utf-8") as f:
    f.writelines(build[:begin])
    _ = f.write(pretty)
    f.writelines(build[end:])
