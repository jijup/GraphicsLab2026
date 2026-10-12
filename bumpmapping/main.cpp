/*CSCI 4471: Computer Graphics (Recitation 04)
  Bump mapping, normal mapping and bilinear texture lookups.
  Scene: an orange (a sphere with a bump map) and a 498-triangle Stanford bunny whose normal map
  was baked from the full 69,451-triangle scan.*/

#include "Triangle.h"
#include "Image.h"
#include "Texture.h"
#include "ObjLoader.h"
#include <cstdlib>
#include <algorithm>
#include <vector>

#ifndef ASSETS_DIR
#define ASSETS_DIR "assets"
#endif

const float PI = 3.14159265f;

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

// The orange: a sphere with a bump map
Vec3 orangeCenter(-1.45f, 0.0f, -4.2f);
float orangeRadius = 1.0f;
float bumpScale = 0.0025f;        // height of the bumps: the surface moves by bumpScale * d, d in [0,1]
int orangeMode = 0;              // 0 = smooth sphere, 1 = bump map (Task 1), 2 = normal map (Task 2)

// The bunny: a triangle mesh standing at bunnyPos, turned by bunnyAngle degrees about the y axis
Vec3 bunnyPos(1.35f, -1.0f, -4.2f);
float bunnyAngle = 0.0f;
int bunnyMode = 1;               // 0 = flat, 1 = smooth, 2 = normal map (Task 2)
Vec3 bunnyColor(0.72f, 0.80f, 0.86f);   // light clay (B, G, R)

// Textures: 0 = nearest texel, 1 = bilinear (Task 3)
int textureFilter = 0;

// View: zoom = 1 shows the whole scene. A larger zoom looks closely at the point viewCenter of the image plane.
float zoom = 1.0f;
Vec2 viewCenter(0.0f, 0.0f);

// Background: colour of rays that hit nothing
Vec3 background(1.0f, 1.0f, 1.0f);      // white

// Light: a point light, Blinn-Phong shading
Vec3 lightPos(-3.0f, 4.0f, 1.0f);
float I = 1.0f;                  // light intensity
float I_a = 0.15f;               // ambient light intensity
float k_s = 0.35f;               // specular coefficient
float p = 30.0f;                 // Phong exponent

float epsilon = 0.00001f;
float eps = 1e-3f;               // hits closer than this along a ray are ignored

// Textures and the mesh, loaded in main()
Texture orangeColorTex, orangeBumpTex, orangeNormalTex, bunnyNormalTex;
std::vector<Triangle> bunny;
Vec3 bunnyCenter; float bunnyRadius;    // a sphere around the bunny, to skip rays that miss it


// ============================== The orange ==============================

// Ray-sphere: returns the distance t to the closest hit in front of the ray, or -1 if there is no hit.
float intersectSphere(Vec3 Origin, Vec3 Direction, Vec3 center, float radius)
{
	Vec3 oc = Origin - center;
	float b = Direction.dot(oc);
	float c = oc.dot(oc) - radius * radius;
	float disc = b * b - c;               // Direction has unit length
	if (disc < 0) return -1;
	float t = -b - sqrt(disc);
	if (t <= eps) t = -b + sqrt(disc);
	return (t > eps) ? t : -1;
}

// Angles of a point on the orange: theta from the +y axis (0 at the top), phi around the y axis
// (0 facing the camera). The point is
//     p(u, v) = c + r (sin(theta) sin(phi), cos(theta), sin(theta) cos(phi))
// with theta = pi (1 - v) and phi = 2 pi u - pi.
void sphereAngles(Vec3 point, float& theta, float& phi)
{
	Vec3 d = (point - orangeCenter) / orangeRadius;
	theta = acos(std::min(std::max(d(1), -1.0f), 1.0f));
	phi = atan2(d(0), d(2));
}

// Texture coordinates (u, v) of a point on the orange
Vec2 sphereUV(Vec3 point)
{
	float theta, phi;
	sphereAngles(point, theta, phi);
	return Vec2((phi + PI) / (2 * PI), 1.0f - theta / PI);
}

// Task 1a: the tangent vectors p_u and p_v of the orange at a point.
void sphereTangents(Vec3 point, Vec3& p_u, Vec3& p_v)
{
	float theta, phi;
	sphereAngles(point, theta, phi);
	float r = orangeRadius;

	// p_u = dp/du is given. It comes from differentiating p(u, v) (see sphereAngles()) with phi = 2 pi u - pi,
	// so dphi/du = 2 pi.
	p_u = 2 * PI * r * Vec3(sin(theta) * cos(phi), 0.0f, -sin(theta) * sin(phi));

	// TODO (Task 1a): p_v = dp/dv.
	// 1. Differentiate p(u, v) = c + r (sin(theta) sin(phi), cos(theta), sin(theta) cos(phi)) with respect to theta.
	// 2. Multiply by dtheta/dv. Since theta = pi (1 - v), dtheta/dv = -pi.
	// Check: p_u x p_v must point out of the orange, along (point - orangeCenter).
	p_v = Vec3(0, 0, 0);   // replace
}

