#include <window.hpp>

#include <thread>
#include <mutex>
#include <latch>
#include <atomic>


LRESULT CALLBACK winproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


namespace Envy::window
{

    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TU locals ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    namespace
    {

        // -- window state

        std::atomic<bool> open {};
        std::atomic<bool> fullscreen {};

        bool sizing {false};

        RECT restore_rect;

        HWND hwnd {nullptr};

        Envy::vector2<i32> min_size;
        Envy::vector2<i32> max_size;
        Envy::vector2<i32> current_size;

        // -- syncronization

        std::mutex mutex_hwnd;
        std::mutex mutex_open;
        std::mutex mutex_size;

        std::latch window_latch {1};

        // -- cursor

        HCURSOR cursor {LoadCursor(NULL, IDC_ARROW)};

        // -- diagnostics

        Envy::logger winlog {"Window"};
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Interface Implementation ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void init(const description& windesc)
    {
        set_size_constraints(windesc.minimum_window_size, windesc.maximum_window_size);

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
        wc.hCursor = cursor;

        winlog.assert( RegisterClassEx(&wc) > 0 , "Could not register window class" );

        // Create Window
        hwnd = CreateWindowEx(
            0,                             // Optional window styles
            L"EnvyWindow",                 // Window class
            windesc.title.c_str(),         // Title
            WS_OVERLAPPEDWINDOW,           // Window style
            CW_USEDEFAULT, CW_USEDEFAULT,  // Pos
            CW_USEDEFAULT, CW_USEDEFAULT,  // Size
            nullptr,                       // Parent
            nullptr,                       // Menu
            GetModuleHandle(nullptr),      // Instance
            nullptr                        // Additional data
        );

        winlog.assert(hwnd, "Could not create window!");

        ShowWindow(hwnd, SW_SHOWDEFAULT);

        open.store(true);

        winlog.info("Window successfully initialized!")();

        // notifies main thread that window has been created
        window_latch.count_down();
    }


    //**********************************************************************
    void wait_for_creation()
    { window_latch.wait(); }


    //**********************************************************************
    void run(std::stop_token stop_token, const description& windesc)
    {
        init(windesc);

        // Message Loop
        MSG msg;
        while(GetMessage(&msg, NULL, 0, 0) && !stop_token.stop_requested())
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        open.store(false);
    }


    //**********************************************************************
    bool is_open()
    { return open.load(); }


    //**********************************************************************
    void request_close()
    { PostMessage(hwnd, WM_CLOSE, 0, 0); }


    //**********************************************************************
    HWND get_hwnd()
    { return hwnd; }


    //**********************************************************************
    void set_size_constraints(Envy::vector2<i32> min, Envy::vector2<i32> max)
    {
        std::scoped_lock l {mutex_size};
        min_size = min;
        max_size = max;
    }


    //**********************************************************************
    void request_fullscreen(bool full)
    {
        if(fullscreen.load() != full)
        {
            fullscreen.store(full);

            if(fullscreen.load())
            {
                // -- Go fullscreen

                ::GetWindowRect(hwnd, &restore_rect);

                u32 borderless = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

                ::SetWindowLongW(hwnd, GWL_STYLE, borderless);

                // Query the name of the nearest display device for the window.
                // This is required to set the fullscreen dimensions of the window
                // when using a multi-monitor setup.
                HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFOEX monitor_info {};
                monitor_info.cbSize = sizeof(MONITORINFOEX);
                ::GetMonitorInfo(monitor, &monitor_info);

                ::SetWindowPos(hwnd, HWND_TOP,
                    monitor_info.rcMonitor.left,
                    monitor_info.rcMonitor.top,
                    monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                    monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                    SWP_FRAMECHANGED | SWP_NOACTIVATE);

                ::ShowWindow(hwnd, SW_MAXIMIZE);
            }
            else
            {
                // -- Restore all the window decorators.

                ::SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

                ::SetWindowPos(hwnd, HWND_NOTOPMOST,
                    restore_rect.left,
                    restore_rect.top,
                    restore_rect.right - restore_rect.left,
                    restore_rect.bottom - restore_rect.top,
                    SWP_FRAMECHANGED | SWP_NOACTIVATE);

                ::ShowWindow(hwnd, SW_NORMAL);
            }
        }
    }


    //**********************************************************************
    Envy::vector2<i32> get_size()
    {
        return current_size;
    }
}


// [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Window Procedure ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


LRESULT CALLBACK winproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    using namespace Envy::window;

    switch(uMsg)
    {

    // -----------------------------------------------------
    // window was destroyed, quit application
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    // -----------------------------------------------------
    // Give windows the window size constraints
    case WM_GETMINMAXINFO:
    {
        std::scoped_lock l {mutex_size};

        LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;

        lpMMI->ptMinTrackSize.x = min_size.x;
        lpMMI->ptMinTrackSize.y = min_size.y;
        // TODO: max_size , maxamized size/pos
        return 0;
    }

    // -----------------------------------------------------
    // window is resizing, keep track of the size
    case WM_SIZE:
    {
        current_size = { LOWORD(lParam) , HIWORD(lParam) };

        Envy::post_event<resizing>( current_size );

        // handle maxamized and restore events
        if(wParam == SIZE_MAXIMIZED || (wParam == SIZE_RESTORED && !sizing))
        {
            Envy::post_event<resized>( current_size );
        }

        return 0;
    }

    // -----------------------------------------------------
    // set sizing flag
    case WM_ENTERSIZEMOVE:
    {
        sizing = true;
        return 0;
    }

    // -----------------------------------------------------
    // window resizing finnished, notify engine
    case WM_EXITSIZEMOVE:
    {
        sizing = false;
        Envy::post_event<resized>( current_size );
        return 0;
    }

    // -----------------------------------------------------
    // keyboard input
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

        if( (alt && wParam == VK_RETURN) || wParam == VK_F11)
        {
            request_fullscreen(!fullscreen.load());
            Envy::post_event<resized>( current_size );
        }
    }

    // -----------------------------------------------------
    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}