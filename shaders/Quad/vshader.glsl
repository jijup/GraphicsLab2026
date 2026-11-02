#version 330 core
// Quad/vshader.glsl: runs once for every vertex of the quad
in vec3 vpoint;       // vertex position (from the VBO)
in vec2 vtexcoord;    // texture coordinate (from the VBO)

out vec2 uv;          // passed on to the fragment shader

uniform float time;   // seconds since the program started
uniform int id;       // 0: pumpkin, 1, 2, 3: bats

///--- Scaling by sx along x and sy along y (given)
// GLSL stores matrices column by column: M[c][r] is column c, row r.
mat4 S(float sx, float sy){
    mat4 M = mat4(1.0);   // identity
    M[0][0] = sx;
    M[1][1] = sy;
    return M;
}

///TODO5: Translation by (tx, ty). Where do tx and ty go in a column-major matrix?
mat4 T(float tx, float ty){
    mat4 M = mat4(1.0);
    return M;
}

///TODO6: Rotation by "degrees" about the z axis (counterclockwise). Use radians().
mat4 R(float degrees){
    mat4 M = mat4(1.0);
    return M;
}

void main() {
    if (id == 0) {
        ///--- The pumpkin sits at the bottom of the screen (given)
        gl_Position = T(0.0, -0.4) * S(1.2, 1.2) * vec4(vpoint, 1.0);
    }
    else {
        ///--- The bats wait in a row near the top, each tilted a little
        float x = -0.6 + 0.6 * float(id - 1);       // id 1, 2, 3 -> x = -0.6, 0.0, 0.6
        float tilt = 20.0 - 20.0 * float(id - 1);   // id 1, 2, 3 -> 20, 0, -20 degrees
        gl_Position = T(x, 0.55) * R(tilt) * S(0.45, 0.45) * vec4(vpoint, 1.0);

        ///TODO7: Make the bats fly. Replace the three lines above with a flight
        // path that changes with time (see the recitation page for the formulas).
    }
    uv = vtexcoord;
}
