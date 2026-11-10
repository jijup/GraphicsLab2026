#pragma once
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"
#include "Helpers/Texture.h"
#include "Helpers/Matrices.h"

/// A unit cube around the camera, textured with a cubemap.
class Skybox {
public:
    GLuint pid = 0;
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLuint cubemap = 0;
    ShaderHelper shader;

    void init() {
        pid = shader.load_shaders("Shaders/skybox_vshader.glsl", "Shaders/skybox_fshader.glsl");
        if (!pid) exit(EXIT_FAILURE);

        ///--- The cube: 8 corners, 14 indices as one triangle strip
        const float corners[] = {
             1, 1, 1,  -1, 1, 1,   1, 1,-1,  -1, 1,-1,
             1,-1, 1,  -1,-1, 1,  -1,-1,-1,   1,-1,-1 };
        const unsigned int strip[] = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);
        GLint pos = glGetAttribLocation(pid, "vposition");
        if (pos >= 0) {
            glEnableVertexAttribArray(pos);
            glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(strip), strip, GL_STATIC_DRAW);
        glBindVertexArray(0);

        ///--- The cubemap texture
        glGenTextures(1, &cubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        // The six images, in the order OpenGL numbers the faces: +X, -X, +Y, -Y, +Z, -Z.
        // These are cubemap directions, which are Y-up. Our world is Z-up (see the vertex shader).
        const char* faces[] = { "miramar_rt", "miramar_lf", "miramar_up",
                                "miramar_dn", "miramar_ft", "miramar_bk" };
        ///TODO C7: Upload the six faces.
        // For i = 0 .. 5: read Textures/<faces[i]>.png with loadImage(file, image, width, height),
        // then upload it with glTexImage2D. The face targets are numbered in order,
        // so face i is GL_TEXTURE_CUBE_MAP_POSITIVE_X + i. Use GL_RGBA8 and GL_RGBA, GL_UNSIGNED_BYTE.
        // Do not flip the images: cubemap faces are stored top row first.
        (void)faces;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);   // filter across face edges, no seams
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        check_error_gl();
    }

    /// Draw the sky first, without depth testing, so everything else is drawn in front of it.
    void draw(const Mat4x4& V, const Mat4x4& P) {
        glUseProgram(pid);
        glBindVertexArray(vao);
        setUniform(pid, "V", V);
        setUniform(pid, "P", P);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        setUniform(pid, "skybox", 0);

        glDisable(GL_DEPTH_TEST);   // the sky is behind everything
        glDrawElements(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, 0);
        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void cleanup() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &cubemap);
        glDeleteProgram(pid);
    }
};
