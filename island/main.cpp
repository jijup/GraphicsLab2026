#include <cmath>
#include <cstdio>
#include <algorithm>
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"
#include "Helpers/Matrices.h"
#include "Helpers/Capture.h"
#include "Terrain.h"
#include "Water.h"

/// Recitation 08: lighting, texturing and water, on the noise terrain of Recitation 07.
/// Your work is in the four shaders in Shaders/.
///
/// Keys:
///   1, 2, 3       Perlin noise, fBm, ridged multifractal
///   UP, DOWN      more or fewer octaves
///   LEFT, RIGHT   turn the terrain
///   SPACE         spin on and off
///   W             waves on and off (Task 4)
///   F             wireframe
///   P             save a screenshot (screenshot_000.png, ...)

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;

Terrain terrain;
Water water;
NoiseSettings noise;
Light light;
bool wireframe = false;
bool spinning = false;
bool waves = true;

///--- Orbit camera: it circles the origin and looks at it. The world is Z-up.
float azimuth = 0.70f;      ///< angle around z
float elevation = 0.62f;    ///< angle above the ground
float distance = 16.5f;

Vec3 cameraPosition() {
    return Vec3(distance * std::cos(elevation) * std::sin(azimuth),
               -distance * std::cos(elevation) * std::cos(azimuth),
                distance * std::sin(elevation));
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_LEFT)  azimuth -= 0.05f;
    if (key == GLFW_KEY_RIGHT) azimuth += 0.05f;
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_1) noise.mode = 1;
    if (key == GLFW_KEY_2) noise.mode = 2;
    if (key == GLFW_KEY_3) noise.mode = 3;
    if (key == GLFW_KEY_UP)   noise.octaves = std::min(10, noise.octaves + 1);
    if (key == GLFW_KEY_DOWN) noise.octaves = std::max(1, noise.octaves - 1);
    if (key == GLFW_KEY_SPACE) spinning = !spinning;
    if (key == GLFW_KEY_W) waves = !waves;
    if (key == GLFW_KEY_F) wireframe = !wireframe;
    if (key == GLFW_KEY_P) {
        static int count = 0;
        char name[32];
        snprintf(name, sizeof(name), "screenshot_%03d.png", count++);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        saveScreenshot(name, w, h);
    }
    const char* names[] = { "", "Perlin", "fBm", "ridged" };
    printf("%s noise, %d octaves, waves %s\n", names[noise.mode], noise.octaves, waves ? "on" : "off");
}

void display(int width, int height, float time) {
    glViewport(0, 0, width, height);
    glClearColor(0.88f, 0.80f, 0.72f, 1.0f);   // evening sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Vec3 eye = cameraPosition();
    Mat4x4 V = lookAt(eye, Vec3(0, 0, -1.2f), Vec3(0, 0, 1));   // aim a little below the middle, to center the terrain
    Mat4x4 P = perspective(40.0f, width / float(height), 0.1f, 50.0f);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    terrain.draw(V, P, eye, noise, light);
    water.draw(V, P, eye, light, time, waves);   // after the terrain, because it is see-through
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    check_error_gl();
}

int main(int argc, char** argv) {
    CaptureOptions capture;   // --shot file.png saves one frame and quits (used for the wiki figures)
    capture.parse(argc, argv);
    wireframe = capture.wire;

    glewExperimental = true;
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);   // needed on macOS
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Recitation 08: Island", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. Your GPU or driver may not support OpenGL 3.3.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    glGetError();   // GLEW can leave a harmless error behind
    glfwSetKeyCallback(window, keyCallback);

    terrain.init();
    water.init();

    double startTime = glfwGetTime();
    double lastTime = startTime;
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        if (spinning) azimuth += 0.15f * float(now - lastTime);
        lastTime = now;
        // Seconds since the start. When recording frames, the time steps by exactly capture.dt per frame.
        float time = capture.shot.empty() ? float(now - startTime) : frame * capture.dt;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        display(width, height, time);

        if (!capture.shot.empty()) {
            saveScreenshot(capture.frameName(frame), width, height);
            if (frame + 1 >= capture.frames) glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        ++frame;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    water.cleanup();
    terrain.cleanup();
    glfwTerminate();
    return EXIT_SUCCESS;
}
