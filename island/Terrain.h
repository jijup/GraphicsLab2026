#pragma once
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"
#include "Helpers/Matrices.h"
#include "Grid.h"
#include "Texture.h"

///--- Grid and scene settings
const int   GRID_CELLS = 256;    ///< cells per side, so (GRID_CELLS + 1)^2 vertices
const float GRID_SIZE = 8.0f;    ///< the grid covers [-4, 4] x [-4, 4]
const float SEA_LEVEL = 0.0f;    ///< height of the water surface

///--- Noise settings, changed with the keys in main.cpp (the same as in Recitation 07)
struct NoiseSettings {
    int mode = 3;                ///< 1 Perlin, 2 fBm, 3 ridged multifractal
    int octaves = 5;             ///< K
    float H = 1.0f;              ///< roughness: larger is smoother
    float lacunarity = 2.0f;     ///< L: frequency gain per octave
    float baseFrequency = 0.5f;  ///< noise cells per world unit at octave 0
    float heightScale = 0.8f;    ///< world height of a noise value of 1
};

///--- The sun: a directional light, in the notation of the lectures and Assignment 1
struct Light {
    Vec3 sunDir = Vec3(-0.78f, 0.50f, 0.42f).normalized();   ///< direction TO the sun: low, in the west
    Vec3 I = Vec3(1.00f, 0.84f, 0.64f);                      ///< I: sunlight, warm
    Vec3 I_a = Vec3(0.32f, 0.36f, 0.46f);                    ///< I_a: ambient light from the sky, cool
};

/// Sets the light uniforms of a shader program. The terrain and the water both use them.
inline void setLight(GLuint pid, const Light& light, const Vec3& cameraPos) {
    setUniform(pid, "sunDir", light.sunDir);
    setUniform(pid, "I", light.I);
    setUniform(pid, "I_a", light.I_a);
    setUniform(pid, "viewPos", cameraPos);
}

class Terrain {
public:
    GLuint pid = 0;
    Grid grid;
    GLuint sand = 0, grass = 0, rock = 0, snow = 0;
    ShaderHelper shader;

    void init() {
        pid = shader.load_shaders("Shaders/terrain_vshader.glsl", "Shaders/terrain_fshader.glsl");
        if (!pid) exit(EXIT_FAILURE);
        grid.init(pid, GRID_CELLS, GRID_SIZE);
        sand = loadTexture("Textures/sand.png");
        grass = loadTexture("Textures/grass.png");
        rock = loadTexture("Textures/rock.png");
        snow = loadTexture("Textures/snow.png");
        check_error_gl();
    }

    void draw(const Mat4x4& V, const Mat4x4& P, const Vec3& cameraPos, const NoiseSettings& n, const Light& light) {
        glUseProgram(pid);
        setUniform(pid, "V", V);
        setUniform(pid, "P", P);
        setLight(pid, light, cameraPos);
        setUniform(pid, "mode", n.mode);
        setUniform(pid, "octaves", n.octaves);
        setUniform(pid, "H", n.H);
        setUniform(pid, "lacunarity", n.lacunarity);
        setUniform(pid, "baseFrequency", n.baseFrequency);
        setUniform(pid, "heightScale", n.heightScale);
        setUniform(pid, "gridStep", GRID_SIZE / GRID_CELLS);   // distance between two neighbouring vertices
        setUniform(pid, "seaLevel", SEA_LEVEL);
        bindTexture(pid, "sandTex", sand, 0);
        bindTexture(pid, "grassTex", grass, 1);
        bindTexture(pid, "rockTex", rock, 2);
        bindTexture(pid, "snowTex", snow, 3);
        grid.draw();
        glUseProgram(0);
    }

    void cleanup() {
        grid.cleanup();
        GLuint t[] = { sand, grass, rock, snow };
        glDeleteTextures(4, t);
        glDeleteProgram(pid);
    }
};
