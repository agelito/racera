#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include "platform/platform.h"

#include "rendering/renderer.h"
#include "rendering/heightmap.h"

typedef struct terrain_chunk {
    float position_x;
    float position_y;

    loaded_mesh     mesh;
    loaded_texture  texture;
    
    material        material;
} terrain_chunk;

typedef struct terrain {
    shader_program* shader;
    
    heightmap heightmap;

    float chunk_width;
    float chunk_height;
    
    int chunk_count;
    int chunk_capacity;

    terrain_chunk* chunks;

    float height_scale;
} terrain;

terrain
terrain_create(float width, float depth, float height_scale, texture_data heightmap_texture);

terrain_chunk
terrain_generate_chunk(terrain* terrain,
		       int heightmap_x, int heightmap_y,
		       int heightmap_w, int heightmap_h,
		       float position_x, float position_y);

void
terrain_destroy(terrain* terrain);

void
terrain_render(render_queue* queue, terrain* terrain);

#endif // TERRAIN_H_INCLUDED
