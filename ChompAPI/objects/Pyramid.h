#pragma once
#include <vector>
#include "Types.h"

class Pyramid {
public:
    Pyramid(float size = 1.0f, float height = 1.0f);

    void Draw(Color color, const Transform& t, int* framebuffer, float* zbuffer, int width, int height);

private:
    std::vector<Triangle> triangles;

    Vec3 RotateVertex(const Vec3& v, const Vec3& rotation);
    Vec3 ProjectVertex(const Vec3& v, int width, int height, float scale);
    void DrawTriangle(int* framebuffer, float* zbuffer, int width, int height,
        const Vec3& v0, const Vec3& v1, const Vec3& v2, Color color);
};
