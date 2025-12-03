#!/usr/bin/env python3
# pyright: strict

import sys
from pathlib import Path

from mesonbuild import mformat

from extract_example_code_from_docs import extract_cpp_code

directory = Path(sys.argv[1])
mesonfile = directory / "meson.build"

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

docs = [
    str(path.relative_to(directory).as_posix())
    for path in directory.glob("**/*.md")
    if extract_cpp_code(path) is not None
]

config = mformat.get_meson_format([mesonfile])
formatter = mformat.Formatter(config, False, False)
pretty = formatter.format(f"{docs = }", mesonfile)

if pretty == "".join(build[begin:end]):
    exit()

with mesonfile.open("w", encoding="utf-8") as f:
    f.writelines(build[:begin])
    _ = f.write(pretty)
    f.writelines(build[end:])
