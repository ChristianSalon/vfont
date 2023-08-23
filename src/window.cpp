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
#include "kb_input.h"

#if defined(USE_WIN32)

HINSTANCE Window::_hInstance = GetModuleHandle(NULL);

/**
 * @brief Main entry point for Win32 apps
 * 
 * @param hwnd 
 * @param uMsg 
 * @param wParam 
 * @param lParam
 */
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

        case WM_CHAR: {
            if (IS_HIGH_SURROGATE(wParam)) {
                // Utf-16 encoded character is 2 code units long, high code unit is being processed
                KbInput::currentCharacter.type = KbInput::Utf16Type::TWO_CODE_UNITS;
                KbInput::currentCharacter.bytes_4[0] = wParam;
            }
            else if (IS_LOW_SURROGATE(wParam)) {
                // Utf-16 encoded character is 2 code units long, low code unit is being processed
                KbInput::currentCharacter.bytes_4[1] = wParam;
                KbInput::registerCharacter(KbInput::currentCharacter);
            }
            else {
                // Utf-16 encoded character is 1 code unit long
                KbInput::utf16_t c;
                c.type = KbInput::Utf16Type::ONE_CODE_UNIT;
                c.bytes_2 = wParam;
                KbInput::registerCharacter(c);
            }

            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/**
 * @brief Creates and initializes window
 * 
 * @throws runtime_error Error creating window
 */
void Window::init() {
    const wchar_t CLASS_NAME[] = L"KIO Window Class";

    WNDCLASSEX wc = {};
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

    RegisterClassEx(&wc);

    // Create the window.
    this->_hwnd =
        CreateWindowEx(0,                  // Optional window styles
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

/**
 * @brief Shows window
 */
void Window::show() {
    ShowWindow(this->_hwnd, SW_NORMAL);
}

/**
 * @brief Hides window
 */
void Window::hide() {
    ShowWindow(this->_hwnd, SW_HIDE);
}

/**
 * @brief Runs main loop of window
 */
void Window::mainLoop() {
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg); // Generates WM_CHAR messages
        DispatchMessage(&msg);
    }
}

/**
 * @brief Closes window
 */
void Window::close() {
    PostQuitMessage(0);
}

#elif defined(USE_X11)

#endif
