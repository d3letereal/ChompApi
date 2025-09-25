#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "Types.h"

class OBJLoader {
public:
    Transform t; // Position, rotation, scale

    OBJLoader(const std::string& filepath,
        const Vec3& rotation = { 0,0,0 },
        float scale = 1.0f,
        const Vec3& pos = { 0,0,0 }) {
        t.rotation = rotation;
        t.scale = scale;
        t.pos = pos;
        LoadOBJ(filepath);
    }

    // Reset framebuffer + zbuffer
    void Clear(int* framebuffer, float* zbuffer, int width, int height) {
        for (int i = 0; i < width * height; i++) {
            framebuffer[i] = 0x000000; // black
            zbuffer[i] = 1e9f;         // far depth
        }
    }

    // Default draw uses internal transform (t)
    void Draw(int* framebuffer, float* zbuffer, int width, int height, Color baseColor) {
        Draw(t, framebuffer, zbuffer, width, height, baseColor);
    }

    // Overloaded draw with external transform
    void Draw(const Transform& trans, int* framebuffer, float* zbuffer, int width, int height, Color baseColor) {
        for (auto& tri : triangles) {
            // Rotate and scale
            Vec3 v0 = RotateVertex(tri.v0, trans.rotation);
            Vec3 v1 = RotateVertex(tri.v1, trans.rotation);
            Vec3 v2 = RotateVertex(tri.v2, trans.rotation);

            v0 = { v0.x * trans.scale + trans.pos.x, v0.y * trans.scale + trans.pos.y, v0.z * trans.scale + trans.pos.z };
            v1 = { v1.x * trans.scale + trans.pos.x, v1.y * trans.scale + trans.pos.y, v1.z * trans.scale + trans.pos.z };
            v2 = { v2.x * trans.scale + trans.pos.x, v2.y * trans.scale + trans.pos.y, v2.z * trans.scale + trans.pos.z };

            // Project
            Vec3 p0 = ProjectVertex(v0, width, height, 100.0f);
            Vec3 p1 = ProjectVertex(v1, width, height, 100.0f);
            Vec3 p2 = ProjectVertex(v2, width, height, 100.0f);

            // Compute normal
            Vec3 U = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
            Vec3 V = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
            Vec3 normal = { U.y * V.z - U.z * V.y, U.z * V.x - U.x * V.z, U.x * V.y - U.y * V.x };

            if (normal.z >= 0) continue; // backface culling

            // Lighting (Lambert)
            float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            float intensity = std::max(0.1f, -normal.z / len);

            Color shadedColor = {
                (unsigned char)(baseColor.r * intensity),
                (unsigned char)(baseColor.g * intensity),
                (unsigned char)(baseColor.b * intensity)
            };

            DrawTriangle(framebuffer, zbuffer, width, height, p0, p1, p2, shadedColor);

            // Optional outline
            DrawLine(p0, p1, framebuffer, width, height, { 0,0,0 });
            DrawLine(p1, p2, framebuffer, width, height, { 0,0,0 });
            DrawLine(p2, p0, framebuffer, width, height, { 0,0,0 });
        }
    }

private:
    std::vector<Triangle> triangles;

    void LoadOBJ(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::vector<Vec3> vertices;
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            if (prefix == "v") {
                Vec3 v;
                iss >> v.x >> v.y >> v.z;
                vertices.push_back(v);
            }
            else if (prefix == "f") {
                std::vector<int> idx;
                std::string vert;
                while (iss >> vert) {
                    int vi = std::stoi(vert.substr(0, vert.find('/'))) - 1;
                    idx.push_back(vi);
                }
                for (size_t i = 1; i + 1 < idx.size(); ++i) {
                    triangles.push_back({ vertices[idx[0]], vertices[idx[i]], vertices[idx[i + 1]] });
                }
            }
        }
    }

    Vec3 RotateVertex(const Vec3& v, const Vec3& rotation) {
        Vec3 r = v;
        float cx = cos(rotation.x), sx = sin(rotation.x);
        float cy = cos(rotation.y), sy = sin(rotation.y);
        float cz = cos(rotation.z), sz = sin(rotation.z);

        // X
        float y = r.y * cx - r.z * sx;
        float z = r.y * sx + r.z * cx;
        r.y = y; r.z = z;

        // Y
        float x = r.x * cy + r.z * sy;
        z = -r.x * sy + r.z * cy;
        r.x = x; r.z = z;

        // Z
        x = r.x * cz - r.y * sz;
        y = r.x * sz + r.y * cz;
        r.x = x; r.y = y;

        return r;
    }

    Vec3 ProjectVertex(const Vec3& v, int width, int height, float scale) {
        return { v.x * scale + width / 2.0f, v.y * scale + height / 2.0f, v.z };
    }

    void DrawTriangle(int* framebuffer, float* zbuffer, int width, int height,
        const Vec3& v0, const Vec3& v1, const Vec3& v2, Color color) {
        auto Edge = [](const Vec3& a, const Vec3& b, const Vec3& c) {
            return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
            };

        int minX = std::max(0, (int)std::floor(std::min({ v0.x,v1.x,v2.x })));
        int maxX = std::min(width - 1, (int)std::ceil(std::max({ v0.x,v1.x,v2.x })));
        int minY = std::max(0, (int)std::floor(std::min({ v0.y,v1.y,v2.y })));
        int maxY = std::min(height - 1, (int)std::ceil(std::max({ v0.y,v1.y,v2.y })));

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                Vec3 p = { (float)x + 0.5f, (float)y + 0.5f, 0 };
                float w0 = Edge(v1, v2, p);
                float w1 = Edge(v2, v0, p);
                float w2 = Edge(v0, v1, p);
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float area = Edge(v0, v1, v2);
                    if (area == 0) continue;
                    float alpha = w0 / area, beta = w1 / area, gamma = w2 / area;
                    float z = alpha * v0.z + beta * v1.z + gamma * v2.z;
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        int idx = y * width + x;
                        if (idx >= 0 && idx < width * height) {
                            if (z < zbuffer[idx]) {
                                framebuffer[idx] = (color.r << 16) | (color.g << 8) | color.b;
                                zbuffer[idx] = z;
                            }
                        }
                    }
                }
            }
        }
    }

    void DrawLine(const Vec3& a, const Vec3& b, int* framebuffer, int width, int height, Color color) {
        int x0 = (int)a.x, y0 = (int)a.y;
        int x1 = (int)b.x, y1 = (int)b.y;
        int dx = abs(x1 - x0), dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;

        while (true) {
            if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
                int idx = y0 * width + x0;
                if (idx >= 0 && idx < width * height)
                    framebuffer[idx] = (color.r << 16) | (color.g << 8) | color.b;
            }
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
};
