#include <thread>
#include <chrono>
#include "objects/OBJLoader.h"
#include "objects/Skybox.h"
#include "window/Window.h"

#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_Q 0x51
#define KEY_E 0x45

int main() {
    Window window(800, 600, "OASIS");
    window.verbose = true;
    // change to your path
    OBJLoader monkey(R"(C:\Users\MARKKIE\source\repos\d3letereal\ChompApi\ChompAPI\models\Kettle.obj)",
        { 0,0,0 }, 0.06f, { 0,0,5 });

    Skybox sky(20.0f);
    Transform skyT = { {0,0,0},{0,0,0},1.0f };
    Transform monkeyT = monkey.t;

    window.StartRenderLoop([&]() {
        int w = window.GetWidth();
        int h = window.GetHeight();
        int* fb = window.GetFramebuffer();
        float* zb = window.GetZBuffer();

        std::fill(fb, fb + w * h, 0x000000);
        std::fill(zb, zb + w * h, 1e9f);

        sky.Draw(skyT, fb, zb, w, h);         // draw sky first
        monkey.Draw(monkeyT, fb, zb, w, h, Colors::White);
        });

    while (window.IsRunning()) {
        window.ProcessEvents();

        if (window.IsKeyPressed(KEY_W)) monkeyT.rotation.x += 0.05f;
        if (window.IsKeyPressed(KEY_S)) monkeyT.rotation.x -= 0.05f;
        if (window.IsKeyPressed(KEY_A)) monkeyT.rotation.y += 0.05f;
        if (window.IsKeyPressed(KEY_D)) monkeyT.rotation.y -= 0.05f;
        if (window.IsKeyPressed(KEY_Q)) monkeyT.rotation.z += 0.05f;
        if (window.IsKeyPressed(KEY_E)) monkeyT.rotation.z -= 0.05f;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
