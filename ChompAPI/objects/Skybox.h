#pragma once
#include "Types.h"
#include <vector>

class Skybox {
public:
    Skybox(float size = 10.0f);
    void Draw(const Transform& t, int* framebuffer, float* zbuffer, int width, int height);

private:
    std::vector<Triangle> triangles;
    Vec3 RotateVertex(const Vec3& v, const Vec3& rot);
    Vec3 ProjectVertex(const Vec3& v, int w, int h, float scale);
    void DrawTriangleIgnoreZ(int* fb, int w, int h, const Vec3& v0, const Vec3& v1, const Vec3& v2, Color c);
};
