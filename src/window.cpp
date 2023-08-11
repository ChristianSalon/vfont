/**
 * @file window.cpp
 * @author Christian Saloň
 */

#include <stdexcept>

#if defined(USE_WIN32)

#include <windows.h>

#elif defined(USE_X11)

#endif

#include "window.h"

#if defined(USE_WIN32)

HINSTANCE Window::_hInstance = GetModuleHandleW(NULL);

LRESULT CALLBACK Window::_wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint

            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);

            return 0;
        }
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void Window::init() {
    const wchar_t CLASS_NAME[] = L"KIO Window Class";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.style = NULL;
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hInstance = Window::_hInstance;
    wc.lpfnWndProc = Window::_wndProc;

    RegisterClassExW(&wc);

    // Create the window.
    this->_hwnd =
        CreateWindowExW(0,                  // Optional window styles
                       CLASS_NAME,          // Window class
                       L"kio",              // Window text
                       WS_OVERLAPPEDWINDOW, // Window style

                       // Size and position
                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                       NULL,                // Parent window
                       NULL,                // Menu
                       Window::_hInstance,  // Instance handle
                       NULL                 // Additional application data
        );

    if (this->_hwnd == NULL) {
        throw std::runtime_error("Error creating WIN32 window.");
    }
}

void Window::show() {
    ShowWindow(this->_hwnd, SW_NORMAL);
}

void Window::hide() {
    ShowWindow(this->_hwnd, SW_HIDE);
}

void Window::mainLoop() {
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Window::close() {
    PostQuitMessage(0);
}

#elif defined(USE_X11)

#endif
