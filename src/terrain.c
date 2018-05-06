// terrain.c

#include "platform/platform.h"

#include "terrain.h"

#include <stdlib.h>

terrain
terrain_create(float width, float height, texture_data heightmap_texture)
{
    terrain created_terrain = (terrain){0};

    created_terrain.shader = (shader_program*)malloc(sizeof(shader_program));
    *created_terrain.shader = load_shader("shaders/simple.vert", "shaders/textured.frag", 0);
    
    // TODO: Parameterize the mountain size.
    float mountain_size = 100.0f;
    
    created_terrain.heightmap = heightmap_load_from_texture(heightmap_texture,
							    heightmap_texture.width,
							    heightmap_texture.height,
							    mountain_size);

    // TODO: Parameterize the chunk size.
    float chunk_width = 256.0f;
    float chunk_height = 256.0f;
    
    created_terrain.chunk_width = chunk_width;
    created_terrain.chunk_height = chunk_height;
    
    int terrain_chunks_x = (int)(width / chunk_width) + 1;
    int terrain_chunks_y = (int)(height / chunk_height) + 1;

    int chunk_count = (terrain_chunks_x * terrain_chunks_y);

    created_terrain.chunk_count = 0;
    created_terrain.chunk_capacity = chunk_count;
    
    int chunk_array_size = (sizeof(terrain_chunk) * chunk_count);
    created_terrain.chunks = (terrain_chunk*)malloc(chunk_array_size);

    *(created_terrain.chunks + created_terrain.chunk_count++) =
	terrain_generate_chunk(&created_terrain, 0, 0, 256, 256, 0.0f, 0.0f);

    return created_terrain;
}

terrain_chunk
terrain_generate_chunk(terrain* terrain,
		       int heightmap_x, int heightmap_y,
		       int heightmap_w, int heightmap_h,
		       float position_x, float position_y)
{
    terrain_chunk chunk = (terrain_chunk){0};
    
    chunk.position_x = position_x;
    chunk.position_y = position_y;

    int resolution = 16;
    mesh_data mesh_data = mesh_create_from_heightmap(terrain->heightmap,
						     terrain->chunk_width, terrain->chunk_height,
						     heightmap_x, heightmap_y,
						     heightmap_w, heightmap_h,
						     resolution, resolution);

    chunk.mesh = load_mesh(mesh_data, 0);
    mesh_data_free(&mesh_data);

    chunk.texture = load_texture(texture_create_checker(256, 256, 64));
    texture_data_free(&chunk.texture.data);
    
    chunk.material = material_create(terrain->shader, KB(1));
    material_set_texture(&chunk.material, "main_texture", &chunk.texture);

    return chunk;
}

void
terrain_destroy(terrain* terrain)
{
    // TODO: Unload meshes.
    // TODO: Unload textures.
    // TODO: Unload materials.

    heightmap_free(&terrain->heightmap);
    
    free(terrain->chunks);
    terrain->chunks = 0;

    terrain->chunk_capacity = 0;
    terrain->chunk_count = 0;
}

void
terrain_render(render_queue* queue, terrain* terrain)
{
    int i;
    for_range(i, terrain->chunk_count)
    {
	terrain_chunk* chunk = (terrain->chunks + i);

	matrix4 translation = matrix_translate(chunk->position_x, 0.0f, chunk->position_y);
	renderer_queue_push_draw(queue, &chunk->mesh, &chunk->material, translation);
    }
}
