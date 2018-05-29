#ifndef HEIGHTMAP_H_INCLUDED
#define HEIGHTMAP_H_INCLUDED

#include "platform/platform.h"

#include "rendering/texture.h"

typedef struct heightmap
{
    int width;
    int height;

    float* height_data;
} heightmap;

heightmap
heightmap_load_from_texture(texture_data texture, int width, int height);

void
heightmap_free(heightmap* heightmap);

float
heightmap_get(heightmap* heightmap, int x, int y);

float
heightmap_sample(heightmap* heightmap, float x, float y);

float
heightmap_sample_rect(heightmap* heightmap, float u, float v,
		      int heightmap_x, int heightmap_y,
		      int heightmap_w,  int heightmap_h);

#endif // HEIGHTMAP_H_INCLUDED
