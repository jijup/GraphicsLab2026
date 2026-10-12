#include "Texture.h"

int Texture::filter = 1;

Vec3 Texture::lookup(const Vec2& uv)
{
    if (filter == 0) return lookupNearest(uv);
    return lookupBilinear(uv);
}

int Texture::wrap_i(int i)
{
    return ((i % (int)width) + width) % width;
}

int Texture::clamp_j(int j)
{
    return std::min(std::max(j, 0), (int)height - 1);
}


/*Nearest texel: texel (i, j) covers u from i / width to (i + 1) / width, and the same for v.*/
Vec3 Texture::lookupNearest(const Vec2& uv)
{
    int i = wrap_i((int)floor(uv[0] * width));
    int j = clamp_j((int)floor(uv[1] * height));
    cv::Vec3b p = get_pixel(i, j);
    return Vec3(p[0], p[1], p[2]) / 255.0f;
}


/*Task 3: bilinear interpolation (Section 6 of the recitation page).
  The centre of texel (i, j) is at u = (i + 0.5) / width, v = (j + 0.5) / height.*/
Vec3 Texture::lookupBilinear(const Vec2& uv)
{
    // TODO (Task 3a): real-valued texel coordinates of uv.
    // Texel (i, j) has its centre at u = (i + 0.5) / width. Solve for i to get x, and the same for y with v and height.
    // Then x is exactly i at the centre of column i, and 2.3 means 30% of the way from column 2 to column 3.
    float x = 0;   // replace
    float y = 0;   // replace

    int i0 = (int)floor(x);
    int j0 = (int)floor(y);

    // TODO (Task 3b): the interpolation weights, both in [0, 1).
    // au is how far x is from column i0 towards column i0 + 1, and av how far y is from row j0 towards row j0 + 1.
    float au = 0;   // replace
    float av = 0;   // replace

    // the four texels around the point: u wraps around, v is clamped
    int i1 = wrap_i(i0 + 1);
    int j1 = clamp_j(j0 + 1);
    i0 = wrap_i(i0);
    j0 = clamp_j(j0);
    cv::Vec3b p00 = get_pixel(i0, j0), p10 = get_pixel(i1, j0);
    cv::Vec3b p01 = get_pixel(i0, j1), p11 = get_pixel(i1, j1);

    // TODO (Task 3c): blend the four texels, for each channel c = 0, 1, 2.
    // 1. Bottom row: blend p00[c] and p10[c] with au (au = 0 gives p00, au = 1 gives p10).
    // 2. Top row: blend p01[c] and p11[c] the same way.
    // 3. Blend the two rows with av, and divide by 255 to get a float in [0, 1].
    Vec3 colour;
    for (int c = 0; c < 3; ++c) {
        colour[c] = p00[c] / 255.0f;   // replace
    }
    return colour;
}


cv::Vec3b Texture::get_pixel(int i, int j)
{
    assert(i >= 0 && i < (int)width);
    assert(j >= 0 && j < (int)height);

    cv::Vec3b pixel;
    pixel[2] = image[j * width * 4 + 4 * i];        //RGB to BGR
    pixel[1] = image[j * width * 4 + 4 * i + 1];
    pixel[0] = image[j * width * 4 + 4 * i + 2];

    return pixel;
}


bool Texture::loadTexture(const std::string& filename)
{
    // Used snippet from https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
    unsigned w, h;
    unsigned error = lodepng::decode(image, w, h, filename);
    if (error) {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << " (" << filename << ")" << std::endl;
        return false;
    }
    width = w;
    height = h;
    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA...

    // row 0 of a png is the top of the image, but v = 0 is the bottom...lets flip it
    std::vector<unsigned char> row(4 * width);
    for (int i = 0; i < int(height) / 2; ++i) {
        unsigned char* top = &image[4 * i * width];
        unsigned char* bottom = &image[image.size() - 4 * (i + 1) * width];
        std::copy(top, top + 4 * width, row.begin());
        std::copy(bottom, bottom + 4 * width, top);
        std::copy(row.begin(), row.end(), bottom);
    }
    return true;
}