// Height of the bump map at texture coordinates uv, scaled by bumpScale
float bumpHeight(Vec2 uv)
{
	return bumpScale * orangeBumpTex.lookup(uv)[0];
}

// Task 1: the bump mapped normal of the orange at a point (Blinn 1978).
Vec3 bumpNormal(Vec3 point)
{
	Vec3 p_u, p_v;
	sphereTangents(point, p_u, p_v);
	Vec3 n = p_u.cross(p_v).normalized();
	Vec2 uv = sphereUV(point);

	// TODO (Task 1b): the partial derivatives d_u and d_v of the height d(u, v) = bumpHeight(uv).
	// Use central differences, one texel apart:
	// 1. One texel in u is du = 1.0f / orangeBumpTex.width. In v it is dv = 1.0f / orangeBumpTex.height.
	// 2. d_u = (d(u + du, v) - d(u - du, v)) / (2 du), and the same for d_v.
	//    For d(u + du, v) call bumpHeight(uv + Vec2(du, 0)).
	float d_u = 0;   // replace
	float d_v = 0;   // replace

	// TODO (Task 1c): the bump mapped normal n' = p'_u x p'_v (Section 2 of the recitation page).
	// 1. Expand p'_u x p'_v, with p'_u = p_u + d_u n and p'_v = p_v + d_v n. The term with n x n is zero.
	// 2. Return the result normalized.
	Vec3 nPrime = p_u.cross(p_v);   // replace
	return nPrime.normalized();
}


// ============================== Normal maps ==============================

// Task 2: turn a normal map colour (BGR floats in [0,1]) into a unit normal (x, y, z).
Vec3 decodeNormal(Vec3 colour)
{
	// TODO (Task 2): decode the colour into a normal.
	// 1. A normal map stores x in the red channel, y in green and z in blue. colour is in BGR order,
	//    so red is colour(2) and blue is colour(0).
	// 2. Each channel maps [0, 1] back to [-1, 1]: x = 2 r - 1, and the same for y and z.
	// 3. Normalize the result: 8-bit colours and bilinear lookups leave it slightly off unit length.
	return Vec3(0, 0, 1);   // replace
}


// ============================== The bunny ==============================

// Ray-mesh: returns the distance t to the closest triangle, or -1. hitTri is set to that triangle.
float intersectMesh(Vec3 Origin, Vec3 Direction, int& hitTri)
{
	hitTri = -1;
	if (intersectSphere(Origin, Direction, bunnyCenter, bunnyRadius) < 0 &&
		(Origin - bunnyCenter).norm() > bunnyRadius) return -1;

	float tBest = -1;
	for (int k = 0; k < (int)bunny.size(); ++k) {
		Triangle& tri = bunny[k];
		Vec3 tNormal = tri.normal_.normalized();
		float denom = Direction.dot(tNormal);
		if (fabs(denom) < epsilon) continue;
		float t = (tri.vertex1_ - Origin).dot(tNormal) / denom;
		if (t <= eps || (tBest > 0 && t >= tBest)) continue;

		Vec3 baryCoords = tri.BaryCentric(Origin + t * Direction);
		if (baryCoords[0] >= 0 && baryCoords[1] >= 0 && baryCoords[2] >= 0) {
			tBest = t;
			hitTri = k;
		}
	}
	return tBest;
}

// Shading normal of the bunny at a point of triangle tri, for the current bunnyMode
Vec3 bunnyNormal(Triangle& tri, Vec3 point)
{
	if (bunnyMode == 0) return tri.normal_.normalized();             // flat
	if (bunnyMode == 1) return tri.getVertexNormal(point);           // smooth
	return decodeNormal(bunnyNormalTex.lookup(tri.getTextCoord(point)));   // normal map
}

// Shading normal of the orange at a point, for the current orangeMode
Vec3 orangeNormal(Vec3 point)
{
	if (orangeMode == 0) return (point - orangeCenter).normalized();
	if (orangeMode == 1) return bumpNormal(point);
	return decodeNormal(orangeNormalTex.lookup(sphereUV(point)));
}


// ============================== Shading ==============================

// Blinn-Phong with ambient light: k_d (I_a + I max(0, n.l)) + k_s I max(0, n.h)^p
Vec3 blinnPhong(Vec3 point, Vec3 normal, Vec3 Direction, Vec3 k_d)
{
	Vec3 l = (lightPos - point).normalized();
	Vec3 v = -Direction;
	Vec3 h = (l + v).normalized();
	float diffuse = std::max(0.0f, normal.dot(l));
	float specular = (diffuse > 0) ? pow(std::max(0.0f, normal.dot(h)), p) : 0.0f;
	return k_d * (I_a + I * diffuse) + Vec3(1, 1, 1) * k_s * I * specular;
}

