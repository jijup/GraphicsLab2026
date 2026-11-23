#version 330 core
in vec3 vposition;

uniform mat4 V;
uniform mat4 P;
uniform float seaLevel;
uniform float time;       ///< seconds since the start
uniform int waves;        ///< 1: waves on, 0: off (key W)

out vec3 fragPos;         ///< world position
out vec3 waveNormal;      ///< normal of the wave surface (Task 4)
out vec2 uv;              ///< texture coordinates (Task 3)

const float WATER_TILE = 2.0;   ///< world size of one copy of the water textures

///--- Waves (Task 4): direction D, wavelength, amplitude A and speed of each wave
const int NUM_WAVES = 4;
const vec2 DIR[NUM_WAVES] = vec2[](vec2(1.0, 0.35), vec2(0.6, 1.0), vec2(-0.7, 0.8), vec2(1.0, -0.6));
const float WAVELENGTH[NUM_WAVES] = float[](1.7, 1.1, 0.75, 0.5);
const float AMPLITUDE[NUM_WAVES] = float[](0.030, 0.018, 0.010, 0.006);
const float SPEED[NUM_WAVES] = float[](0.45, 0.36, 0.30, 0.24);

void main() {
    vec2 xy = vposition.xy;
    float z = seaLevel;
    vec3 n = vec3(0.0, 0.0, 1.0);   // flat water

    ///TODO Task 4: Waves, as a sum of sines. When waves == 1, for each wave i:
    //    D = normalize(DIR[i]),  k = 2 pi / WAVELENGTH[i],  w = SPEED[i] * k,
    //    phase = k (D . xy) - w time,     z += AMPLITUDE[i] sin(phase)
    // The slopes are the derivatives of the same sum. Add them up in the same loop:
    //    dz/dx += AMPLITUDE[i] k D.x cos(phase),     dz/dy += AMPLITUDE[i] k D.y cos(phase)
    // After the loop, n = normalize(vec3(-dz/dx, -dz/dy, 1.0)).

    uv = xy / WATER_TILE;
    waveNormal = n;
    fragPos = vec3(xy, z);
    gl_Position = P * V * vec4(fragPos, 1.0);
}
