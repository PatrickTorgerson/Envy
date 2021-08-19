#include <window.hpp>

#include <thread>
#include <mutex>
#include <latch>
#include <atomic>


LRESULT CALLBACK winproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


namespace Envy::window
{

    // state
    namespace
    {
        std::atomic<bool> open {};
        HWND hwnd {nullptr};

        Envy::vector2<i32> min_size;
        Envy::vector2<i32> max_size;
        Envy::vector2<i32> current_size;


        std::mutex mutex_hwnd;
        std::mutex mutex_open;
        std::mutex mutex_size;

        std::latch window_latch {1};

        HCURSOR cursor {LoadCursor(NULL, IDC_ARROW)};
    }


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

        RegisterClassEx(&wc);

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

        Envy::assert(hwnd, "Could not create window!");

        ShowWindow(hwnd, SW_SHOWDEFAULT);

        open.store(true);

        Envy::info("Window successfully initialized!")();

        // notifies main thread that window has been created
        window_latch.count_down();
    }


    void wait_for_creation()
    { window_latch.wait(); }


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


    bool is_open()
    { return open.load(); }


    void request_close()
    { PostMessage(hwnd, WM_CLOSE, 0, 0); }


    HWND get_hwnd()
    { return hwnd; }


    void set_size_constraints(Envy::vector2<i32> min, Envy::vector2<i32> max)
    {
        std::scoped_lock l {mutex_size};
        min_size = min;
        max_size = max;
    }

}


LRESULT CALLBACK winproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    using namespace Envy::window;

    switch(uMsg)
    {

    // window was destroyed, quit application
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

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

    // window is resizing, keep track of the size
    case WM_SIZE:
    {
        current_size = { LOWORD(lParam) , HIWORD(lParam) };
        return 0;
    }

    // window resizing finnished, notify engine
    case WM_EXITSIZEMOVE:
    {
        Envy::post_event<resized>( current_size );
        return 0;
    }

    // case WM_SETCURSOR:
    //     SetCursor(c);
    //     return 0;

    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}