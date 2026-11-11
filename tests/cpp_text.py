"""C++ source text as the compiler reads it, for the static checks that read the code."""

import os
from pathlib import Path

SOURCE_SUFFIXES = (".cpp", ".h")


def source_files(root, skipped_dirs, suffixes=SOURCE_SUFFIXES):
    """The files with these suffixes under root, sorted, without entering any directory named in skipped_dirs."""
    found = []
    for directory, subdirectories, files in os.walk(root):
        subdirectories[:] = [name for name in subdirectories if name not in skipped_dirs]
        found += [Path(directory, name) for name in files if Path(name).suffix in suffixes]
    return sorted(found)


def blank(text, start, stop):
    return "".join(c if c == "\n" else " " for c in text[start:stop])


def is_digit_separator(text, at):
    """An apostrophe inside a number (1'000, 0xFF'FF), not the start of a character literal (L'}')."""
    start = at
    while start > 0 and (text[start - 1].isalnum() or text[start - 1] in "_.'"):
        start -= 1
    return start < at and text[start].isdigit() and at + 1 < len(text) and text[at + 1].isalnum()


def line_end(text, at):
    """Where a line ends, a line continued by a backslash going on to the next."""
    while at < len(text) and not (text[at] == "\n" and text[at - 1] != "\\"):
        at += 1
    return at


def skipped_spans(text):
    """The comments, preprocessor lines and string and character literals in the text, in order, as
    (start, stop, is_literal)."""
    spans = []
    at, end = 0, len(text)
    line_start = True
    while at < end:
        c = text[at]
        two = text[at : at + 2]
        stop = None
        if line_start and c == "#":
            stop = line_end(text, at)
        elif two == "//":
            stop = line_end(text, at)
        elif two == "/*":
            stop = text.find("*/", at + 2)
            stop = end if stop < 0 else stop + 2
        elif c == '"':
            stop = at + 1
            if at > 0 and text[at - 1] == "R":
                delimiter = text[at + 1 : text.find("(", at)]
                stop = text.find(")" + delimiter + '"', at)
                stop = end if stop < 0 else stop + len(delimiter) + 2
            else:
                while stop < end and text[stop] != '"' and text[stop] != "\n":
                    stop += 2 if text[stop] == "\\" else 1
                stop = min(stop + 1, end)
        elif c == "'" and not is_digit_separator(text, at):
            stop = at + 1
            while stop < end and text[stop] != "'" and text[stop] != "\n":
                stop += 2 if text[stop] == "\\" else 1
            stop = min(stop + 1, end)
        if stop is None:
            if c == "\n":
                line_start = True
            elif not c.isspace():
                line_start = False
            at += 1
            continue
        spans.append((at, stop, c in "\"'"))
        at = stop
    return spans


def with_spans_blanked(text, spans, keep_literals):
    out = []
    at = 0
    for start, stop, is_literal in spans:
        out.append(text[at:start])
        out.append(text[start:stop] if keep_literals and is_literal else blank(text, start, stop))
        at = stop
    out.append(text[at:])
    return "".join(out)


def code_only(text):
    """The text as the compiler reads it, with comments, preprocessor lines and string and character literals turned
    into spaces; every character keeps its place, so positions and lines stay."""
    return with_spans_blanked(text, skipped_spans(text), keep_literals=False)


def code_and_source(text):
    """code_only(text), and the same text with its string and character literals kept, from one pass."""
    spans = skipped_spans(text)
    return with_spans_blanked(text, spans, keep_literals=False), with_spans_blanked(text, spans, keep_literals=True)
