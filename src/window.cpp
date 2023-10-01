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

/**
 * @brief Creates and initializes window
 */
void MainWindow::create() {
    this->_display = XOpenDisplay(NULL);
    if (this->_display == NULL) {
        throw std::runtime_error("Error creating X11 window");
        return;
    }

    this->_screen = DefaultScreen(this->_display);
    this->_window = XCreateWindow(
        this->_display,
        RootWindow(this->_display, this->_screen),
        100, 100,
        DEFAULT_WIDTH, DEFAULT_HEIGHT,
        1,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        0,
        NULL
    );
    XStoreName(this->_display, this->_window, title.c_str());
    XSelectInput(this->_display, this->_window, StructureNotifyMask | KeyPressMask);

    this->_inputMethod = XOpenIM(this->_display, NULL, NULL, NULL);
    if (this->_inputMethod == NULL) {
        throw std::runtime_error("Error creating X11 window");
        return;
    }

    this->_inputContext = XCreateIC(this->_inputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, this->_window, NULL);
    if (this->_inputContext == NULL) {
        throw std::runtime_error("Error creating X11 window");
        return;
    }

    XSetICFocus(this->_inputContext);
}

/**
 * @brief Runs main loop of window
 */
void MainWindow::pollEvents() {
    XEvent ev;
    XNextEvent(this->_display, &ev);

    if (XFilterEvent(&ev, this->_window))
        return;

    switch (ev.type) {
        case KeyPress: {
            int maxBufferBytes = 4;
            char buffer[4];
            int nOfCodeUnits = 0;
            KeySym codePoint = 0;
            Status status = 0;
            nOfCodeUnits = Xutf8LookupString(this->_inputContext, (XKeyPressedEvent *)&ev, buffer, maxBufferBytes, &codePoint, &status);
                
            if (status == XBufferOverflow) {
                throw std::runtime_error("Error processing character");
            }
            else if (nOfCodeUnits != 0 && (status == XLookupKeySym || status == XLookupBoth)) {
                KbInput::utf8_t character;
                switch (nOfCodeUnits) {
                    case 1: {
                        character.type = KbInput::Utf8Type::ONE_CODE_UNIT;
                        character.bytes_1 = buffer[0];

                        break;
                    }

                    case 2: {
                        character.type = KbInput::Utf8Type::TWO_CODE_UNITS;
                        character.bytes_2[0] = buffer[0];
                        character.bytes_2[1] = buffer[1];

                        break;
                    }

                    case 3: {
                        character.type = KbInput::Utf8Type::THREE_CODE_UNITS;
                        character.bytes_3[0] = buffer[0];
                        character.bytes_3[1] = buffer[1];
                        character.bytes_3[2] = buffer[2];

                        break;
                    }
                        
                    case 4: {
                        character.type = KbInput::Utf8Type::FOUR_CODE_UNITS;
                        character.bytes_4[0] = buffer[0];
                        character.bytes_4[1] = buffer[1];
                        character.bytes_4[2] = buffer[2];
                        character.bytes_4[3] = buffer[3];

                        break;
                    }

                    default:
                        throw std::runtime_error("Error processing character");
                }

                KbInput::registerCharacter(character);

                break;
            }

            case ConfigureNotify: {
                XConfigureEvent xce = ev.xconfigure;

                if (xce.width != this->_width || xce.height != this->_height) {
                    this->_resized = true;
                    this->_width = xce.width;
                    this->_height = xce.height;
                }
            }
        }
    }
}

/**
 * @brief Show window after creation
 */
void MainWindow::show() {
    XMapWindow(this->_display, this->_window);
}

/**
 * @brief Wait for window events
 */
void MainWindow::wait() {
    // TODO
}

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
