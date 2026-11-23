#version 330 core
in vec3 fragPos;    ///< world position
in vec3 normal;     ///< world normal, interpolated from the vertices

uniform vec3 sunDir;      ///< direction to the sun (unit length)
uniform vec3 I;           ///< sunlight
uniform vec3 I_a;         ///< ambient light
uniform vec3 viewPos;     ///< camera position
uniform float seaLevel;
uniform sampler2D sandTex;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D snowTex;

out vec4 color;

///--- Material of the terrain. k_a and k_d are the colour of the ground (from Task 2).
const vec3 k_s = vec3(0.10);   ///< specular colour: the ground is not shiny
const float p = 16.0;          ///< specular exponent

///--- Texturing levels (world heights; the sea is at 0)
const float SAND_TOP = 0.10;   ///< sand below, grass above
const float GRASS_TOP = 0.52;  ///< grass below, rock above
const float SNOW_LINE = 0.80;  ///< rock below, snow above
const float BLEND = 0.05;      ///< half width of the blend between two layers
const float TEX_REPEAT = 1.5;  ///< texture repeats per world unit

///--- The round edge of the world: the same sky colour as glClearColor in main.cpp
const vec3 SKY = vec3(0.88, 0.80, 0.72);
const float EDGE_START = 3.55, EDGE_END = 3.98;

///--- Blinn-Phong: ambient + diffuse + specular, for the directional light of the sun.
vec3 blinnPhong(vec3 n, vec3 k_a, vec3 k_d) {
    ///TODO Task 1b: Blinn-Phong, in the notation of the lectures:
    //    L = k_a I_a  +  k_d I max(0, n . l)  +  k_s I max(0, n . h)^p
    // with l the direction to the light (sunDir), v the unit direction from fragPos to the camera
    // (viewPos), and h the unit half vector between l and v. n is already unit length.
    // k_s and p are the constants at the top of this file.
    // Until then, the terrain shows its colour with no lighting at all.
    return k_d;
}

///--- The colour of the ground at height h. uv are the texture coordinates.
vec3 terrainColor(float h, vec2 uv) {
    vec3 sand = texture(sandTex, uv).rgb;
    vec3 grass = texture(grassTex, uv).rgb;
    vec3 rock = texture(rockTex, uv).rgb;
    vec3 snow = texture(snowTex, uv).rgb;
    ///TODO Task 2: Texturing by height. Start with sand, then blend in the next layer above each level:
    //    grass around SAND_TOP, rock around GRASS_TOP, snow around SNOW_LINE.
    // For each level, the weight of the upper layer is smoothstep(level - BLEND, level + BLEND, h):
    // 0 below the blend band, 1 above it. Blend with mix(lower, upper, weight).
    // Until then, the ground is a plain grey brown.
    return vec3(0.72, 0.68, 0.62);
}

void main() {
    vec3 n = normalize(normal);   // interpolation makes the normal a little shorter than 1
    vec3 ground = terrainColor(fragPos.z, fragPos.xy * TEX_REPEAT);
    vec3 c = blinnPhong(n, ground, ground);

    // Under the sea (given): the light gets bluer and dimmer with depth.
    float depth = seaLevel - fragPos.z;
    if (depth > 0.0)
        c = mix(c * vec3(0.55, 0.80, 0.85), vec3(0.02, 0.10, 0.18), smoothstep(0.0, 0.45, depth));

    // The edge of the world (given): fade into the sky colour near the border of the grid.
    c = mix(c, SKY, smoothstep(EDGE_START, EDGE_END, length(fragPos.xy)));

    color = vec4(c, 1.0);
}
