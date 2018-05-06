#ifndef OPENGL_H_INCLUDED
#define OPENGL_H_INCLUDED

#include <GL/gl.h>
#include <GL/glext.h>

#define GL_PROC_DEF(function) GL_PROC_##function function
#define GL_PROC_ADDR(function) gl_get_address((const GLubyte*)#function)

typedef struct
{
    PFNGLGENVERTEXARRAYSPROC            glGenVertexArrays;
    PFNGLBINDVERTEXARRAYPROC		glBindVertexArray;
    PFNGLGENBUFFERSPROC			glGenBuffers;
    PFNGLDELETEBUFFERSPROC		glDeleteBuffers;
    PFNGLBINDBUFFERPROC			glBindBuffer;
    PFNGLBUFFERDATAPROC			glBufferData;
    PFNGLBUFFERSUBDATAPROC		glBufferSubData;
    PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC	glVertexAttribPointer;
    PFNGLCREATESHADERPROC		glCreateShader;
    PFNGLSHADERSOURCEPROC		glShaderSource;
    PFNGLCOMPILESHADERPROC		glCompileShader;
    PFNGLGETSHADERIVPROC		glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC		glGetShaderInfoLog;
    PFNGLCREATEPROGRAMPROC		glCreateProgram;
    PFNGLATTACHSHADERPROC		glAttachShader;
    PFNGLLINKPROGRAMPROC		glLinkProgram;
    PFNGLGETPROGRAMIVPROC		glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC		glGetProgramInfoLog;
    PFNGLUSEPROGRAMPROC			glUseProgram;
    PFNGLGETUNIFORMLOCATIONPROC		glGetUniformLocation;
    PFNGLGETACTIVEUNIFORMPROC		glGetActiveUniform;
    PFNGLGETATTRIBLOCATIONPROC		glGetAttribLocation;

    PFNGLUNIFORMMATRIX2FVPROC	glUniformMatrix2fv;
    PFNGLUNIFORMMATRIX3FVPROC	glUniformMatrix3fv;
    PFNGLUNIFORMMATRIX4FVPROC	glUniformMatrix4fv;
    
    PFNGLUNIFORM1FVPROC glUniform1fv;
    PFNGLUNIFORM2FVPROC glUniform2fv;
    PFNGLUNIFORM3FVPROC glUniform3fv;
    PFNGLUNIFORM4FVPROC glUniform4fv;

    PFNGLUNIFORM1IVPROC glUniform1iv;
    PFNGLUNIFORM2IVPROC glUniform2iv;
    PFNGLUNIFORM3IVPROC glUniform3iv;
    PFNGLUNIFORM4IVPROC glUniform4iv;    
    
    PFNGLGENERATEMIPMAPPROC	glGenerateMipmap;
} gl_functions;

void
opengl_initialize();

void*
gl_get_address(const GLubyte* function);

gl_functions
opengl_load_functions();

void
opengl_trace(char* function, char* gl_function, char* file, int line);

void
opengl_check_error();

extern gl_functions global_gl;

#ifdef OPENGL_DEBUG
#define GL_CALL(function, ...) global_gl.function(__VA_ARGS__);		\
    opengl_trace((char*)__FUNCTION__, #function, __FILE__, __LINE__);	\
    opengl_check_error(#function, __FILE__, __LINE__)

#else // !OPENGL_DEBUG
#define GL_CALL(function, ...) global_gl.function(__VA_ARGS__)
#endif // OPENGL_DEBUG

#endif // OPENGL_H_INCLUDED
