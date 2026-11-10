#pragma once
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <GL/glew.h>
#include "Helpers/lodepng.h"

/// Screenshots and recorded frames. You do not need to change this file.
///
/// While the program runs, press P to save screenshot_000.png, screenshot_001.png, ...
/// next to the executable.
///
/// From the command line (or Visual Studio: Project > Properties > Debugging > Command Arguments):
///   virtuallandscape --shot terrain.png              render one frame from the reference view, save it, quit
///   virtuallandscape --shot water.png --frames 150   save water_0000.png ... water_0149.png at 30 frames per second
///   virtuallandscape --shot grid.png --wire          the same, in wireframe
///   virtuallandscape --shot top.png --camera 0 -0.5 9  0 0.5 0   camera at (0, -0.5, 9) looking at (0, 0.5, 0)
/// Turn frames into a video with ffmpeg:
///   ffmpeg -framerate 30 -i water_%04d.png -pix_fmt yuv420p water.mp4

struct CaptureOptions {
    std::string shot;        ///< output file, empty for normal interactive use
    int frames = 1;          ///< number of frames to record
    float dt = 1.0f / 30.0f; ///< time step between recorded frames (seconds)
    bool wire = false;       ///< wireframe
    bool camera = false;     ///< use the camera below instead of the reference view
    float eye[3], target[3]; ///< camera position, and the point it looks at
    std::vector<std::string> extra;  ///< any other arguments (used by the advanced features)

    void parse(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "--shot" && i + 1 < argc) shot = argv[++i];
            else if (a == "--frames" && i + 1 < argc) frames = std::atoi(argv[++i]);
            else if (a == "--dt" && i + 1 < argc) dt = float(std::atof(argv[++i]));
            else if (a == "--wire") wire = true;
            else if (a == "--camera" && i + 6 < argc) {
                camera = true;
                for (int k = 0; k < 3; ++k) eye[k] = float(std::atof(argv[++i]));
                for (int k = 0; k < 3; ++k) target[k] = float(std::atof(argv[++i]));
            }
            else extra.push_back(a);
        }
    }
    bool has(const std::string& flag) const {
        for (const std::string& e : extra) if (e == flag) return true;
        return false;
    }
    /// File name for frame i: "water.png" becomes "water_0007.png" when recording several frames.
    std::string frameName(int i) const {
        if (frames <= 1) return shot;
        std::string base = shot, ext = ".png";
        size_t dot = shot.rfind('.');
        if (dot != std::string::npos) { base = shot.substr(0, dot); ext = shot.substr(dot); }
        char buf[16];
        snprintf(buf, sizeof(buf), "_%04d", i);
        return base + buf + ext;
    }
};

/// Saves the current framebuffer (width x height pixels) to a PNG file.
inline void saveScreenshot(const std::string& filename, int width, int height) {
    std::vector<unsigned char> pixels(4 * width * height), flipped(4 * width * height);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    for (int y = 0; y < height; ++y)   // OpenGL's first row is the bottom one
        memcpy(&flipped[4 * width * y], &pixels[4 * width * (height - 1 - y)], 4 * width);
    for (int i = 3; i < 4 * width * height; i += 4) flipped[i] = 255;   // opaque
    unsigned error = lodepng::encode(filename, flipped, width, height);
    if (error) printf("Could not save %s: %s\n", filename.c_str(), lodepng_error_text(error));
    else printf("Saved %s\n", filename.c_str());
}
