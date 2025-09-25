#pragma once

struct Vec3 {
    float x, y, z;
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
    const Color Red{ 255, 0, 0 };
    const Color Green{ 0, 255, 0 };
    const Color Blue{ 0, 0, 255 };
    const Color Yellow{ 255, 255, 0 };
    const Color Cyan{ 0, 255, 255 };
    const Color Magenta{ 255, 0, 255 };
    const Color Orange{ 255, 165, 0 };
    const Color Purple{ 128, 0, 128 };
    const Color Pink{ 255, 192, 203 };
    const Color Brown{ 139, 69, 19 };
    const Color Gray{ 128, 128, 128 };
    const Color White{ 255, 255, 255 };
    const Color Black{ 0, 0, 0 };
}
