/**
 * @file window.cpp
 * @author Christian Saloň
 */

#include <stdexcept>
#include <string>

#if defined(USE_WIN32)

#include <windows.h>

#elif defined(USE_X11)

#include <X11/Xlib.h>
#include <cstring>

#endif

#include "window.h"
#include "kb_input.h"

const std::string MainWindow::DEFAULT_WINDOW_TITLE = "kio";     /**< Default window title */
const int MainWindow::DEFAULT_WIDTH = 1280;                     /**< Default window width */
const int MainWindow::DEFAULT_HEIGHT = 780;                     /**< Default window height */

#if defined(USE_WIN32)

/**
 * @brief Sets window attributes to default values
 */
MainWindow::MainWindow() {
    this->_hwnd = NULL;
    this->_hInstance = GetModuleHandle(NULL);

    this->_isActive = false;
    this->_resized = false;
    this->_width = DEFAULT_WIDTH;
    this->_height = DEFAULT_HEIGHT;
}

/**
 * @brief MainWindow destructor
 */
MainWindow::~MainWindow() {
   
}

/**
 * @brief Proccesses messages sent to WIN32 window
 * 
 * @param hwnd 
 * @param uMsg 
 * @param wParam 
 * @param lParam 
 */
LRESULT CALLBACK MainWindow::_wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow *pThis;

    switch (uMsg) {
        case WM_CREATE: {
            pThis = static_cast<MainWindow *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
            pThis->_isActive = true;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

            break;
        }

        case WM_CLOSE: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            pThis->_isActive = false;
            DestroyWindow(pThis->_hwnd);
            PostQuitMessage(0);

            return 0;
        }

        // Handle window resize
        case WM_SIZE: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            pThis->_resized = true;
            pThis->_setWidth(LOWORD(lParam));
            pThis->_setHeight(HIWORD(lParam));

            break;
        }

        // Handle UTF-16 input
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
 */
void MainWindow::create() {
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
    wc.hInstance = this->_hInstance;
    wc.lpfnWndProc = this->_wndProc;

    RegisterClassEx(&wc);
    
    this->_hwnd =
        CreateWindowEx(0,                       // Optional window styles
                       CLASS_NAME,              // Window class
                       L"kio",                  // Window text
                       WS_OVERLAPPEDWINDOW,     // Window style
                       // Size and position
                       CW_USEDEFAULT, CW_USEDEFAULT, DEFAULT_WIDTH, DEFAULT_HEIGHT,
                       NULL,                    // Parent window
                       NULL,                    // Menu
                       MainWindow::_hInstance,  // Instance handle
                       this                     // Additional application data
        );

    if (this->_hwnd == NULL) {
        throw std::runtime_error("Error creating WIN32 window");
    }
}

/**
 * @brief Registers events sent to a WIN32 window
 */
void MainWindow::pollEvents() {
    MSG msg = {};
    GetMessage(&msg, NULL, 0, 0);
    TranslateMessage(&msg); // Generates WM_CHAR messages
    DispatchMessage(&msg);
}

/**
 * @brief Shows window
 */
void MainWindow::show() {
    ShowWindow(this->_hwnd, SW_NORMAL);
}

/**
 * @brief Waits for new events sent to WIN32 window
 */
void MainWindow::wait() {
    WaitMessage();
}

/**
 * @brief Getter for WIN32 window handle
 * 
 * @return Window handle
 */
HWND MainWindow::getHwmd() {
    return this->_hwnd;
}

/**
 * @brief Getter for WIN32 HINSTANCE
 *
 * @return Window HINSTANCE
 */
HINSTANCE MainWindow::getHInstance() {
    return this->_hInstance;
}

#elif defined(USE_X11)

#endif

/**
 * @brief Checks if window is still being used
 * 
 * @return True if window is being used, false if window was destroyed
 */
bool MainWindow::isActive() {
    return this->_isActive;
}

/**
 * @brief Checks if window was resized
 *
 * @return True if window was resized, else false
 */
bool MainWindow::wasResized() {
    return this->_resized;
}

/**
 * @brief Sets if window was resized to false
 */
void MainWindow::resetResized() {
    this->_resized = false;
}

/**
 * @brief Checks if window is minimized
 *
 * @return True if window is minimized, else false
 */
bool MainWindow::isMinimized() {
    return this->_width == 0 || this->_height == 0;
}

/**
 * @brief Getter for window width
 *
 * @return Window width
 */
int MainWindow::getWidth() {
    return this->_width;
}

/**
 * @brief Setter for window width
 */
void MainWindow::_setWidth(int width) {
    if (width < 0) {
        throw std::runtime_error("Window width is negative");
    }

    this->_width = width;
    // std::cout << "Window width: " << this->_width << std::endl;
}

/**
 * @brief Getter for window height
 *
 * @return Window height
 */
int MainWindow::getHeight() {
    return this->_height;
}

/**
 * @brief Setter for window height
 */
void MainWindow::_setHeight(int height) {
    if (height < 0) {
        throw std::runtime_error("Window heigth is negative");
    }

    this->_height = height;
    // std::cout << "Window height: " << this->_height << std::endl;
}
