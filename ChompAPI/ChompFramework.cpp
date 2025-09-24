#include "window/Window.h"
#include "objects/Cube.h"
#include <algorithm>
#include <chrono>
#include <thread>



int main() {
    Window window(800, 600, "OASIS");
    window.verbose = true;

    Cube cube;
	// POS X, Y, Z, ROT X, Y, Z, SCALE
    Transform t{ {0,0,0}, {0,0,0}, 2.5f };

window.StartRenderLoop([&]() {
    int w = window.GetWidth();
    int h = window.GetHeight();

    auto fb = window.GetFramebuffer();
    auto zb = window.GetZBuffer();

    if (fb && zb) {
        std::fill(fb, fb + w * h, 0x000000);
        std::fill(zb, zb + w * h, 1.0f);

        // Draw cube with fixed rotation
        cube.Draw({ 255,0,0 }, t, fb, zb, w, h);
    }

    // <- REMOVE ANY rotation changes here
});


    while (true) {
        window.ProcessEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
