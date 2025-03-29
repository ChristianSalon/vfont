/**
 * @file window.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

#include "kb_input.h"

#if defined(USE_WIN32)

#include <windows.h>

#elif defined(USE_X11)

#include <cstring>

#include <X11/Xlib.h>

#elif defined(USE_WAYLAND)

#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon.h>

#include "xdg-decoration-client-protocol.h"
#include "xdg-shell-client-protocol.h"

#endif

/**
 * @brief Abstraction for handling platform specific window operations (WIN32, X11, Wayland)
 */
class MainWindow {
public:
    static constexpr int DEFAULT_WIDTH = 512;      /**< Default window width */
    static constexpr int DEFAULT_HEIGHT = 512;     /**< Default window height */
    static const std::string DEFAULT_WINDOW_TITLE; /**< Default window title */

private:
    bool _isActive;                    /**< Indicates that the window is oppened */
    bool _resized;                     /**< Indicates that the window was resized */
    int _width;                        /**< Width of window */
    int _height;                       /**< Height of window */
    bool _leftMouseButtonDown{false};  /**< Indicates whether the left button of mouse is pressed */
    bool _rightMouseButtonDown{false}; /**< Indicates whether the right button of mouse is pressed */

    /**
     * Resize callback function
     * First parameter: Window width
     * Second parameter: Window height
     */
    std::function<void(int, int)> _resizeCallback;

    /**
     * Left mouse key drag callback function
     * First parameter: Delta x
     * Second parameter: Delta y
     */
    std::function<void(float, float)> _leftDragCallback;

    /**
     * Left mouse key drag callback function
     * First parameter: Delta x
     * Second parameter: Delta y
     */
    std::function<void(float, float)> _rightDragCallback;

    /**
     * Scroll callback function
     * First parameter: Delta z
     */
    std::function<void(float)> _scrollCallback;

    /**
     * Keypress callback function
     * First parameter: Composed unicode code point
     */
    std::function<void(uint32_t)> _keypressCallback;

public:
    MainWindow();
    ~MainWindow();

    void create();
    void show();
    void pollEvents();
    void wait();

    void setResizeCallback(std::function<void(int, int)> resizeCallback);
    void setLeftDragCallback(std::function<void(float, float)> dragCallback);
    void setRightDragCallback(std::function<void(float, float)> dragCallback);
    void setScrollCallback(std::function<void(float)> scrollCallback);
    void setKeypressCallback(std::function<void(uint32_t)> keypressCallback);

    bool isActive();
    bool isMinimized();

    bool wasResized();
    void resetResized();

    int getWidth();
    int getHeight();

private:
    void _setWidth(int width);
    void _setHeight(int heigth);

#if defined(USE_WIN32)

    HWND _hwnd;
    HINSTANCE _hInstance;
    static LRESULT CALLBACK _wndProc(HWND hwndMain, UINT uMsg, WPARAM wParam, LPARAM lParam);

    POINT _lastMousePosition;

#elif defined(USE_X11)

    Display *_display;
    int _screen;
    Window _window;
    XIM _inputMethod;
    XIC _inputContext;

    Atom _wmDeleteMessage;

#elif defined(USE_WAYLAND)

    bool _isMinimized;

    wl_display *_display;
    wl_registry *_registry;
    wl_compositor *_compositor;
    wl_surface *_surface;
    wl_seat *_kbSeat;
    wl_keyboard *_wlKeyboard;
    wl_pointer *_wlPointer;

    xdg_wm_base *_xdgWmBase;
    xdg_surface *_xdgSurface;
    xdg_toplevel *_xdgToplevel;

    zxdg_decoration_manager_v1 *_decorationManager;
    zxdg_toplevel_decoration_v1 *_toplevelDecoration;

    struct xkb_context *_xkbContext;
    struct xkb_keymap *_xkbKeymap;
    struct xkb_state *_xkbState;
    struct xkb_compose_state *_xkbComposeState;

    static struct wl_registry_listener _registryListener;
    static struct wl_seat_listener _kbSeatListener;
    static struct wl_keyboard_listener _wlKeyboardListener;
    static struct wl_pointer_listener _wlPointerListener;
    static struct xdg_wm_base_listener _xdgWmBaseListener;
    static struct xdg_surface_listener _xdgSurfaceListener;
    static struct xdg_toplevel_listener _xdgToplevelListener;

#endif

public:
#if defined(USE_WIN32)

    HWND getHwmd();
    HINSTANCE getHInstance();

#elif defined(USE_X11)

    Display *getDisplay();
    int getScreen();
    Window getWindow();

#elif defined(USE_WAYLAND)

    wl_display *getDisplay();
    wl_surface *getSurface();
    wl_registry *getRegistry();
    wl_compositor *getCompositor();

#endif
};
