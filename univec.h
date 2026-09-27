#pragma once
#include <string>

struct Vec2 {
	int x, y;

	Vec2 operator+(const Vec2& other) {
		return Vec2{ x + other.x, y + other.y };
	}

	std::string toString() const {
		return std::to_string(x) + "/" + std::to_string(y);
	}
};
struct fVec2 {
	float x, y;
	Vec2 toVec2() {
		return Vec2{ (int)x, (int)y };
	}
};
struct dVec2 {
	double x, y;
	Vec2 toVec2() {
		return Vec2{ (int)x, (int)y };
	}
};


struct Vec3 {
	int x, y, z;
};
struct fVec3 {
	float x, y, z;
	Vec3 toVec3() {
		return Vec3{ (int)x, (int)y, (int)z };
	}
};
struct dVec3 {
	double x, y, z;
	Vec3 toVec3() {
		return Vec3{ (int)x, (int)y, (int)z };
	}
};

struct Vec4 {
	int x, y, z, w;
};
struct fVec4 {
	float x, y, z, w;
	Vec4 toVec4() {
		return Vec4{ (int)x, (int)y, (int)z, (int)w };
	}
};
struct dVec4 {
	double x, y, z, w;
	Vec4 toVec4() {
		return Vec4{ (int)x, (int)y, (int)z, (int)w };
	}
};

struct uint8Vec2 {
	uint8_t x, y;
	Vec2 toVec2() {
		return Vec2{ static_cast<int>(x), static_cast<int>(y) };
	}
};
struct uint8Vec3 {
	uint8_t x, y, z;
	Vec3 toVec3() {
		return Vec3{ static_cast<int>(x), static_cast<int>(y), static_cast<int>(z) };
	}
};
struct uint8Vec4 {
	uint8_t x, y, z, w;
	Vec4 toVec4() {
		return Vec4{ static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), static_cast<int>(w) };
	}
};