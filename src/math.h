#pragma once

#include "prefix.h"
#include <cmath>
#include <limits>

#undef INFINITY

namespace std {

class std::ostream;

}

namespace hiab {

using std::abs;
using std::sqrt;
using std::sin;
using std::cos;
using std::tan;
using std::isinf;

constexpr float QUARTER_PI = 0.78539816339f;
constexpr float HALF_PI = 1.57079632679f;
constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = 6.28318530718f;
constexpr float INFINITY = std::numeric_limits<float>::infinity();

template <typename T>
T const& min(T const& a, T const& b)
{
    return a < b ? a : b;
}

template <typename T>
T const& max(T const& a, T const& b)
{
    return a > b ? a : b;
}

template <typename T>
T const& max(T const& a, T const& b, T const& c)
{
    return a > b
        ? (a > c ? a : c)
        : (b > c ? b : c);
}

template <typename T>
T const& clamp(T const& value, T const& min_value, T const& max_value)
{
    if (value <= min_value)
        return min_value;
    if (value >= max_value)
        return max_value;
    return value;
}

struct vec2f
{
    float x, y;

    vec2f& operator += (vec2f const& u)
    {
        x += u.x; y += u.y;
        return *this;
    }
};

struct vec3f
{
    float x, y, z;

    vec3f& operator += (vec3f const& u)
    {
        x += u.x; y += u.y; z += u.z;
        return *this;
    }

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

inline bool operator == (vec3f const& u, vec3f const& v)
{
    return u.x == v.x && u.y == v.y && u.z == v.z;
}

inline vec3f operator - (vec3f const& u)
{
    return { -u.x, -u.y, -u.z };
}

inline vec3f operator - (vec3f u, vec3f const& v)
{
    return u -= v;
}

inline vec3f operator + (vec3f u, vec3f const& v)
{
    return u += v;
}

inline vec3f operator * (float s, vec3f u)
{
    return u *= s;
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

inline vec3f min(vec3f const& u, vec3f const& v)
{
    return { min(u.x, v.x), min(u.y, v.y), min(u.z, v.z) };
}

inline vec3f max(vec3f const& u, vec3f const& v)
{
    return { max(u.x, v.x), max(u.y, v.y), max(u.z, v.z) };
}

inline float max(vec3f const& u)
{
    return max(u.x, u.y, u.z);
}

inline vec3f abs(vec3f const& u)
{
    return { abs(u.x), abs(u.y), abs(u.z) };
}

struct mat4f
{
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;

    mat4f& load_identity();

    mat4f& apply(mat4f const& other);

    mat4f& translate(vec3f const& translation);

    mat4f& rotate_x(float angle);

    mat4f& rotate_y(float angle);

    mat4f& rotate_z(float angle);

    mat4f& scale(float x, float y, float z);

    mat4f& scale(float s) { return scale(s, s, s); }

    float const* p() const { return reinterpret_cast<float const*>(this); }

    static mat4f perspective_aov(
        float aspect, float near, float far, float aov);

    static void get_perspective_aov_bounds(
        float aspect, float near, float aov, float* right, float* top);

    static mat4f perspective_bounds(
        float left, float right, float bottom, float top, float near, float far);

    vec3f transform_v(vec3f const& u) const;
};

inline mat4f eye4f() { return mat4f().load_identity(); }

mat4f operator * (mat4f const& A, mat4f const& B);

struct box3f
{
    vec3f p0, p1;

    box3f& clear()
    {
        p0 = { INFINITY, INFINITY, INFINITY };
        p1 = { -INFINITY, -INFINITY, -INFINITY };
        return *this;
    }

    box3f& expand(vec3f const& p)
    {
        p0 = min(p0, p);
        p1 = max(p1, p);
        return *this;
    }

    box3f& expand(box3f const& box)
    {
        p0 = min(p0, box.p0);
        p1 = max(p1, box.p1);
        return *this;
    }

    template <typename R>
    box3f& expand(R const& items)
    {
        for (auto const& item : items)
            expand(item);
        return *this;
    }

    static box3f empty() { return box3f().clear(); }
};

inline vec3f box_diameter(box3f const& box)
{
    return abs(box.p1 - box.p0);
}

inline vec3f box_center(box3f const& box)
{
    return 0.5f * (box.p0 + box.p1);
}

template <typename R>
box3f get_bounds(R const& points)
{
    return box3f().clear().expand(points);
}

mat4f get_box_mapping_to_symunit(box3f const& box);

vec3f face_normal(vec3f const& a, vec3f const& b, vec3f const& c);

std::ostream& operator << (std::ostream& ostr, vec2f const& u);

} // namespace hiab
