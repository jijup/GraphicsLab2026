#pragma once
#include <vector>
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"
#include "Helpers/Matrices.h"

///--- Grid settings
const int   GRID_CELLS = 48;     ///< cells per side, so (GRID_CELLS + 1)^2 vertices (Task 3: make it 256)
const float GRID_SIZE = 8.0f;    ///< the grid covers [-4, 4] x [-4, 4]

///--- Noise settings, changed with the keys in main.cpp
struct NoiseSettings {
    int mode = 3;                ///< 1 Perlin, 2 fBm, 3 ridged multifractal
    int octaves = 5;             ///< K
    float H = 1.0f;              ///< roughness: larger is smoother
    float lacunarity = 2.0f;     ///< L: frequency gain per octave
    float baseFrequency = 0.5f;  ///< noise cells per world unit at octave 0
    float heightScale = 0.8f;    ///< world height of a noise value of 1
};

class Terrain {
public:
    GLuint pid = 0;
    GLuint vao = 0, vboPosition = 0, ebo = 0;
    GLsizei numIndices = 0;
    ShaderHelper shader;

    void init() {
        pid = shader.load_shaders("Shaders/terrain_vshader.glsl", "Shaders/terrain_fshader.glsl");
        if (!pid) exit(EXIT_FAILURE);
        genGrid();
        check_error_gl();
    }

    /// A flat grid in the plane z = 0, drawn as indexed triangles.
    void genGrid() {
        const int N = GRID_CELLS + 1;   // vertices per side
        std::vector<Vec3> points;
        std::vector<unsigned int> indices;

        // Vertices, row by row: vertex (i, j) is number i + j * N
        for (int j = 0; j < N; ++j)
            for (int i = 0; i < N; ++i)
                points.push_back(Vec3((i / float(GRID_CELLS) - 0.5f) * GRID_SIZE,
                                      (j / float(GRID_CELLS) - 0.5f) * GRID_SIZE, 0.0f));

        ///TODO Task 1: Two triangles for each cell.
        // The cell (i, j), for i and j from 0 to GRID_CELLS - 1, has four corners:
        //   a = i + j * N        (bottom left)     b = a + 1        (bottom right)
        //   c = a + N            (top left)        d = c + 1        (top right)
        // Push the indices of two triangles that cover it: (a, b, d) and (a, d, c).
        // Keep the same winding (counterclockwise seen from above) for both.
        numIndices = GLsizei(indices.size());

        ///--- Upload to the GPU
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vboPosition);
        glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vec3), points.data(), GL_STATIC_DRAW);
        GLint pos = glGetAttribLocation(pid, "vposition");
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glBindVertexArray(0);
    }

    void draw(const Mat4x4& V, const Mat4x4& P, const Vec3& cameraPos, const NoiseSettings& n) {
        glUseProgram(pid);
        setUniform(pid, "V", V);
        setUniform(pid, "P", P);
        setUniform(pid, "viewPos", cameraPos);
        setUniform(pid, "mode", n.mode);
        setUniform(pid, "octaves", n.octaves);
        setUniform(pid, "H", n.H);
        setUniform(pid, "lacunarity", n.lacunarity);
        setUniform(pid, "baseFrequency", n.baseFrequency);
        setUniform(pid, "heightScale", n.heightScale);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void cleanup() {
        glDeleteBuffers(1, &vboPosition);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(pid);
    }
};
