/* Starter Code for Recitation 05 of CSCI 4471: Computer Graphics
   Transformations: placing many objects with one unit sphere and 4 x 4 matrices, and a bouncing ball.
   Please refer to the corresponding wiki page and read the instructions carefully
   while completing the code. */

#include <iostream>
#include <iomanip>
#include <cmath>

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "opencv2/videoio.hpp"
#include "Image.h"

//Data types: typedef gives an existing type a shorter name.
//Vec3 is a 3D vector, Vec2 a 2D vector, as in Recitation 2.
//Vec4 is a 4D vector and Mat4 a 4 x 4 matrix: the homogeneous coordinates from the lecture.
typedef Eigen::Matrix<float, 3, 1> Vec3;
typedef Eigen::Matrix<float, 2, 1> Vec2;
typedef Eigen::Matrix<float, 4, 1> Vec4;
typedef Eigen::Matrix<float, 4, 4> Mat4;

//Colour is just another name for cv::Vec3b: 3 bytes (0 to 255) holding one pixel in BGR order.
using Colour = cv::Vec3b;

const float PI = 3.14159265f;

//bounding the channel wise pixel color between 0 to 255
unsigned char Clamp(int pixelCol)
{
	if (pixelCol < 0) return 0;
	if (pixelCol >= 255) return 255;
	return pixelCol;
}


// ============================== Transformations ==============================
// All matrices are 4 x 4 and act on homogeneous coordinates (x, y, z, w).
// Eigen: Mat4::Identity() is the identity matrix, and A(row, col) is one entry (starting at 0).

// Scaling by sx, sy, sz along the three axes (Given): the scale factors go on the diagonal.
Mat4 scale(float sx, float sy, float sz)
{
	Mat4 S = Mat4::Identity();
	S(0, 0) = sx;
	S(1, 1) = sy;
	S(2, 2) = sz;
	return S;
}

// Translation by (tx, ty, tz)
Mat4 translate(float tx, float ty, float tz)
{
	Mat4 T = Mat4::Identity();
	///TODO (Task 1a): Put the translation in the last column (see the lecture slides on homogeneous coordinates)
	//  | 1 0 0 tx |
	//  | 0 1 0 ty |
	//  | 0 0 1 tz |
	//  | 0 0 0 1  |


	return T;
}

// Rotation by "degrees" about the z axis (counter-clockwise when you look down the z axis at the origin)
Mat4 rotateZ(float degrees)
{
	float theta = degrees * PI / 180.0f;   // cos() and sin() want radians
	Mat4 R = Mat4::Identity();
	///TODO (Task 1a): Fill in the 2D rotation in the x-y block (lecture slides, "3D Rotations"):
	//  | cos -sin 0 0 |
	//  | sin  cos 0 0 |
	//  |  0    0  1 0 |
	//  |  0    0  0 1 |


	return R;
}

// Apply a 4 x 4 matrix to a POINT: append w = 1, multiply, keep x, y, z
Vec3 transformPoint(Mat4 A, Vec3 p)
{
	///TODO (Task 1b): Build the Vec4 (p(0), p(1), p(2), 1), multiply it by A,
	//and return the first three components. In Eigen, v.head<3>() gives the first three.

	return p;   // replace
}

// Apply a 4 x 4 matrix to a VECTOR (a direction): append w = 0, multiply, keep x, y, z
Vec3 transformVector(Mat4 A, Vec3 v)
{
	///TODO (Task 1b): The same as transformPoint(), but with w = 0.

	return v;   // replace
}


// ============================== Scene ==============================
// All colours are BGR floats in [0,1]. Materials follow the textbook (k_a, k_d, k_s, I_a, I, p).
//
// Every object in this scene is the SAME unit sphere (centre at the origin, radius 1).
// Each copy, called an instance, has its own matrix M that moves it into the world,
// and its own colour. M[k] maps object space to world space, Minv[k] maps world space back.
const int nInstances = 3;
Mat4 M[nInstances];
Mat4 Minv[nInstances];
Vec3 inst_kd[nInstances] = {
	Vec3(0.12f, 0.67f, 0.92f),      // 0: gold   (B, G, R)
	Vec3(0.20f, 0.25f, 0.85f),      // 1: red
	Vec3(0.85f, 0.45f, 0.15f) };    // 2: blue
