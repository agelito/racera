#ifndef WINDOW_X11_H_INCLUDED
#define WINDOW_X11_H_INCLUDED

#include <X11/Xlib.h>

#include "keyboard_x11.h"
#include "mouse_x11.h"

typedef struct
{
    Display* display;
    Window window;
    GLXContext gl_context;
    
    int width;
    int height;
}  window_and_gl_context;

window_and_gl_context
create_window_and_gl_context(int width, int height, char* title);

void
destroy_gl_context(window_and_gl_context* window_context);

void
destroy_window(window_and_gl_context* window_context);

void
resize_viewport(window_and_gl_context* window_context);

void
redraw_window(window_and_gl_context* window_context);

int
handle_window_events(window_and_gl_context* window_context, keyboard_input* keyboard, mouse_input* mouse);

#endif // WINDOW_X11_H_INCLUDED