// Colour seen along one ray, as BGR floats in [0,1].
Vec3 trace(Vec3 Origin, Vec3 Direction)
{
	float tOrange = intersectSphere(Origin, Direction, orangeCenter, orangeRadius);
	int tri;
	float tBunny = intersectMesh(Origin, Direction, tri);

	if (tOrange > 0 && (tBunny < 0 || tOrange < tBunny)) {
		Vec3 point = Origin + tOrange * Direction;
		Vec3 k_d = orangeColorTex.lookup(sphereUV(point));
		return blinnPhong(point, orangeNormal(point), Direction, k_d);
	}
	if (tBunny > 0) {
		Vec3 point = Origin + tBunny * Direction;
		return blinnPhong(point, bunnyNormal(bunny[tri], point), Direction, bunnyColor);
	}
	return background;
}


// ============================== Loading ==============================

// Loads the bunny, turns it by bunnyAngle about y, moves it to bunnyPos and computes its vertex normals
bool loadBunny(const std::string& filename)
{
	std::vector<Eigen::Vector3f> vertices;
	std::vector<std::array<int, 3>> faces;
	std::vector<Eigen::Vector2f> uvs;
	std::vector<std::array<int, 3>> uvFaces;
	if (!loadOBJ(filename, vertices, faces, uvs, uvFaces)) return false;

	float a = bunnyAngle * PI / 180.0f;
	for (Vec3& v : vertices)
		v = Vec3(cos(a) * v(0) + sin(a) * v(2), v(1), -sin(a) * v(0) + cos(a) * v(2)) + bunnyPos;

	// vertex normals: the sum of the (area weighted) normals of the triangles around each vertex
	std::vector<Vec3> vnormals(vertices.size(), Vec3(0, 0, 0));
	for (auto& f : faces) {
		Vec3 n = (vertices[f[1]] - vertices[f[0]]).cross(vertices[f[2]] - vertices[f[0]]);
		for (int k = 0; k < 3; ++k) vnormals[f[k]] += n;
	}
	for (Vec3& n : vnormals) n.normalize();

	bunny.clear();
	for (size_t k = 0; k < faces.size(); ++k) {
		Triangle tri(vertices[faces[k][0]], vertices[faces[k][1]], vertices[faces[k][2]]);
		tri.text_coord1 = uvs[uvFaces[k][0]];
		tri.text_coord2 = uvs[uvFaces[k][1]];
		tri.text_coord3 = uvs[uvFaces[k][2]];
		tri.vnormal1_ = vnormals[faces[k][0]];
		tri.vnormal2_ = vnormals[faces[k][1]];
		tri.vnormal3_ = vnormals[faces[k][2]];
		bunny.push_back(tri);
	}

	// bounding sphere
	Vec3 lo = vertices[0], hi = vertices[0];
	for (Vec3& v : vertices) { lo = lo.cwiseMin(v); hi = hi.cwiseMax(v); }
	bunnyCenter = (lo + hi) / 2;
	bunnyRadius = (hi - lo).norm() / 2 + eps;
	return true;
}


int main(int, char**) {

	std::string assets = ASSETS_DIR;
	bool ok = orangeColorTex.loadTexture(assets + "/orange_color.png")
		&& orangeBumpTex.loadTexture(assets + "/orange_bump.png")
		&& orangeNormalTex.loadTexture(assets + "/orange_normal.png")
		&& bunnyNormalTex.loadTexture(assets + "/bunny_normal.png")
		&& loadBunny(assets + "/bunny_low.obj");
	if (!ok) {
		std::cerr << "Could not load the assets from " << assets << std::endl;
		return EXIT_FAILURE;
	}

	Texture::filter = textureFilter;

	Image image = Image(800, 400);

	Vec3 llc = Vec3(viewCenter(0) - 0.8f / zoom, viewCenter(1) - 0.4f / zoom, -1);
	Vec3 urc = Vec3(viewCenter(0) + 0.8f / zoom, viewCenter(1) + 0.4f / zoom, -1);
	float width = urc(0) - llc(0);
	float height = urc(1) - llc(1);
	Vec2 pixelUV = Vec2(width / image.cols, height / image.rows);

	Vec3 CameraPoint(0, 0, 0);
	Vec3 Origin = CameraPoint;

	float u, v, w;
	for (int row = 0; row < image.rows; ++row) {
		for (int col = 0; col < image.cols; ++col) {
			u = llc(0) + pixelUV(0) * (col + 0.5f);
			v = urc(1) - pixelUV(1) * (row + 0.5f);
			w = -1;

			Vec3 pixelPos = Vec3(u, v, w);

			Vec3 Direction = pixelPos - Origin;
			Direction.normalize();

			Vec3 pixelColour = trace(Origin, Direction);
			image(row, col) = Colour(Clamp(pixelColour[0] * 255), Clamp(pixelColour[1] * 255), Clamp(pixelColour[2] * 255));
		}
	}

	image.save("./result.png");
	image.display();

	return EXIT_SUCCESS;
}
