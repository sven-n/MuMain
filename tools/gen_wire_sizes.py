#!/usr/bin/env python3
"""Generate wire_sizes.generated.h: static_assert size guards for client
packet structs, derived from OpenMU's authoritative packet XML.

The XML lives in the MUnique.OpenMU.Network.Packets NuGet package (the same
package the .NET ClientLibrary consumes). CMake resolves the path and passes
it via --xml.

For each entry in PACKET_MAPPING, look up the wire <Length> and emit:

    static_assert(sizeof(<cpp_struct>) <= <wire_length>,
                  "wire size drift -- generated from <xml_name> (<N> bytes)");

We use <= rather than == because some client structs intentionally do not
decode all trailing fields (e.g. PreviewData). The bug class we are guarding
against (PR #402) is "client struct grew larger than the wire packet, so
safe_cast rejects it" -- <= catches that exactly.

To add a packet: append to PACKET_MAPPING. There is no naming convention
linking XML names to C++ struct names, so the mapping must be hand-maintained.
"""

import argparse
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


# XML packet name (without subcode/code prefix) -> client C++ struct name.
# Seeded with the structs PR #66 covered by hand.
PACKET_MAPPING = [
    ("CharacterCreationSuccessful",   "PRECEIVE_CREATE_CHARACTER"),
    ("CharacterInformationExtended",  "PRECEIVE_JOIN_MAP_SERVER_EXTENDED"),
    ("RespawnAfterDeathExtended",     "PRECEIVE_REVIVAL_EXTENDED"),
]


def load_lengths(xml_path: Path) -> dict[str, int]:
    """Return {packet_name: wire_length_bytes} from a Packets XML file."""
    tree = ET.parse(xml_path)
    root = tree.getroot()
    # Default namespace handling: strip any xmlns prefix so we can match
    # element names directly.
    def localname(tag: str) -> str:
        return tag.rsplit("}", 1)[-1]

    lengths: dict[str, int] = {}
    for packet in root.iter():
        if localname(packet.tag) != "Packet":
            continue
        name = None
        length = None
        for child in packet:
            tag = localname(child.tag)
            if tag == "Name":
                name = (child.text or "").strip()
            elif tag == "Length":
                try:
                    length = int((child.text or "").strip())
                except ValueError:
                    length = None
        if name and length is not None:
            lengths[name] = length
    return lengths


def emit_header(lengths: dict[str, int], out: Path, source_label: str) -> int:
    missing = [xml for xml, _ in PACKET_MAPPING if xml not in lengths]
    if missing:
        print(f"error: XML packets not found in {source_label}: {missing}",
              file=sys.stderr)
        return 1

    lines = [
        "// THIS FILE IS GENERATED. DO NOT EDIT.",
        f"// Source: {source_label}",
        "// Generator: tools/gen_wire_sizes.py",
        "//",
        "// Static-assert that each client packet struct fits within the wire packet",
        "// length declared by OpenMU. Guards against the PR #402 class of bug where a",
        "// dropped #pragma pack(1) silently inflates the client struct above the wire",
        "// size, causing safe_cast to reject every packet (freezes the loading screen).",
        "",
        "#pragma once",
        "",
    ]
    for xml_name, cpp_name in PACKET_MAPPING:
        n = lengths[xml_name]
        lines += [
            f"static_assert(sizeof({cpp_name}) <= {n},",
            f"              \"wire size drift -- generated from {xml_name} ({n} bytes)\");",
            "",
        ]
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {out} ({len(PACKET_MAPPING)} asserts)", file=sys.stderr)
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--xml", required=True, type=Path,
                    help="Path to ServerToClientPackets.xml "
                         "(from the MUnique.OpenMU.Network.Packets NuGet "
                         "package contentFiles)")
    ap.add_argument("--source-label", default=None,
                    help="Optional label written into the generated header's "
                         "'Source:' comment (e.g. the NuGet package "
                         "version). Defaults to the XML basename.")
    ap.add_argument("--output", required=True, type=Path,
                    help="Output header path")
    args = ap.parse_args()

    if not args.xml.exists():
        print(f"error: not found: {args.xml}", file=sys.stderr)
        return 1

    lengths = load_lengths(args.xml)
    label = args.source_label or args.xml.name
    return emit_header(lengths, args.output, source_label=label)


if __name__ == "__main__":
    raise SystemExit(main())
