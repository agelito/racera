// win32_main.c

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "racera.h"

#include "win32_window.c"

extern void game_update_and_render(struct game_state* state);

int main(int argc, char* argv[])
{
    window_win32* window = window_create(500, 500, "racera");

    game_state racera_state = (game_state){0};

    while(window->is_open)
    {
        window_process_messages(window);

        racera_state.screen_width = window->width;
        racera_state.screen_height = window->height;

        game_update_and_render(&racera_state);
        if(racera_state.should_quit)
        {
            window_destroy(window);
            break;
        }

        window_redraw(window);

        platform_sleep(1);
    }

    window_destroy(window);
    window = 0;

    return 0;

}

#include "keyboard.h"

int 
keyboard_is_down(keyboard_state* keyboard, virtual_key key)
{
    return 0;
}

int 
keyboard_is_pressed(keyboard_state* keyboard, virtual_key key)
{
    return 0;
}

int 
keyboard_is_released(keyboard_state* keyboard, virtual_key key)
{
    return 0;
}
