#version 330 core
// Quad/fshader.glsl: runs once for every pixel (fragment) the quad covers
in vec2 uv;           // texture coordinate, interpolated from the vertex shader
out vec4 color;

uniform sampler2D tex;
uniform float time;   // seconds since the program started
uniform int id;       // 0: pumpkin, 1, 2, 3: bats

void main() {
    color = texture(tex, uv);

    ///TODO3: The texels around the pumpkin and the bat are transparent (alpha = 0).
    // Throw these fragments away with "discard", so the quad's square corners vanish.

    ///TODO4: Make the pumpkin (id 0) flicker like a candle: multiply its rgb by a
    // brightness that changes with time, e.g. 0.85 + 0.15 * sin(20.0 * time) * sin(7.0 * time)

}