Vec3 inst_ks = Vec3(0.6f, 0.6f, 0.6f);   // grey highlight for all three

// The floor: the plane y = floorY
float floorY = -1.5f;
Vec3 floor_kd = Vec3(0.41f, 0.39f, 0.08f);      // deep teal (B, G, R)
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


// Animation (Given): the video is "duration" seconds long, with "fps" frames per second.
float duration = 0.0f;    // length of the video in seconds. 0 = no video, only result.png. Task 3: set it to 8
int fps = 60;             // frames per second
float stillTime = 0.96f;  // the moment (in seconds) shown in result.png: the top of the first bounce


// The physics of the bounce (Given)
float g = 9.8f;              // gravity
float dropHeight = 1.0f;     // the ball is dropped from this height above the floor
float keep = 0.75f;          // each bounce reaches this fraction of the previous height
float contactTime = 0.12f;   // seconds the ball stays on the floor at each landing
float maxSquash = 0.5f;      // how much the ball flattens at the hardest landing (0.5 = half its height)
float maxStretch = 0.15f;    // how much it stretches when it moves fastest

// Stretch along the direction of motion, growing with the speed
float stretch(float speed)
{
	float fastest = sqrt(2.0f * g * dropHeight);                    // the speed when it first hits the floor
	return 1.0f + maxStretch * std::min(1.0f, speed / fastest);
}

// Height h of the ball above the floor and its squash (below 1 = flatter, above 1 = taller) at a given time
void bounceState(float time, float& h, float& squash)
{
	float t = time;                     // time since the drop
	float H = dropHeight;

	// the first fall, from rest at height H
	float fall = sqrt(2.0f * H / g);
	if (t < fall) {
		h = H - 0.5f * g * t * t;
		squash = stretch(g * t);
		return;
	}
	t -= fall;

	for (int n = 0; n < 100; ++n) {
		// on the floor: squash down and back up, harder for faster landings
		if (t < contactTime) {
			float amount = maxSquash * sqrt(H / dropHeight);
			h = 0.0f;
			squash = 1.0f - amount * sin(PI * t / contactTime);
			return;
		}
		t -= contactTime;

		// in the air: a parabola that reaches keep * H
		H *= keep;
		if (H < 0.005f) break;           // too small to see: the ball comes to rest
		float T = 2.0f * sqrt(2.0f * H / g);
		if (t < T) {
			float v0 = sqrt(2.0f * g * H);   // speed when it leaves the floor
			h = v0 * t - 0.5f * g * t * t;
			squash = stretch(fabs(v0 - g * t));
			return;
		}
		t -= T;
	}
	h = 0.0f;                            // resting on the floor
	squash = 1.0f;
}


// Build the matrix of every instance at the given time (in seconds). Called once per image.
void buildScene(float time)
{
	// Instance 0, the gold ball: scale by 0.65, then move it into place (Given).
	// Read the product from RIGHT to LEFT: the scale is applied first, then the translation.
	M[0] = translate(0.0f, -0.85f, -3.8f) * scale(0.65f, 0.65f, 0.65f);

	///TODO (Task 1c): Build the other two matrices in the same way.
	//Instance 1, the red disc: scale by (0.85, 0.25, 0.85), then translate by (-1.45, -1.25, -3.3).
	//Instance 2, the blue egg: scale by (0.35, 0.8, 0.35), then rotate by -30 degrees about z,
	//                          then translate by (1.3, -0.78, -3.3).

	M[1] = Mat4::Identity();   // replace
	M[2] = Mat4::Identity();   // replace

	// The bounce (Given): height h of the ball above the floor and its squash at this time
	float h, squash;
	bounceState(time, h, squash);
	float wide = 1.0f / sqrt(squash);   // wider when flatter, narrower when stretched, so the ball keeps its volume

	///TODO (Task 3): The bouncing ball. Replace M[0] with ONE product that, read from RIGHT to LEFT,
	//  1. scales the unit sphere by 0.65                      (the ball, centred at the origin)
	//  2. translates it by (0, 0.65, 0)                       (it now sits ON the origin, which is its contact point)
	//  3. scales by (wide, squash, wide)                      (squashes or stretches it about the contact point)
	//  4. translates it by (0, floorY + h, -3.8)              (puts the contact point h above the floor)


	// The inverse matrices, computed once here so we do not invert for every ray (Given)
	for (int k = 0; k < nInstances; ++k)
		Minv[k] = M[k].inverse();
}


