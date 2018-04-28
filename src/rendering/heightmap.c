// heightmap.c

#include "heightmap.h"

// TODO: For malloc.
#include <stdlib.h>

heightmap
heightmap_load_from_texture(texture_data texture, int width, int height, float height_scale)
{
    heightmap result = (heightmap){0};

    result.width = width;
    result.height = height;

    result.height_data = (float*)malloc(sizeof(float) * width * height);

    int x, y;
    float u, v;
    for(y = 0; y < height; ++y)
    {
	float* height_data = (result.height_data + y * width);

	v = (float)y / height;
	
	for(x = 0; x < width; ++x)
	{
	    u = (float)x / width;
	    
	    vector4 color = texture_bilinear_sample(u, v, texture);
	    float height = color.x * height_scale;

	    *height_data++ = height;
	}
    }

    return result;
}

void
heightmap_free(heightmap* heightmap)
{
    if(heightmap->height_data)
    {
	free(heightmap->height_data);
	heightmap->height_data = 0;
    }
}

float heightmap_get(heightmap* heightmap, int x, int y)
{
    int clamped_x = x;
    if(clamped_x >= heightmap->width)
	clamped_x = heightmap->width - 1;
    else if(clamped_x < 0)
	clamped_x = 0;

    int clamped_y = y;
    if(clamped_y >= heightmap->height)
	clamped_y = heightmap->height - 1;
    else if(clamped_y < 0)
	clamped_y = 0;

    return heightmap->height_data[clamped_x + clamped_y * heightmap->width];
}

float heightmap_sample(heightmap* heightmap, float x, float y)
{
    x = (x * (heightmap->width - 1));
    y = (y * (heightmap->height - 1));

    float fraction_x = x - (int)x;
    float fraction_y = y - (int)y;

    float one_minus_fx = 1.0f - fraction_x;
    float one_minus_fy = 1.0f - fraction_y;

    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // TODO: Inline fetching, without need for clamping.
    float heightx0y0 = heightmap_get(heightmap, x0, y0);
    float heightx1y0 = heightmap_get(heightmap, x1, y0);
    float heightx0y1 = heightmap_get(heightmap, x0, y1);
    float heightx1y1 = heightmap_get(heightmap, x1, y1);

    float heighta = ((heightx0y0 * one_minus_fx) + (heightx1y0 * fraction_x));
    float heightb = ((heightx0y1 * one_minus_fx) + (heightx1y1 * fraction_x));

    return ((heighta * one_minus_fy) + (heightb * fraction_y));
}
