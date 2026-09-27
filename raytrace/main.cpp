/* Starter Code for Recitation 02 of CSCI 4471/6671: Computer Graphics
   A first recursive ray tracer: a golden sphere on a shiny floor.
   Please refer to the corresponding wiki page and read the instructions carefully
   while completing the code. */

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "Image.h"

//Data types: typedef gives an existing type a shorter name.
//Vec3 is Eigen's 3x1 matrix of floats (a 3D vector), Vec2 is a 2x1 one (a 2D vector).
typedef Eigen::Matrix<float, 3, 1> Vec3;
typedef Eigen::Matrix<float, 2, 1> Vec2;

//Color type: "using" gives an existing type a shorter name, the same as typedef (it is the newer C++11 form).
//Colour is just another name for cv::Vec3b: 3 bytes (0 to 255) holding one pixel in BGR order.
using Colour = cv::Vec3b;

//bounding the channel wise pixel color between 0 to 255
unsigned char Clamp(int pixelCol)
{
	if (pixelCol < 0) return 0;
	if (pixelCol >= 255) return 255;
	return pixelCol;
}

// ============================== Scene ==============================
// All colours are BGR floats in [0,1], the same order OpenCV uses.
// Materials and lights follow the textbook (Marschner & Shirley, Section 4.5):
//   k_a, k_d, k_s  ambient, diffuse and specular coefficients of a surface (one value each for B, G, R)
//   I_a, I         ambient light intensity and light source intensity
//   p              Phong exponent (shininess)

// The sphere
Vec3 SpherePos = Vec3(0.0f, -0.5f, -3.5f);
float sphereRadius = 1.0f;
Vec3 sphere_ka = Vec3(0.12f, 0.67f, 0.92f);     // gold (B, G, R)
Vec3 sphere_kd = Vec3(0.12f, 0.67f, 0.92f);     // gold
Vec3 sphere_ks = Vec3(0.6f, 0.6f, 0.6f);        // grey: the highlight takes the light's colour

// The floor: the plane y = floorY
float floorY = -1.5f;
Vec3 floor_ka = Vec3(0.41f, 0.39f, 0.08f);      // deep teal (B, G, R)
Vec3 floor_kd = Vec3(0.41f, 0.39f, 0.08f);      // deep teal
Vec3 floor_ks = Vec3(0.6f, 0.6f, 0.6f);         // grey
float floorReflect = 0.4f;                      // 0 = matte, 1 = perfect mirror

// Background: colour of rays that hit nothing
Vec3 background = Vec3(1.0f, 1.0f, 1.0f);       // white

// Light
Vec3 lightSource(3.0f, 5.0f, 1.0f);             // position of the point light
Vec3 I = Vec3(1.0f, 1.0f, 1.0f);                // light intensity: white
Vec3 I_a = Vec3(0.25f, 0.25f, 0.25f);           // ambient light intensity: dim grey
float p = 64.0f;                                // Phong exponent (shininess)

// Recursion
int maxDepth = 6;       // how many bounces a ray may make
float eps = 1e-3f;      // small offset so a new ray does not hit its own surface


// ============================== Intersections ==============================

// Ray-sphere: returns the distance t to the hit point, or -1 if there is no hit.
float intersectSphere(Vec3 Origin, Vec3 direction)
{
	///TODO (Step 2): Ray-sphere intersection (refer to the lecture slides and Section 4.4.1 of the textbook)

	//Build the coefficients of the quadratic equation, A, B & C
	//  A = d.d,   B = 2 d.(o - c),   C = (o - c).(o - c) - r^2
	//  where o = Origin, d = direction, c = SpherePos, r = sphereRadius


	//Compute the discriminant B^2 - 4AC. If it is negative, the ray misses: return -1


	//Find the smaller root t2 = (-B - sqrt(discriminant)) / (2A): the front of the sphere.
	//If t2 > eps, return it. Otherwise the sphere is behind the ray: return -1


	return -1;   // remove this line when you are done
}

// Ray-plane intersection (lecture slides):  t = -((o - p) . n) / (d . n)
//   o = Origin, d = direction, p = any point on the plane, n = plane normal
// For the floor: n = (0, 1, 0) and p = (0, floorY, 0), so
//   (o - p) . n = Origin(1) - floorY     and     d . n = direction(1). NOTE: direction(1) means the y component of direction vector
// which gives  t = (floorY - Origin(1)) / direction(1)
float intersectFloor(Vec3 Origin, Vec3 direction)
{
	if (direction(1) == 0) return -1;              // d . n = 0: ray parallel to the floor
	float t = (floorY - Origin(1)) / direction(1);
	if (t > eps) return t;                         // t <= 0: the floor is behind the ray
	return -1;
}


