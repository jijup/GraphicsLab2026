#version 330 core
in vec3 uvw;

out vec4 color;

uniform samplerCube skybox;

void main() {
    ///TODO C7: Look up the sky color in the cubemap: texture(skybox, uvw).
    // Until then, the sky is the same plain blue as the background (glClearColor in main.cpp).
    color = vec4(0.62, 0.75, 0.88, 1.0);
}
