#pragma once
#include <map>
#include <string>
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"
#include "Helpers/Texture.h"
#include "Helpers/Matrices.h"
#include "noise.h"

///--- Terrain settings
const int   GRID_RES = 512;          ///< vertices per side of the grid
const float GRID_SIZE = 10.0f;       ///< the grid covers [-5, 5] x [-5, 5]
const float HEIGHT_SCALE = 0.8f;     ///< world height of a height map value of 1
const float SEA_LEVEL = -0.2f;        ///< world height of the water surface
const unsigned RESTART_INDEX = 0xFFFFFFFF;

/// The light and the camera position, shared by everything we draw.
struct SceneUniforms {
    Mat4x4 V, P;
    Vec3 cameraPos;
    Vec3 sunDir;      ///< unit vector pointing toward the sun
    float time;       ///< seconds since the start
};

class Terrain {
public:
    GLuint pid = 0;            ///< shader program
    GLuint vao = 0;
    GLuint vboPosition = 0, vboTexcoord = 0, ebo = 0;
    GLsizei numIndices = 0;
    GLuint heightTex = 0;      ///< the height map (GL_R32F)
    std::vector<float> heights;              ///< CPU copy of the height map
    std::map<std::string, GLuint> textures;  ///< grass, rock, sand, snow, water
    ShaderHelper shader;

    void init() {
        pid = shader.load_shaders("Shaders/terrain_vshader.glsl", "Shaders/terrain_fshader.glsl");
        if (!pid) exit(EXIT_FAILURE);

        genTerrainMesh();

        printf("Generating the height map...\n");
        heightTex = fBm2DTexture(heights);

        const char* names[] = { "grass", "rock", "sand", "snow", "water" };
        for (const char* n : names)
            textures[n] = loadTexture(std::string("Textures/") + n + ".png");
        check_error_gl();
    }

    /// A flat GRID_RES x GRID_RES grid in the plane z = 0, drawn as triangle strips.
    void genTerrainMesh() {
        std::vector<Vec3> points;
        std::vector<Vec2> texCoords;
        std::vector<unsigned int> indices;

        ///TODO C1: Build the grid.
        // 1. Vertices: for j = 0 .. GRID_RES - 1 (rows, along y) and i = 0 .. GRID_RES - 1 (columns, along x),
        //    push the position (x, y, 0) with x and y going from -GRID_SIZE / 2 to +GRID_SIZE / 2,
        //    and the texture coordinate (u, v) going from 0 to 1. Vertex (i, j) is then number i + j * GRID_RES.
        //    Make u follow x and v follow y. The shaders rely on it.
        // 2. Indices: one triangle strip per pair of rows j and j + 1. For each column i, push the vertex
        //    in row j and then the vertex in row j + 1. End each strip with RESTART_INDEX.
        numIndices = GLsizei(indices.size());

        ///--- Upload to the GPU
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vboPosition);
        glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vec3), points.data(), GL_STATIC_DRAW);
        GLint pos = glGetAttribLocation(pid, "vposition");
        if (pos >= 0) {
            glEnableVertexAttribArray(pos);
            glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        glGenBuffers(1, &vboTexcoord);
        glBindBuffer(GL_ARRAY_BUFFER, vboTexcoord);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(Vec2), texCoords.data(), GL_STATIC_DRAW);
        GLint tc = glGetAttribLocation(pid, "vtexcoord");
        if (tc >= 0) {
            glEnableVertexAttribArray(tc);
            glVertexAttribPointer(tc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void draw(const SceneUniforms& s) {
        glUseProgram(pid);
        glBindVertexArray(vao);

        ///--- Transformations and scene settings
        setUniform(pid, "M", Mat4x4(Mat4x4::Identity()));
        setUniform(pid, "V", s.V);
        setUniform(pid, "P", s.P);
        setUniform(pid, "viewPos", s.cameraPos);
        setUniform(pid, "sunDir", s.sunDir);
        setUniform(pid, "time", s.time);
        setUniform(pid, "heightScale", HEIGHT_SCALE);
        setUniform(pid, "seaLevel", SEA_LEVEL);
        setUniform(pid, "gridSize", GRID_SIZE);

        ///--- Height map on texture unit 0, the other textures on units 1, 2, ...
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, heightTex);
        setUniform(pid, "noiseTex", 0);
        int unit = 1;
        for (auto& t : textures) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, t.second);
            setUniform(pid, t.first.c_str(), unit);
            ++unit;
        }

        ///--- Draw all the strips with one call
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(RESTART_INDEX);
        glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_INT, 0);
        glDisable(GL_PRIMITIVE_RESTART);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void cleanup() {
        glDeleteBuffers(1, &vboPosition);
        glDeleteBuffers(1, &vboTexcoord);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(pid);
        glDeleteTextures(1, &heightTex);
        for (auto& t : textures) glDeleteTextures(1, &t.second);
    }
};
