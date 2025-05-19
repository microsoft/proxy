import dataclasses
from typing import List, Tuple, Generator, Dict

@dataclasses.dataclass(init=True)
class MarkdownParserState:
    inside_code_block: bool      = False
    code_lines:        list[str] = dataclasses.field(default_factory=list)
    required_ticks:    int       = 0
    current_language:  str       = ''
    current_paragraph: list[str] = dataclasses.field(default_factory=list)

    def reset(self):
        self.inside_code_block = False
        self.code_lines = []
        self.required_ticks = 0
        self.current_language = ''
        self.current_paragraph = []

    def begin_code_block(self, ticks: int, language: str):
        self.inside_code_block = True
        self.required_ticks = ticks
        self.current_language = language
        self.code_lines = []

    def end_code_block(self):
        self.inside_code_block = False
        self.required_ticks = 0
        self.current_language = ''

    def add_line_to_code(self, line: str):
        self.code_lines.append(line)

    def get_paragraph(self) -> List[str]:
        paragraph = self.current_paragraph
        self.current_paragraph = []
        return paragraph

    def add_line_to_paragraph(self, line: str):
        self.current_paragraph.append(line)

    def has_paragraph(self) -> bool:
        return len(self.current_paragraph) > 0 and not self.has_empty_paragraph()

    def has_empty_paragraph(self) -> bool:
        return len(self.current_paragraph) == 1 and len(self.current_paragraph[0].strip()) == 0


def count_leading_chars(line: str, char: str) -> int:
    return len(line) - len(line.lstrip(char))


def parse_markdown_elements(text: str) -> Generator[Tuple[str, Dict | str], None, None]:
    """
    Parse a Markdown-formatted string into a stream of structured elements,
    including code blocks, headings, and paragraphs.

    Args:
        text (str): The input Markdown text as a single string.

    Yields:
        Generator yielding tuples where:
            - First element is the type of element ('code', 'heading', or 'paragraph').
            - Second element is either a:
                - dictionary: for 'code': {'language': str, 'content': str}
                - dictionary: for 'heading': {'level': int, 'text': str}
                - string:     for 'paragraph' containing the paragraph content.
    """

    # Split the input text into individual lines for processing
    lines: List[str] = text.splitlines()

    # Initialize parser state to track context like code blocks and paragraphs
    state = MarkdownParserState()

    for line in lines:
        if state.inside_code_block: # Handle lines inside a code block
            # Count leading backticks
            line_ticks: int = count_leading_chars(line, '`')
            rest: str = line[line_ticks:]

            # Check if this line could close the code block
            valid_closing: bool = all(c.isspace() for c in rest) or rest == ""

            if line_ticks == state.required_ticks and valid_closing:
                # End of code block: yield it
                code_content = '\n'.join(state.code_lines)
                yield 'code', {'language': state.current_language, 'content': code_content}
                state.end_code_block()
            else:
                # Continue collecting code lines
                state.add_line_to_code(line)

        else:
            # Not inside a code block: process other Markdown elements
            stripped_line: str = line.strip()

            if stripped_line.startswith('#'):
                # Heading detected

                # If there was an ongoing paragraph, yield it first
                if state.has_paragraph():
                    paragraph_lines: list[str] = state.get_paragraph()
                    yield 'paragraph', '\n'.join(paragraph_lines)

                # Count number of '#' characters to determine heading level
                hash_count: int = count_leading_chars(stripped_line, '#')
                heading_text: str = stripped_line[hash_count:].lstrip()
                yield 'heading', {'level': hash_count, 'text': heading_text}

            elif line.startswith('```'):
                # Start of a code block

                # If there was an ongoing paragraph, yield it first
                if state.has_paragraph():
                    paragraph_lines: list[str] = state.get_paragraph()
                    yield 'paragraph', '\n'.join(paragraph_lines)

                # Determine number of opening backticks and language
                line_ticks: int = count_leading_chars(line, '`')
                language: str = line[line_ticks:].strip()
                state.begin_code_block(line_ticks, language)

            else:
                # Regular line: check if it's blank or part of a paragraph
                if stripped_line == '':
                    # Blank line: flush current paragraph if any
                    if state.has_paragraph():
                        paragraph_lines: list[str] = state.get_paragraph()
                        yield 'paragraph', '\n'.join(paragraph_lines)
                else:
                    # Part of a paragraph: add to current paragraph buffer
                    state.add_line_to_paragraph(line)

    # After loop: flush remaining paragraph if any
    if state.has_paragraph():
        paragraph_lines: list[str] = state.get_paragraph()
        yield 'paragraph', '\n'.join(paragraph_lines)

    # If got stuck inside a code block at the end, yield it anyway
    if state.inside_code_block:
        code_content: str = '\n'.join(state.code_lines)
        yield 'code', {'language': state.current_language, 'content': code_content}
        state.end_code_block()
