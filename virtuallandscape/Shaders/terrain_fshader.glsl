#version 330 core
in vec2 uv;
in vec3 fragPos;              ///< world position

uniform sampler2D noiseTex;   ///< height map, values in [-1, 1]
uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D sand;
uniform sampler2D snow;
uniform sampler2D water;      ///< a normal map

uniform vec3 viewPos;         ///< camera position
uniform vec3 sunDir;          ///< unit vector toward the sun
uniform float time;           ///< seconds
uniform float heightScale;
uniform float seaLevel;
uniform float gridSize;       ///< world size of the whole grid

out vec4 color;

///--- Texturing levels, in world height above sea level
const float SAND_TOP  = 0.06;
const float GRASS_TOP = 0.42;
const float SNOW_LINE = 0.70;
const float TEX_REPEAT = 48.0;   ///< how many times the textures repeat across the terrain

///--- Light: I is the sun, I_a the ambient (sky) light
const vec3 I   = vec3(1.00, 0.95, 0.85);
const vec3 I_a = vec3(0.32, 0.36, 0.45);

void main() {
    // World height of the ground at this fragment (below the water too)
    float h = heightScale * texture(noiseTex, uv).r;

    vec3 n = vec3(0.0, 0.0, 1.0);                   // surface normal
    vec3 albedo = vec3(0.5 + 0.5 * h / heightScale); // grey: dark low, light high (until C5)
    vec3 k_s = vec3(0.0);                           // specular color
    float p = 1.0;                                  // specular exponent

    ///TODO C4 (part 1): The surface normal, by finite differences on the height map.
    // One texel is texel = 1.0 / vec2(textureSize(noiseTex, 0)).
    // Read the height (times heightScale) one texel to the left and right, and one texel down and up.
    // dz/dx = (h_right - h_left) / (2 * texel.x * gridSize). Note texel.x * gridSize is one texel in world units.
    // dz/dy the same way, then n = normalize(vec3(-dz/dx, -dz/dy, 1.0)).

    ///TODO C5: Texture by height and slope.
    // 1. Read the four textures at uv * TEX_REPEAT, so they repeat across the terrain.
    // 2. slope = 1.0 - n.z is 0 on flat ground and grows on steep ground.
    // 3. Blend by height above sea level (h - seaLevel) with mix() and smoothstep(): sand below SAND_TOP, grass up to GRASS_TOP,
    //    then rock, then snow above SNOW_LINE.
    // 4. Steep slopes show rock. Snow only stays on flatter ground.
    // 5. Give the snow a little shine: a small k_s and a larger p.
    // Tip: add a little noise to h first (for example from noiseTex at uv * 7.0), so the borders are not straight lines.

    ///TODO C6: Water. If the ground is below seaLevel, this fragment is water.
    // 1. Read the water normal map twice, at two scales of uv, each moving with time in its own direction.
    // 2. Turn each color into a normal (rgb * 2 - 1) and combine them:
    //    n = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)). The water is flat, so no tangent frame is needed.
    // 3. Water color: lighter where the water is shallow (seaLevel - h is small), darker where it is deep.
    // 4. A strong, sharp highlight: k_s close to 1 and a large p (a few hundred).

    vec3 c_out = albedo;
    ///TODO C4 (part 2): Blinn-Phong with the sun as a directional light, the same as Assignment 1:
    //    c = k_a I_a + k_d I max(0, n.l) + k_s I max(0, n.h)^p
    // with k_a = k_d = albedo, l = sunDir, v = the unit vector from fragPos to viewPos, h = normalize(l + v).
    color = vec4(c_out, 1.0);
}
