/**
 * @file window.cpp
 * @author Christian Saloň
 */

#include <stdexcept>
#include <string>
#include <functional>

#if defined(USE_WIN32)

#include <windows.h>

#elif defined(USE_X11)

#include <cstring>

#include <X11/Xlib.h>

#elif defined(USE_WAYLAND)

#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

#endif

#include "window.h"
#include "kb_input.h"

const std::string MainWindow::DEFAULT_WINDOW_TITLE = "vfont-demo";  /**< Default window title */
const int MainWindow::DEFAULT_WIDTH = 512;                          /**< Default window width */
const int MainWindow::DEFAULT_HEIGHT = 512;                         /**< Default window height */

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
    this->_dragging = false;

    this->setResizeCallback([](int, int) -> void {});
    this->setDragCallback([](float, float, bool) -> void {});
    this->setScrollCallback([](float) -> void {});
    this->setKeypressCallback([](uint32_t) -> void {});
}

/**
 * @brief MainWindow destructor
 */
MainWindow::~MainWindow() {}

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

    switch(uMsg) {
        case WM_CREATE: {
            pThis = static_cast<MainWindow *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
            pThis->_isActive = true;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

            break;
        }

        case WM_CLOSE: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(!pThis) {
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
            if(!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            // Window minimization
            if(LOWORD(lParam) == 0 || HIWORD(lParam) == 0) {
                break;
            }

            pThis->_resized = true;
            pThis->_setWidth(LOWORD(lParam));
            pThis->_setHeight(HIWORD(lParam));

            break;
        }

        // End of window resizing or window repositioning
        case WM_EXITSIZEMOVE: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            pThis->_resizeCallback(pThis->_width, pThis->_height);

            break;
        }

        case WM_LBUTTONDOWN: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            SetCapture(hwnd);
            pThis->_dragging = true;
            GetCursorPos(&pThis->_lastMousePosition);

            break;
        }

        case WM_LBUTTONUP: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            ReleaseCapture();
            pThis->_dragging = false;

            break;
        }

        case WM_MOUSEMOVE: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            if(pThis->_dragging) {
                POINT currentMousePos;
                GetCursorPos(&currentMousePos);

                int dx = currentMousePos.x - pThis->_lastMousePosition.x;
                int dy = currentMousePos.y - pThis->_lastMousePosition.y;
                bool shiftPressed = wParam & MK_SHIFT;

                pThis->_dragCallback(dx, dy, shiftPressed);
                pThis->_lastMousePosition = currentMousePos;
            }

            break;
        }

        case WM_MOUSEWHEEL: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            float z = GET_WHEEL_DELTA_WPARAM(wParam);
            pThis->_scrollCallback(z);

            break;
        }

        // Handle UTF-16 input
        case WM_CHAR: {
            pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(!pThis) {
                throw std::runtime_error("Error getting window data");
            }

            if (IS_HIGH_SURROGATE(wParam)) {
                // Utf-16 encoded character is 2 code units long, high code unit is being processed
                KbInput::currentCharacter.type = KbInput::Utf16Type::TWO_CODE_UNITS;
                KbInput::currentCharacter.bytes_4[0] = wParam;
            }
            else if (IS_LOW_SURROGATE(wParam)) {
                // Utf-16 encoded character is 2 code units long, low code unit is being processed
                KbInput::currentCharacter.bytes_4[1] = wParam;
                pThis->_keypressCallback(KbInput::utf16ToCodePoint(KbInput::currentCharacter));
            }
            else {
                // Utf-16 encoded character is 1 code unit long
                KbInput::utf16_t character;
                character.type = KbInput::Utf16Type::ONE_CODE_UNIT;
                character.bytes_2 = wParam;
                pThis->_keypressCallback(KbInput::utf16ToCodePoint(character));
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
    const wchar_t CLASS_NAME[] = L"VFONT DEMO Window Class";

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

    RECT rect = { 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    this->_hwnd = CreateWindowEx(
        0,                       // Optional window styles
        CLASS_NAME,              // Window class
        L"vfont-demo",           // Window text
        WS_OVERLAPPEDWINDOW,     // Window style
        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL,                    // Parent window
        NULL,                    // Menu
        MainWindow::_hInstance,  // Instance handle
        this                     // Additional application data
    );

    if(this->_hwnd == NULL) {
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

/**
 * @brief Checks if window is minimized
 *
 * @return True if window is minimized, else false
 */
bool MainWindow::isMinimized() {
    return this->_width == 0 && this->_height == 0;
}

#elif defined(USE_X11)

/**
 * @brief Sets window attributes to default values
 */
MainWindow::MainWindow() {
    this->_isActive = false;
    this->_resized = false;
    this->_width = DEFAULT_WIDTH;
    this->_height = DEFAULT_HEIGHT;

    this->setResizeCallback([](int, int) -> void {});
    this->setDragCallback([](float, float, bool) -> void {});
    this->setScrollCallback([](float) -> void {});
    this->setKeypressCallback([](uint32_t) -> void {});

    this->_display = nullptr;
    this->_inputMethod = nullptr;
    this->_inputContext = nullptr;
}

/**
 * @brief MainWindow destructor
 */
MainWindow::~MainWindow() {
    // Closes window
    XDestroyWindow(this->_display, this->_window);
    this->_display = nullptr;
}

/**
 * @brief Creates and initializes window
 */
void MainWindow::create() {
    // Get current locale, and if not set, set it to default locale
    // Locale is used for detecting key sequences
    const char *locale = setlocale(LC_CTYPE, "");
    if (!locale) {
        locale = setlocale(LC_CTYPE, "en_US.UTF-8");
    }
    
    // Open X11 display
    this->_display = XOpenDisplay(nullptr);
    if (this->_display == nullptr) {
        throw std::runtime_error("Error opening X11 display");
        return;
    }

    // Handle window close
    this->_wmDeleteMessage = XInternAtom(this->_display, "WM_DELETE_WINDOW", False);

    // Create window
    this->_screen = DefaultScreen(this->_display);
    this->_window = XCreateWindow(
        this->_display,
        RootWindow(this->_display, this->_screen),
        0, 0,
        DEFAULT_WIDTH, DEFAULT_HEIGHT,
        1,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        0,
        nullptr
    );
    XStoreName(this->_display, this->_window, this->DEFAULT_WINDOW_TITLE.c_str());
    XSelectInput(this->_display, this->_window, StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
    XSetWMProtocols(this->_display, this->_window, &(this->_wmDeleteMessage), 1);

    this->_inputMethod = XOpenIM(this->_display, nullptr, nullptr, nullptr);
    if (this->_inputMethod == nullptr) {
        throw std::runtime_error("Error creating X11 input method");
        return;
    }

    // Create input context for window
    this->_inputContext = XCreateIC(this->_inputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, this->_window, nullptr);
    if (this->_inputContext == nullptr) {
        throw std::runtime_error("Error creating X11 input context");
        return;
    }

    // Set input focus to window
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
        case ConfigureNotify: {
            XConfigureEvent xce = ev.xconfigure;

            if (xce.width != this->_width || xce.height != this->_height) {
                // Handle window resize
                this->_resized = true;
                this->_width = xce.width;
                this->_height = xce.height;
                this->_resizeCallback(this->_width, this->_height);
            }

            break;
        }

        case ClientMessage: {
            Atom protocol = ev.xclient.data.l[0];

            if(protocol == this->_wmDeleteMessage) {
                // Window should close
                this->_isActive = false;
            }

            break;
        }

        case MotionNotify: {
            static int lastMouseX = -1;
            static int lastMouseY = -1;

            XMotionEvent motionEvent = ev.xmotion;

            if(lastMouseX == -1 && lastMouseY == -1) {
                lastMouseX = motionEvent.x;
                lastMouseY = motionEvent.y;
                break;
            }

            // Check if left button of mouse is pressed
            if(motionEvent.state & Button1Mask) {
                int deltaX = motionEvent.x - lastMouseX;
                int deltaY = motionEvent.y - lastMouseY;

                // Check if shift key is pressed
                if(motionEvent.state & ShiftMask) {
                    this->_dragCallback(deltaX, deltaY, true);
                }
                else {
                    this->_dragCallback(deltaX, deltaY, false);
                }
            }

            lastMouseX = motionEvent.x;
            lastMouseY = motionEvent.y;

            break;
        }

        case ButtonPress: {
            static float scrollFactor = 40;

            // Check if user scrolled up
            if(ev.xbutton.button == Button4) {
                this->_scrollCallback(scrollFactor);
            }
            // Check if user scrolled down
            else if(ev.xbutton.button == Button5) {
                this->_scrollCallback(-scrollFactor);
            }
        }

        case KeyPress: {
            int maxBufferBytes = 8;
            char buffer[maxBufferBytes];
            int nOfCodeUnits = 0; // Size of utf-8 character in bytes
            Status status = 0;
            nOfCodeUnits = Xutf8LookupString(this->_inputContext, (XKeyPressedEvent *)&ev, buffer, maxBufferBytes, nullptr, &status);
                
            if(status == XBufferOverflow) {
                throw std::runtime_error("Error processing character");
            }
            else if(status == XLookupChars) {
                KbInput::utf8_t character;
                switch(nOfCodeUnits) {
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

                this->_keypressCallback(KbInput::utf8ToCodePoint(character));

                break;
            }
        }
    }
}

/**
 * @brief Show window after creation
 */
void MainWindow::show() {
    XMapWindow(this->_display, this->_window);
    this->_isActive = true;
}

/**
 * @brief Wait for window events
 */
void MainWindow::wait() {
    // TODO
}

/**
 * @brief Checks if window is minimized
 *
 * @return True if window is minimized, else false
 */
bool MainWindow::isMinimized() {
    return this->_width == 0 && this->_height == 0;
}

/**
 * @brief Getter for x11 display
 * 
 * @returns Pointer to x11 Display
 */
Display* MainWindow::getDisplay() {
    return this->_display;
}

/**
 * @brief Getter for x11 Screen
 * 
 * @returns x11 screen
 */
int MainWindow::getScreen() {
    return this->_screen;
}

/**
 * @brief Getter for x11 window
 * 
 * @returns Pointer to x11 Window
 */
Window MainWindow::getWindow() {
    return this->_window;
}

#elif defined(USE_WAYLAND)

/**
 * @brief Wayland registry listener
 */
struct wl_registry_listener MainWindow::_registryListener = {
    .global = [](void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        if (strcmp(interface, wl_compositor_interface.name) == 0) {
            pThis->_compositor = static_cast<wl_compositor *>(wl_registry_bind(registry, name, &wl_compositor_interface, 1));
        }
        else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
            pThis->_xdgWmBase = static_cast<xdg_wm_base *>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
        }
        else if(strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
            pThis->_decorationManager = static_cast<zxdg_decoration_manager_v1 *>(wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
        }
        else if (strcmp(interface, wl_seat_interface.name) == 0) {
            pThis->_kbSeat = static_cast<wl_seat *>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
        }
    }
};

/**
 * @brief Wayland xdg-shell wm base listener
 */
struct xdg_wm_base_listener MainWindow::_xdgWmBaseListener = {
    .ping = [](void *data, struct xdg_wm_base *xdgWmBase, uint32_t serial) {
        xdg_wm_base_pong(xdgWmBase, serial);
    }
};

/**
 * @brief Wayland xdg-shell surface listener
 */
struct xdg_surface_listener MainWindow::_xdgSurfaceListener = {
    .configure = [](void *data, struct xdg_surface *xdgSurface, uint32_t serial) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        xdg_surface_ack_configure(xdgSurface, serial);
        wl_surface_commit(pThis->_surface);
    }
};

/**
 * @brief Wayland xdg-shell toplevel listener
 */
struct xdg_toplevel_listener MainWindow::_xdgToplevelListener = {
    .configure = [](void *data, struct xdg_toplevel *xdgToplevel, int32_t width, int32_t height, struct wl_array *states) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        // Check if window is minimized
        uint32_t *state;
        bool isMinimized = true;
        for(int i = 0; i < states->size; i++) {
            state = (uint32_t *) ((char *) states->data + i * sizeof(uint32_t));
            if(*state == XDG_TOPLEVEL_STATE_ACTIVATED) {
                isMinimized = false;
            }
        }
        pThis->_isMinimized = isMinimized;

        if((width != 0 && height != 0) && (pThis->_width != width || pThis->_height != height)) {
            // Update window dimensions
            pThis->_setWidth(width);
            pThis->_setHeight(height);
            pThis->_resized = true;
            pThis->_resizeCallback(pThis->_width, pThis->_height);
        }
    },
    .close = [](void *data, struct xdg_toplevel *xdgToplevel) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        // Window should be closed
        pThis->_isActive = false;
    }
};

/**
 * @brief Wayland seat listener
 */
struct wl_seat_listener MainWindow::_kbSeatListener = {
    .capabilities = [](void *data, struct wl_seat *seat, uint32_t capabilities) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        // Check if keyboard input is supported
        if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
            pThis->_wlKeyboard = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(pThis->_wlKeyboard, &(pThis->_wlKeyboardListener), pThis);
        }

        if(capabilities & WL_SEAT_CAPABILITY_POINTER) {
            pThis->_wlPointer = wl_seat_get_pointer(seat);
            wl_pointer_add_listener(pThis->_wlPointer, &(pThis->_wlPointerListener), pThis);
        }
    }
};

