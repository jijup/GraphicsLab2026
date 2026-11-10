#pragma once
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include "Helpers/ShaderHelper.h"

///--- Height map settings. Try changing these!
const int   HEIGHTMAP_SIZE = 1024;  ///< texels per side of the height map
const int   BASE_CELLS = 4;         ///< Perlin lattice cells across the map, at octave 0
const float H_ROUGHNESS = 1.0f;     ///< H: larger is smoother
const float LACUNARITY = 2.0f;      ///< L: frequency gain per octave (keep it a whole number so the map tiles)
const int   OCTAVES = 8;            ///< K: number of octaves
const unsigned NOISE_SEED = 2026;   ///< change the seed to get a new world

inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

/// Quintic fade curve 6t^5 - 15t^4 + 10t^3 (Perlin 2002).
inline float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

/// A random number in [0, 1) from the Mersenne Twister. Written out (instead of
/// std::uniform_real_distribution) so the terrain is the same with every compiler.
inline float rand01(std::mt19937& rng) {
    float r = float(rng()) / 4294967296.0f;
    return r < 1.0f ? r : 0.99999994f;
}

/// A fixed random permutation of 0 .. 255, as in Perlin's reference implementation.
/// It picks a gradient for each lattice corner.
static const int PERM[256] = {
    232, 171, 23, 241, 129, 183, 14, 214, 198, 159, 82, 100, 138, 71, 125, 92,
    132, 13, 242, 131, 83, 85, 216, 230, 12, 234, 107, 161, 225, 154, 204, 63,
    89, 48, 139, 9, 25, 53, 208, 45, 210, 197, 102, 146, 93, 106, 36, 235,
    148, 20, 51, 43, 252, 90, 184, 42, 73, 168, 236, 212, 147, 116, 66, 76,
    75, 149, 56, 133, 251, 27, 130, 143, 126, 88, 190, 104, 69, 186, 193, 110,
    77, 167, 108, 109, 112, 118, 52, 26, 173, 164, 255, 18, 207, 15, 0, 96,
    249, 121, 87, 155, 189, 101, 162, 19, 11, 144, 209, 3, 46, 39, 219, 32,
    182, 68, 2, 58, 179, 21, 172, 50, 99, 98, 30, 79, 226, 124, 10, 81,
    94, 239, 247, 215, 74, 49, 157, 38, 196, 24, 70, 37, 243, 57, 137, 62,
    113, 136, 175, 237, 127, 7, 105, 44, 115, 86, 165, 64, 111, 47, 169, 250,
    151, 177, 213, 61, 217, 222, 188, 34, 223, 135, 170, 8, 176, 227, 178, 220,
    123, 211, 35, 4, 95, 233, 120, 201, 84, 60, 174, 103, 166, 205, 67, 142,
    145, 122, 200, 245, 181, 218, 29, 194, 185, 5, 221, 33, 191, 203, 153, 206,
    1, 54, 91, 119, 248, 59, 40, 152, 156, 246, 31, 22, 72, 55, 134, 224,
    192, 180, 28, 195, 254, 6, 228, 114, 229, 140, 97, 231, 41, 158, 160, 240,
    17, 187, 65, 238, 199, 16, 202, 150, 163, 78, 253, 141, 80, 244, 128, 117
};

/// 256 random unit gradients, and the permutation used to pick one for each lattice corner.
struct GradientTable {
    Vec2 grad[256];
    int perm[512];
    GradientTable(unsigned seed) {
        std::mt19937 rng(seed);
        for (int i = 0; i < 256; ++i) {
            float a = rand01(rng) * (2.0f * 3.14159265f);
            grad[i] = Vec2(std::cos(a), std::sin(a));
        }
        for (int i = 0; i < 512; ++i) perm[i] = PERM[i & 255];
    }
    /// The gradient at integer lattice corner (i, j). The caller wraps i and j first.
    Vec2 at(int i, int j) const {
        return grad[perm[perm[i & 255] + (j & 255)]];
    }
};
static const GradientTable gradients(NOISE_SEED);

/// Wraps a lattice index into [0, period), also for negative i.
inline int wrap(int i, int period) {
    return ((i % period) + period) % period;
}

/// 2D Perlin (gradient) noise at point (x, y), in lattice units: corners are at whole numbers.
/// The noise repeats every "period" cells, so the height map tiles without a seam.
/// Returns a value of roughly [-0.7, 0.7].
inline float perlin2D(float x, float y, int period) {
    float noise = 0.0f;
    ///TODO C2: Perlin noise. See the lecture slides and Recitation 07.
    // 1. The lattice cell that contains (x, y): its lower corner is (x0, y0) = (floor(x), floor(y)),
    //    and its upper corner is (x0 + 1, y0 + 1). Wrap the corner indices with wrap(i, period).
    // 2. The position inside the cell: dx = x - x0, dy = y - y0, both in [0, 1).
    // 3. The random gradient at each of the four corners: gradients.at(i, j).
    // 4. The vector from each corner to (x, y), for example (dx - 1, dy) for the corner (x0 + 1, y0).
    // 5. Dot each gradient with its vector. This gives four scalars s, t, u, v.
    // 6. Blend them with lerp, using fade(dx) along x and then fade(dy) along y.
    return noise;
}

/// Fractional Brownian motion: K octaves of Perlin noise, each one L times the frequency
/// of the previous one, weighted by w_k = L^(-k H).
/// (x, y) is in [0, 1) across the height map.
inline float fBm(float x, float y) {
    float value = 0.0f;
    ///TODO C3: fBm. For k = 0 .. OCTAVES - 1, add w_k * perlin2D(f * x, f * y, f)
    // where f = BASE_CELLS * L^k is the number of lattice cells across the map at octave k
    // (pass it as the period too, so every octave tiles), and w_k = L^(-k * H_ROUGHNESS).
    // Until you do this, the height map is a single octave of Perlin noise:
    value = perlin2D(BASE_CELLS * x, BASE_CELLS * y, BASE_CELLS);
    return value;
}

/// Fills "heights" (HEIGHTMAP_SIZE x HEIGHTMAP_SIZE, row by row) with fBm,
/// rescaled to [-1, 1] so the texturing levels do not depend on H and K.
inline void fBm2D(std::vector<float>& heights) {
    const int N = HEIGHTMAP_SIZE;
    heights.resize(N * N);
    for (int j = 0; j < N; ++j)
        for (int i = 0; i < N; ++i)
            heights[i + j * N] = fBm(i / float(N), j / float(N));

    float lo = *std::min_element(heights.begin(), heights.end());
    float hi = *std::max_element(heights.begin(), heights.end());
    if (hi - lo < 1e-6f) return;   // flat (noise not implemented yet)
    for (float& h : heights) h = 2.0f * (h - lo) / (hi - lo) - 1.0f;
}

/// Uploads a height map as a one channel float texture (GL_R32F) that repeats.
inline GLuint uploadHeightTexture(const std::vector<float>& heights) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, HEIGHTMAP_SIZE, HEIGHTMAP_SIZE, 0, GL_RED, GL_FLOAT, heights.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

/// Builds the fBm height map and returns it as a texture. The CPU copy stays in "heights".
inline GLuint fBm2DTexture(std::vector<float>& heights) {
    fBm2D(heights);
    return uploadHeightTexture(heights);
}
