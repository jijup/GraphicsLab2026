/* Starter Code for Assignment 1 of CSCI 4471: Computer Graphics
   A recursive ray tracer for the Cornell box.
   Read the assignment handout carefully while completing the code.

   What is given: the data types, the complete Cornell box scene (C1), the camera and pixel loop,
   and saving the image.
   What you write: everything marked TODO, for the core requirements C2 to C6.
   The program compiles and runs as it is, but the image is black: nothing can be hit yet.
   A good order to work in:
     1. intersectTriangle and the triangle loop in closestHit: the walls appear, ambient only
     2. intersectSphere: the two spheres appear (the glass one black for now)
     3. blinnPhong, inShadow: lit walls with hard shadows
     4. reflect and the mirror: the mirror sphere
     5. glass: Snell, total internal reflection, Fresnel
     6. jittered supersampling: smooth edges
   Compare your result with the reference image of C1 in the handout.

   Menu features (F1 to F7) are not stubbed: design and add them yourself. */

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <vector>
#include <random>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <string>

#include "Image.h"
#include "ObjLoader.h"   // for F6 (triangle meshes); not needed for the core

// Folder holding the .obj files, set by CMakeLists.txt
#ifndef ASSETS_DIR
#define ASSETS_DIR "assets"
#endif

typedef Eigen::Matrix<float, 3, 1> Vec3;
typedef Eigen::Matrix<float, 2, 1> Vec2;
using Colour = cv::Vec3b;

//bounding the channel wise pixel color between 0 to 255
unsigned char Clamp(int pixelCol)
{
	if (pixelCol < 0) return 0;
	if (pixelCol >= 255) return 255;
	return pixelCol;
}

// ============================== Scene description (Given) ==============================
// All colours are BGR floats in [0,1], the same order OpenCV uses.
// The handout lists colours as RGB, so red (0.63, 0.06, 0.05) becomes (0.05, 0.06, 0.63) here.
// Notation follows Marschner & Shirley:
//   k_a, k_d, k_s  ambient, diffuse and specular coefficients
//   p              Phong exponent
//   k_m            mirror reflectance
//   n              index of refraction (glass only)

struct Material {
	Vec3 k_a;
	Vec3 k_d;
	Vec3 k_s;
	float p;
	Vec3 k_m;
	bool isGlass;     // true: reflection and refraction weighted by Fresnel
	float n;          // index of refraction, used when isGlass is true
	bool isEmissive;  // true: the light panel, returned as white without shading
};

struct Sphere {
	Vec3 centre;
	float radius;
	int material;     // index into materials
};

struct Triangle {
	Vec3 a, b, c;     // vertices
	Vec3 normal;      // points into the box
	int material;     // each triangle carries its own material
};

std::vector<Material> materials;
std::vector<Sphere> spheres;
std::vector<Triangle> triangles;

// Material indices
enum { WHITE, RED, GREEN, MIRROR, GLASS, EMISSIVE };

// Light (C1)
Vec3 lightSource(0.0f, 1.95f, 0.0f);            // point light just below the light panel
Vec3 I = Vec3(1.0f, 1.0f, 1.0f);                // light intensity
Vec3 I_a = Vec3(1.0f, 1.0f, 1.0f);              // ambient light intensity

// Background: colour of rays that leave the box through the open front
Vec3 background = Vec3(0.0f, 0.0f, 0.0f);

// Camera (C1): eye point and image plane corners, as in Recitation 02.
// The plane is 1 unit in front of the eye; its half-size tan(20 deg) = 0.364 gives a 40 degree field of view.
Vec3 eye(0.0f, 1.0f, 3.8f);
Vec3 llc(-0.364f, 0.636f, 2.8f);                // lower left corner of the image plane
Vec3 urc(0.364f, 1.364f, 2.8f);                 // upper right corner of the image plane

