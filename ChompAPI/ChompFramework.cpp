#include "window/Window.h"
#include "objects/Cube.h"
#include "objects/Skybox.h"
#include "objects/OBJLoader.h"
#include "objects/Pyramid.h"
#include "objects/Renderer.h"
#include "Objects/Types.h"   
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
    Pyramid pyramid(1.0f, 1.5f);
    Skybox sky(20.0f);
    OBJLoader monkey(
        "C:/Users/MARKKIE/source/repos/d3letereal/ChompApi/ChompAPI/models/Kettle.obj",
        { 0, 0, 0 }, 0.06f, { 0, 0, 5 }
    );

    // Object transforms
    Transform pyramidT{ {0, 0, 5}, {0, 0, 0}, 2.5f };
    Transform skyT{ {0, 0, 0}, {0, 0, 0}, 1.0f };
    Transform monkeyT = monkey.t;

    window.StartRenderLoop([&]() {
        int w = window.GetWidth();
        int h = window.GetHeight();
        int* fb = window.GetFramebuffer();
        float* zb = window.GetZBuffer();

        if (fb && zb) {
            // Clear framebuffer and z-buffer
            std::fill(fb, fb + w * h, 0x000000);
            std::fill(zb, zb + w * h, 1e9f);

            // Draw background
            sky.Draw(skyT, fb, nullptr, w, h);

            // Renderer for shading and shadows
            Renderer renderer(fb, zb, w, h);

            
            

            // Draw monkey without shadows (optional)
            monkey.Draw(monkeyT, fb, zb, w, h, Colors::Red);
        }

        // -------- Handle Pyramid rotation --------


        // -------- Handle Monkey rotation --------
        if (window.IsKeyPressed(KEY_W)) monkeyT.rotation.x += 0.05f;
        if (window.IsKeyPressed(KEY_S)) monkeyT.rotation.x -= 0.05f;
        if (window.IsKeyPressed(KEY_A)) monkeyT.rotation.y += 0.05f;
        if (window.IsKeyPressed(KEY_D)) monkeyT.rotation.y -= 0.05f;
        if (window.IsKeyPressed(KEY_Q)) monkeyT.rotation.z += 0.05f;
        if (window.IsKeyPressed(KEY_E)) monkeyT.rotation.z -= 0.05f;
        });

    // Main event loop
    while (true) {
        window.ProcessEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
