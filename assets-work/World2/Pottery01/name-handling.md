# Dungeon pottery legacy model-name handling

All work is read-only against Data, with diagnostics/export targets under /tmp. No engine, converter or repository-tool changes required.

Object28: raw32byte name field44617461325c4f626a656374325cc7d7bec6b8ae30312e736d64000000000000;26 nonzero bytes. CP949 display `Data2\Object2\항아리01.smd`.
Object29: raw32byte name field44617461325c4f626a656374325cc7d7bec6b8ae30322e736d64000000000000;26 nonzero bytes. CP949 display `Data2\Object2\항아리02.smd`.
Object30: raw32byte name field44617461325c4f626a656374325cb1fac1f8c7d7bec6b8ae30332e736d640000;30 nonzero bytes. CP949 display `Data2\Object2\깨진항아리03.smd`.

These are original BMDv10 files (plain payload starts byte4). Converter outputs v12 encrypted BMD; compare the decrypted payload's first32bytes using existing read-only Architecture03/raw_bindings.payload, not on-disk offset4. Every field is its nonzero name plus zero padding, so supported converter--name(max31bytes) can reproduce all32 exactly. Do not decodeCP949 and then encodeUTF8 as the export value: it changes the bytes, and Object30 grows to35bytes and is truncated by converter's31byte field limit.

Verified with unchanged official converter:
- bmd2smd writes an actions manifest whose `model ` metadata line includes these rawCP949 bytes. This is why official Python importer's whole-fileUTF8 text decoding fails before it can skip the irrelevant model line.
- All reference/animation SMD text for these3 models is ASCII, including bone names/material paths. Action metadata lines are ASCII. No geometric data transcoding is needed.
- smd2bmd --manifest accepts the original manifest; it reads action lines and does not restore the model field automatically.
- Existing --name option accepts exact bytes via a POSIX subprocess argv list. BmdSmdConverter.cpp336–344 copies the requested std::string into the32byte namefield (zeroes then memcpy). Python subprocess bytes arguments preserveCP949 bytes without shell quoting.
- All3 direct roundtrips passed full32byte name equality and converter EQUIVALENT (0 unmatched triangles,0 bone distance,0 differing bone names). /tmp/astra-dungeon-name-roundtrip/report.json retains hashes/results; /tmp/astra-dungeon-name-roundtrip.py reproduces.

Safest focused production adapter around the unchanged official Blender pipeline:
1. Retain original32byte field as hex metadata/provenance. Never put lossy replacement characters into export identity. Store a decodedCP949 display only as optional readable documentation.
2. Import: replace only the helper reading the manifest in the standalone batch wrapper. Read binary lines; parse only ASCII `action ` lines with strictASCII; preserve source manifest untouched. Existing /tmp/astra-dungeon-import.py already imported the3 previews through official SourceTools/geometry/material handling this way.
3. Export: call unchanged mu_bmd_export.main. Adapt its common.run_bmdconv subprocess boundary in the batch wrapper: for smd2bmd, replace the --name value with the stored original bytes; collect stdout/stderr as bytes and use UTF8 backslashreplace for display only. This also avoids info output failing UTF8 decoding after a successful export. Keep return-code checks strict. All geometry, rig, UV, materials and animation remain processed by official exporter/SourceTools/converter.
4. Assert raw32byte field equality on decrypted output. Converter compare does not explicitly compare the top-level model name, so it cannot replace this audit. Also assert action/bone/material contracts and complete authored correspondence in production as usual.

/tmp/astra-dungeon-official-name-export.py is the bounded proof wrapper for existing temporary imported blends, with only /tmp export targets. It avoids passing surrogate codepoints through Blender CLI/UI; bytes are introduced only at subprocess boundary. This approach is verified on current POSIX/macOS host; it makes no Windows argv encoding claim.

Official Blender proof completed: all3 existing temporary official imports exported successfully through the metadata adapter. Full32byte names exact and converter EQUIVALENT for all3. Evidence /tmp/astra-dungeon-official-name-export/report.json. This confirms the supported --name byte transport works end-to-end with SourceTools, not only direct converter extraction.
