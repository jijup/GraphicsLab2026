/*CSCI 4471/6671: Computer Graphics (Recitation 03)
  Antialiasing with supersampling, and soft shadows from an area light.
  Scene: a red triangle standing on a checkerboard floor, lit by a square area light.*/

#include "Triangle.h"
#include "Image.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>


////Color functions
using Colour = cv::Vec3b; // BGR Value

uchar Clamp(int color)
{
	if (color < 0) return 0;
	if (color >= 255) return 255;
	return color;
}

// ============================== Scene ==============================
// All colours are BGR floats in [0,1], the same order OpenCV uses.

// The triangle: its base sits on the floor
Triangle triangle = Triangle(Vec3(-1.0f, -1.0f, -2.8f), Vec3(0.4f, -1.0f, -2.6f), Vec3(-0.5f, 0.5f, -2.9f));
Vec3 triangleColor(0.15f, 0.15f, 0.85f);       // red (B, G, R)

// The floor: the plane y = floorY, a checkerboard of squares of size checkSize
float floorY = -1.0f;
float checkSize = 0.5f;
Vec3 floorColor1(0.95f, 0.95f, 0.95f);         // white
Vec3 floorColor2(0.60f, 0.85f, 0.60f);         // green

// Background: colour of rays that hit nothing
Vec3 background(1.0f, 1.0f, 1.0f);             // white

// Light: a square of side lightSize, centred at lightPos, parallel to the floor
Vec3 lightPos(-2.5f, 2.5f, -5.5f);
float lightSize = 1.0f;                         // 0 gives a point light and a hard shadow
float I = 1.2f;                                 // light intensity
float I_a = 0.15f;                              // ambient light intensity

// Sampling
int samplingMode = 2;   // 0 = regular, 1 = random, 2 = jittered
int h_samples = 1;      // samples per pixel: h_samples x v_samples. TODO (Step 3): set both to 4
int v_samples = 1;
int light_samples = 4;  // samples on the light: light_samples x light_samples

float epsilon = 0.00001f;
float eps = 1e-3f;      // hits closer than this along a ray are ignored


// ============================== Sampling ==============================

// A random number in [0, 1)
float random01()
{
	return (float)rand() / ((float)RAND_MAX + 1.0f);
}

// Position of sample k out of n along one axis of a cell, as a number in [0, 1).
// The same function places samples inside a pixel and on the area light.
float sampleOffset(int k, int n)
{
	// TODO (Step 3): replace each 0.5f with the position of sample k out of n.
	// Split the cell into n sub-cells of width 1/n. Sub-cell k starts at k/n.
	//   regular:  the centre of sub-cell k
	//   random:   anywhere in the cell, use random01() and ignore k and n
	//   jittered: anywhere in sub-cell k, start of sub-cell k plus a random fraction of its width
	if (samplingMode == 0) return 0.5f;     // regular: replace
	if (samplingMode == 1) return 0.5f;     // random: replace
	return 0.5f;                            // jittered: replace
}


// ============================== Intersections ==============================

// Ray-triangle: returns the distance t to the hit point, or -1 if there is no hit.
float intersectTriangle(Vec3 Origin, Vec3 Direction)
{
	//Ray-plane intersection...use the triangle normal (triangle.normal_), normalize it, refer to Section 4.4.3 of the textbook
	Vec3 tNormal = triangle.normal_;
	tNormal.normalize();

	// TODO (Step 1a): ray-plane intersection.
	// 1. Compute the denominator Direction . tNormal.
	//    If fabs(denominator) < epsilon, the ray is parallel to the plane: return -1.
	// 2. t = ((triangle.vertex1_ - Origin) . tNormal) / denominator
	// 3. If t <= eps, the plane is behind the ray: return -1.


	// TODO (Step 1b): ray-triangle intersection.
	// 1. Intersection point: Origin + t * Direction
	// 2. Its barycentric coordinates: triangle.BaryCentric(Intersection)
	// 3. Inside test: the coordinates sum to 1 (use epsilon) and each lies in [0, 1].
	//    If the point is inside, return t.


	return -1;   // remove this line
}

