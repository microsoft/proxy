#!/usr/bin/env python3
# pyright: strict

import sys
from pathlib import Path

from extract_example_code_from_docs import extract_cpp_code

infile = Path(sys.argv[1])
if infile.is_dir():
    for path in infile.glob("**/*.md"):
        if extract_cpp_code(path) is None:
            continue
        print(str(path.relative_to(infile).as_posix()), end="\0")
    exit()

outfile = Path(sys.argv[2])

cpp_code = extract_cpp_code(infile)

if not cpp_code:
    raise ValueError

with open(outfile, "w") as f:
    _ = f.write(cpp_code)
