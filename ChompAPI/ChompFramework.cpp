#include "window/Window.h"
#include "objects/Cube.h"
#include "objects/Skybox.h"
#include <algorithm>
#include <chrono>
#include <thread>

// Key codes (Windows VK codes)
#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_Q 0x51
#define KEY_E 0x45

int main() {
    // Create a window
    Window window(800, 600, "OASIS");
    window.verbose = true;

    // Create objects
    Cube cube(1.0f);
    Skybox sky(20.0f);

    Transform cubeT{ {0,0,5}, {0,0,0}, 2.5f };
    Transform skyT{ {0,0,0}, {0,0,0}, 1.0f };
    Color red{ 255,0,0 };

    window.StartRenderLoop([&]() {
        int w = window.GetWidth();
        int h = window.GetHeight();
        int* fb = window.GetFramebuffer();
        float* zb = window.GetZBuffer();

        if (fb && zb) {
            // Clear buffers
            std::fill(fb, fb + w * h, 0x000000);
            std::fill(zb, zb + w * h, 1e9f);

            // Draw skybox first (background)
            sky.Draw(skyT, fb, nullptr, w, h);

            // Draw cube in front
            cube.Draw(red, cubeT, fb, zb, w, h);
        }

        // -------- CUBE ROTATION (no console click needed) --------
        if (window.IsKeyPressed(KEY_W)) cubeT.rotation.x += 0.05f;
        if (window.IsKeyPressed(KEY_S)) cubeT.rotation.x -= 0.05f;
        if (window.IsKeyPressed(KEY_A)) cubeT.rotation.y += 0.05f;
        if (window.IsKeyPressed(KEY_D)) cubeT.rotation.y -= 0.05f;
        if (window.IsKeyPressed(KEY_Q)) cubeT.rotation.z += 0.05f;
        if (window.IsKeyPressed(KEY_E)) cubeT.rotation.z -= 0.05f;
        });

    // Main event loop
    while (true) {
        window.ProcessEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
