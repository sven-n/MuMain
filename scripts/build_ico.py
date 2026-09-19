#!/usr/bin/env python3
import struct
import sys
from pathlib import Path


def main() -> None:
    output = Path(sys.argv[1])
    images = [
        (int(size), Path(path).read_bytes())
        for size, path in (argument.split("=", 1) for argument in sys.argv[2:])
    ]
    offset = 6 + 16 * len(images)
    entries = []
    payload = bytearray()
    for size, data in images:
        dimension = 0 if size == 256 else size
        entries.append(
            struct.pack(
                "<BBBBHHII",
                dimension,
                dimension,
                0,
                0,
                1,
                32,
                len(data),
                offset + len(payload),
            )
        )
        payload.extend(data)
    output.write_bytes(
        struct.pack("<HHH", 0, 1, len(images)) + b"".join(entries) + payload
    )


if __name__ == "__main__":
    main()
