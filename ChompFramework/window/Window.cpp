#include "Window.h"
#include <iostream>
#include <algorithm>
#include <chrono>

#ifdef _WIN32
#include <windows.h>

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* win = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg) {
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

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#endif

Window::Window(int w, int h, const std::string& t)
    : width(w), height(h), title(t), running(false)
{
    framebuffer.resize(width * height, 0x000000);
    zbuffer.resize(width * height, 1.0f);

#ifdef __APPLE__
    isMac = true;
#else
    isMac = false;
#endif

    if (verbose)
        std::cout << "Running on " << (isMac ? "macOS" : "Windows") << std::endl;

    if (isMac) {
#ifdef __APPLE__
        InitMac();
#endif
    }
    else {
#ifdef _WIN32
        InitWindows();
#endif
    }
}

Window::~Window() {
    StopRenderLoop();

    if (isMac) {
#ifdef __APPLE__
        if (nsWindow)[(__bridge NSWindow*)nsWindow close];
#endif
    }
    else {
#ifdef _WIN32
        if (hwnd) DestroyWindow((HWND)hwnd);
#endif
    }
}

int* Window::GetFramebuffer() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    return framebuffer.data();
}

float* Window::GetZBuffer() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    return zbuffer.data();
}

int Window::GetWidth() const { return width; }
int Window::GetHeight() const { return height; }

void Window::HandleResize(int newW, int newH) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    if (newW == width && newH == height) return;

    width = newW;
    height = newH;
    framebuffer.resize(width * height, 0x000000);
    zbuffer.resize(width * height, 1.0f);

    if (verbose)
        std::cout << "Resized to " << width << "x" << height << std::endl;
}

void Window::StartRenderLoop(std::function<void()> onFrame) {
    running = true;
    renderThread = std::thread([this, onFrame]() {
        while (running) {
            // Clear buffers at the start of each frame
            {
                std::lock_guard<std::mutex> lock(bufferMutex);
                std::fill(framebuffer.begin(), framebuffer.end(), 0x000000);
                std::fill(zbuffer.begin(), zbuffer.end(), 1.0f);
            }

            if (onFrame) onFrame();
            PlatformRender();
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
        });
}

void Window::StopRenderLoop() {
    running = false;
    if (renderThread.joinable()) renderThread.join();
}

void Window::ProcessEvents() {
#ifdef _WIN32
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) running = false;
    }
#endif

#ifdef __APPLE__
    @autoreleasepool{
        NSEvent * event;
        while ((event = [NSApp nextEventMatchingMask : NSEventMaskAny
                                          untilDate : [NSDate distantPast]
                                             inMode : NSDefaultRunLoopMode
                                            dequeue : YES])) {
            [NSApp sendEvent : event] ;
        }
    }
#endif
}

void Window::PlatformRender() {
    std::lock_guard<std::mutex> lock(bufferMutex);

    if (isMac) {
#ifdef __APPLE__
        NSWindow* window = (__bridge NSWindow*)nsWindow;
        NSView* view = [window contentView];
        [[view layer]setBackgroundColor:[[NSColor colorWithRed : 0.39 green : 0.59 blue : 0.78 alpha : 1.0]CGColor] ];

        // If you want, you can later draw framebuffer using CALayer or OpenGL/Metal
#endif
    }
    else {
#ifdef _WIN32
        HDC hdc = GetDC((HWND)hwnd);
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height,
            framebuffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC((HWND)hwnd, hdc);
#endif
    }
}

#ifdef _WIN32
void Window::InitWindows() {
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
#endif

#ifdef __APPLE__
void Window::InitMac() {
    [NSApplication sharedApplication] ;
    [NSApp setActivationPolicy : NSApplicationActivationPolicyRegular] ;

    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable;
    NSRect rect = NSMakeRect(0, 0, width, height);
    nsWindow = (__bridge_retained void*) [[NSWindow alloc]initWithContentRect:rect
        styleMask : style
        backing : NSBackingStoreBuffered
        defer : NO];
    [(__bridge NSWindow*)nsWindow makeKeyAndOrderFront : nil] ;
    [NSApp activateIgnoringOtherApps : YES] ;

    // Enable layer-backed view
    NSView* view = [(__bridge NSWindow*)nsWindow contentView];
    [view setWantsLayer : YES] ;

    // Resize notification with Retina support
    [[NSNotificationCenter defaultCenter]addObserverForName:NSWindowDidEndLiveResizeNotification
        object : (__bridge NSWindow*)nsWindow
        queue : [NSOperationQueue mainQueue]
        usingBlock : ^ (NSNotification * _Nonnull note) {
        NSWindow* window = (__bridge NSWindow*)nsWindow;
        CGFloat scale = window.backingScaleFactor;
        NSRect frame = [window contentRectForFrameRect : window.frame];
        HandleResize(frame.size.width * scale, frame.size.height * scale);
    }];
}
#endif
