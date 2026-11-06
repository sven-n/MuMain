// skinned_lit.frag.hlsl — Enhanced per-pixel lighting for GPU-skinned BMD meshes.
// Pairs with skinned_textured.vert.hlsl in Enhanced mode (palette.w == 2), which leaves the
// vertex colour unlit and passes the normal, the view vector and the per-object light direction.
//   diffuse  = Classic curve max(dot(n, L) * scale + ambient, floor) * object light, per pixel
//   rim      = strength * (1 - |dot(n, v)|)^power * object light, added after the texture
//   dynamic  = sum of the frame's point lights (Enhanced dynamic lights): colour * linear falloff
//              over the horizontal distance * wrapped diffuse, added to the diffuse light
//   specular = Blinn-Phong, opt-in per item model, scaled by the scene and object light
//   fog      = Enhanced exponential fog (see basic_textured_fog.frag.hlsl)
//   normal   = with a normal map (glTF models, Enhanced lighting only): the
//              map's tangent-space normal (2 rgb - 1, +Y up, xy times the map's scale) in the frame
//              of the skinned tangent T, bitangent cross(N, T) x handedness and N, for the diffuse,
//              rim, specular and point-light terms; without one (scale 0) the interpolated normal
//              as before, and the flat default map bound at t1 is not read
// FogUniforms mirrors FogUniform, SceneLighting mirrors mu::SceneLightingUniform and DynamicLights
// mirrors mu::DynamicLightsUniform (std140).
Texture2D tex : register(t0, space2);
SamplerState s : register(s0, space2);
Texture2D normalMap : register(t1, space2);
SamplerState normalSampler : register(s1, space2);
cbuffer FogUniforms : register(b0, space3)
{
    uint fogEnabled;
    uint alphaDiscardEnabled;
    float alphaThreshold;
    uint alphaToCoverage; // 1: the pipeline turns alpha into multisample coverage
    float fogStart;
    float fogEnd;
    float4 fogColor;
};
cbuffer SceneLighting : register(b1, space3)
{
    float4 sceneLightDirection;
    float4 sceneLightColor;
    float4 diffuseCurve; // scale, ambient, floor, highlight knee
    float4 rimParameters;
    float4 specularParameters;
    float4 sceneFogColor;
    float4 sceneFogParameters;
    float4 cameraPosition;
};
static const uint MaxDynamicLights = 32; // mu::kMaxDynamicLights
cbuffer DynamicLights : register(b2, space3)
{
    uint dynamicLightCount;   // lights used
    float dynamicLightWrap;   // 0: Lambert, 0.5: half-Lambert (mu::WrappedDiffuse)
    uint2 dynamicLightPadding;
    float4 dynamicLightPositionAndRadius[MaxDynamicLights];
    float4 dynamicLightColor[MaxDynamicLights];
};

// The light mood's highlight shoulder (Render::Lighting::LightMood::RollOffHighlight): a lit
// colour channel above the knee approaches 1 smoothly instead of being cut off there. Only a knee
// between 0 and 1 rolls off; the callers keep the Classic cut-off expression verbatim otherwise,
// so that the driver compiles the Classic path to the same arithmetic as before.
bool RollsOffHighlight(float knee)
{
    return knee > 0.0 && knee < 1.0;
}

float RollOffChannel(float value, float knee)
{
    const float range = 1.0 - knee;
    return value > knee ? knee + range * (1.0 - exp(-max(value - knee, 0.0) / range)) : value;
}

float3 RollOffHighlight(float3 color, float knee)
{
    return saturate(float3(RollOffChannel(color.r, knee), RollOffChannel(color.g, knee),
                           RollOffChannel(color.b, knee)));
}

struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float eyeDepth : TEXCOORD3;
    float3 normal : TEXCOORD4;
    float3 viewVector : TEXCOORD5;
    nointerpolation float4 lightDirectionAndSpecular : TEXCOORD6;
    float4 worldPositionAndDynamicLights : TEXCOORD7; // w: 1 when the point lights apply
    float3 worldNormal : TEXCOORD8;
    float4 tangent : TEXCOORD9;                       // w: handedness, 0 without a normal map
    float4 worldTangentAndNormalMapScale : TEXCOORD10; // w: the map's scale, 0 without one
};

// Same curve as basic_textured_fog.frag.hlsl and mu::EnhancedFogAmount() (SceneLighting.cpp).
float EnhancedFogAmount(float eyeDepth)
{
    const float scaledDistance = max(eyeDepth - sceneFogParameters.x, 0.0) * sceneFogParameters.z;
    return saturate(1.0 - exp(-scaledDistance * scaledDistance));
}

float3 SafeNormalize(float3 value)
{
    const float lengthSquared = dot(value, value);
    return lengthSquared > 1e-12 ? value * rsqrt(lengthSquared) : float3(0.0, 0.0, 0.0);
}

