// opengl_wgl.c

#include "opengl.h"
#include <wingdi.h>

void* gl_get_address(const GLubyte* function)
{
    return (void*)wglGetProcAddress(function);
}