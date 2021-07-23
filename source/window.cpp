#include <window.hpp>

#include <Windows.h>

LRESULT CALLBACK winproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

namespace Envy
{

    // state
    namespace
    {
        bool open {};
        HWND hwnd {nullptr};
    }

    void init_window(const std::wstring& title)
    {
        // Window Class
        WNDCLASSEX wc {};
        wc.cbSize = sizeof( wc );
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = winproc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hIcon = nullptr;
        wc.lpszClassName = L"EnvyWindow";

        RegisterClassEx(&wc);

        // Create Window
        hwnd = CreateWindowEx(
            0,                             // Optional window styles
            L"EnvyWindow",                 // Window class
            title.c_str(),                 // Title
            WS_OVERLAPPEDWINDOW,           // Window style
            CW_USEDEFAULT, CW_USEDEFAULT,  // Pos
            CW_USEDEFAULT, CW_USEDEFAULT,  // Size
            nullptr,                       // Parent
            nullptr,                       // Menu
            GetModuleHandle(nullptr),      // Instance
            nullptr                        // Additional data
        );

        Envy::assert(hwnd, "Could not create window!");

        ShowWindow(hwnd, SW_SHOWDEFAULT);
        open = true;
    }


    void run_window(std::stop_token stop_token)
    {
        init_window(L"Wnvy Application");

        // Message Loop
        MSG msg;
        while(GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // postquit
        open = false;
    }


    bool window_is_open()
    { return open; }


    void window_request_close()
    { SendMessage(hwnd, WM_DESTROY, 0, 0); }

}


LRESULT CALLBACK winproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    // etc
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}