// win32_main.c

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>
#include <wingdi.h>

#include <stdio.h>
#include <stdlib.h>

#include "opengl.h"
#include "platform.h"
#include "racera.h"

typedef struct win32_mouse win32_mouse;

struct win32_mouse
{
    int absolute_x;
    int absolute_y;
    int relative_x;
    int relative_y;
};

typedef struct window_win32 window_win32;

struct window_win32
{
    HDC context;
    HWND handle;
    HGLRC gl_context;
    int is_open;
    int width;
    int height;
};

typedef struct win32_platform win32_platform;

struct win32_platform
{
    window_win32 window;
    win32_mouse mouse;
};

static win32_platform global_win32;

#include "win32_platform.c"
#include "win32_window.c"
#include "win32_opengl.c"

extern void game_update_and_render(struct game_state* state);

int main(int argc, char* argv[])
{
    global_win32.window = window_create(500, 500, "racera");

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    game_state racera_state = (game_state){0};

    window_win32* window = &global_win32.window;

    while(window->is_open)
    {
        window_process_messages(window);

        racera_state.screen_width = window->width;
        racera_state.screen_height = window->height;

        racera_state.mouse.absolute_x = global_win32.mouse.absolute_x;
        racera_state.mouse.absolute_y = global_win32.mouse.absolute_y;
        racera_state.mouse.relative_x = global_win32.mouse.relative_x;
        racera_state.mouse.relative_y = global_win32.mouse.relative_y;

        game_update_and_render(&racera_state);
        if(racera_state.should_quit)
        {
            window_destroy(window);
            break;
        }

        window_redraw(window);
        platform_sleep(1);

        global_win32.mouse.relative_x = 0;
        global_win32.mouse.relative_y = 0;
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
