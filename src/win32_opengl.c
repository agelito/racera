// win32_opengl.c

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x1
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x2

typedef HGLRC (*wglCreateContextAttribsARB_proc)(HDC, HGLRC, const int*);

wglCreateContextAttribsARB_proc wglCreateContextAttribsARB;

void* 
gl_get_address(const GLubyte* function)
{
    return (void*)wglGetProcAddress(function);
}

HGLRC
gl_escalate_context(HDC device_context, HGLRC gl_context, int major, int minor)
{
    int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        0
    };

    wglCreateContextAttribsARB = gl_get_address("wglCreateContextAttribsARB");
    
    HGLRC escalated_context = wglCreateContextAttribsARB(device_context, 0, attributes);

    wglMakeCurrent(device_context, escalated_context);
    wglDeleteContext(gl_context);

    return escalated_context;
}
