#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <X11/Xlib.h>

typedef struct
{
    Display* display;
    Window window;
    GLXContext gl_context;
}  window_and_gl_context;

window_and_gl_context
create_window_and_gl_context(int width, int height, char* title);

void
destroy_gl_context(window_and_gl_context* window_context);

void
destroy_window(window_and_gl_context* window_context);

void
redraw_window(window_and_gl_context* window_context);

#endif // WINDOW_H_INCLUDED
