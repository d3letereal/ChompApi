#include "Cube.h"
#include <cmath>
#include <algorithm>

Cube::Cube(float s) {
    float hs = s / 2.0f; // half size for centering at origin

    // Front face
    triangles.push_back({{-hs,-hs, hs}, {hs,-hs, hs}, {hs, hs, hs}});
    triangles.push_back({{-hs,-hs, hs}, {hs, hs, hs}, {-hs, hs, hs}});

    // Back face
    triangles.push_back({{-hs,-hs,-hs}, {hs, hs,-hs}, {hs,-hs,-hs}});
    triangles.push_back({{-hs,-hs,-hs}, {-hs, hs,-hs}, {hs, hs,-hs}});

    // Left face
    triangles.push_back({{-hs,-hs,-hs}, {-hs,-hs, hs}, {-hs, hs, hs}});
    triangles.push_back({{-hs,-hs,-hs}, {-hs, hs, hs}, {-hs, hs,-hs}});

    // Right face
    triangles.push_back({{hs,-hs,-hs}, {hs, hs, hs}, {hs,-hs, hs}});
    triangles.push_back({{hs,-hs,-hs}, {hs, hs,-hs}, {hs, hs, hs}});

    // Top face
    triangles.push_back({{-hs, hs,-hs}, {-hs, hs, hs}, {hs, hs, hs}});
    triangles.push_back({{-hs, hs,-hs}, {hs, hs, hs}, {hs, hs,-hs}});

    // Bottom face
    triangles.push_back({{-hs,-hs,-hs}, {hs,-hs, hs}, {-hs,-hs, hs}});
    triangles.push_back({{-hs,-hs,-hs}, {hs,-hs,-hs}, {hs,-hs, hs}});
}

// Rotate vertex around X,Y,Z axes
Vec3 Cube::RotateVertex(const Vec3& v, const Vec3& rotation) {
    Vec3 r = v;

    float cx = cos(rotation.x), sx = sin(rotation.x);
    float cy = cos(rotation.y), sy = sin(rotation.y);
    float cz = cos(rotation.z), sz = sin(rotation.z);

    // Rotate X
    float y = r.y * cx - r.z * sx;
    float z = r.y * sx + r.z * cx;
    r.y = y; r.z = z;

    // Rotate Y
    float x = r.x * cy + r.z * sy;
    z = -r.x * sy + r.z * cy;
    r.x = x; r.z = z;

    // Rotate Z
    x = r.x * cz - r.y * sz;
    y = r.x * sz + r.y * cz;
    r.x = x; r.y = y;

    return r;
}

// Simple orthographic projection
Vec3 Cube::ProjectVertex(const Vec3& v, int width, int height, float scale) {
    return { v.x * scale + width / 2.0f, v.y * scale + height / 2.0f, v.z };
}

// Draw triangle using simple Z-buffer
void Cube::DrawTriangle(int* framebuffer, float* zbuffer, int width, int height,
    const Vec3& v0, const Vec3& v1, const Vec3& v2, Color color)
{
    auto Edge = [](const Vec3& a, const Vec3& b, const Vec3& c) {
        return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
        };

    Vec3 U = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
    Vec3 V = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
    Vec3 normal = { U.y * V.z - U.z * V.y, U.z * V.x - U.x * V.z, U.x * V.y - U.y * V.x };
    if (normal.z >= 0) return; // back-face culling

    int minX = std::max(0, (int)std::floor(std::min({ v0.x,v1.x,v2.x })));
    int maxX = std::min(width - 1, (int)std::ceil(std::max({ v0.x,v1.x,v2.x })));
    int minY = std::max(0, (int)std::floor(std::min({ v0.y,v1.y,v2.y })));
    int maxY = std::min(height - 1, (int)std::ceil(std::max({ v0.y,v1.y,v2.y })));

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Vec3 p = { (float)x + 0.5f, (float)y + 0.5f, 0 };
            float w0 = Edge(v1, v2, p);
            float w1 = Edge(v2, v0, p);
            float w2 = Edge(v0, v1, p);
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float area = Edge(v0, v1, v2);
                float alpha = w0 / area, beta = w1 / area, gamma = w2 / area;
                float z = alpha * v0.z + beta * v1.z + gamma * v2.z;
                int idx = y * width + x;
                if (z < zbuffer[idx]) {
                    framebuffer[idx] = (color.r << 16) | (color.g << 8) | color.b;
                    zbuffer[idx] = z;
                }
            }
        }
    }
}

void Cube::Draw(Color color, const Transform& t, int* framebuffer, float* zbuffer, int width, int height) {
    for (auto& tri : triangles) {
        Vec3 v0 = RotateVertex(tri.v0, t.rotation);
        Vec3 v1 = RotateVertex(tri.v1, t.rotation);
        Vec3 v2 = RotateVertex(tri.v2, t.rotation);

        v0 = { v0.x * t.scale + t.pos.x, v0.y * t.scale + t.pos.y, v0.z * t.scale + t.pos.z };
        v1 = { v1.x * t.scale + t.pos.x, v1.y * t.scale + t.pos.y, v1.z * t.scale + t.pos.z };
        v2 = { v2.x * t.scale + t.pos.x, v2.y * t.scale + t.pos.y, v2.z * t.scale + t.pos.z };

        Vec3 p0 = ProjectVertex(v0, width, height, 100.0f);
        Vec3 p1 = ProjectVertex(v1, width, height, 100.0f);
        Vec3 p2 = ProjectVertex(v2, width, height, 100.0f);

        DrawTriangle(framebuffer, zbuffer, width, height, p0, p1, p2, color);
    }
}
