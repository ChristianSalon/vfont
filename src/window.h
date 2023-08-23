/**
 * @file window.h
 * @author Christian Saloň
 */

#pragma once

#if defined(USE_WIN32)

#include <windows.h>

#elif defined(USE_X11)

#endif

class Window {

public:

    void init();

    void show();
    void hide();
    void mainLoop();
    void close();

private:

#if defined(USE_WIN32)

    HWND _hwnd;
    static HINSTANCE _hInstance;
    static LRESULT CALLBACK _wndProc(HWND hwndMain, UINT uMsg, WPARAM wParam, LPARAM lParam);

#elif defined(USE_X11)

#endif

};
