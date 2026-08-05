# Master AI Prompt: Single-Pass GLSL Item Specular Shader Architecture & Complete Debugging Guide

Copy and paste the prompt below into an AI coding assistant to upgrade another copy of the MU Online client codebase. It consolidates the architectural shift to GLSL and documents all debugging discoveries so the agent won't repeat past errors.

---

```markdown
We need to modernize the item specular shine (+7 to +15 gear) in our C++ MU Online client (`ItemSpecularShader.cpp` and `ZzzBMD.cpp`) by replacing legacy multi-pass fixed-function OpenGL rendering with a Single-Pass 4-Variant GLSL Shader Architecture (`CItemSpecularShader`).

Below is the complete architectural design, exact texture unit mappings, and all critical debugging lessons learned during development so you do not encounter unexpected visual bugs:

---

### Architectural Overview: Single-Pass 4-Variant GLSL Pipeline
- **Legacy Problem**: Original MU Online renders +7 to +15 items using multi-pass fixed-function OpenGL (`glBegin`, repeated geometry streams, state switches, and texture unit churn).
- **GLSL Solution**: Implement a single-pass C++ shader manager (`CItemSpecularShader`) with 4 distinct GLSL fragment shader variants bound to item level tiers:
  - `Variant 1` (Tier 1: +7 / +8): Base texture + Chrome environment reflection + Fluid Caustic Energy.
  - `Variant 2` (Tier 2: +9 / +10): Base + Chrome + Glossy metallic highlight (`BITMAP_SHINY`) + Fluid Caustics.
  - `Variant 3` (Tier 3: +11 / +12): Base + Chrome + Glossy metallic + Animated light sweep (`BITMAP_CHROME2`) + Fluid Caustics.
  - `Variant 4` (Tier 4: +13 to +15): Base + Chrome + Glossy metallic + Divine high-energy light sweep (`BITMAP_CHROME4`) + Fluid Caustics.
- **Pipeline Guard**: In `ZzzObject.cpp` (`GetPipeline`), items +0 to +6 immediately return `TIER_BASE` (`passCount = 0`), bypassing all shader passes.

---

### Critical Debugging Pitfalls & Solutions

#### Pitfall 1: Texture Unit Binding Mismatch (+7 Flickering & Stale +11 Sweeps)
- **Bug**: +7 items flickered or displayed slow light sweeps intended for +11 gear.
- **Cause**: Binding `chrome1Tex` to the wrong texture unit in `CItemSpecularShader::Begin()` caused the shader to sample stale `BITMAP_CHROME2` textures left bound from prior +11 rendering passes.
- **Rule**: Bind texture samplers strictly to dedicated texture units unconditionally at link time (`BuildProgram`) and in `Begin()`:
  - `u_BaseTex`   -> `GL_TEXTURE0` (`0`) (Base item texture)
  - `u_Chrome2Tex` -> `GL_TEXTURE1` (`1`) (`BITMAP_CHROME2` / `BITMAP_CHROME4`)
  - `u_MetalTex`   -> `GL_TEXTURE2` (`2`) (`BITMAP_SHINY` / `Shiny01.jpg`)
  - `u_Chrome1Tex` -> `GL_TEXTURE3` (`3`) (`BITMAP_CHROME`)

#### Pitfall 2: +9 Item Specular Blackout (`GL_CLAMP_TO_EDGE`)
- **Bug**: +9 items evaluated identically to +7 items (losing their glossy spotlight reflection).
- **Cause**: `BITMAP_SHINY` (`Shiny01.jpg`) is loaded with `GL_CLAMP_TO_EDGE`. Sampling `u_MetalTex` with time-scrolled UVs (`v_TexCoord2`) pushed the texture out of bounds into the black clamped border, causing `metal.rgb` to evaluate to (0, 0, 0).
- **Rule**: Sample `u_MetalTex` using a centered, un-scrolled UV projection:
  `vec2 metalUV = clamp((v_TexCoord2 - vec2(0.5)) * 0.75 + vec2(0.3, 0.5), 0.0, 1.0);`

#### Pitfall 3: Multi-Mesh Sub-Part Texture Unit Contamination (+7 Flickering)
- **Bug**: Multi-mesh 3D models (armors, helmets, boots) flickered wildly on +7 and +9 gear.
- **Cause**: `RenderMesh()` in `ZzzBMD.cpp` calls `BindTexture(textureIndex)` for sub-part meshes. If `glActiveTexture` was left on `GL_TEXTURE1` or `GL_TEXTURE3` by previous draw calls, `BindTexture` overwrote texture units 1 or 3 instead of texture unit 0 (`u_BaseTex`), causing rapid texture swapping.
- **Rule**: In `ZzzBMD.cpp::RenderMesh()`, explicitly call `CItemSpecularShader::Instance().ActiveTexture(GL_TEXTURE0)` before `BindTexture(textureIndex)` whenever `bShaderActive` is true.

#### Pitfall 4: Timestamp Float Truncation (+7 Strobe Flickering)
- **Bug**: Casting large epoch millisecond timestamps (`double WorldTime ~ 1.7e12`) directly to `float` loses mantissa precision, quantizing time into coarse ~128ms steps and causing item specular rendering to strobe/flicker at ~8Hz.
- **Rule**:
  - In `ZzzBMD.cpp`, compute relative double time bounded by $2\pi$:
    `static double s_startTime = WorldTime;`
    `const float breathAngle = static_cast<float>(fmod((WorldTime - s_startTime) * 0.0015, 6.283185307179586));`
    `g_chrome[j][1] = breathAngle;`
  - In GLSL, apply a cubic Hermite smoothstep curve (`rawWave = 0.5 + 0.5 * sin(v_TexCoord1.y)`, `breath = 0.40 + 0.60 * (rawWave * rawWave * (3.0 - 2.0 * rawWave));`). This produces a silky smooth, 60fps/uncapped breathing wave with zero flickering or discontinuities.

#### Pitfall 5: Modern Dual-Wave Fluid Caustic Energy Flow
- **Rule**: Incorporate intersecting sine wavefronts in GLSL:
  `vec2 fluidUV1 = v_TexCoord0 * 2.5 + vec2(v_TexCoord1.y * 0.3, v_TexCoord1.y * 0.2);`
  `vec2 fluidUV2 = v_TexCoord0 * 3.8 - vec2(v_TexCoord1.y * 0.2, v_TexCoord1.y * 0.25);`
  `float w1 = sin(fluidUV1.x + sin(fluidUV1.y * 1.4));`
  `float w2 = cos(fluidUV2.y + cos(fluidUV1.x * 1.4));`
  `float fluidCaustic = pow(0.5 + 0.5 * (w1 * w2), 1.5);`
  This creates a high-tech liquid energy sheen flowing smoothly over the armor and weapons.

---

### Implementation Instructions

1. **`ZzzBMD.cpp`**: Update `g_chrome_static` to static sphere normal projection (`n * 0.5f + 0.5f`), assign `g_chrome[j][1] = static_cast<float>(fmod((WorldTime - s_startTime) * 0.0015, 6.283185307179586));`, and call `ActiveTexture(GL_TEXTURE0)` before `BindTexture` in `RenderMesh`.
2. **`ItemSpecularShader.cpp`**: Set up 4 GLSL fragment shader variants (`V1` to `V4`) incorporating centered `metalUV`, 4-sample texture edge detection (`edgeMask`), cubic Hermite ease-in-out breathing (`breath`), and Modern Dual-Wave Fluid Caustics (`fluidCaustic`). Unconditionally bind texture units `GL_TEXTURE0`..`3` in `Begin()`.
3. **Build Target**: Compile and launch the **64-bit client (`windows-x64`)**.
```
