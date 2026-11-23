#version 330 core
in vec3 vposition;

uniform mat4 V;
uniform mat4 P;
uniform int mode;             ///< 1 Perlin, 2 fBm, 3 ridged multifractal
uniform int octaves;          ///< K
uniform float H;              ///< roughness
uniform float lacunarity;     ///< L
uniform float baseFrequency;  ///< noise cells per world unit at octave 0
uniform float heightScale;
uniform float gridStep;       ///< distance between two neighbouring vertices (8 / 256)

out vec3 fragPos;             ///< world position
out vec3 normal;              ///< world normal (Task 1a)

// The noise functions below are your solutions to Recitation 07.

///--- Random gradients (given). A hash turns the integer corner (i, j) into
/// a random angle, so every corner gets its own unit gradient, always the same one.
uint hash(uint x) {
    x ^= x >> 16; x *= 0x7feb352du;
    x ^= x >> 15; x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}
vec2 gradient(ivec2 corner) {
    uint h = hash(uint(corner.x) * 1597334677u ^ hash(uint(corner.y) * 3812015801u));
    float angle = float(h) * (6.2831853 / 4294967296.0);
    return vec2(cos(angle), sin(angle));
}

///--- Fade curve for the interpolation weights
float fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);   // quintic
    // return t * t * (3.0 - 2.0 * t);                  // cubic
    // return t;                                        // no fade
}

///--- 2D Perlin noise at point p. Lattice corners are at whole numbers. Returns roughly [-0.7, 0.7].
float perlin(vec2 p) {
    ivec2 c0 = ivec2(floor(p));
    vec2 d = p - floor(p);

    float s = dot(gradient(c0),               d);
    float t = dot(gradient(c0 + ivec2(1, 0)), d - vec2(1.0, 0.0));
    float u = dot(gradient(c0 + ivec2(0, 1)), d - vec2(0.0, 1.0));
    float v = dot(gradient(c0 + ivec2(1, 1)), d - vec2(1.0, 1.0));

    float st = mix(s, t, fade(d.x));   // along x, bottom edge
    float uv = mix(u, v, fade(d.x));   // along x, top edge
    return mix(st, uv, fade(d.y));     // along y
}

///--- Fractional Brownian motion: K octaves of Perlin noise
float fBm(vec2 p) {
    float value = 0.0;
    float frequency = 1.0;
    for (int k = 0; k < octaves; ++k) {
        value += pow(lacunarity, -float(k) * H) * perlin(frequency * p);
        frequency *= lacunarity;
    }
    return value;
}

///--- Ridged multifractal (Musgrave): sharp crests where the noise crosses zero
float ridged(vec2 p) {
    float result = 0.0;
    float weight = 1.0;
    float frequency = 1.0;
    for (int k = 0; k < octaves; ++k) {
        float signal = 1.0 - abs(perlin(frequency * p));
        signal *= signal;          // sharpen the crests
        signal *= weight;          // less detail where the previous octave was low
        result += signal * pow(lacunarity, -float(k) * H);
        weight = clamp(signal * 2.0, 0.0, 1.0);
        frequency *= lacunarity;
    }
    return result * 0.8 - 1.0;
}

///--- The island (given). 1 in the middle, falling smoothly to 0 near the edges of the grid.
float islandShape(vec2 xy) {
    float r = length(xy) / 4.0;   // 0 in the middle, 1 at 4 units
    return 1.0 - smoothstep(0.3, 1.0, r);
}

///--- Height of the terrain (world z) at the point xy (given).
/// The noise of Recitation 07, raised in the middle and pushed under the sea near the edges.
float terrainHeight(vec2 xy) {
    vec2 p = xy * baseFrequency;
    float n;
    if (mode == 1)      n = perlin(p);
    else if (mode == 2) n = fBm(p);
    else                n = ridged(p);
    float s = islandShape(xy);
    return heightScale * (n + 0.9) * s - 0.6 * (1.0 - s) - 0.15;
}

void main() {
    vec2 xy = vposition.xy;
    float z = terrainHeight(xy);

    float eps = gridStep;   // step for the finite differences: one grid cell
    ///TODO Task 1a: The normal, by finite differences of terrainHeight().
    // 1. A tangent along x: from the point eps to the left to the point eps to the right,
    //      tx = (2 eps, 0, terrainHeight(xy + (eps, 0)) - terrainHeight(xy - (eps, 0)))
    // 2. A tangent along y, the same way, with (0, eps).
    // 3. The normal is perpendicular to both: normalize(cross(tx, ty)).
    //    Take care of the order: the normal must point up (positive z) on flat ground.
    normal = vec3(0.0, 0.0, 1.0);

    fragPos = vec3(xy, z);
    gl_Position = P * V * vec4(fragPos, 1.0);
}