// ============================== Intersections ==============================

// Ray against the unit sphere (centre at the origin, radius 1), in OBJECT space (Given).
// The same quadratic as Recitation 2 with c = 0 and r = 1. Returns t, or -1 for a miss.
float intersectUnitSphere(Vec3 o, Vec3 d)
{
	float A = d.dot(d);
	float B = 2.0f * d.dot(o);
	float C = o.dot(o) - 1.0f;
	float disc = B * B - 4.0f * A * C;
	if (disc < 0) return -1;
	float t = (-B - sqrt(disc)) / (2.0f * A);
	if (t > eps) return t;
	t = (-B + sqrt(disc)) / (2.0f * A);   // we are inside the sphere: take the far side
	if (t > eps) return t;
	return -1;
}

// Ray against instance k, given in WORLD space. Returns t, or -1 for a miss.
float intersectInstance(int k, Vec3 Origin, Vec3 direction)
{
	///TODO (Task 2a): Intersect the ray with instance k
	//1. Move the ray into object space with Minv[k]:
	//   the origin is a POINT (transformPoint), the direction is a VECTOR (transformVector).
	//   Do NOT normalize the new direction.
	//2. Intersect the moved ray with the unit sphere (call intersectUnitSphere with o and d) and return its t.

	return -1;   // remove this line
}

// Ray-plane intersection with the floor y = floorY (Given, from Recitation 2)
float intersectFloor(Vec3 Origin, Vec3 direction)
{
	if (direction(1) == 0) return -1;
	float t = (floorY - Origin(1)) / direction(1);
	if (t > eps) return t;
	return -1;
}

// Is the light blocked by an instance? (Given)
// The shadow ray is not normalized, so it reaches the light at t = 1.
bool inShadow(Vec3 point)
{
	Vec3 ShadowDir = lightSource - point;
	for (int k = 0; k < nInstances; ++k) {
		float t = intersectInstance(k, point, ShadowDir);
		if (t > 0 && t < 1) return true;
	}
	return false;
}


// ============================== Shading ==============================

// Blinn-Phong (textbook Section 4.5.3), with k_a = k_d (Given, from Recitation 2):
//   L = k_a I_a + k_d I max(0, n.l) + k_s I max(0, n.h)^p
// In shadow only the ambient term is left.
Vec3 blinnPhong(Vec3 point, Vec3 Normal, Vec3 direction, Vec3 k_d, Vec3 k_s)
{
	Vec3 ambient = k_d.cwiseProduct(I_a);
	if (inShadow(point + eps * Normal)) return ambient;

	Vec3 LightVector = lightSource - point;
	LightVector.normalize();
	float diffuse = std::max(0.0f, LightVector.dot(Normal));

	Vec3 HalfVec = LightVector - direction;
	HalfVec.normalize();
	float specular = std::max(0.0f, HalfVec.dot(Normal));

	return ambient + k_d.cwiseProduct(I) * diffuse + k_s.cwiseProduct(I) * pow(specular, p);
}


// ============================== Tracing ==============================

