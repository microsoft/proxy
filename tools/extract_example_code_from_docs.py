#!/usr/bin/env python3
# pyright: strict

import re
from typing import Optional
from pathlib import Path

EXAMPLE_PATTERN = re.compile(r"## Example\r?\n\r?\n```cpp\r?\n(.*?)\r?\n```", re.DOTALL)


def extract_cpp_code(md_path: Path) -> Optional[str]:
    with open(md_path, "r", encoding="utf-8") as f:
        content = f.read()

    code_blocks: list[str] = re.findall(EXAMPLE_PATTERN, content)

    if len(code_blocks) == 0:
        return  # No match, skip
    elif len(code_blocks) > 1:
        msg = f"File '{md_path}' contains more than one '## Example' C++ code block."
        raise ValueError(msg)

    cpp_code = code_blocks[0]
    header = f"""
// This file was auto-generated from:
// {md_path}

""".lstrip()

    if "pro::skills::format" in cpp_code:
        cpp_code = f"""
#include <proxy/proxy.h>
#ifdef PRO4D_HAS_FORMAT
{cpp_code}
#else
int main() {{
  // std::format not available
  return 77;
}}
#endif
""".strip()

    return header + cpp_code


def main() -> None:
    import argparse
    import os
    from dataclasses import dataclass

    @dataclass(frozen=True)
    class Args:
        input_dir: Path
        output_dir: Path

    parser = argparse.ArgumentParser()
    _ = parser.add_argument("input_dir", type=Path, help="Path to Markdown documents")
    _ = parser.add_argument("output_dir", type=Path, help="Source code output path")
    args = parser.parse_args(namespace=Args)

    input_dir = args.input_dir
    output_dir = args.output_dir

    for root, _, files in os.walk(input_dir):
        for file in files:
            if file.endswith(".md"):
                md_path = Path(root) / file
                rel_path = md_path.relative_to(input_dir)
                rel_base = "_".join([*rel_path.parent.parts, rel_path.stem])
                cpp_path = output_dir / f"example_{rel_base}.cpp"
                cpp_code = extract_cpp_code(md_path)
                if cpp_code is None:
                    continue
                with open(cpp_path, "w", encoding="utf-8") as f:
                    _ = f.write(cpp_code)


if __name__ == "__main__":
    main()
