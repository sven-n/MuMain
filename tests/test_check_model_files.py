#!/usr/bin/env python3
"""The rules of check_model_files.py, on made-up headers."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_model_files import problem  # noqa: E402

assert problem(b"BMD\x00", 42) is not None, "version 0 is unreadable"
assert problem(b"BMD\x0a", 42) is not None, "a plain model with only its header has no body"
assert problem(b"BMD\x0c", 46) is not None, "an encrypted model with only its header has no body"
assert problem(b"BMD\x0e", 80) is not None, "a cipher model with only its header has no body"
assert problem(b"BMD\x0a", 6648) is None, "a plain model with a body is readable"
assert problem(b"BMD\x0c", 5000) is None and problem(b"BMD\x0e", 5000) is None, "the encrypted versions are readable"
assert problem(b"BMD\x0b", 5000) is not None, "a version the loader does not know is unreadable"
assert problem(b"BMD", 3) is not None, "a file cut before its version is unreadable"
assert problem(b"\x01\x00\x00\x00", 4) is None, "a data table named .bmd is no model"
assert problem(b"", 0) is not None, "an empty .bmd file is neither a model nor a table"
print("check_model_files rules hold")
