import os
import re
import sys
from typing import List, Tuple, Generator, Dict


def parse_markdown_elements(text: str) -> Generator[Tuple[str, Dict | str], None, None]:
    """
    Parse a Markdown-formatted string into a stream of structured elements,
    including code blocks, headings, and paragraphs.

    Args:
        text (str): The input Markdown text.

    Yields:
        Tuple[str, Union[Dict, str]]: A tuple representing each parsed element.
            - First element is the type ('code', 'heading', or 'paragraph').
            - Second element is either a dictionary (for code/heading) or a string (for paragraph).
    """

    # Split the input text into lines for line-by-line parsing
    lines: List[str] = text.splitlines()

    # State tracking variables
    inside_code_block: bool = False
    code_lines: List[str] = []
    required_ticks: int = 0
    current_language: str = ''
    current_paragraph: List[str] = []

    for line in lines:

        if inside_code_block:
            # We are inside a code block. Check if this line ends it.

            # Count leading backticks
            line_ticks = 0
            for c in line:
                if c == '`':
                    line_ticks += 1
                else:
                    break

            # Remaining part after ticks
            rest: str = line[line_ticks:]

            # Valid closing line must have only whitespace or be empty after ticks
            valid_closing: bool = all(c.isspace() for c in rest) or rest == ""

            if line_ticks == required_ticks and valid_closing:
                # End of code block reached
                code_content: str = '\n'.join(code_lines)
                yield 'code', {'language': current_language, 'content': code_content}
                # Reset state
                inside_code_block = False
                code_lines = []
                current_language = ''
            else:
                # Continue collecting lines for the code block
                code_lines.append(line)

        else:
            # Not inside a code block. Try to match heading or start of a code block.
            stripped_line: str = line.strip()

            if stripped_line.startswith('#'):
                # It's a heading. Yield any accumulated paragraph first.
                if current_paragraph:
                    # Skip empty paragraph
                    if len(current_paragraph) == 1 and len(current_paragraph[0]) == 0:
                        pass
                    else:
                        yield 'paragraph', '\n'.join(current_paragraph)
                    current_paragraph = []

                # Determine heading level by counting '#'
                hash_count: int = 0
                for c in stripped_line:
                    if c == '#':
                        hash_count += 1
                    else:
                        break

                # Extract heading text after '#' and leading space
                heading_text: str = stripped_line[hash_count:].lstrip()
                yield 'heading', {'level': hash_count, 'text': heading_text}

            elif line.startswith('```'):
                # Start of a code block. Yield any accumulated paragraph first.
                if current_paragraph:
                    # Skip empty paragraph
                    if len(current_paragraph) == 1 and len(current_paragraph[0]) == 0:
                        pass
                    else:
                        yield 'paragraph', '\n'.join(current_paragraph)
                    current_paragraph = []

                # Count opening backticks
                line_ticks = 0
                for c in line:
                    if c == '`':
                        line_ticks += 1
                    else:
                        break

                required_ticks = line_ticks
                # Extract language identifier if present
                current_language = line[line_ticks:].strip()
                inside_code_block = True
                code_lines = []

            else:
                # Regular paragraph line
                current_paragraph.append(line)

    # Yield any remaining paragraph at the end of the input
    if current_paragraph:
        # Skip empty paragraph
        if len(current_paragraph) == 1 and len(current_paragraph[0]) == 0:
            pass
        else:
            yield 'paragraph', '\n'.join(current_paragraph)
        current_paragraph = []
    # If currently got stuck in a code block because the
    # backtick pair is unmatched. yield the content anyway.
    if inside_code_block:
        code_content: str = '\n'.join(code_lines)
        yield 'code', {'language': current_language, 'content': code_content}
        # Reset state
        inside_code_block = False
        code_lines = []
        current_language = ''


def extract_cpp_code_in_example(text: str) -> List[str]:
    """
    Extract all C++ code blocks that occur within the "Example" section of a Markdown document.

    Args:
        text (str): The input Markdown text.

    Returns:
        List[str]: A list of C++ code block contents found within the "Example" section.
    """
    elements: Generator = parse_markdown_elements(text)
    in_example: bool = False
    cpp_code_blocks: List[str] = []

    for element_type, content in elements:
        if element_type == 'heading':
            level = content['level']
            heading_text = content['text'].strip()
            if level == 2:
                if heading_text == 'Example':
                    in_example = True
                elif in_example:
                    in_example = False
        elif element_type == 'code' and in_example:
            language = content['language'].lower()
            if language == 'cpp':
                cpp_code_blocks.append(content['content'])

    return cpp_code_blocks


def extract_code_blocks_from_md_file(md_path):
    """
    Extract all C++ code blocks under the '## Example' section of a Markdown file.

    Args:
        md_path (str): Path to the Markdown file.

    Returns:
        list: A list of strings, each containing a C++ code block.
    """
    with open(md_path, 'r', encoding='utf-8') as f:
        content = f.read()
    return extract_cpp_code_in_example(content)


