#ifndef HEIGHTMAP_H_INCLUDED
#define HEIGHTMAP_H_INCLUDED

#include "../platform/platform.h"

#include "texture.h"

typedef struct heightmap
{
    int width;
    int height;

    float* height_data;
} heightmap;

heightmap
heightmap_load_from_texture(texture_data texture, int width, int height, float height_scale);

void
heightmap_free(heightmap* heightmap);

float
heightmap_get(heightmap* heightmap, int x, int y);

float
heightmap_sample(heightmap* heightmap, float x, float y);

#endif // HEIGHTMAP_H_INCLUDED
