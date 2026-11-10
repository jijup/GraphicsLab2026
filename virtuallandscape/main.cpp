#include <cmath>
#include <cstdio>
#include <algorithm>
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"
#include "Helpers/Matrices.h"
#include "Helpers/Capture.h"
#include "Terrain.h"
#include "Skybox.h"

///--- Window
const int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720;
const float PI = 3.14159265f;

///--- Scene
Terrain terrain;
Skybox skybox;
Vec3 sunDir = Vec3(0.55f, 0.45f, 0.35f).normalized();   ///< toward the sun
float simTime = 0.0f;     ///< seconds since the start, drives the animation
bool wireframe = false;

///--- Camera. The world is Z-up.
struct Camera {
    Vec3 pos;
    float yaw;      ///< angle around z, 0 looks along +y
    float pitch;    ///< angle above the horizon
    float fov;      ///< vertical field of view, degrees
    float speed;    ///< world units per second

    /// Unit vector the camera looks along
    Vec3 front() const {
        return Vec3(std::sin(yaw) * std::cos(pitch),
                    std::cos(yaw) * std::cos(pitch),
                    std::sin(pitch));
    }
};
Camera camera;
const Vec3 worldUp(0.0f, 0.0f, 1.0f);

/// Places the camera at "eye", looking at "target".
void placeCamera(const Vec3& eye, const Vec3& target) {
    camera.pos = eye;
    Vec3 d = (target - eye).normalized();
    camera.yaw = std::atan2(d.x(), d.y());
    camera.pitch = std::asin(d.z());
}

/// The reference view: used for screenshots, and when you press R.
void resetCamera() {
    placeCamera(Vec3(-4.4f, -4.6f, 1.55f), Vec3(0.6f, 0.4f, 0.0f));
    camera.fov = 60.0f;
    camera.speed = 1.0f;
}

///--- Mouse state
bool mouseDown = false;
double lastX = 0.0, lastY = 0.0;

/// Called every frame. dt is the time since the last frame, in seconds.
void updateCamera(GLFWwindow* window, float dt) {
    ///TODO C8 (part 1): Move the camera while W, A, S or D is held down.
    // Test a key with glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS.
    // W and S move along camera.front(), A and D along the "right" vector front x worldUp (normalized).
    // Move camera.speed * dt per frame, so the speed does not depend on the frame rate.
    (void)window; (void)dt;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_R) resetCamera();
    if (key == GLFW_KEY_F && action == GLFW_PRESS) wireframe = !wireframe;
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        static int count = 0;
        char name[32];
        snprintf(name, sizeof(name), "screenshot_%03d.png", count++);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        saveScreenshot(name, w, h);
    }
    ///TODO C8 (part 2): The arrow keys.
    // UP and DOWN change camera.fov (zoom in and out). Keep it between 10 and 90 degrees.
    // RIGHT and LEFT make camera.speed faster or slower (for example times or divided by 1.25).
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mouseDown = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &lastX, &lastY);
    }
}

/// Called when the mouse moves. Look around while the left button is held down.
void cursorPosCallback(GLFWwindow*, double x, double y) {
    if (!mouseDown) return;
    float dx = float(x - lastX), dy = float(y - lastY);
    lastX = x;
    lastY = y;
    ///TODO C8 (part 3): Mouse look. dx and dy are how far the mouse moved, in pixels.
    // Add dx * sensitivity to camera.yaw and subtract dy * sensitivity from camera.pitch (try 0.004).
    // Clamp the pitch to just inside (-PI / 2, PI / 2), so the camera cannot flip over.
    (void)dx; (void)dy;
}

void init() {
    glClearColor(0.62f, 0.75f, 0.88f, 1.0f);   // plain sky blue
    glEnable(GL_DEPTH_TEST);
    skybox.init();
    terrain.init();
    resetCamera();
}

void display(int width, int height) {
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SceneUniforms s;
    s.cameraPos = camera.pos;
    s.V = lookAt(camera.pos, camera.pos + camera.front(), worldUp);
    s.P = perspective(camera.fov, width / float(height), 0.01f, 50.0f);
    s.sunDir = sunDir;
    s.time = simTime;

    skybox.draw(s.V, s.P);

    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    terrain.draw(s);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    check_error_gl();
}

void cleanup() {
    terrain.cleanup();
    skybox.cleanup();
}

int main(int argc, char** argv) {
    CaptureOptions capture;
    capture.parse(argc, argv);
    wireframe = capture.wire;

    glewExperimental = true;   // for core profile
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);   // needed on macOS

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Virtual Landscape", NULL, NULL);
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
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    init();
    if (capture.camera)
        placeCamera(Vec3(capture.eye[0], capture.eye[1], capture.eye[2]),
                    Vec3(capture.target[0], capture.target[1], capture.target[2]));

    double lastTime = glfwGetTime();
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        // Time since the last frame. When recording, use a fixed step instead.
        double now = glfwGetTime();
        float dt = capture.shot.empty() ? float(now - lastTime) : capture.dt;
        lastTime = now;
        simTime += dt;
        if (!capture.shot.empty()) simTime = frame * capture.dt;

        updateCamera(window, dt);

        // On a Mac (Retina) the framebuffer has more pixels than the window
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        display(width, height);

        if (!capture.shot.empty()) {
            saveScreenshot(capture.frameName(frame), width, height);
            if (frame + 1 >= capture.frames) glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        ++frame;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    glfwTerminate();
    return EXIT_SUCCESS;
}
