#ifndef HEIGHTMAP_H_INCLUDED
#define HEIGHTMAP_H_INCLUDED

typedef struct heightmap heightmap;

struct heightmap
{
    int width;
    int height;
    float* heights;
};

heightmap heightmap_create_from_texture(texture_data texture);

void heightmap_destroy(heightmap* heightmap);

#endif // HEIGHTMAP_H_INCLUDED
