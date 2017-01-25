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

typedef struct win32_keyboard win32_keyboard;

struct win32_keyboard
{
    int pressed[256];
    int released[256];
    int down[256];
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
    win32_keyboard keyboard;
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
    win32_keyboard* keyboard = &global_win32.keyboard;

    while(window->is_open)
    {
        window_process_messages(window);

        racera_state.screen_width = window->width;
        racera_state.screen_height = window->height;

        racera_state.mouse.absolute_x = global_win32.mouse.absolute_x;
        racera_state.mouse.absolute_y = global_win32.mouse.absolute_y;
        racera_state.mouse.relative_x = global_win32.mouse.relative_x;
        racera_state.mouse.relative_y = global_win32.mouse.relative_y;

        *(racera_state.keyboard.pressed + VKEY_W) = keyboard->pressed[0x57];
        *(racera_state.keyboard.pressed + VKEY_A) = keyboard->pressed[0x41];
        *(racera_state.keyboard.pressed + VKEY_S) = keyboard->pressed[0x53];
        *(racera_state.keyboard.pressed + VKEY_D) = keyboard->pressed[0x44];
        *(racera_state.keyboard.pressed + VKEY_Q) = keyboard->pressed[0x51];
        *(racera_state.keyboard.pressed + VKEY_Z) = keyboard->pressed[0x5A];

        *(racera_state.keyboard.released + VKEY_W) = keyboard->released[0x57];
        *(racera_state.keyboard.released + VKEY_A) = keyboard->released[0x41];
        *(racera_state.keyboard.released + VKEY_S) = keyboard->released[0x53];
        *(racera_state.keyboard.released + VKEY_D) = keyboard->released[0x44];
        *(racera_state.keyboard.released + VKEY_Q) = keyboard->released[0x51];
        *(racera_state.keyboard.released + VKEY_Z) = keyboard->released[0x5A];

        *(racera_state.keyboard.down + VKEY_W) = keyboard->down[0x57];
        *(racera_state.keyboard.down + VKEY_A) = keyboard->down[0x41];
        *(racera_state.keyboard.down + VKEY_S) = keyboard->down[0x53];
        *(racera_state.keyboard.down + VKEY_D) = keyboard->down[0x44];
        *(racera_state.keyboard.down + VKEY_Q) = keyboard->down[0x51];
        *(racera_state.keyboard.down + VKEY_Z) = keyboard->down[0x5A];

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

        int key;
        for(key = 0; key < 256; ++key)
        {
            global_win32.keyboard.pressed[key] = 0;
            global_win32.keyboard.released[key] = 0;
        }
    }


    window_destroy(window);
    window = 0;

    return 0;
}

