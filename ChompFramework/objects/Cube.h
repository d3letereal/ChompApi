#pragma once
#include <vector>

struct Vec3 { float x, y, z; };
struct Color { unsigned char r, g, b; };
struct Transform { Vec3 pos; Vec3 rotation; float scale; };
struct Triangle { Vec3 v0, v1, v2; };

class Cube {
public:
    Cube(float size = 1.0f);
    void Draw(Color color, const Transform& t, int* framebuffer, float* zbuffer, int width, int height);

private:
    std::vector<Triangle> triangles;

    Vec3 RotateVertex(const Vec3& v, const Vec3& rotation);
    Vec3 ProjectVertex(const Vec3& v, int width, int height, float scale);
    void DrawTriangle(int* framebuffer, float* zbuffer, int width, int height, const Vec3& v0, const Vec3& v1, const Vec3& v2, Color color);
};