// ============================== Shading ==============================

// Blinn-Phong (textbook Section 4.5.3):
//   L = k_a I_a + k_d I max(0, n.l) + k_s I max(0, n.h)^p
// Returns the colour as BGR floats (not yet multiplied by 255).
Vec3 blinnPhong(Vec3 point, Vec3 Normal, Vec3 direction, Vec3 k_a, Vec3 k_d, Vec3 k_s)
{
	///TODO (Step 3): Blinn-Phong shading

	//Light vector l: lightSource - point. Normalize it.


	//Diffuse term max(0, n.l): LightVector.dot(Normal). If it is negative, set it to 0.


	//Half vector h = l + v, where the view vector v is the opposite of the ray direction (-direction).
	//Normalize it.


	//Specular term max(0, n.h): HalfVec.dot(Normal). If it is negative, set it to 0.


	//Combine: L = k_a I_a + k_d I max(0, n.l) + k_s I max(0, n.h)^p
	//k_a I_a means B*B, G*G, R*R, one channel at a time. In Eigen that is k_a.cwiseProduct(I_a).
	//(Plain * between two Vec3 does not compile: Eigen reads it as a matrix product.)
	//Use pow(x, p) for x^p.


	return k_a.cwiseProduct(I_a);   // ambient only: replace with your result
}


// ============================== Tracing ==============================

// Follow one ray and return its colour. The shiny floor calls trace() again.
Vec3 trace(Vec3 Origin, Vec3 direction, int depth)
{
	if (depth > maxDepth) return Vec3(0, 0, 0);

	// Find the closest object (Given)
	float tSphere = intersectSphere(Origin, direction);
	float tFloor = intersectFloor(Origin, direction);

	// nothing hit: background
	if (tSphere < 0 && tFloor < 0) return background;

	// sphere is closer
	if (tFloor < 0 || (tSphere > 0 && tSphere < tFloor)) {
		Vec3 Intersection = Origin + direction * tSphere;
		Vec3 Normal = Intersection - SpherePos;
		Normal.normalize();
		return blinnPhong(Intersection, Normal, direction, sphere_ka, sphere_kd, sphere_ks);
	}

	// ---- floor is closer ----
	Vec3 Intersection = Origin + direction * tFloor;
	Vec3 Normal = Vec3(0, 1, 0);
	Vec3 local = blinnPhong(Intersection, Normal, direction, floor_ka, floor_kd, floor_ks);

	///TODO (Step 4): Reflection

	//Reflected direction: r = d - 2 (d.n) n, where d = direction and n = Normal. Normalize it.
	//Check: for n = (0, 1, 0), r is just d with its y component flipped.


	//Start the new ray slightly above the floor (Intersection + eps * Normal)
	//and call trace() again with depth + 1 to get the reflected colour.


	//Blend the floor's own shading with the reflection:
	//  c = (1 - floorReflect) * local + floorReflect * reflected


	Vec3 c = local;   // no reflection yet: replace with your blend

	float fog = std::min(1.0f, tFloor / 40.0f);   // blend the far floor into the background (Given)
	return (1 - fog) * c + fog * background;  //LERP: Linear interpolation
}


int main(int, char**) {

	//Create an image object with 500 x 500 resolution.
	Image image = Image(500, 500);

	//Coordinates of the image rectangle, shifted down a little so the floor fills more of the picture
	Vec3 llc = Vec3(-1, -1.4f, -1);   //lower left corner
	Vec3 urc = Vec3(1, 0.6f, -1);     //upper right corner
	float width = urc(0) - llc(0);    //width of the image plane
	float height = urc(1) - llc(1);   //height of the image plane
	Vec2 pixelUV = Vec2(width / image.cols, height / image.rows); //pixel spacing

	Vec3 cameraPoint = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 Origin = cameraPoint;

	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			///TODO (Step 1): Build the primary ray.
			//Find the pixel position (pixelPos) on the image plane for row i and column j:
			//  x: start at llc(0) and move right by pixelUV(0) * (j + 0.5)
			//  y: row 0 is the TOP, so start at urc(1) and move down by pixelUV(1) * (i + 0.5)
			//  z: -1
			//The + 0.5 puts the ray through the centre of the pixel.
			//Then the ray direction is pixelPos - Origin. Normalize it.

			Vec3 direction(0, 0, -1);   // replace with your ray direction


			Vec3 c = trace(Origin, direction, 0);

			Colour colour(0, 0, 0);
			colour[0] = Clamp(c[0] * 255);
			colour[1] = Clamp(c[1] * 255);
			colour[2] = Clamp(c[2] * 255);
			image(i, j) = colour;
		}
	}

	image.save("./result.png");
	image.display();

	return EXIT_SUCCESS;
}