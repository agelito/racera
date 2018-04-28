// heightmap.c

#include "heightmap.h"

heightmap
heightmap_create_from_texture(texture_data texture)
{
    heightmap created_heightmap = (heightmap){0};
    created_heightmap.width = texture.width;
    created_heightmap.height = texture.height;

    int heightmap_points_size = texture.width * texture.height * sizeof(heightmap_point);
    created_heightmap.points = (heightmap_point*)malloc(heightmap_points_size);

    int x, y;
    for(y = 0; y < texture.height; ++y)
    {
	for(x = 0; x < texture.width; ++x)
	{
	    float* height = (created_heightmap.heights + (x + y * texture.width));
	    uint8* texture_color = (texture.colors + (x + y * texture.width) * texture.components);

	    // TODO: Currently assuming texture is black and white. Decode height based
	    // on the color depth of the texture later when necessary.
	    
	    *height = (float)texture_color[0] / 255.0f;
	}
    }
}

void
heightmap_destroy(heightmap* heightmap)
{
    if(heightmap->heights != 0)
    {
	free(heightmap->heights);
	heightmap->heights = 0;
    }

    heightmap->width = 0;
    heightmap->height = 0;
}