def write_code_file(cpp_path, code, md_path):
    """
    Write the extracted C++ code to a file with a header.

    Args:
        cpp_path (str): Path to the output C++ file.
        code (str): The C++ code to write.
        md_path (str): Path to the original Markdown file for the header.
    """
    header = f"// This file was auto-generated from: {md_path}\n// Do not edit this file manually.\n\n"
    with open(cpp_path, 'w', encoding='utf-8') as out:
        out.write(header)
        out.write(code)


def generate_subdir_cmake(subdir_path, target_name, cpp_files, md_path):
    """
    Generate a CMakeLists.txt in the subdirectory using a customizable template.

    The following placeholders are replaced:
      - $COMMON$ -> Currently expands to:
        - $DIAGNOSTIC_FLAGS$

      - $NAME$ -> Sub-directory name

      - $FILES$ -> A list of all cpp files

      - $DIAGNOSTIC_FLAGS$ -> See below

    Args:
        subdir_path (str): Output directory path.
        target_name (str): Name of the target.
        cpp_files (list): List of generated .cpp files.
        md_path (str): Path to the original .md file (to find template).
    """
    # Path to the custom template file
    template_path: str = md_path.replace('.md', '.cmake.in')

    # Use the custom template if it exists
    if os.path.exists(template_path):
        with open(template_path, 'r', encoding='utf-8') as f:
            template_content = f.read()
    else:
        # Fallback to default template
        template_content = """
add_executable($NAME$ $FILES$)
target_link_libraries($NAME$ msft_proxy)

$COMMON$

"""
        pass

    # Note the indent here because it's Python.
    common_snippet = """
$DIAGNOSTIC_FLAGS$
"""

    diagnostic_flags = """
if (MSVC)
  target_compile_options($NAME$ PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options($NAME$ PRIVATE -Wall -Wextra -Wpedantic -Wno-c++2b-extensions)
else()
  target_compile_options($NAME$ PRIVATE -Wall -Wextra -Wpedantic)
endif()   
"""

    # Replace placeholders
    files_str = ' '.join(cpp_files)

    # Note: Be aware of the `replace` order. This is not the (turing complete) C++ template,
    # it's not going to recursively expand the template automatically.
    cmake_content = (template_content
                     .replace('$COMMON$', common_snippet)
                     .replace('$DIAGNOSTIC_FLAGS$', diagnostic_flags)
                     .replace('$NAME$', target_name)
                     .replace('$FILES$', files_str))

    # Write the final CMakeLists.txt
    cmake_path = os.path.join(subdir_path, "CMakeLists.txt")
    with open(cmake_path, 'w', encoding='utf-8') as f:
        f.write(f"# This file was auto-generated from: {md_path}\n# Do not edit this file manually.\n\n")
        f.write(cmake_content)


def main():
    """
    Main function to process Markdown files and generate C++ code and CMake configurations.
    """
    if len(sys.argv) != 3:
        print("Usage: python extract_example_code_from_docs.py <input_dir> <output_dir>")
        sys.exit(1)

    input_dir = sys.argv[1]
    output_dir = sys.argv[2]

    total_cmake_lines = []  # For top-level CMakeLists.txt

    for root, _, files in os.walk(input_dir):
        for file in files:
            if file.endswith('.md'):
                md_path = os.path.join(root, file)
                rel_path = os.path.relpath(md_path, input_dir)
                rel_base = os.path.splitext(rel_path)[0].replace(os.sep, '_')

                # Extract C++ code blocks
                code_blocks = extract_code_blocks_from_md_file(md_path)
                if not code_blocks:
                    continue  # Skip if no code

                # Create subdirectory for this example
                subdir_name = f"example_{rel_base}"
                subdir_path = os.path.join(output_dir, subdir_name)
                os.makedirs(subdir_path, exist_ok=True)

                # Generate code files and collect names
                cpp_files = []
                for i, code in enumerate(code_blocks, 1):
                    cpp_file = os.path.join(subdir_path, f"code_{i}.cpp")
                    write_code_file(cpp_file, code, md_path)
                    cpp_files.append(f"code_{i}.cpp")

                # Generate subdirectory's CMakeLists.txt
                generate_subdir_cmake(subdir_path, subdir_name, cpp_files, md_path)

                total_cmake_lines.append(f"add_subdirectory({subdir_name})")

    # Write the final top-level CMakeLists.txt
    total_cmake_path = os.path.join(output_dir, "CMakeLists.txt")
    with open(total_cmake_path, 'w', encoding='utf-8') as f:
        f.write(f"# This file was auto-generated from: {input_dir}\n# Do not edit this file manually.\n\n")
        for line in total_cmake_lines:
            f.write(line + '\n')

if __name__ == '__main__':
    main()