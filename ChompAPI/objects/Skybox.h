#pragma once
#include <vector>
#include "Cube.h"

class Skybox {
public:
    Skybox(float size = 10.0f);
    void Draw(const Transform& t, int* framebuffer, int* zbuffer, int width, int height);

private:
    std::vector<Triangle> triangles;

    Vec3 RotateVertex(const Vec3& v, const Vec3& rotation);
    Vec3 ProjectVertex(const Vec3& v, int width, int height, float scale);
    void DrawTriangleIgnoreZ(int* framebuffer, int width, int height,
        const Vec3& v0, const Vec3& v1, const Vec3& v2, Color color);
};