/**
 * @brief Wayland keyboard listener
 */
struct wl_keyboard_listener MainWindow::_wlKeyboardListener = {
    // Creates keymap file
    // Describes the keyboard layout used by user
    .keymap = [](void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        // Unref currently used xkb keymap
        xkb_keymap_unref(pThis->_xkbKeymap);

        // Create new xkb keymap
        char *keymap_string = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
        pThis->_xkbKeymap = xkb_keymap_new_from_string(pThis->_xkbContext, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(keymap_string, size);
        close(fd);

        // Update xkb state with new keymap
        xkb_state_unref(pThis->_xkbState);
        pThis->_xkbState = xkb_state_new(pThis->_xkbKeymap);
    },
    .enter = [](void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {},
    .leave = [](void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {},
    // Entered on key press
    .key = [](void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        // Check if left shift was pressed
        if(key == 0x2A) {
            if(state == WL_KEYBOARD_KEY_STATE_PRESSED) {
                pThis->_isShiftPressed = true;
            }
            else {
                pThis->_isShiftPressed = false;
            }
        }

        if(state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            int maxBufferSize = 8;
            char buffer[maxBufferSize];
            int nOfCodeUnits = 0; // Size of utf-8 character in bytes
            xkb_keysym_t keysym = xkb_state_key_get_one_sym(pThis->_xkbState, key + 8);
            xkb_compose_state_feed(pThis->_xkbComposeState, keysym);

            // Check if a valid key sequence is detected
            // Valid key sequence is at least two key symbols long
            // Key sequence does not include modifier keys (SHIFT, CAPS LOCK, ...)
            if(xkb_compose_state_get_status(pThis->_xkbComposeState) == XKB_COMPOSE_COMPOSED) {
                // Valid key sequence is detected
                nOfCodeUnits = xkb_compose_state_get_utf8(pThis->_xkbComposeState, buffer, maxBufferSize);
                xkb_compose_state_reset(pThis->_xkbComposeState);
            }
            else {
                // Process current character
                nOfCodeUnits = xkb_state_key_get_utf8(pThis->_xkbState, key + 8, buffer, maxBufferSize);
            }

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

                default: {
                    // Current key does not have an utf-8 value
                    return;
                }
            }

            pThis->_keypressCallback(KbInput::utf8ToCodePoint(character));
        }
    },
    .modifiers = [](void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        // Notify xkb of changed modifiers keys (SHIFT, CAPS LOCK, ...)
        xkb_state_update_mask(pThis->_xkbState, mods_depressed, mods_latched, mods_locked, 0, 0, group);
    },
    .repeat_info = [](void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);
        // TODO
    }
};

/**
 * @brief Wayland pointer listener
 */
struct wl_pointer_listener MainWindow::_wlPointerListener = {
    .enter = [](void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y) {},
    .leave = [](void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface) {},
    .motion = [](void *data, wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
        static int lastMouseX = -1;
        static int lastMouseY = -1;

        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        int newX = wl_fixed_to_int(x);
        int newY = wl_fixed_to_int(y);

        if(lastMouseX == -1 && lastMouseY == -1) {
            lastMouseX = newX;
            lastMouseY = newY;
            return;
        }

        // Check if left button of mouse is pressed
        if(pThis->_isLeftMouseButtonPressed) {
            int deltaX = newX - lastMouseX;
            int deltaY = newY - lastMouseY;

            // Check if shift key is pressed
            if(pThis->_isShiftPressed) {
                pThis->_dragCallback(deltaX, deltaY, true);
            }
            else {
                pThis->_dragCallback(deltaX, deltaY, false);
            }
        }

        lastMouseX = newX;
        lastMouseY = newY;
    },
    .button = [](void *data, wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        if(button == 0x0110 && state == WL_POINTER_BUTTON_STATE_PRESSED) {
            pThis->_isLeftMouseButtonPressed = true;
        }
        else if(button == 0x0110 && state == WL_POINTER_BUTTON_STATE_RELEASED) {
            pThis->_isLeftMouseButtonPressed = false;
        }
    },
    .axis = [](void *data, wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
        static float scrollFactor = 40;
        
        MainWindow *pThis = reinterpret_cast<MainWindow *>(data);

        if(axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
            int direction = wl_fixed_to_int(value);

            // Check if user scrolled up
            if(direction > 0) {
                pThis->_scrollCallback(-scrollFactor);
            }
            // Check if user scrolled down
            else if(direction < 0) {
                pThis->_scrollCallback(scrollFactor);
            }
        }
    }
};

/**
 * @brief Sets window attributes to default values
 */
MainWindow::MainWindow() {
    this->_isActive = false;
    this->_isMinimized = false;
    this->_resized = false;
    this->_width = DEFAULT_WIDTH;
    this->_height = DEFAULT_HEIGHT;
    this->_isLeftMouseButtonPressed = false;
    this->_isShiftPressed = false;

    this->setResizeCallback([](int, int) -> void {});
    this->setDragCallback([](float, float, bool) -> void {});
    this->setScrollCallback([](float) -> void {});
    this->setKeypressCallback([](uint32_t) -> void {});

    this->_display = nullptr;
    this->_registry = nullptr;
    this->_compositor = nullptr;
    this->_surface = nullptr;
    this->_kbSeat = nullptr;
    this->_wlKeyboard = nullptr;
    this->_wlPointer = nullptr;

    this->_xkbContext = nullptr;
    this->_xkbKeymap = nullptr;
    this->_xkbState = nullptr;

    this->_xdgWmBase = nullptr;
    this->_xdgSurface = nullptr;
    this->_xdgToplevel = nullptr;

    this->_decorationManager = nullptr;
    this->_toplevelDecoration = nullptr;
}

/**
 * @brief MainWindow destructor
 */
MainWindow::~MainWindow() {
    // Destroy xdg-shell and xdg decoration objects
    zxdg_toplevel_decoration_v1_destroy(this->_toplevelDecoration);
    xdg_toplevel_destroy(this->_xdgToplevel);
    xdg_surface_destroy(this->_xdgSurface);
    wl_surface_destroy(this->_surface);
    xdg_wm_base_destroy(this->_xdgWmBase);

    // Destroy keyboard input handling objects
    xkb_state_unref(this->_xkbState);
    xkb_keymap_unref(this->_xkbKeymap);
    xkb_context_unref(this->_xkbContext);
    wl_keyboard_destroy(this->_wlKeyboard);
    wl_seat_destroy(this->_kbSeat);

    // Destroy wayland objects
    wl_compositor_destroy(this->_compositor);
    wl_registry_destroy(this->_registry);
    wl_display_disconnect(this->_display);
}

/**
 * @brief Creates and initializes window
 */
void MainWindow::create() {
    // Connect to wayland server
    this->_display = wl_display_connect(nullptr);
    if(this->_display == nullptr) {
        throw std::runtime_error("Error connecting to Wayland server");
    }

    // Initialize wl_registry object and listener
    this->_registry = wl_display_get_registry(this->_display);
    wl_registry_add_listener(this->_registry, &(this->_registryListener), this);
    wl_display_roundtrip(this->_display);

    // Create objects for handling keyboard input
    this->_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    wl_seat_add_listener(this->_kbSeat, &(this->_kbSeatListener), this);
    wl_display_roundtrip(this->_display);

    // Get current locale, and if not set, set it to default locale
    // Locale is used for detecting key sequences
    const char *locale = setlocale(LC_CTYPE, nullptr);
    if (!locale) {
        locale = setlocale(LC_CTYPE, "en_US.UTF-8");
    }

    // Create table containing all possible key sequences defined by selected locale
    xkb_compose_table *composeTable = xkb_compose_table_new_from_locale(this->_xkbContext, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    this->_xkbComposeState = xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);

    xdg_wm_base_add_listener(this->_xdgWmBase, &(this->_xdgWmBaseListener), nullptr);

    // Initialize objects for creating a xdg-shell surface
    this->_surface = wl_compositor_create_surface(this->_compositor);
    this->_xdgSurface = xdg_wm_base_get_xdg_surface(this->_xdgWmBase, this->_surface);
    xdg_surface_add_listener(this->_xdgSurface, &(this->_xdgSurfaceListener), this);

    this->_xdgToplevel = xdg_surface_get_toplevel(this->_xdgSurface);

    if(this->_decorationManager) {
        // If xdg-shell decorations are available, initialize them
        this->_toplevelDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(this->_decorationManager, this->_xdgToplevel);
        zxdg_toplevel_decoration_v1_set_mode(this->_toplevelDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    xdg_toplevel_add_listener(this->_xdgToplevel, &(this->_xdgToplevelListener), this);
    xdg_toplevel_set_title(this->_xdgToplevel, DEFAULT_WINDOW_TITLE.c_str());

    wl_surface_commit(this->_surface);
    wl_display_flush(this->_display);
}

/**
 * @brief Runs main loop of window
 */
void MainWindow::pollEvents() {
    while(wl_display_prepare_read(this->_display) != 0) {
        wl_display_dispatch_pending(this->_display);
    }

    wl_display_flush(this->_display);
    wl_display_read_events(this->_display);
    wl_display_dispatch_pending(this->_display);
    wl_display_flush(this->_display);
}

/**
 * @brief Show window after creation
 */
void MainWindow::show() {
    this->_isActive = true;
}

/**
 * @brief Wait for window events
 */
void MainWindow::wait() {
    // TODO
}

/**
 * @brief Checks if window is minimized
 *
 * @return True if window is minimized, else false
 */
bool MainWindow::isMinimized() {
    return _isMinimized;
}

/**
 * @brief Getter for wayland display
 * 
 * @returns Pointer to wl_display
 */
wl_display* MainWindow::getDisplay() {
    return this->_display;
}

/**
 * @brief Getter for wayland surface
 * 
 * @returns Pointer to wl_surface
 */
wl_surface* MainWindow::getSurface() {
    return this->_surface;
}

/**
 * @brief Getter for wayland registry
 * 
 * @returns Pointer to wl_registry
 */
wl_registry* MainWindow::getRegistry() {
    return this->_registry;
}

/**
 * @brief Getter for wayland compositor
 * 
 * @returns Pointer to wl_compositor
 */
wl_compositor* MainWindow::getCompositor() {
    return this->_compositor;
}

#endif

/**
 * @brief Set the resize callback
 * 
 * @param resizeCallback Callback function
 */
void MainWindow::setResizeCallback(std::function<void(int, int)> resizeCallback) {
    this->_resizeCallback = resizeCallback;
}

/**
 * @brief Set the drag callback
 *
 * @param resizeCallback Callback function
 */
void MainWindow::setDragCallback(std::function<void(float, float, bool)> dragCallback) {
    this->_dragCallback = dragCallback;
}

/**
 * @brief Set the scroll callback
 *
 * @param resizeCallback Callback function
 */
void MainWindow::setScrollCallback(std::function<void(float)> scrollCallback) {
    this->_scrollCallback = scrollCallback;
}

/**
 * @brief Set the keypress callback
 *
 * @param resizeCallback Callback function
 */
void MainWindow::setKeypressCallback(std::function<void(uint32_t)> keypressCallback) {
    this->_keypressCallback = keypressCallback;
}

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
}
