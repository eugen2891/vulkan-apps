#pragma once

#include <math.h>

#define M_PI    3.14159265358979323846f
#define RCP_180 0.00555555555555555556f

#define DEG2RAD(d) (M_PI * (d) * RCP_180)

struct Vec3
{

    float x, y, z;

    float dot(const Vec3 &v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

};

struct Vec4
{

    float x, y, z, w;

    Vec4 operator=(const Vec3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    float dot(const Vec4 &v) const
    {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }

};

struct Quat : Vec4
{

    Quat(const Vec3& axis, float angle)
    {
        float r = DEG2RAD(angle * 0.5f);
        float sr = sinf(r);
        x = axis.x * sr;
        y = axis.y * sr;
        z = axis.z * sr;
        w = cosf(r);
    }

};

union Mat3
{

    Vec3 row[3];

    Mat3()
    {
        row[0] = { 1.f, 0.f, 0.f };
        row[1] = { 0.f, 1.f, 0.f };
        row[2] = { 0.f, 0.f, 1.f };
    }    

    Mat3(const Quat& q)
    {
        float qx2 = q.x * q.x;
        float qy2 = q.y * q.y;
        float qz2 = q.z * q.z;
        float qxy = q.x * q.y;
        float qxz = q.x * q.z;
        float qxw = q.x * q.w;
        float qyz = q.y * q.z;
        float qyw = q.y * q.w;
        float qzw = q.z * q.w;
        row[0].x = 1.f - 2.f * (qy2 + qz2);
        row[0].y = 2.f * (qxy - qzw);
        row[0].z = 2.f * (qxz + qyw);
        row[1].x = 2.f * (qxy + qzw);
        row[1].y = 1.f - 2.f * (qx2 + qz2);
        row[1].z = 2.f * (qyz - qxw);
        row[2].x = 2.f * (qxz - qyw);
        row[2].y = 2.f * (qyz + qxw);
        row[2].z = 1.f - 2.f * (qx2 + qy2);
    }

    Vec3 operator*(const Vec3& v)
    {   
        return { row[0].dot(v), row[1].dot(v), row[2].dot(v) };
    }

};

union Mat4
{

    Vec4 row[4];

    static const int NO_INIT = -1;

    Mat4()
    {
        row[0] = { 1.f, 0.f, 0.f, 0.f };
        row[1] = { 0.f, 1.f, 0.f, 0.f };
        row[2] = { 0.f, 0.f, 1.f, 0.f };
        row[3] = { 0.f, 0.f, 0.f, 1.f };
    }

    Mat4(int)
    {
    }

    Mat4(const Mat3& m)
    {
        row[0] = m.row[0]; row[0].w = 0.f;
        row[1] = m.row[1]; row[1].w = 0.f;
        row[2] = m.row[2]; row[2].w = 0.f;
        row[3] = { 0.f, 0.f, 0.f, 1.f };
    }

    Mat4(const Quat& q)
    {
        float qx2 = q.x * q.x;
        float qy2 = q.y * q.y;
        float qz2 = q.z * q.z;
        float qxy = q.x * q.y;
        float qxz = q.x * q.z;
        float qxw = q.x * q.w;
        float qyz = q.y * q.z;
        float qyw = q.y * q.w;
        float qzw = q.z * q.w;
        row[0].x = 1.f - 2.f * (qy2 + qz2);
        row[0].y = 2.f * (qxy - qzw);
        row[0].z = 2.f * (qxz + qyw);
        row[0].w = 0.f;
        row[1].x = 2.f * (qxy + qzw);
        row[1].y = 1.f - 2.f * (qx2 + qz2);
        row[1].z = 2.f * (qyz - qxw);
        row[1].w = 0.f;
        row[2].x = 2.f * (qxz - qyw);
        row[2].y = 2.f * (qyz + qxw);
        row[2].z = 1.f - 2.f * (qx2 + qy2);
        row[2].w = 0.f;
        row[3] = { 0.f, 0.f, 0.f, 1.f };
    }

};

inline Vec3 rotate(const Vec3& p, const Vec3& axis, float deg)
{
    Mat3 matrix(Quat(axis, deg));
    return matrix * p;
}

inline Mat4 operator*(const Mat4& a, const Mat4&b)
{
    Mat4 ret(Mat4::NO_INIT);
    Vec4 b0 = { b.row[0].x, b.row[1].x, b.row[2].x, b.row[3].x };
    Vec4 b1 = { b.row[0].y, b.row[1].y, b.row[2].y, b.row[3].y };
    Vec4 b2 = { b.row[0].z, b.row[1].z, b.row[2].z, b.row[3].z };
    Vec4 b3 = { b.row[0].w, b.row[1].w, b.row[2].w, b.row[3].w };
    ret.row[0] = { a.row[0].dot(b0), a.row[0].dot(b1), a.row[0].dot(b2), a.row[0].dot(b3) };
    ret.row[1] = { a.row[1].dot(b0), a.row[1].dot(b1), a.row[1].dot(b2), a.row[1].dot(b3) };
    ret.row[2] = { a.row[2].dot(b0), a.row[2].dot(b1), a.row[2].dot(b2), a.row[2].dot(b3) };
    ret.row[3] = { a.row[3].dot(b0), a.row[3].dot(b1), a.row[3].dot(b2), a.row[3].dot(b3) };
    return ret;
}
