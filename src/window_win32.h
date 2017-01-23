#ifndef WINDOW_WIN32_H_INCLUDED
#define WINDOW_WIN32_H_INCLUDED



window_win32*
window_win32_create(int width, int height, char* title);

void
window_win32_destroy(window_win32* window);

void
window_win32_process_messages(window_win32* window);

void
window_win32_redraw(window_win32* window);

#endif // WINDOW_WIN32_H_INCLUDED
