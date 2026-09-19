#!/usr/bin/env python3
import struct
import sys
from pathlib import Path


def main() -> None:
    output = Path(sys.argv[1])
    chunks = []
    for icon_type, path in (argument.split("=", 1) for argument in sys.argv[2:]):
        data = Path(path).read_bytes()
        chunks.append(icon_type.encode("ascii") + struct.pack(">I", len(data) + 8) + data)
    payload = b"".join(chunks)
    output.write_bytes(b"icns" + struct.pack(">I", len(payload) + 8) + payload)


if __name__ == "__main__":
    main()
