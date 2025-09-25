#pragma once
#include "Types.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

class OBJLoader {
public:
    Transform t;
    std::vector<Triangle> triangles;

    OBJLoader(const std::string& path, const Vec3& rotation = { 0,0,0 }, float scale = 1.0f, const Vec3& pos = { 0,0,0 }) {
        t.rotation = rotation;
        t.scale = scale;
        t.pos = pos;
        LoadOBJ(path);
    }

    void Draw(int* framebuffer, float* zbuffer, int width, int height, Color baseColor) {
        Draw(t, framebuffer, zbuffer, width, height, baseColor);
    }

    void Draw(const Transform& trans, int* framebuffer, float* zbuffer, int width, int height, Color baseColor) {
        for (auto& tri : triangles) {
            Vec3 v0 = RotateVertex(tri.v0, trans.rotation) * trans.scale + trans.pos;
            Vec3 v1 = RotateVertex(tri.v1, trans.rotation) * trans.scale + trans.pos;
            Vec3 v2 = RotateVertex(tri.v2, trans.rotation) * trans.scale + trans.pos;

            Vec3 p0 = ProjectVertex(v0, width, height);
            Vec3 p1 = ProjectVertex(v1, width, height);
            Vec3 p2 = ProjectVertex(v2, width, height);

            Vec3 normal = Cross(v1 - v0, v2 - v0);
            float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            if (len == 0) continue;

            float intensity = std::max(0.1f, -normal.z / len); // simple Lambert
            Color shaded = { (unsigned char)(baseColor.r * intensity),
                            (unsigned char)(baseColor.g * intensity),
                            (unsigned char)(baseColor.b * intensity) };
            DrawTriangle(framebuffer, zbuffer, width, height, p0, p1, p2, shaded);
        }
    }

private:
    void LoadOBJ(const std::string& file) {
        std::ifstream f(file);
        if (!f.is_open()) return;

        std::vector<Vec3> verts;
        std::string line;
        while (std::getline(f, line)) {
            std::istringstream iss(line);
            std::string prefix; iss >> prefix;
            if (prefix == "v") {
                Vec3 v; iss >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }
            else if (prefix == "f") {
                std::vector<int> idx;
                std::string s; while (iss >> s) {
                    int i = std::stoi(s.substr(0, s.find('/'))) - 1;
                    idx.push_back(i);
                }
                for (size_t i = 1; i + 1 < idx.size(); i++)
                    triangles.push_back({ verts[idx[0]], verts[idx[i]], verts[idx[i + 1]] });
            }
        }
    }

    Vec3 RotateVertex(const Vec3& v, const Vec3& r) {
        Vec3 res = v;
        float cx = cos(r.x), sx = sin(r.x);
        float cy = cos(r.y), sy = sin(r.y);
        float cz = cos(r.z), sz = sin(r.z);

        float y = res.y * cx - res.z * sx; float z = res.y * sx + res.z * cx; res.y = y; res.z = z;
        float x = res.x * cy + res.z * sy; z = -res.x * sy + res.z * cy; res.x = x; res.z = z;
        x = res.x * cz - res.y * sz; y = res.x * sz + res.y * cz; res.x = x; res.y = y;
        return res;
    }

    Vec3 ProjectVertex(const Vec3& v, int w, int h, float scale = 100.0f) {
        return { v.x * scale + w / 2.0f, v.y * scale + h / 2.0f, v.z };
    }

    Vec3 Cross(const Vec3& a, const Vec3& b) {
        return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
    }

    void DrawTriangle(int* fb, float* zb, int w, int h, const Vec3& v0, const Vec3& v1, const Vec3& v2, Color c) {
        auto Edge = [](const Vec3& a, const Vec3& b, const Vec3& p) {return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x); };

        int minX = std::max(0, (int)std::floor(std::min({ v0.x,v1.x,v2.x })));
        int maxX = std::min(w - 1, (int)std::ceil(std::max({ v0.x,v1.x,v2.x })));
        int minY = std::max(0, (int)std::floor(std::min({ v0.y,v1.y,v2.y })));
        int maxY = std::min(h - 1, (int)std::ceil(std::max({ v0.y,v1.y,v2.y })));

        float area = Edge(v0, v1, v2);
        if (area == 0) return;

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                Vec3 p = { (float)x + 0.5f,(float)y + 0.5f,0 };
                float w0 = Edge(v1, v2, p);
                float w1 = Edge(v2, v0, p);
                float w2 = Edge(v0, v1, p);
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float alpha = w0 / area, beta = w1 / area, gamma = w2 / area;
                    float z = alpha * v0.z + beta * v1.z + gamma * v2.z;
                    int idx = y * w + x;
                    if (idx >= 0 && idx < w * h && z < zb[idx]) {
                        fb[idx] = (c.r << 16) | (c.g << 8) | c.b;
                        zb[idx] = z;
                    }
                }
            }
        }
    }
};
