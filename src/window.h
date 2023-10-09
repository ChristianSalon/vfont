/**
 * @file window.h
 * @author Christian Saloň
 */

#pragma once

#include <string>

#if defined(USE_WIN32)

#include <windows.h>

#elif defined(USE_X11)

#include <X11/Xlib.h>

#endif

/**
 * @class MainWindow
 * 
 * @brief Abstraction for handling platform specific window operations
 */
class MainWindow {

public:

    static const int DEFAULT_WIDTH;
    static const int DEFAULT_HEIGHT;
    static const std::string DEFAULT_WINDOW_TITLE;

private:

    bool _isActive;
    bool _resized;
    int _width;
    int _height;

public:

    MainWindow();
    ~MainWindow();

    void create();
    void show();
    void pollEvents();
    void wait();

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

#elif defined(USE_X11)

    Display *_display;
    int _screen;
    Window _window;
    XIM _inputMethod;
    XIC _inputContext;

    Atom _wmDeleteMessage;
    
#endif

public:

#if defined(USE_WIN32)

    HWND getHwmd();
    HINSTANCE getHInstance();

#elif defined(USE_X11)

    Display* getDisplay();
    int getScreen();
    Window getWindow();

#endif

};