// Ray-plane (y = floorY): returns t, or -1 if there is no hit.
float intersectFloor(Vec3 Origin, Vec3 Direction)
{
	if (Direction(1) == 0) return -1;            // ray parallel to the floor
	float t = (floorY - Origin(1)) / Direction(1);
	if (t > eps) return t;
	return -1;
}


// ============================== Shadows ==============================

// Fraction of the area light that is visible from point (0 = full shadow, 1 = fully lit).
float lightVisibility(Vec3 point)
{
	// TODO (Step 4): soft shadows.
	// 1. Loop i and j from 0 to light_samples - 1.
	// 2. A point on the square light:
	//      lightPos + Vec3(lightSize * (sampleOffset(i, light_samples) - 0.5f), 0.0f,
	//                      lightSize * (sampleOffset(j, light_samples) - 0.5f))
	// 3. Shadow ray direction: lightPoint - point. Do NOT normalize it, so the light is at t = 1.
	// 4. t = intersectTriangle(point, ShadowDir). The ray is blocked only if 0 < t < 1.
	//    Count the rays that are not blocked.
	// 5. Return the count divided by the number of shadow rays, as a float.


	return 1.0f;   // remove this line
}


// ============================== Shading ==============================

// Colour seen along one ray, as BGR floats in [0,1].
Vec3 trace(Vec3 Origin, Vec3 Direction)
{
	float tTri = intersectTriangle(Origin, Direction);
	float tFloor = intersectFloor(Origin, Direction);

	// Triangle in front: plain (ambient) shading with its colour
	if (tTri > 0 && (tFloor < 0 || tTri < tFloor)) {
		return triangleColor;
	}

	// Floor: checkerboard colour, diffuse shading, darkened by the shadow
	if (tFloor > 0) {
		Vec3 point = Origin + tFloor * Direction;
		// TODO (Step 2): checkerboard floor.
		// 1. Square number along x: floor(point(0) / checkSize), along z: floor(point(2) / checkSize).
		//    Use floor(), not a plain (int) cast. Convert both to int.
		// 2. Add the two numbers. Use floorColor1 if the sum is even, floorColor2 if it is odd.
		Vec3 k_d = floorColor1;   // replace

		Vec3 Normal(0, 1, 0);
		Vec3 LightVector = lightPos - point;
		LightVector.normalize();
		float diffuse = std::max(0.0f, Normal.dot(LightVector));

		// k_a I_a + V k_d I max(0, n . l), with k_a = k_d and V the visible fraction of the light
		Vec3 k_a = k_d;
		float V = lightVisibility(point);
		return k_a * I_a +  V * k_d * I * diffuse;
	}

	return background;
}


int main(int, char**) {

	Image image = Image(200, 200);

	Vec3 llc = Vec3(-1, -1.5, -1);
	Vec3 urc = Vec3(1, 0.5, -1);
	float width = urc(0) - llc(0);
	float height = urc(1) - llc(1);
	Vec2 pixelUV = Vec2(width / image.cols, height / image.rows);

	Vec3 CameraPoint(0, 0, 0);
	Vec3 Origin = CameraPoint;

	//For random number generator
	srand((unsigned)time(NULL));
	float u, v, w;

	for (int row = 0; row < image.rows; ++row) {
		for (int col = 0; col < image.cols; ++col) {
			Vec3 pixelColour(0, 0, 0);

			// supersampling: h_samples x v_samples rays per pixel
			for (int dx = 0; dx < h_samples; ++dx) {
				for (int dy = 0; dy < v_samples; ++dy) {
					// TODO (Step 3): replace the two 0.5f with sampleOffset(dx, h_samples) and sampleOffset(dy, v_samples)
					u = llc(0) + pixelUV(0) * (col + 0.5f);
					v = urc(1) - pixelUV(1) * (row + 0.5f);
					w = -1;

					Vec3 pixelPos = Vec3(u, v, w);

					Vec3 Direction = pixelPos - Origin;
					Direction.normalize();

					pixelColour += trace(Origin, Direction);
				}
			}

			// TODO (Step 3): average the samples. pixelColour holds the sum of all sample colours,
			// so divide it by the number of samples.


			image(row, col) = Colour(Clamp(pixelColour[0] * 255), Clamp(pixelColour[1] * 255), Clamp(pixelColour[2] * 255));
		}
	}

	image.save("./result.png");
	image.display();

	return EXIT_SUCCESS;
}
