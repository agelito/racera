#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include "../platform/opengl.h"

#include "../math.h"

typedef struct
{
    int width;
    int height;
    uint8 components;
    uint8* colors;
} texture_data;

typedef struct
{
    GLuint handle;
    texture_data data;
} loaded_texture;

loaded_texture load_texture(texture_data data, int mipmap);
loaded_texture load_texture_depth(int width, int height);
void unload_texture(loaded_texture* texture);

texture_data texture_create_checker(int width, int height, int checker_size);
texture_data texture_create_from_tga(char* path);

vector4 texture_unpack_bw(uint8* color);
vector4 texture_unpack_rgb(uint8* color);
vector4 texture_unpack_rgba(uint8* color);

uint8* texture_pack_rgba(uint8* pixel, vector4 rgba);

vector4 texture_bilinear_sample(float tx, float ty, texture_data texture);

void texture_data_free(texture_data* data);

#endif // TEXTURE_H_INCLUDED
