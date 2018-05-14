#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include "../platform/opengl.h"

#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "material.h"
#include "render_target.h"

typedef struct render_queue render_queue;

struct render_queue
{
    uint32 queue_used;
    uint32 queue_capacity;
    uint8* queue_items;

    shader_uniform_group uniforms;
    shader_uniform_group uniforms_per_object;

    int render_width;
    int render_height;
};

void
renderer_apply_uniforms(shader_program* shader, shader_uniform_group* group);

render_queue
renderer_queue_create(uint32 capacity);

void
renderer_queue_push_projection(render_queue* queue, matrix4 projection);

void
renderer_queue_push_view(render_queue* queue, matrix4 view);

void
renderer_queue_push_clear(render_queue* queue, uint32 clear_flags, float clear_color[4]);

void
renderer_queue_push_draw(render_queue* queue, loaded_mesh* mesh,
			 material* material, matrix4 transform);

void
renderer_queue_push_draw_elements(render_queue* queue, loaded_mesh* mesh,
				  material* material, matrix4 transform,
				  uint32 element_offset, uint32 element_count);

void
renderer_queue_push_target(render_queue* queue, render_target* target);

void
renderer_queue_clear(render_queue* queue);

void
renderer_queue_process(render_queue* queue);

void
renderer_queue_set_render_size(render_queue* queue, int width, int height);

void
renderer_queue_set_projection(render_queue* queue, matrix4 projection);

void
renderer_queue_set_view(render_queue* queue, matrix4 view);

void
renderer_check_error();

#endif // RENDERER_H_INCLUDED
