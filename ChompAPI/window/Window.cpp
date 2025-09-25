#include "Window.h"
#include <iostream>
#include <algorithm>
#include <chrono>

#ifdef _WIN32
#include <windows.h>

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window* win = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
    case WM_SIZE:
        if (win) {
            int newW = LOWORD(lParam);
            int newH = HIWORD(lParam);
            if (newW > 0 && newH > 0)
                win->HandleResize(newW, newH);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

Window::Window(int w, int h, const std::string& t)
    : width(w), height(h), title(t), running(false)
{
    framebuffer.resize(width * height, 0x000000);
    zbuffer.resize(width * height, 1e9f);

#ifdef __APPLE__
    isMac = true;
#else
    isMac = false;
#endif

    if (verbose)
        std::cout << "Running on " << (isMac ? "macOS" : "Windows") << std::endl;

    if (isMac)
    {
#ifdef __APPLE__
        InitMac();
#endif
    }
    else
    {
#ifdef _WIN32
        InitWindows();
#endif
    }
}

Window::~Window()
{
    StopRenderLoop();
#ifdef _WIN32
    if (hwnd) DestroyWindow((HWND)hwnd);
#endif
#ifdef __APPLE__
    // macOS cleanup
#endif
}

int* Window::GetFramebuffer()
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    return framebuffer.data();
}

float* Window::GetZBuffer()
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    return zbuffer.data();
}

int Window::GetWidth() const { return width; }
int Window::GetHeight() const { return height; }

void Window::HandleResize(int newW, int newH)
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    if (newW == width && newH == height) return;
    width = newW; height = newH;
    framebuffer.resize(width * height, 0x000000);
    zbuffer.resize(width * height, 1e9f);
    if (verbose) std::cout << "Resized to " << width << "x" << height << std::endl;
}

void Window::StartRenderLoop(std::function<void()> onFrame)
{
    running = true;
    renderThread = std::thread([this, onFrame]() {
        while (running)
        {
            if (onFrame) onFrame();
#ifdef _WIN32
            PlatformRender();
#endif
#ifdef __APPLE__
            PlatformRender();
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
        });
}

void Window::StopRenderLoop()
{
    running = false;
    if (renderThread.joinable()) renderThread.join();
}

void Window::ProcessEvents()
{
#ifdef _WIN32
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) running = false;
    }
#endif
#ifdef __APPLE__
    // macOS placeholder
#endif
}

bool Window::IsKeyPressed(int key)
{
#ifdef _WIN32
    return (GetAsyncKeyState(key) & 0x8000) != 0;
#else
    return false;
#endif
}

#ifdef _WIN32
void* Window::GetHWND() const { return hwnd; }

void Window::InitWindows()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {};
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ChompWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    hwnd = CreateWindowEx(0, wc.lpszClassName, title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, nullptr);

    SetWindowLongPtr((HWND)hwnd, GWLP_USERDATA, (LONG_PTR)this);
}

void Window::PlatformRender()
{
    HDC hdc = GetDC((HWND)hwnd);
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height,
            framebuffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
    }
    ReleaseDC((HWND)hwnd, hdc);
}
#endif

#ifdef __APPLE__
void Window::InitMac() {}
void Window::PlatformRender() {}
#endif
