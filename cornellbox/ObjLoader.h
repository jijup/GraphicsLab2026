/* Minimal OBJ loader for Assignment 1 of CSCI 4471: Computer Graphics
   Reads only what a ray tracer needs: vertex positions (v lines) and faces (f lines).
   Texture coordinates, normals, materials and groups are ignored.

   Usage:
       std::vector<Eigen::Vector3f> vertices;
       std::vector<std::array<int, 3>> faces;
       if (!loadOBJ("assets/bunny_cornell.obj", vertices, faces)) { ...error... }

       // Triangle k has corners vertices[faces[k][0]], vertices[faces[k][1]], vertices[faces[k][2]]

   Indices in faces start at 0 (the file's 1-based indices are converted).
   Faces with more than three corners are split into triangles (a fan around the first corner).
   Eigen::Vector3f is the same type as the Vec3 in main.cpp. */

#pragma once

#include <Eigen/Dense>

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

inline bool loadOBJ(const std::string& filename,
	std::vector<Eigen::Vector3f>& vertices,
	std::vector<std::array<int, 3>>& faces)
{
	std::ifstream file(filename);
	if (!file) {
		std::cerr << "loadOBJ: cannot open " << filename << std::endl;
		return false;
	}

	vertices.clear();
	faces.clear();

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream in(line);
		std::string tag;
		in >> tag;

		if (tag == "v") {
			float x, y, z;
			in >> x >> y >> z;
			vertices.push_back(Eigen::Vector3f(x, y, z));
		}
		else if (tag == "f") {
			// Each corner looks like "7", "7/2", "7//3" or "7/2/3": keep only the number before the first '/'
			std::vector<int> corner;
			std::string token;
			while (in >> token) {
				int index = std::stoi(token.substr(0, token.find('/')));
				if (index < 0) index = (int)vertices.size() + index;   // negative: counts back from the end
				else index = index - 1;                                 // OBJ indices start at 1
				corner.push_back(index);
			}
			for (size_t k = 1; k + 1 < corner.size(); ++k)
				faces.push_back({ corner[0], corner[k], corner[k + 1] });
		}
		// every other line (comments, vt, vn, usemtl, ...) is skipped
	}

	std::cout << "loadOBJ: " << filename << ": " << vertices.size() << " vertices, "
		<< faces.size() << " triangles" << std::endl;
	return true;
}