// Rendering parameters
int imageSize = 800;    // 800 x 800
int samplesN = 4;       // n x n jittered samples per pixel (C6)
int maxDepth = 5;       // maximum recursion depth (C2)
float eps = 1e-3f;      // offset so a new ray does not hit its own surface

// Random numbers in [0, 1) for jittering (C6): call uniform(rng)
std::mt19937 rng(4471);
std::uniform_real_distribution<float> uniform(0.0f, 1.0f);


Material makeDiffuse(Vec3 kd)
{
	Material m;
	m.k_a = 0.1f * kd;
	m.k_d = kd;
	m.k_s = Vec3(0, 0, 0);
	m.p = 1.0f;
	m.k_m = Vec3(0, 0, 0);
	m.isGlass = false;
	m.n = 1.0f;
	m.isEmissive = false;
	return m;
}

// Adds a quad (A, B, C, D) as the two triangles (A, B, C) and (A, C, D)
void addQuad(Vec3 A, Vec3 B, Vec3 C, Vec3 D, Vec3 normal, int material)
{
	triangles.push_back({ A, B, C, normal, material });
	triangles.push_back({ A, C, D, normal, material });
}

// The Cornell box of C1 (Given)
void buildCornellBox()
{
	// Materials, in the order of the enum above
	materials.push_back(makeDiffuse(Vec3(0.73f, 0.73f, 0.73f)));   // WHITE
	materials.push_back(makeDiffuse(Vec3(0.05f, 0.06f, 0.63f)));   // RED   (BGR)
	materials.push_back(makeDiffuse(Vec3(0.09f, 0.45f, 0.14f)));   // GREEN (BGR)

	Material mirror;
	mirror.k_a = Vec3(0, 0, 0);
	mirror.k_d = Vec3(0.05f, 0.05f, 0.05f);
	mirror.k_s = Vec3(0.5f, 0.5f, 0.5f);
	mirror.p = 200.0f;
	mirror.k_m = Vec3(0.9f, 0.9f, 0.9f);
	mirror.isGlass = false;
	mirror.n = 1.0f;
	mirror.isEmissive = false;
	materials.push_back(mirror);                                    // MIRROR

	Material glass;
	glass.k_a = Vec3(0, 0, 0);
	glass.k_d = Vec3(0, 0, 0);
	glass.k_s = Vec3(0.5f, 0.5f, 0.5f);
	glass.p = 200.0f;
	glass.k_m = Vec3(0, 0, 0);                                      // not used: Fresnel decides
	glass.isGlass = true;
	glass.n = 1.5f;
	glass.isEmissive = false;
	materials.push_back(glass);                                     // GLASS

	Material light = makeDiffuse(Vec3(1, 1, 1));
	light.isEmissive = true;
	materials.push_back(light);                                     // EMISSIVE

	// Walls: x in [-1, 1], y in [0, 2], z in [-1, 1], open at z = 1
	addQuad(Vec3(-1, 0, -1), Vec3(1, 0, -1), Vec3(1, 0, 1), Vec3(-1, 0, 1), Vec3(0, 1, 0), WHITE);    // floor
	addQuad(Vec3(-1, 2, -1), Vec3(-1, 2, 1), Vec3(1, 2, 1), Vec3(1, 2, -1), Vec3(0, -1, 0), WHITE);   // ceiling
	addQuad(Vec3(-1, 0, -1), Vec3(-1, 2, -1), Vec3(1, 2, -1), Vec3(1, 0, -1), Vec3(0, 0, 1), WHITE);  // back wall
	addQuad(Vec3(-1, 0, 1), Vec3(-1, 2, 1), Vec3(-1, 2, -1), Vec3(-1, 0, -1), Vec3(1, 0, 0), RED);    // left wall
	addQuad(Vec3(1, 0, -1), Vec3(1, 2, -1), Vec3(1, 2, 1), Vec3(1, 0, 1), Vec3(-1, 0, 0), GREEN);    // right wall
	addQuad(Vec3(-0.3f, 1.99f, -0.3f), Vec3(0.3f, 1.99f, -0.3f), Vec3(0.3f, 1.99f, 0.3f),
		Vec3(-0.3f, 1.99f, 0.3f), Vec3(0, -1, 0), EMISSIVE);                                          // light panel

	// Spheres
	spheres.push_back({ Vec3(-0.45f, 0.35f, -0.35f), 0.35f, MIRROR });
	spheres.push_back({ Vec3(0.45f, 0.35f, 0.35f), 0.35f, GLASS });
}


