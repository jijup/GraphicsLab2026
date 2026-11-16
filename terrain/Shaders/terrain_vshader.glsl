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

out float height;             ///< noise value at this vertex, for the colors
out vec3 fragPos;             ///< world position

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
    ///TODO Task 2a: The quintic fade curve 6t^5 - 15t^4 + 10t^3.
    // Later, try "return t;" (no fade) and the cubic 3t^2 - 2t^3, and compare the contour lines.
    return t;
}

///--- 2D Perlin noise at point p. Lattice corners are at whole numbers. Returns roughly [-0.7, 0.7].
float perlin(vec2 p) {
    ///TODO Task 2b: Perlin noise.
    // 1. The cell corner below and left of p: c0 = ivec2(floor(p)). The others are c0 + (1, 0), (0, 1), (1, 1).
    // 2. The position of p inside the cell: d = p - floor(p), both components in [0, 1).
    // 3. For each corner, the dot product of its gradient(...) with the vector from the corner to p,
    //    for example dot(gradient(c0 + ivec2(1, 0)), d - vec2(1, 0)).
    // 4. Blend the four values with mix(), along x with fade(d.x), then along y with fade(d.y).
    return 0.0;
}

///--- Fractional Brownian motion: K octaves of Perlin noise
float fBm(vec2 p) {
    ///TODO Task 3a: fBm. Add up "octaves" copies of the noise:
    //    value += w_k * perlin(f_k * p),  with  f_k = lacunarity^k  and  w_k = lacunarity^(-k * H)
    // Until then, fBm is a single octave.
    return perlin(p);
}

///--- Ridged multifractal (Musgrave): sharp crests where the noise crosses zero
float ridged(vec2 p) {
    ///TODO Task 4: Ridged multifractal.
    // For each octave k: signal = (1 - |perlin(f_k * p)|)^2, then multiply it by "weight".
    // Add signal * lacunarity^(-k * H) to the result, and set weight = clamp(2 * signal, 0, 1)
    // for the next octave. Start with weight = 1. Return result * 0.8 - 1.0, so it sits around [-1, 1].
    // Until then, ridged is plain fBm.
    return fBm(p);
}

void main() {
    vec2 p = vposition.xy * baseFrequency;
    if (mode == 1)      height = perlin(p);
    else if (mode == 2) height = fBm(p);
    else                height = ridged(p);

    float z = 0.0;
    ///TODO Task 3b: Displace the vertex: z = heightScale * height.
    // Then set GRID_CELLS in Terrain.h to 256: the noise is only computed at the vertices,
    // so a coarse grid cannot show the fine detail of the higher octaves.
    fragPos = vec3(vposition.xy, z);
    gl_Position = P * V * vec4(fragPos, 1.0);
}
