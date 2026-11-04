"""C++ source text as the compiler reads it, for the static checks that read the code."""


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


def code_only(text, keep_literals=False):
    """The text as the compiler reads it, with comments, preprocessor lines and (unless keep_literals) string and
    character literals turned into spaces; every character keeps its place, so positions and lines stay."""
    out = []
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
            out.append(c)
            if c == "\n":
                line_start = True
            elif not c.isspace():
                line_start = False
            at += 1
            continue
        is_literal = c in "\"'"
        out.append(text[at:stop] if keep_literals and is_literal else blank(text, at, stop))
        at = stop
    return "".join(out)
