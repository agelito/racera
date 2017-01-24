// win32_window.c

static char* WINDOW_WIN32_CLASS = "window_win32-windowclass";

extern HGLRC
gl_escalate_context(HDC device_context, HGLRC gl_context, int major, int minor);

static void 
resize_viewport(window_win32* window, int width, int height)
{
    glViewport(0, 0, width, height);

    window->width = width;
    window->height = height;
}

static LRESULT CALLBACK 
window_message_callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    window_win32* window = &global_win32.window;

    if(window == 0 || window->handle != hWnd) 
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    int x, y, rel_x, rel_y;

    switch(uMsg)
    {
    case WM_MOUSEMOVE:
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
        rel_x = (x - global_win32.mouse.absolute_x);
        rel_y = (y - global_win32.mouse.absolute_y);

        global_win32.mouse.absolute_x = x;
        global_win32.mouse.absolute_y = y;
        global_win32.mouse.relative_x = rel_x;
        global_win32.mouse.relative_y = rel_y;
        break;
    case WM_SIZE:
        resize_viewport(window, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_DESTROY:
        window->is_open = 0;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

static window_win32
window_create(int width, int height, char* title)
{
    char* window_title = title;
    char* window_class_name = WINDOW_WIN32_CLASS;

    window_win32 window = (window_win32){0};

    HINSTANCE hinstance = GetModuleHandle(NULL);
    
    WNDCLASSEX window_class;
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.lpfnWndProc = window_message_callback;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 8;
    window_class.style = CS_OWNDC;
    window_class.hInstance = hinstance;
    window_class.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_APPLICATION));
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = window_class_name;
    window_class.hIconSm = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if(!RegisterClassEx(&window_class))
    {
        MessageBox(NULL, "couldn't register window class", "error!", 0);
        return window;
    }

    HWND window_handle = CreateWindow(window_class_name, window_title, WS_OVERLAPPEDWINDOW, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL,
                                      hinstance, NULL);

    if(!window_handle)
    {
        MessageBox(NULL, "couldn't create window", "error!", 0);
        return window;
    }

    PIXELFORMATDESCRIPTOR pixel_format = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 
        0, 
        0, 
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    HDC device_context = GetDC(window_handle);
    int chosen_pixel_format = ChoosePixelFormat(device_context, &pixel_format);
    if(chosen_pixel_format == 0)
    {
        MessageBox(NULL, "failed to select suitable pixel format", "error!", 0);
        return window;
    }

    if(!SetPixelFormat(device_context, chosen_pixel_format, &pixel_format))
    {
        MessageBox(NULL, "failed to set pixel format", "error!", 0);
        return window;
    }

    HGLRC gl_context = wglCreateContext(device_context);
    wglMakeCurrent(device_context, gl_context);

    gl_context = gl_escalate_context(device_context, gl_context, 3, 2);

    window.context = device_context;
    window.handle = window_handle;
    window.gl_context = gl_context;
    window.width = width;
    window.height = height;

    ShowWindow(window_handle, SW_SHOWNORMAL);
    UpdateWindow(window_handle);

    window.is_open = 1;

    return window;
}

static void
window_destroy(window_win32* window)
{
    if(window->gl_context)
    {
        HDC device_context = GetDC(window->handle);
        wglMakeCurrent(device_context, 0);
        wglDeleteContext(window->gl_context);
    }

    if(window->handle)
    {
        DestroyWindow(window->handle);
    }
}

static void
window_process_messages(window_win32* window)
{
    MSG message;
    while(PeekMessage(&message, window->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

static void
window_redraw(window_win32* window)
{
    SwapBuffers(window->context);
    InvalidateRect(window->handle, 0, 0);
}
