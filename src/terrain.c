// terrain.c

#include "platform/platform.h"

#include "terrain.h"

#include <stdlib.h>

terrain
terrain_create(float width, float depth, float height_scale, texture_data heightmap_texture)
{
    terrain created_terrain = (terrain){0};

    created_terrain.shader = (shader_program*)malloc(sizeof(shader_program));
    *created_terrain.shader = load_shader("shaders/simple.vert", "shaders/textured.frag", 0);

    created_terrain.height_scale = height_scale;

    
    created_terrain.heightmap = heightmap_load_from_texture(heightmap_texture,
							    heightmap_texture.width,
							    heightmap_texture.height);

    // TODO: Parameterize the chunk size.
    float chunk_width = 1024.0f;
    float chunk_height = 1024.0f;
    
    created_terrain.chunk_width = chunk_width;
    created_terrain.chunk_height = chunk_height;
    
    int terrain_chunks_x = (int)(width / chunk_width);
    int terrain_chunks_y = (int)(depth / chunk_height);

    int chunk_count = (terrain_chunks_x * terrain_chunks_y);

    created_terrain.chunk_count = 0;
    created_terrain.chunk_capacity = chunk_count;
    
    int chunk_array_size = (sizeof(terrain_chunk) * chunk_count);
    created_terrain.chunks = (terrain_chunk*)malloc(chunk_array_size);

    int heightmap_size_x = (int)(created_terrain.heightmap.width / terrain_chunks_x);
    int heightmap_size_y = (int)(created_terrain.heightmap.height / terrain_chunks_y);

    platform_log("creating terrain\n");
    platform_log(" size:\t\t%.0fx%.0f\n",               width, depth);
    platform_log(" chunks:\t%.0fx%.0f, %d\n",	        chunk_width, chunk_height, chunk_count);
    platform_log(" heightmap:\t%dx%d\n",		created_terrain.heightmap.width,
		                                        created_terrain.heightmap.height);
    platform_log(" height scale:\t %.0f\n",             height_scale);

    float chunk_location_y = -depth * 0.5f;
    
    int heightmap_x = 0;
    int heightmap_y = 0;
    
    // TODO: Replace these for loops with on-demand chunk generation
    // based on camera position and view in world.
    int chunk_x, chunk_y;
    for(chunk_y = 0; chunk_y < terrain_chunks_y; ++chunk_y)
    {
	float chunk_location_x = -width * 0.5f;
	
	heightmap_x = 0;
	
	for(chunk_x = 0; chunk_x < terrain_chunks_x; ++chunk_x)
	{
	    platform_log("terrain generating chunk: %dx%d\n", chunk_x, chunk_y);
	    
	    *(created_terrain.chunks + created_terrain.chunk_count++) =
		terrain_generate_chunk(&created_terrain, heightmap_x, heightmap_y,
				       heightmap_size_x, heightmap_size_y,
				       chunk_location_x, chunk_location_y);

	    chunk_location_x += chunk_width;
	    heightmap_x += heightmap_size_x;
	}

	chunk_location_y += chunk_height;
	heightmap_y += heightmap_size_y;
    }

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

    int resolution = 256;
    mesh_data mesh_data = mesh_create_from_heightmap(terrain->heightmap,
						     terrain->chunk_width, terrain->chunk_height,
						     heightmap_x, heightmap_y,
						     heightmap_w, heightmap_h,
						     resolution, resolution,
	                                             terrain->height_scale);

    chunk.mesh = load_mesh(mesh_data, 0);
    mesh_data_free(&mesh_data);

    texture_data texture = (texture_data){0};
    texture.width  = 1024;
    texture.height = 1024;

    uint32 pixel_count = texture.width * texture.height;

    texture.components = 4;
    texture.colors = (uint8*)malloc(sizeof(uint8) * pixel_count * texture.components);

    uint8* write_pixel = texture.colors;
    
    int pixel_y, pixel_x;
    for(pixel_y = 0; pixel_y < texture.height; ++pixel_y)
    {
	for(pixel_x = 0; pixel_x < texture.width; ++pixel_x)
	{
	    float u = (float)pixel_x / texture.width;
	    float v = (float)pixel_y / texture.height;

	    float height = heightmap_sample_rect(&terrain->heightmap, u, v,
						 heightmap_x, heightmap_y,
						 heightmap_w, heightmap_h);

	    vector4 color;

	    if(height > 0.75f)
	    {
		color = vector4_create(1.0f, 1.0f, 1.0f, 1.0f);
	    }
	    else if(height > 0.5f)
	    {
		vector4 stone_light = vector4_create(0.66f, 0.66f, 0.66f, 1.0f);
		vector4 stone_dark = vector4_create(0.5f, 0.5f, 0.5f, 1.0f);

		float stone_t = 1.0f - ((height - 0.5f) / 0.25f);
		
		color = vector4_lerp(stone_light, stone_dark, stone_t);
	    }
	    else if(height > 0.12f)
	    {
		vector4 grass_light = vector4_create(0.15f, 0.95f, 0.2f, 1.0f);
		vector4 grass_deep = vector4_create(0.25f, 0.85f, 0.4f, 1.0f);

		float grass_t = 1.0f - ((height - 0.12f) / 0.38f);
		
		color = vector4_lerp(grass_light, grass_deep, grass_t);
	    }
	    else if(height > 0.1f)
	    {
		vector4 sand_light = vector4_create(0.75f, 0.69f, 0.5f, 1.0f);
		vector4 sand_dark = vector4_create(0.65f, 0.59f, 0.4f, 1.0f);

		float sand_t = 1.0f - ((height - 0.1f) / 0.02f);
		
		color = vector4_lerp(sand_light, sand_dark, sand_t);
	    }
	    else
	    {
		vector4 sea_light = vector4_create(0.1f, 0.4f, 0.6f, 1.0f);
		vector4 sea_deep = vector4_create(0.05f, 0.1f, 0.9f, 1.0f);

		float depth = 1.0f - (height / 0.1f);
		
		color = vector4_lerp(sea_light, sea_deep, depth);
	    }

	    write_pixel = texture_pack_rgba(write_pixel, color);
	}
    }

    chunk.texture = load_texture(texture);
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
