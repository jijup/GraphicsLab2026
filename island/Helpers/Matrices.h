#pragma once
#include <cmath>
#include "Helpers/ShaderHelper.h"

/// The viewing and projection matrices from the Viewing lecture.
/// Same as glm::lookAt and glm::perspective.

/// Camera at "eye", looking at "center", with "up" pointing up.
inline Mat4x4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 w = (eye - center).normalized();   // camera looks along -w
    Vec3 u = up.cross(w).normalized();      // right
    Vec3 v = w.cross(u);                    // up
    Mat4x4 V = Mat4x4::Identity();
    V.block<1, 3>(0, 0) = u.transpose();
    V.block<1, 3>(1, 0) = v.transpose();
    V.block<1, 3>(2, 0) = w.transpose();
    V(0, 3) = -u.dot(eye);
    V(1, 3) = -v.dot(eye);
    V(2, 3) = -w.dot(eye);
    return V;
}

/// Perspective projection. fovy is the vertical field of view in degrees.
inline Mat4x4 perspective(float fovy, float aspect, float zNear, float zFar) {
    float t = std::tan(fovy * 3.14159265f / 360.0f);   // tan(fovy / 2)
    Mat4x4 P = Mat4x4::Zero();
    P(0, 0) = 1.0f / (aspect * t);
    P(1, 1) = 1.0f / t;
    P(2, 2) = -(zFar + zNear) / (zFar - zNear);
    P(2, 3) = -2.0f * zFar * zNear / (zFar - zNear);
    P(3, 2) = -1.0f;
    return P;
}

/// Sets a mat4 uniform of the current program (Eigen is column major, like OpenGL).
inline void setUniform(GLuint pid, const char* name, const Mat4x4& m) {
    glUniformMatrix4fv(glGetUniformLocation(pid, name), 1, GL_FALSE, m.data());
}
inline void setUniform(GLuint pid, const char* name, const Vec3& v) {
    glUniform3f(glGetUniformLocation(pid, name), v.x(), v.y(), v.z());
}
inline void setUniform(GLuint pid, const char* name, float f) {
    glUniform1f(glGetUniformLocation(pid, name), f);
}
inline void setUniform(GLuint pid, const char* name, int i) {
    glUniform1i(glGetUniformLocation(pid, name), i);
}
