// terrain.c

#include "platform/platform.h"

#include "terrain.h"

#include <stdlib.h>

terrain
terrain_create(float width, float depth, float height_scale, float chunk_width, float chunk_depth,
	       texture_data heightmap_texture)
{
    terrain created_terrain = (terrain){0};

    created_terrain.shader = (shader_program*)malloc(sizeof(shader_program));
    *created_terrain.shader = load_shader("shaders/simple.vert", "shaders/textured.frag", 0);

    created_terrain.generate_detail_texture_shader = (shader_program*)malloc(sizeof(shader_program));
    *created_terrain.generate_detail_texture_shader =
	load_shader("shaders/simple.vert", "shaders/generate_terrain_detail.frag", 0);

    created_terrain.height_scale = height_scale;

    
    created_terrain.heightmap = heightmap_load_from_texture(heightmap_texture,
							    heightmap_texture.width,
							    heightmap_texture.height);
    created_terrain.heightmap_texture = heightmap_texture;

    created_terrain.chunk_width = chunk_width;
    created_terrain.chunk_depth = chunk_depth;
    
    int terrain_chunks_x = (int)(width / chunk_width);
    int terrain_chunks_y = (int)(depth / chunk_depth);

    int chunk_count = (terrain_chunks_x * terrain_chunks_y);

    created_terrain.chunk_count = 0;
    created_terrain.chunk_capacity = chunk_count;
    
    int chunk_array_size = (sizeof(terrain_chunk) * chunk_count);
    created_terrain.chunks = (terrain_chunk*)malloc(chunk_array_size);

    int heightmap_size_x = (int)(created_terrain.heightmap.width / terrain_chunks_x);
    int heightmap_size_y = (int)(created_terrain.heightmap.height / terrain_chunks_y);

    platform_log("creating terrain\n");
    platform_log(" size:\t\t%.0fx%.0f\n",               width, depth);
    platform_log(" chunks:\t%.0fx%.0f, %d\n",	        chunk_width, chunk_depth, chunk_count);
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

	chunk_location_y += chunk_depth;
	heightmap_y += heightmap_size_y;
    }

    return created_terrain;
}

internal void
terrain_chunk_setup_detail_texture(render_queue* queue, terrain_chunk* chunk)
{
    if(chunk->setup.state == terrain_chunk_setup_draw)
    {
	// TODO: Pushing the projection and view happens in the
	// middle of any frame. May need to restore the previous
	// projection and view matrix after the detail texture
	// has been rendered.

	matrix4 projection = matrix_orthographic((float)1.0f, (float)1.0f, 0.1f, 1.0f);
	
	renderer_queue_push_projection(queue, projection);

	vector3 eye = (vector3){{{0.0f, 0.0f, -0.5f}}};
	vector3 at = (vector3){{{0.0f, 0.0f, 0.5f}}};
	vector3 up = (vector3){{{0.0f, 1.0f, 0.0f}}};
	
	matrix4 view = matrix_look_at(eye, at, up);
	
	renderer_queue_push_view(queue, view);
	
	renderer_queue_push_target(queue, &chunk->setup.target);
	float clear_color[] = {1.0f, 0.0f, 0.0f, 1.0f};
	renderer_queue_push_clear(queue, GL_COLOR_BUFFER_BIT, clear_color);

	renderer_queue_push_draw(queue, &chunk->setup.quad, &chunk->setup.material, matrix_identity());
	renderer_queue_push_target(queue, 0);

	chunk->setup.state = terrain_chunk_setup_finalize;
    }
    else if(chunk->setup.state == terrain_chunk_setup_finalize)
    {
	render_target_destroy(&chunk->setup.target);
	unload_texture(&chunk->setup.heightmap);
	material_free(&chunk->setup.material);
	unload_mesh(&chunk->setup.quad);

	chunk->setup_is_finished = 1;
    }
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
						     terrain->chunk_width, terrain->chunk_depth,
						     heightmap_x, heightmap_y,
						     heightmap_w, heightmap_h,
						     resolution, resolution,
	                                             terrain->height_scale);

    chunk.mesh = load_mesh(mesh_data, 0);
    mesh_data_free(&mesh_data);

    
    chunk.material = material_create(terrain->shader, KB(1));

    { // NOTE: Init setup parameters.
	int texture_resolution = 1024;
	
	chunk.setup.heightmap = load_texture(terrain->heightmap_texture, 0);

	chunk.setup.material = material_create(terrain->generate_detail_texture_shader, KB(1));
	material_set_texture(&chunk.setup.material, "heightmap", &chunk.setup.heightmap);

	vector2 u_st = vector2_create((real32)heightmap_w / terrain->heightmap_texture.width,
				      (real32)heightmap_x / terrain->heightmap_texture.width);
	vector2 v_st = vector2_create((real32)heightmap_h / terrain->heightmap_texture.height,
				      (real32)heightmap_y / terrain->heightmap_texture.height);
	vector4 heightmap_st = vector4_create(u_st.x, u_st.y, v_st.x, v_st.y);
	material_set_vector(&chunk.setup.material, "heightmap_st", heightmap_st);
	
	// TODO: Reuse one render target for all generated chunks.
	chunk.setup.target = render_target_create(texture_resolution, texture_resolution, 0, 0);

	texture_data chunk_texture = (texture_data){0};
	chunk_texture.width = texture_resolution;
	chunk_texture.height = texture_resolution;
	chunk_texture.components = 4;
	
	chunk.texture = load_texture(chunk_texture, 0);
	material_set_texture(&chunk.material, "main_texture", &chunk.texture);

	render_target_attach_color(&chunk.setup.target, 0, &chunk.texture);

	// TODO: Reuse one quad for all generated chunks.
	chunk.setup.quad = load_mesh(mesh_create_quad(1.0f), 0);
	mesh_data_free(&chunk.setup.quad.data);
	
	chunk.setup.state = terrain_chunk_setup_draw;
    }

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
	if(chunk->setup_is_finished)
	{
	    matrix4 translation = matrix_translate(chunk->position_x, 0.0f, chunk->position_y);
	    renderer_queue_push_draw(queue, &chunk->mesh, &chunk->material, translation);
	}
	else
	{
	    terrain_chunk_setup_detail_texture(queue, chunk);
	}
    }
}
