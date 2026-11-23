#version 330 core
in vec3 fragPos;      ///< world position
in vec3 waveNormal;   ///< normal of the wave surface (flat until Task 4)
in vec2 uv;           ///< texture coordinates

uniform vec3 sunDir;      ///< direction to the sun (unit length)
uniform vec3 I;           ///< sunlight
uniform vec3 I_a;         ///< ambient light
uniform vec3 viewPos;     ///< camera position
uniform float time;       ///< seconds since the start
uniform sampler2D waterTex;    ///< colour of the water
uniform sampler2D normalTex;   ///< small ripples, as a normal map

out vec4 color;

///--- Material of the water: dark, very shiny, and see-through
const vec3 k_s = vec3(0.9);        ///< specular colour
const float p = 300.0;             ///< specular exponent: a small, sharp highlight
const float RIPPLE = 0.5;          ///< how much the normal map tilts the normal
const float WATER_ALPHA = 0.72;    ///< opacity: 1 hides the ground under the water

void main() {
    ///TODO Task 3a: Moving water. Read waterTex three times, at uv moved by a time dependent offset:
    //    uv1 = uv + (0.2  (1 + sin(time / 5)),  0.2  (1 + sin(time / 10)))
    //    uv2 = uv + (0.1  (1 + sin(time / 2)),  0.1  (1 + sin(time / 5)))
    //    uv3 = uv + (0.05 (1 + sin(time)),      0.05 (1 + sin(time / 2)))
    // and blend the three colours with the weights 0.5, 0.3 and 0.2.
    // Until then, the water is one plain colour.
    vec3 waterColor = vec3(0.10, 0.35, 0.50);

    vec3 n = normalize(waveNormal);
    vec3 c = waterColor;
    ///TODO Task 3b: Ripples and light.
    // 1. Read the normal map at uv * 3.0 + time * vec2(0.04, 0.03), so it drifts slowly.
    //    A normal map stores a unit vector in the colour: turn [0, 1] back into [-1, 1] with * 2.0 - 1.0.
    // 2. Tilt the wave normal by the ripple: n = normalize(n + RIPPLE * vec3(ripple.xy, 0.0)).
    // 3. Light the water with Blinn-Phong, as in Task 1b, with k_a = k_d = waterColor,
    //    and k_s and p from the top of this file.

    // The edge of the world (given): the water fades out near the border of the grid, as the ground does.
    float alpha = WATER_ALPHA * (1.0 - smoothstep(3.55, 3.98, length(fragPos.xy)));
    color = vec4(c, alpha);
}
