#pragma once
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

class Window {
public:
    bool verbose = false;

    Window(int w = 800, int h = 600, const std::string& title = "Window");
    ~Window();

    void StartRenderLoop(std::function<void()> onFrame);
    void StopRenderLoop();
    void ProcessEvents();

    int* GetFramebuffer();
    float* GetZBuffer();
    int GetWidth() const;
    int GetHeight() const;

    bool IsRunning() const { return running.load(); }
    void HandleResize(int newW, int newH);

    bool IsKeyPressed(int key); // <- check key state

private:
    int width, height;
    std::string title;
    std::vector<int> framebuffer;
    std::vector<float> zbuffer;
    bool isMac;
    std::atomic<bool> running;
    std::thread renderThread;
    mutable std::mutex bufferMutex;

#ifdef _WIN32
    void* hwnd = nullptr;
    void InitWindows();
#endif

#ifdef __APPLE__
    void* nsWindow = nullptr;
    void InitMac();
#endif

    void PlatformRender();
};
