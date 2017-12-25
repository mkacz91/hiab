#pragma once

#include <cmath>

namespace hiab {

using std::sqrt;
using std::sin;
using std::cos;
using std::tan;

constexpr float PI = 3.14159265359f;

struct vec2f
{
    float x, y;
};

struct vec3f
{
    float x, y, z;

    vec3f& operator -= (vec3f const& u)
    {
        x -= u.x; y -= u.y; z -= u.z;
        return *this;
    }

    vec3f& operator *= (float s)
    {
        x *= s; y *= s; z *= s;
        return *this;
    }

    vec3f& operator /= (float s)
    {
        return operator *= (1.0f / s);
    }
};

inline vec3f operator - (vec3f u, vec3f const& v)
{
    return u -= v;
}

inline vec3f operator / (vec3f u, float s)
{
    return u /= s;
}

inline float length_sq(vec3f const& u)
{
    return u.x * u.x + u.y * u.y + u.z * u.z;
}

inline float length(vec3f const& u)
{
    return sqrt(length_sq(u));
}

inline vec3f normalize(vec3f const& u)
{
    return u / length(u);
}

inline vec3f cross(vec3f const& u, vec3f const& v)
{
    return
    {
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x
    };
}

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

    mat4f& scale(float s) { return scale(s, s, s); }

    float const* p() const { return reinterpret_cast<float const*>(this); }

    static mat4f perspective_aov(
        int width, int height, float near, float far, float aov);

    static mat4f perspective_bounds(
        float left, float right, float bottom, float top, float near, float far);
};

inline mat4f eye4f() { return mat4f().load_identity(); }

mat4f operator * (mat4f const& A, mat4f const& B);

vec3f face_normal(vec3f const& a, vec3f const& b, vec3f const& c);

} // namespace hiab
