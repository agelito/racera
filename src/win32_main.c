// win32_main.c

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "platform.h"
#include "racera.h"

#include "window_win32.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    window_win32* window = window_win32_create(500, 500, "racera");

    game_state racera_state = (game_state){0};

    while(window->is_open)
    {
        window_win32_process_messages(window);

        racera_state.screen_width = window->width;
        racera_state.screen_height = window->height;

        game_update_and_render(&racera_state);
        if(racera_state.should_quit)
        {
            destroy_window(window);
            break;
        }

        redraw_window(window);

        platform_sleep(1);
    }

    window_win32_destroy(window);
    window = 0;

    return 0;

}


