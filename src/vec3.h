/*
alphaclock - transparent desktop clock
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef VEC3_H_
#define VEC3_H_

#include <math.h>

class Vec3 {
public:
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

inline Vec3 operator +(const Vec3 &a, const Vec3 &b)
{
	return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec3 operator -(const Vec3 &a, const Vec3 &b)
{
	return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vec3 operator *(const Vec3 &a, const Vec3 &b)
{
	return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline Vec3 operator /(const Vec3 &a, const Vec3 &b)
{
	return Vec3(a.x / b.x, a.y / b.y, a.z / b.z);
}

inline Vec3 &operator +=(Vec3 &a, const Vec3 &b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

inline Vec3 &operator -=(Vec3 &a, const Vec3 &b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

inline Vec3 operator *(const Vec3 &v, float s)
{
	return Vec3(v.x * s, v.y * s, v.z * s);
}

inline Vec3 operator *(float s, const Vec3 &v)
{
	return Vec3(v.x * s, v.y * s, v.z * s);
}

inline float dot(const Vec3 &a, const Vec3 &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float length(const Vec3 &v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float length_sq(const Vec3 &v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline Vec3 normalize(const Vec3 &v)
{
	float len = length(v);
	if(len == 0.0f) {
		return v;
	}
	return Vec3(v.x / len, v.y / len, v.z / len);
}

inline float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

inline Vec3 lerp(const Vec3 &a, const Vec3 &b, float t)
{
	return Vec3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}

#endif	// VEC3_H_
