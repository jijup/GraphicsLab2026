#pragma once
#include "lodepng.h"
#include<iostream>
#include<vector>
#include<algorithm>
#include<cassert>
#include "opencv2/core/core.hpp"
#include <Eigen/Dense>
#include<math.h>

//Data types
typedef Eigen::Matrix<float, 3, 1> Vec3;
typedef Eigen::Matrix<float, 2, 1> Vec2;

class Texture
{
public:
	Texture() {};
	bool loadTexture(const std::string&);
	cv::Vec3b get_pixel(int, int);

	// Colour at texture coordinates uv, as BGR floats in [0,1].
	// Uses lookupNearest() or lookupBilinear(), depending on Texture::filter.
	Vec3 lookup(const Vec2& uv);
	Vec3 lookupNearest(const Vec2& uv);     // the texel that contains uv
	Vec3 lookupBilinear(const Vec2& uv);    // the four nearest texels, blended (Task 3)

	int wrap_i(int i);     // u repeats: column i wraps around the texture
	int clamp_j(int j);    // v is clamped: row j stays inside the texture

	static int filter;     // 0 = nearest texel, 1 = bilinear, for every texture

public:
	unsigned int height = 0;
	unsigned int width = 0;
	std::vector<unsigned char> image;
};