// ============================== Intersections ==============================

// Ray-sphere (textbook Section 4.4.1). Returns t of the closest hit in front of the ray, or -1.
float intersectSphere(const Sphere& s, Vec3 Origin, Vec3 direction)
{
	///TODO (C2): Ray-sphere intersection. You wrote most of this in Recitation 02.
	//  A = d.d,   B = 2 d.(o - c),   C = (o - c).(o - c) - r^2
	//If the discriminant is negative, return -1.
	//Return the near root if it is > eps.
	//NEW: otherwise return the far root if it is > eps. A refracted ray starts INSIDE the glass sphere,
	//so the near root is behind it and its only hit is the back of the sphere.


	return -1;   // remove this line when you are done
}

// Ray-triangle (C5, textbook Section 4.4.2). Returns t, or -1 if there is no hit.
float intersectTriangle(const Triangle& tri, Vec3 e, Vec3 d)
{
	///TODO (C5): Ray-triangle intersection with Cramer's rule.
	//Solve  e + t d = a + beta (b - a) + gamma (c - a)  for beta, gamma and t.
	//Follow the textbook: name the matrix entries a to l as in Section 4.4.2,
	//compute M, then t, gamma and beta, rejecting the hit as early as possible:
	//  - |M| very small: the ray is parallel to the triangle
	//  - t < eps
	//  - gamma < 0 or gamma > 1
	//  - beta < 0 or beta > 1 - gamma


	return -1;   // remove this line when you are done
}

// Result of finding the closest object along a ray (Given)
struct Hit {
	float t = -1;       // -1: nothing hit
	Vec3 point;
	Vec3 normal;        // outward normal for spheres, the stored normal for triangles
	int material = -1;
};

// Tests every object and keeps the closest hit
Hit closestHit(Vec3 Origin, Vec3 direction)
{
	Hit hit;

	// Spheres (Given)
	for (const Sphere& s : spheres) {
		float t = intersectSphere(s, Origin, direction);
		if (t > 0 && (hit.t < 0 || t < hit.t)) {
			hit.t = t;
			hit.point = Origin + t * direction;
			hit.normal = (hit.point - s.centre).normalized();
			hit.material = s.material;
		}
	}

	///TODO (C5): Do the same for every triangle. Its normal is tri.normal.


	return hit;
}

// Shadow ray: true if any object blocks the path from point to the light.
bool inShadow(Vec3 point, Vec3 Normal)
{
	///TODO (C2): Shadow ray (textbook Section 4.7).
	//Start the ray at point + eps * Normal and aim it at lightSource.
	//Return true if a sphere or triangle is hit at a t between 0 and the distance to the light.
	//Skip the light panel (materials[tri.material].isEmissive): it must not block its own light.
	//Glass casts an ordinary (opaque) shadow.


	return false;   // remove this line when you are done
}


// ============================== Shading ==============================

// Blinn-Phong with a shadow test (textbook Section 4.5.3 and 4.7):
//   L = k_a I_a + k_d I max(0, n.l) + k_s I max(0, n.h)^p
// In shadow, only the ambient term remains.
Vec3 blinnPhong(Vec3 point, Vec3 Normal, Vec3 direction, const Material& m)
{
	Vec3 L = m.k_a.cwiseProduct(I_a);

	///TODO (C2): The rest of Blinn-Phong, as in Recitation 02, now with a shadow test.
	//If inShadow(point, Normal), return the ambient term L only.
	//Otherwise add the diffuse and specular terms for the light at lightSource.


	return L;
}

