#pragma once
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <GL/glew.h>
#include "Helpers/lodepng.h"

/// Loads a PNG file into an OpenGL texture, as in the Textures lecture. Nothing to do here.
///   GL_REPEAT:               the texture tiles, so texture coordinates above 1 wrap around.
///   GL_LINEAR_MIPMAP_LINEAR: far away, OpenGL reads smaller, prefiltered copies (mipmaps),
///                            so the texture does not shimmer.
inline GLuint loadTexture(const std::string& filename) {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, filename);
    if (error) {
        printf("Could not load %s: %s\n", filename.c_str(), lodepng_error_text(error));
        exit(EXIT_FAILURE);
    }
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

/// Binds texture "id" to texture unit "unit" and tells the sampler "name" to read from that unit.
inline void bindTexture(GLuint pid, const char* name, GLuint id, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
    glUniform1i(glGetUniformLocation(pid, name), unit);
}
