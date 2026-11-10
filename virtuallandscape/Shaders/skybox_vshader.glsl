#version 330 core
in vec3 vposition;

out vec3 uvw;   ///< direction to look up in the cubemap

uniform mat4 V;
uniform mat4 P;

void main() {
    // Our world is Z-up. Cubemaps are Y-up, with -Z forward.
    // World (x, y, z) is cubemap (x, z, -y).
    uvw = vec3(vposition.x, vposition.z, -vposition.y);

    ///TODO C7: Keep the sky at infinity.
    // 1. Remove the translation from V, so the cube moves with the camera: mat4(mat3(V)).
    // 2. Write gl_Position = clip.xyww, where clip = P * (that matrix) * vec4(vposition, 1).
    //    Then z / w = 1 after the perspective divide: the sky sits on the far plane.
    gl_Position = P * V * vec4(vposition, 1.0);
}