// Follow one ray and return its colour. The shiny floor calls trace() again.
Vec3 trace(Vec3 Origin, Vec3 direction, int depth)
{
	if (depth > maxDepth) return Vec3(0, 0, 0);

	// Find the closest instance (Given)
	int hitK = -1;
	float tMin = 1e30f;
	for (int k = 0; k < nInstances; ++k) {
		float t = intersectInstance(k, Origin, direction);
		if (t > 0 && t < tMin) { tMin = t; hitK = k; }
	}
	float tFloor = intersectFloor(Origin, direction);

	// nothing hit: background
	if (hitK < 0 && tFloor < 0) return background;

	// an instance is closer
	if (tFloor < 0 || (hitK >= 0 && tMin < tFloor)) {
		Vec3 Intersection = Origin + direction * tMin;

		// The hit point on the unit sphere, in object space. For the unit sphere the normal is the point itself.
		Vec3 objPoint = transformPoint(Minv[hitK], Intersection);
		Vec3 objNormal = objPoint;

		///TODO (Task 2b): Bring the normal into world space with the inverse transpose of M:
		//use transformVector() with Minv[hitK].transpose() instead of M[hitK].
		Vec3 Normal = transformVector(M[hitK], objNormal);   // WRONG when the scale is not uniform: replace
		Normal.normalize();

		direction.normalize();   // blinnPhong() needs a unit view direction
		return blinnPhong(Intersection, Normal, direction, inst_kd[hitK], inst_ks);
	}

	// ---- floor is closer (Given, from Recitation 2) ----
	Vec3 Intersection = Origin + direction * tFloor;
	Vec3 Normal = Vec3(0, 1, 0);
	direction.normalize();
	Vec3 local = blinnPhong(Intersection, Normal, direction, floor_kd, floor_ks);

	Vec3 reflectDir = direction - 2 * direction.dot(Normal) * Normal;
	reflectDir.normalize();
	Vec3 reflected = trace(Intersection + eps * Normal, reflectDir, depth + 1);
	Vec3 c = (1 - floorReflect) * local + floorReflect * reflected;

	float fog = std::min(1.0f, tFloor / 40.0f);   // blend the far floor into the background
	return (1 - fog) * c + fog * background;
}


// Ray trace the scene into an image (Given)
void render(Image& image)
{
	//Coordinates of the image rectangle, shifted down a little so the floor fills more of the picture
	Vec3 llc = Vec3(-0.75f, -0.95f, -1);   //lower left corner
	Vec3 urc = Vec3(0.75f, 0.55f, -1);     //upper right corner
	float width = urc(0) - llc(0);
	float height = urc(1) - llc(1);
	Vec2 pixelUV = Vec2(width / image.cols, height / image.rows); //pixel spacing

	Vec3 Origin = Vec3(0.0f, 0.0f, 0.0f);   //the camera

	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			//The primary ray through the centre of pixel (i, j), as in Recitation 2
			Vec3 pixelPos = Vec3(llc(0) + pixelUV(0) * (j + 0.5f), urc(1) - pixelUV(1) * (i + 0.5f), -1.0f);
			Vec3 direction = pixelPos - Origin;
			direction.normalize();

			Vec3 c = trace(Origin, direction, 0);

			Colour colour(0, 0, 0);
			colour[0] = Clamp(c[0] * 255);
			colour[1] = Clamp(c[1] * 255);
			colour[2] = Clamp(c[2] * 255);
			image(i, j) = colour;
		}
	}
}


int main(int, char**) {

	// A quick test of Task 1 (Given). Compare what is printed with the expected values.
	Vec3 a = transformPoint(translate(1, 2, 3) * rotateZ(90), Vec3(1, 0, 0));
	Vec3 b = transformVector(translate(1, 2, 3) * rotateZ(90), Vec3(1, 0, 0));
	std::cout << std::fixed << std::setprecision(2) << "Task 1 check" << std::endl;
	std::cout << "  point  (1, 0, 0) rotated and moved: " << a.transpose() << "    (expected: 1.00 3.00 3.00)" << std::endl;
	std::cout << "  vector (1, 0, 0) rotated and moved: " << b.transpose() << "    (expected: 0.00 1.00 0.00)" << std::endl;

	//Create an image object with 500 x 500 resolution.
	Image image = Image(500, 500);

	// The video: one image per frame, written to bounce.avi
	int nFrames = int(duration * fps);
	if (nFrames > 0) {
		cv::VideoWriter video("./bounce.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(image.cols, image.rows));
		for (int f = 0; f < nFrames; ++f) {
			buildScene(float(f) / fps);   // the time of frame f, in seconds
			render(image);
			video.write(image.image);
			cv::imshow("image", image.image);   // show each frame as it is finished
			cv::waitKey(1);
			std::cout << "\r  frame " << f + 1 << " of " << nFrames << std::flush;
		}
		std::cout << "\n  saved bounce.avi" << std::endl;
	}

	// The still image at time = stillTime
	buildScene(stillTime);
	render(image);
	image.save("./result.png");
	image.display();

	return EXIT_SUCCESS;
}
