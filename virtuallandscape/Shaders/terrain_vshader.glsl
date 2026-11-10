#version 330 core
in vec3 vposition;
in vec2 vtexcoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform sampler2D noiseTex;   ///< height map, values in [-1, 1]
uniform float heightScale;    ///< world height of a height map value of 1
uniform float seaLevel;       ///< world height of the water surface

out vec2 uv;
out vec3 fragPos;             ///< world position

void main() {
    uv = vtexcoord;
    float h = 0.0;

    ///TODO C3: Displace the vertex. Read the height map at uv and scale it by heightScale.
    // Use textureLod(noiseTex, uv, 0.0).r: a vertex shader has no mipmap level of its own.

    ///TODO C6: Flatten everything below sea level to the water surface: h = max(h, seaLevel).

    fragPos = (M * vec4(vposition.xy, h, 1.0)).xyz;
    gl_Position = P * V * vec4(fragPos, 1.0);
}
