#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <GL/glew.h>
#include "Helpers/lodepng.h"

/// Reads a PNG file into "image" (4 bytes per pixel, RGBARGBA...).
/// The first row in "image" is the top row of the picture.
inline void loadImage(const std::string& filename, std::vector<unsigned char>& image,
                      unsigned& width, unsigned& height) {
    image.clear();
    unsigned error = lodepng::decode(image, width, height, filename);
    if (error) {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error)
                  << " (" << filename << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

/// Loads a PNG as a repeating, mipmapped 2D texture and returns its id.
/// The rows are flipped, because OpenGL expects the bottom row first.
inline GLuint loadTexture(const std::string& filename) {
    std::vector<unsigned char> image;
    unsigned width, height;
    loadImage(filename, image, width, height);

    std::vector<unsigned char> row(4 * width);
    for (unsigned i = 0; i < height / 2; ++i) {
        unsigned char* a = &image[4 * i * width];
        unsigned char* b = &image[4 * (height - 1 - i) * width];
        memcpy(row.data(), a, 4 * width);
        memcpy(a, b, 4 * width);
        memcpy(b, row.data(), 4 * width);
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}