// Mirror reflection: r = d - 2 (d.n) n
Vec3 reflect(Vec3 d, Vec3 n)
{
	///TODO (C2): Return the normalized reflected direction.


	return d;   // replace with your result
}


// ============================== Tracing ==============================

Vec3 trace(Vec3 Origin, Vec3 direction, int depth)
{
	if (depth > maxDepth) return Vec3(0, 0, 0);

	Hit hit = closestHit(Origin, direction);
	if (hit.t < 0) return background;

	const Material& m = materials[hit.material];

	// Light panel: visible as plain white (Given)
	if (m.isEmissive) return Vec3(1, 1, 1);

	Vec3 Normal = hit.normal;
	Vec3 point = hit.point;

	// ---- Glass: reflection and refraction weighted by Fresnel ----
	if (m.isGlass) {
		///TODO (C2, C3, C4): Glass.
		//1. Entering or leaving? Use the sign of direction.dot(Normal). When leaving the glass,
		//   flip the normal and swap the indices n1 and n2 (air is 1, glass is m.n).
		//2. Specular highlight: blinnPhong(point, Normal, direction, m).
		//3. Reflected colour: trace the reflected ray from point + eps * Normal, with depth + 1.
		//4. Refraction direction with Snell's law (textbook Section 13.1).
		//   If the term under the square root is negative: total internal reflection (C4).
		//   Return the highlight plus the reflected colour.
		//5. Refracted colour: trace from point - eps * Normal, with depth + 1.
		//6. Schlick's approximation (C3): R = R0 + (1 - R0)(1 - cos theta)^5,
		//   using the angle on the side with the lower index.
		//7. Return highlight + R * reflected + (1 - R) * refracted.


		return Vec3(0, 0, 0);   // replace with your result
	}

	// ---- Opaque surfaces: Blinn-Phong plus mirror reflection (textbook Section 4.8) ----
	Vec3 c = blinnPhong(point, Normal, direction, m);

	///TODO (C2): Mirror reflection.
	//If the material reflects (m.k_m has a non-zero entry), trace the reflected ray
	//from point + eps * Normal with depth + 1, and add k_m times its colour:
	//  c = c + k_m * reflected   (channel by channel: cwiseProduct)


	return c;
}


int main(int, char**) {

	buildCornellBox();

	Image image = Image(imageSize, imageSize);

	float width = urc(0) - llc(0);    //width of the image plane
	float height = urc(1) - llc(1);   //height of the image plane
	Vec2 pixelUV = Vec2(width / image.cols, height / image.rows); //pixel spacing

	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {

			///TODO (C6): Jittered supersampling.
			//Split the pixel into a samplesN x samplesN grid and send one ray through a random point
			//in each cell: replace the + 0.5 below with (cell index + uniform(rng)) / samplesN.
			//Average the colours of all the rays.

			// One ray through the centre of the pixel (Given, same as Recitation 02)
			Vec3 pixelPos;
			pixelPos(0) = llc(0) + pixelUV(0) * (j + 0.5f);
			pixelPos(1) = urc(1) - pixelUV(1) * (i + 0.5f);   // row 0 is the top
			pixelPos(2) = llc(2);

			Vec3 direction = (pixelPos - eye).normalized();
			Vec3 c = trace(eye, direction, 0);

			Colour colour(0, 0, 0);
			colour[0] = Clamp(c[0] * 255);
			colour[1] = Clamp(c[1] * 255);
			colour[2] = Clamp(c[2] * 255);
			image(i, j) = colour;
		}
		if (i % 100 == 0) std::cout << "row " << i << " of " << image.rows << std::endl;
	}

	image.save("./cornell.png");
	image.display();

	return EXIT_SUCCESS;
}
