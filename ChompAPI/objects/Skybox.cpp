#include "Skybox.h"
#include <algorithm>
#include <cmath>

Skybox::Skybox(float s) {
    float hs = s / 2.0f;
    triangles = {
        {{-hs,-hs, hs},{hs,-hs, hs},{hs, hs, hs}}, {{-hs,-hs, hs},{hs, hs, hs},{-hs, hs, hs}}, // front
        {{-hs,-hs,-hs},{hs, hs,-hs},{hs,-hs,-hs}}, {{-hs,-hs,-hs},{-hs, hs,-hs},{hs, hs,-hs}}, // back
        {{-hs,-hs,-hs},{-hs,-hs, hs},{-hs, hs, hs}}, {{-hs,-hs,-hs},{-hs, hs, hs},{-hs, hs,-hs}}, // left
        {{hs,-hs,-hs},{hs, hs, hs},{hs,-hs, hs}}, {{hs,-hs,-hs},{hs, hs,-hs},{hs, hs, hs}}, // right
        {{-hs, hs,-hs},{-hs, hs, hs},{hs, hs, hs}}, {{-hs, hs,-hs},{hs, hs, hs},{hs, hs,-hs}}, // top
        {{-hs,-hs,-hs},{hs,-hs, hs},{-hs,-hs, hs}}, {{-hs,-hs,-hs},{hs,-hs,-hs},{hs,-hs, hs}} // bottom
    };
}

Vec3 Skybox::RotateVertex(const Vec3& v, const Vec3& r) {
    Vec3 res = v;
    float cx = cos(r.x), sx = sin(r.x), cy = cos(r.y), sy = sin(r.y), cz = cos(r.z), sz = sin(r.z);
    float y = res.y * cx - res.z * sx; float z = res.y * sx + res.z * cx; res.y = y; res.z = z;
    float x = res.x * cy + res.z * sy; z = -res.x * sy + res.z * cy; res.x = x; res.z = z;
    x = res.x * cz - res.y * sz; y = res.x * sz + res.y * cz; res.x = x; res.y = y;
    return res;
}

Vec3 Skybox::ProjectVertex(const Vec3& v, int w, int h, float scale) { return { v.x * scale + w / 2.0f,v.y * scale + h / 2.0f,v.z }; }

void Skybox::DrawTriangleIgnoreZ(int* fb, int w, int h, const Vec3& v0, const Vec3& v1, const Vec3& v2, Color c) {
    auto Edge = [](const Vec3& a, const Vec3& b, const Vec3& p) {return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x); };
    int minX = std::max(0, (int)std::floor(std::min({ v0.x,v1.x,v2.x })));
    int maxX = std::min(w - 1, (int)std::ceil(std::max({ v0.x,v1.x,v2.x })));
    int minY = std::max(0, (int)std::floor(std::min({ v0.y,v1.y,v2.y })));
    int maxY = std::min(h - 1, (int)std::ceil(std::max({ v0.y,v1.y,v2.y })));
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Vec3 p = { (float)x + 0.5f,(float)y + 0.5f,0 };
            float w0 = Edge(v1, v2, p), w1 = Edge(v2, v0, p), w2 = Edge(v0, v1, p);
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) fb[y * w + x] = (c.r << 16) | (c.g << 8) | c.b;
        }
    }
}

void Skybox::Draw(const Transform& t, int* fb, float* zb, int w, int h) {
    Color colors[6] = { {135,206,235},{70,130,180},{255,140,0},{128,0,128},{255,255,255},{30,30,30} };
    for (size_t i = 0; i < triangles.size(); i++) {
        Triangle& tri = triangles[i];
        Vec3 v0 = RotateVertex(tri.v0, t.rotation) * t.scale + t.pos;
        Vec3 v1 = RotateVertex(tri.v1, t.rotation) * t.scale + t.pos;
        Vec3 v2 = RotateVertex(tri.v2, t.rotation) * t.scale + t.pos;
        Vec3 p0 = ProjectVertex(v0, w, h, 100.0f);
        Vec3 p1 = ProjectVertex(v1, w, h, 100.0f);
        Vec3 p2 = ProjectVertex(v2, w, h, 100.0f);
        DrawTriangleIgnoreZ(fb, w, h, p0, p1, p2, colors[i / 2]);
    }
}
