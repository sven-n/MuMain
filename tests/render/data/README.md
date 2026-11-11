# Test data of the render tests

`Monster14.glb` (Elite Yeti, `Data/Monster/Monster14.bmd`), `Monster01.glb`
(Bull Fighter, `Monster01.bmd`) and `Monster33.glb` (Bali, `Monster33.bmd`)
are these BMDs exported to glTF 2.0 binaries as the loader expects them
(`src/source/Render/Models/Gltf/GlbLoader.h`): the BMD's bones as joints with
their names, its meshes on one skin, one clip `actionNN` per action with the
BMD's keys. They carry no game texture (a grey placeholder). SHA-256:

    04e7d9912ab6ba52767d95a54455fb7f2df15570866dcde7869da004e3fd500e  Monster01.glb
    a31c33c0b0b2ca02e985ff310876e3fe5013bcd5afbf0792184ed14eb72f003e  Monster14.glb
    50c7d0561d5f0e567680b6bc749a248dab726c15663b74e8a2d652d0e21c522a  Monster33.glb

`test_glb_model.cpp` loads the Yeti into the BMD and compares bones, rest
pose, meshes, animation and skinning with the BMD itself. The Bull Fighter and
Bali hold neighbouring keys within `QuaternionCompare`'s epsilon at chain ends
(toes, fingers); the test checks that their key times lie where the client
samples MU's keys, that their rotation keys are the BMD's quaternions bit for
bit, and that their poses equal the BMD's.
