// main.c

#include <stdio.h>
#include <stdlib.h>

#include "aren.h"
#include "platform.h"
#include "gl_extensions.h"
#include "window.h"
#include "keyboard_x11.h"

typedef struct
{
    GLuint vao;
    GLuint vbo;
} mesh_buffer;

typedef struct
{
    GLuint vertex;
    GLuint fragment;
    GLuint program;
} shader_program;

shader_program load_shader(char* vertex_source, int vertex_source_length, char* fragment_source, int fragment_source_length)
{
    shader_program shader;

    shader.vertex = glCreateShader(GL_VERTEX_SHADER);
    shader.fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(shader.vertex, 1, (const GLchar**)&vertex_source, &vertex_source_length);
    glShaderSource(shader.fragment, 1, (const GLchar**)&fragment_source, &fragment_source_length);

    int successful_compile = 0;
    
    glCompileShader(shader.vertex);
    glGetShaderiv(shader.vertex, GL_COMPILE_STATUS, &successful_compile);
    if(!successful_compile)
    {
	printf("failed to compile vertex shader:\n %s\n", vertex_source);
    }

    glCompileShader(shader.fragment);
    glGetShaderiv(shader.fragment, GL_COMPILE_STATUS, &successful_compile);
    if(!successful_compile)
    {
	printf("failed to compile fragment shader:\n %s\n", fragment_source);
    }

    shader.program = glCreateProgram();
    glAttachShader(shader.program, shader.vertex);
    glAttachShader(shader.program, shader.fragment);
    glLinkProgram(shader.program);

    int successful_link = 0;
    glGetProgramiv(shader.program, GL_LINK_STATUS, &successful_link);
    if(!successful_link)
    {
	GLsizei info_log_length;
	char info_log[1024];
	glGetProgramInfoLog(shader.program, 1024, &info_log_length, (GLchar*)info_log);
	printf("failed to link shader program:\n%s\n\n", info_log);
	printf("sources:\n%s\n\n%s\n\n", vertex_source, fragment_source);
    }

    return shader;
}

mesh_buffer load_triangle()
{
    mesh_buffer mesh;
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    mesh.vao = vao;

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    mesh.vbo = vbo;

    GLfloat vertices[] = {
	0.0f, 0.75f, 0.0f,
	-0.75f, -0.75f, 0.0f,
	0.75f, -0.75f, 0.0
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // TODO: Vertex attribute bind point hardcoded.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return mesh;
}

int read_file(char* path, char* destination, int destination_size)
{
    FILE* file = fopen(path, "rb");
    if(!file)
    {
	printf("can't open file %s\n", path);
	return 0;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);

    if(size >= destination_size)
    {
	size = (destination_size - 1);
    }
    
    fseek(file, 0, SEEK_SET);
    fread(destination, 1, size, file);
    fclose(file);

    destination[size] = 0;
    return size;
}



int handle_window_events(window_and_gl_context* window_context, keycode_map* keyboard)
{
    int keep_window_open = 1;

    while(XPending(window_context->display))
    {
	XEvent event;
	XNextEvent(window_context->display, &event);

	if(event.type == Expose)
	{
	    redraw_window(window_context);
	}
	else if(event.type == KeyPress)
	{
	    if(keycode_is_symbol(keyboard, event.xkey.keycode, XK_Escape))
	    {
		keep_window_open = 0;
	    }
	}
    }

    return keep_window_open;
}

int main(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    char exe_dir[1024];
    int exe_dir_length = 1024;
    
    exe_dir_length = platform_executable_directory(exe_dir, exe_dir_length);
    platform_set_working_directory(exe_dir);
    
    window_and_gl_context window_context = create_window_and_gl_context(500, 500, "aren");

    keycode_map keyboard = create_keycode_map(window_context.display);
    
    load_gl_functions();

    char vertex_source[1024];
    int vertex_source_length = 0;
    
    char fragment_source[1024];
    int fragment_source_length = 0;
    
    FILE* file = fopen("simple.vert", "rb");
    if(!file)
    {
	printf("can't open file simple.vert\n");
	exit(0);
    }

    vertex_source_length = read_file("simple.vert", vertex_source, vertex_source_length);
    fragment_source_length = read_file("simple.frag", fragment_source, fragment_source_length);

    shader_program shader = load_shader(vertex_source, vertex_source_length, fragment_source, fragment_source_length);
    mesh_buffer triangle_buffer = load_triangle();
    
    while(1)
    {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader.program);
	glBindVertexArray(triangle_buffer.vao);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer.vbo);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// TODO: handle_window_events is currently very misleading in name and usage
	// it only process one event and it doesn't return 0 when closed.
	int window_open = handle_window_events(&window_context, &keyboard);
	if(!window_open)
	{
	    break;
	}

	platform_sleep(1);
    }

    destroy_window_and_gl_context(&window_context);

    return 0;
}