// Point lights reach as far as the terrain light they come from (linear falloff over the
// horizontal distance); the wrapped diffuse keeps MU's soft shading and leaves the far side dark.
float3 DynamicLighting(float3 worldPosition, float3 worldNormal)
{
    float3 light = float3(0.0, 0.0, 0.0);
    const uint count = min(dynamicLightCount, MaxDynamicLights);
    [loop]
    for (uint i = 0; i < count; ++i)
    {
        const float4 positionAndRadius = dynamicLightPositionAndRadius[i];
        const float3 toLight = positionAndRadius.xyz - worldPosition;
        const float radius = positionAndRadius.w;
        const float horizontalDistance = length(toLight.xy);
        if (horizontalDistance >= radius)
            continue;
        const float falloff = (radius - horizontalDistance) / radius;
        const float wrapped =
            saturate(dot(worldNormal, SafeNormalize(toLight)) * (1.0 - dynamicLightWrap) + dynamicLightWrap);
        light += dynamicLightColor[i].rgb * (falloff * wrapped);
    }
    return light;
}

// The map's normal `mapped` (tangent space) in the frame of `normal` and `tangent`, unit length.
float3 PerturbNormal(float3 normal, float3 tangent, float handedness, float3 mapped)
{
    const float3 n = SafeNormalize(normal);
    const float3 t = SafeNormalize(tangent);
    const float3 b = cross(n, t) * handedness;
    return SafeNormalize(t * mapped.x + b * mapped.y + n * mapped.z);
}

float4 main(FSInput input) : SV_Target
{
    // Sampled first: the MSL translation numbers textures and samplers in the order of first use,
    // and SDL binds the texture at slot 0, the normal map at slot 1.
    const float4 texel = tex.Sample(s, input.uv);
    // The normal map is sampled with derivatives taken here, outside the per-pixel branch.
    const float2 uvDx = ddx(input.uv);
    const float2 uvDy = ddy(input.uv);
    float3 shadingNormal = input.normal;
    float3 worldNormal = input.worldNormal;
    const float normalMapScale = input.worldTangentAndNormalMapScale.w;
    if (normalMapScale > 0.0 && input.tangent.w != 0.0)
    {
        float3 mapped = normalMap.SampleGrad(normalSampler, input.uv, uvDx, uvDy).xyz * 2.0 - 1.0;
        mapped.xy *= normalMapScale;
        const float handedness = input.tangent.w > 0.0 ? 1.0 : -1.0;
        // Keeps the interpolated normal's length, which the Classic diffuse curve reads.
        shadingNormal = PerturbNormal(input.normal, input.tangent.xyz, handedness, mapped) * length(input.normal);
        worldNormal = PerturbNormal(input.worldNormal, input.worldTangentAndNormalMapScale.xyz, handedness, mapped);
    }

    const float3 objectLight = input.color.rgb;
    const float3 normal = SafeNormalize(shadingNormal);
    const float3 view = SafeNormalize(input.viewVector);
    // Neither the per-object direction nor the skinned normal is unit length in Classic. The
    // diffuse term uses both as they are, so that it is the Classic per-vertex value wherever the
    // curve is not clamped (dot() is linear in the interpolated normal) and keeps the palette.
    const float3 light = input.lightDirectionAndSpecular.xyz;
    const float lightDot = dot(shadingNormal, light);

    const float luminosity = max(lightDot * diffuseCurve.x + diffuseCurve.y, diffuseCurve.z);
    const float3 dynamicLight = input.worldPositionAndDynamicLights.w > 0.5
                                    ? DynamicLighting(input.worldPositionAndDynamicLights.xyz,
                                                      SafeNormalize(worldNormal))
                                    : float3(0.0, 0.0, 0.0);
    float3 diffuse = saturate(objectLight * luminosity + dynamicLight);
    if (RollsOffHighlight(diffuseCurve.w))
        diffuse = RollOffHighlight(objectLight * luminosity + dynamicLight, diffuseCurve.w);

    // Two-sided meshes show back faces; |dot| keeps their interiors from rim-lighting.
    const float facing = saturate(abs(dot(normal, view)));
    const float3 rim = rimParameters.x * pow(1.0 - facing, rimParameters.y) * objectLight;

    const float specularStrength = input.lightDirectionAndSpecular.w * specularParameters.x;
    float3 specular = float3(0.0, 0.0, 0.0);
    if (specularStrength > 0.0 && lightDot > 0.0)
    {
        const float3 halfVector = SafeNormalize(SafeNormalize(light) + view);
        const float highlight = pow(saturate(dot(normal, halfVector)), specularParameters.y);
        specular = specularStrength * highlight * sceneLightColor.rgb * objectLight;
    }

    // Rim and specular are light added on top of the surface, so dark textures still get an
    // outline; both are bounded by the object light.
    float4 color = float4(texel.rgb * diffuse + rim + specular, texel.a * input.color.a);
    if (alphaToCoverage)
    {
        // Anti-aliased cutout, as in basic_textured.frag: coverage keeps the
        // texel's translucency; only the alpha test's step becomes a ramp.
        const float edge = saturate((color.a - alphaThreshold) / max(fwidth(color.a), 0.0001) + 0.5);
        color.a *= edge;
        if (color.a <= 0.0) discard;
    }
    else if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;
    if (fogEnabled && sceneFogColor.w > 0.0)
        color.rgb = lerp(color.rgb, sceneFogColor.rgb, EnhancedFogAmount(input.eyeDepth));
    return color;
}
