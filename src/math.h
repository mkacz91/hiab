#pragma once

#include <cmath>

namespace hiab {

using std::sin;
using std::cos;
using std::tan;

constexpr float PI = 3.14159265359f;

struct mat4f
{
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;

    mat4f() = default;

    mat4f(
      float m00, float m01, float m02, float m03,
      float m10, float m11, float m12, float m13,
      float m20, float m21, float m22, float m23,
      float m30, float m31, float m32, float m33);

    mat4f& load_identity();

    mat4f& apply(mat4f const& other);

    mat4f& translate(float x, float y, float z);

    mat4f& rotate_x(float angle);

    mat4f& rotate_y(float angle);

    mat4f& rotate_z(float angle);

    mat4f& scale(float x, float y, float z);

    float const* p() const { return reinterpret_cast<float const*>(this); }

    static mat4f perspective_aov(
        int width, int height, float near, float far, float aov);

    static mat4f perspective_bounds(
        float left, float right, float bottom, float top, float near, float far);
};

} // namespace hiab
