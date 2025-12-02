#!/usr/bin/env python3
# pyright: strict

import json
import sys
from pathlib import Path

from mesonbuild import mformat

testfile = Path(sys.argv[1])
mesonfile = Path(sys.argv[2])

with testfile.open(encoding="utf-8") as f:
    tests = {
        suites["name"]: [suite["name"] for suite in suites["testsuite"]]
        for suites in json.load(f)["testsuites"]
    }

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

config = mformat.get_meson_format([mesonfile])
formatter = mformat.Formatter(config, False, False)
pretty = formatter.format(f"{tests = }", mesonfile)

if pretty == "".join(build[begin:end]):
    exit()

with mesonfile.open("w", encoding="utf-8") as f:
    f.writelines(build[:begin])
    _ = f.write(pretty)
    f.writelines(build[end:])
