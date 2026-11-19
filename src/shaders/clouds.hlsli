// clouds.hlsli — the shadows of drifting clouds: how much of the sun's light reaches a point on the ground
// through a soft, procedural cloud cover. The same function lights the ground and the characters,
// so a cloud's shadow falls on all of them alike.
#ifndef MU_CLOUDS_HLSLI
#define MU_CLOUDS_HLSLI

float CloudHash(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

// Smooth value noise in [0, 1].
float CloudNoise(float2 p)
{
    const float2 cell = floor(p);
    const float2 f = frac(p);
    const float2 u = f * f * (3.0 - 2.0 * f);
    const float a = CloudHash(cell);
    const float b = CloudHash(cell + float2(1.0, 0.0));
    const float c = CloudHash(cell + float2(0.0, 1.0));
    const float d = CloudHash(cell + float2(1.0, 1.0));
    return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}

// The share of the sun's light that reaches world point `xy`: 1 in clear sky, 1 - strength under a cloud.
// `parameters`: x strength (0: no clouds), y 1 / pattern size in world units, z sky cover (0 to 1).
// `offset`: the clouds' drift plus the sun's slant through the cloud layer, in world units.
float CloudLight(float2 xy, float4 parameters, float2 offset)
{
    if (parameters.x <= 0.0)
        return 1.0;
    const float2 p = (xy + offset) * parameters.y;
    const float cover = 0.55 * CloudNoise(p) + 0.3 * CloudNoise(p * 2.03 + 17.1) + 0.15 * CloudNoise(p * 4.07 + 31.7);
    const float threshold = 1.0 - parameters.z;
    const float cloud = smoothstep(threshold - 0.12, threshold + 0.12, cover);
    return 1.0 - parameters.x * cloud;
}
#endif
