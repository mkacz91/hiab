#include "math.h"

namespace hiab {

mat4f::mat4f(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) :
    m00(m00), m01(m01), m02(m02), m03(m03),
    m10(m10), m11(m11), m12(m12), m13(m13),
    m20(m20), m21(m21), m22(m22), m23(m23),
    m30(m30), m31(m31), m32(m32), m33(m33)
{ }

mat4f& mat4f::load_identity()
{
    m00 = 1; m01 = 0; m02 = 0; m03 = 0;
    m10 = 0; m11 = 1; m12 = 0; m13 = 0;
    m20 = 0; m21 = 0; m22 = 1; m23 = 0;
    m30 = 0; m31 = 0; m32 = 0; m33 = 1;
    return *this;
}

mat4f& mat4f::apply(mat4f const& other)
{
#define mat4f_apply_compute_cell(i, j) \
    float m##i##j##_ = \
        other.m##i##0 * m0##j + \
        other.m##i##1 * m1##j + \
        other.m##i##2 * m2##j + \
        other.m##i##3 * m3##j

#define mat4f_apply_compute_row(i) \
    mat4f_apply_compute_cell(i, 0); \
    mat4f_apply_compute_cell(i, 1); \
    mat4f_apply_compute_cell(i, 2); \
    mat4f_apply_compute_cell(i, 3)

#define mat4f_apply_store_row(i) \
    m##i##0 = m##i##0_; \
    m##i##1 = m##i##1_; \
    m##i##2 = m##i##2_; \
    m##i##3 = m##i##3_

    mat4f_apply_compute_row(0);
    mat4f_apply_compute_row(1);
    mat4f_apply_compute_row(2);
    mat4f_apply_compute_row(3);

    mat4f_apply_store_row(0);
    mat4f_apply_store_row(1);
    mat4f_apply_store_row(2);
    mat4f_apply_store_row(3);

    return *this;

#undef mat4f_apply_store_row
#undef mat4f_apply_compute_row
#undef mat4f_apply_compute_cell
}

mat4f& mat4f::translate(float x, float y, float z)
{
    m03 += x; m13 += y; m23 += z;
    return *this;
}

#define mat4f_rotate_sub(ix, iy, j) \
    x = m##ix##j, y = m##iy##j; \
    m##ix##j = c * x - s * y; \
    m##iy##j = s * x + c * y

#define mat4f_rotate(ix, iy) \
    float s = sin(angle), c = cos(angle); \
    float x, y; \
    mat4f_rotate_sub(ix, iy, 0); \
    mat4f_rotate_sub(ix, iy, 1); \
    mat4f_rotate_sub(ix, iy, 2); \
    mat4f_rotate_sub(ix, iy, 3)

mat4f& mat4f::rotate_x(float angle)
{
    mat4f_rotate(1, 2);
    return *this;
}

mat4f& mat4f::rotate_y(float angle)
{
    mat4f_rotate(2, 0);
    return *this;
}

mat4f& mat4f::rotate_z(float angle)
{
    mat4f_rotate(0, 1);
    return *this;
}

#undef mat4f_rotate_sub
#undef mat4f_rotate

mat4f& mat4f::scale(float x, float y, float z)
{
    m00 *= x; m01 *= x; m02 *= x; m03 *= x;
    m10 *= y; m11 *= y; m12 *= y; m13 *= y;
    m20 *= z; m21 *= z; m22 *= z; m23 *= z;
    return *this;
}

mat4f mat4f::perspective_aov(
    int width, int height, float near, float far, float aov)
{
    float y_max = near * tan(0.5f * aov);
    float x_max = width > 0 && height > 0 ? width * y_max / height : y_max;
    return perspective_bounds(-x_max, x_max, -y_max, y_max, near, far);
}

mat4f mat4f::perspective_bounds(
    float left, float right, float bottom, float top, float near, float far)
{
    return mat4f(
        2 * near / (right - left), 0, (right + left) / (right - left), 0,
        0, 2 * near / (top - bottom), (top + bottom) / (top - bottom), 0,
        0, 0, -(far + near) / (far - near), -2 * far * near / (far - near),
        0, 0, -1, 0);
}

mat4f operator * (mat4f const& A, mat4f const& B)
{
#define mat4f_mult_cell(i, j) \
    A.m##i##0 * B.m0##j + \
    A.m##i##1 * B.m1##j + \
    A.m##i##2 * B.m2##j + \
    A.m##i##3 * B.m3##j

#define mat4f_mult_row(i) \
    mat4f_mult_cell(i, 0), \
    mat4f_mult_cell(i, 1), \
    mat4f_mult_cell(i, 2), \
    mat4f_mult_cell(i, 3)

    return mat4f(
        mat4f_mult_row(0),
        mat4f_mult_row(1),
        mat4f_mult_row(2),
        mat4f_mult_row(3));

#undef mat4_mult_row
#undef mat4_mult_cell
}

} // namespace hiab
