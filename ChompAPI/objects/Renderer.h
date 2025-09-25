#pragma once
#include "Types.h"
#include <vector>
#include <algorithm>
#include <cmath>



class Renderer {
public:
    int* framebuffer;
    float* zbuffer;
    int width, height;
    Vec3 lightDir = { 0.5f, 1.0f, -0.5f };
    float groundY = 0.0f;

    Renderer(int* fb, float* zb, int w, int h)
        : framebuffer(fb), zbuffer(zb), width(w), height(h)
    {
        float len = sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
        lightDir.x /= len; lightDir.y /= len; lightDir.z /= len;
    }

    // Draw any triangle
    void Draw(const RenderTriangle& tri) {
        // Shadow projection
        Vec3 s0 = ProjectShadow(tri.v0);
        Vec3 s1 = ProjectShadow(tri.v1);
        Vec3 s2 = ProjectShadow(tri.v2);

        DrawTriangle(s0, s1, s2, { 50,50,50 }); // shadow color

        // Lighting
        Vec3 normal = ComputeNormal(tri.v0, tri.v1, tri.v2);
        float brightness = std::max(0.0f, Dot(normal, lightDir));
        Color shadedColor = {
            (unsigned char)(tri.color.r * brightness),
            (unsigned char)(tri.color.g * brightness),
            (unsigned char)(tri.color.b * brightness)
        };

        DrawTriangle(tri.v0, tri.v1, tri.v2, shadedColor);
    }

private:
    Vec3 ProjectShadow(const Vec3& v) {
        float t = (v.y - groundY) / lightDir.y;
        return { v.x - lightDir.x * t, groundY, v.z - lightDir.z * t };
    }

    Vec3 ComputeNormal(const Vec3& a, const Vec3& b, const Vec3& c) {
        Vec3 U = { b.x - a.x, b.y - a.y, b.z - a.z };
        Vec3 V = { c.x - a.x, c.y - a.y, c.z - a.z };
        Vec3 N = { U.y * V.z - U.z * V.y, U.z * V.x - U.x * V.z, U.x * V.y - U.y * V.x };
        float len = sqrt(N.x * N.x + N.y * N.y + N.z * N.z);
        if (len > 0) { N.x /= len; N.y /= len; N.z /= len; }
        return N;
    }

    float Dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    void DrawTriangle(Vec3 v0, Vec3 v1, Vec3 v2, Color color) {
        auto Edge = [](const Vec3& a, const Vec3& b, const Vec3& c) {
            return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
            };

        int minX = std::max(0, (int)std::floor(std::min({ v0.x,v1.x,v2.x })));
        int maxX = std::min(width - 1, (int)std::ceil(std::max({ v0.x,v1.x,v2.x })));
        int minY = std::max(0, (int)std::floor(std::min({ v0.y,v1.y,v2.y })));
        int maxY = std::min(height - 1, (int)std::ceil(std::max({ v0.y,v1.y,v2.y })));

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                Vec3 p = { (float)x + 0.5f,(float)y + 0.5f,0 };
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
};
