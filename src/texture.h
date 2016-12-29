#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

typedef struct
{
    int width;
    int height;
    char* colors;
} texture_data;

typedef struct
{
    GLuint handle;
    texture_data data;
} loaded_texture;

loaded_texture load_texture(gl_functions* gl, texture_data data);

texture_data texture_create_checker(int width, int height);

void texture_data_free(texture_data* data);

#endif // TEXTURE_H_INCLUDED