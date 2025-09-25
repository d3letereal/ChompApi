#pragma once

struct Vec3 {
    float x, y, z;

    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
};

struct Color {
    unsigned char r, g, b;
};

struct Transform {
    Vec3 pos;
    Vec3 rotation;
    float scale;
};

struct Triangle {
    Vec3 v0, v1, v2;
};

struct RenderTriangle {
    Vec3 v0, v1, v2;
    Color color;
};

namespace Colors {
    const Color Red{ 255,0,0 };
    const Color Green{ 0,255,0 };
    const Color Blue{ 0,0,255 };
    const Color White{ 255,255,255 };
    const Color Black{ 0,0,0 };
}
