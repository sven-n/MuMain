# cgltf

Single-header glTF 2.0 parser by Johannes Kuhlmann, MIT license (`LICENSE`,
also at the end of `cgltf.h`). The client reads monster models from glTF
binaries with it (`source/Render/Models/Gltf/`).

- Version: 1.13 (the header's own `Version:` line), unmodified.
- Upstream: https://github.com/jkuhlmann/cgltf
- Taken from: the copy shipped by MaterialX 1.39.5
  (`/usr/include/MaterialXRender/External/Cgltf/cgltf.h`, Arch package
  `materialx 1.39.5-1`), SHA-256
  `017aa061da0e6db11d416b82521d12d2818ea2819e1dca90e2eb86ce183324c4`.

Exactly one translation unit defines `CGLTF_IMPLEMENTATION` before including
the header. Update by replacing `cgltf.h` with a released version as a
whole and recording the version and checksum here.
