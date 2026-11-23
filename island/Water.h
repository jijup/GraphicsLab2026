#pragma once
#include "Terrain.h"

/// The water: a second grid, at sea level, drawn after the terrain and see-through.
/// Your work for the water is in Shaders/water_vshader.glsl and Shaders/water_fshader.glsl.
class Water {
public:
    GLuint pid = 0;
    Grid grid;
    GLuint colorTex = 0, normalTex = 0;
    ShaderHelper shader;

    void init() {
        pid = shader.load_shaders("Shaders/water_vshader.glsl", "Shaders/water_fshader.glsl");
        if (!pid) exit(EXIT_FAILURE);
        grid.init(pid, GRID_CELLS, GRID_SIZE);   // as fine as the terrain, so the waves of Task 4 can bend it
        colorTex = loadTexture("Textures/water.png");
        normalTex = loadTexture("Textures/water_normal.png");
        check_error_gl();
    }

    void draw(const Mat4x4& V, const Mat4x4& P, const Vec3& cameraPos, const Light& light, float time, bool waves) {
        glUseProgram(pid);
        setUniform(pid, "V", V);
        setUniform(pid, "P", P);
        setLight(pid, light, cameraPos);
        setUniform(pid, "seaLevel", SEA_LEVEL);
        setUniform(pid, "time", time);
        setUniform(pid, "waves", waves ? 1 : 0);
        bindTexture(pid, "waterTex", colorTex, 0);
        bindTexture(pid, "normalTex", normalTex, 1);

        // Blending: the new colour is mixed with the colour already in the window (the terrain under
        // the water), with the alpha of the water as the weight:  alpha * water + (1 - alpha) * terrain.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid.draw();
        glDisable(GL_BLEND);
        glUseProgram(0);
    }

    void cleanup() {
        grid.cleanup();
        GLuint t[] = { colorTex, normalTex };
        glDeleteTextures(2, t);
        glDeleteProgram(pid);
    }
};
