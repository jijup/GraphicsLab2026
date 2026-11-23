#pragma once
#include <vector>
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"

/// A flat, square grid of triangles in the plane z = 0, centered at the origin.
/// This is the grid mesh of Recitation 07 (Task 1), moved into its own file so that
/// the terrain and the water can both use it. Nothing to do here.
class Grid {
public:
    GLuint vao = 0, vboPosition = 0, ebo = 0;
    GLsizei numIndices = 0;

    /// cells x cells squares, covering [-size/2, size/2] x [-size/2, size/2].
    /// pid is the shader program, so the grid can find the attribute "vposition".
    void init(GLuint pid, int cells, float size) {
        const int N = cells + 1;   // vertices per side
        std::vector<Vec3> points;
        std::vector<unsigned int> indices;

        // Vertices, row by row: vertex (i, j) is number i + j * N
        for (int j = 0; j < N; ++j)
            for (int i = 0; i < N; ++i)
                points.push_back(Vec3((i / float(cells) - 0.5f) * size,
                                      (j / float(cells) - 0.5f) * size, 0.0f));

        // Two triangles per cell, counterclockwise seen from above
        for (int j = 0; j < cells; ++j)
            for (int i = 0; i < cells; ++i) {
                unsigned int a = i + j * N, b = a + 1, c = a + N, d = c + 1;
                indices.insert(indices.end(), { a, b, d });
                indices.insert(indices.end(), { a, d, c });
            }
        numIndices = GLsizei(indices.size());

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
        check_error_gl();
    }

    void draw() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup() {
        glDeleteBuffers(1, &vboPosition);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }
};
