// win32_main.c

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../platform.h"
#include "win32_window.h"

#include "../../racera.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    unused_arg(hInstance);
    unused_arg(hPrevInstance);
    unused_arg(lpCmdLine);
    unused_arg(nCmdShow);

    char exe_dir[1024];
    int exe_dir_length = 1024;
    
    exe_dir_length = platform_executable_directory(exe_dir, exe_dir_length);
    platform_set_working_directory(exe_dir);

    PROFILER_BEGIN("create window");
    win32_window* window = win32_window_create(500, 500, "racera");
    PROFILER_END();

    game_state racera_state = (game_state){0};

    while(window->is_open)
    {
        racera_state.frame_stats = profiler_process_frame();

        win32_window_process_messages(window);

        racera_state.screen_width = window->width;
        racera_state.screen_height = window->height;

        game_update_and_render(&racera_state);
        if(racera_state.should_quit)
        {
            break;
        }
    }

    win32_window_destroy(window);
    window = 0;

    return 0;

}


