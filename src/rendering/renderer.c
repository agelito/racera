// renderer.c

#include "renderer.h"

#include "mesh.c"
#include "shader.c"
#include "texture.c"
#include "material.c"

#define glUniformScalar(type, u, c, d) GL_CALL(type, u->location, c, (void*)d)
#define glUniformMatrix(type, u, c, d) GL_CALL(type, u->location, c, GL_FALSE, (void*)d)

typedef enum render_queue_type render_queue_type;

typedef struct render_queue_item render_queue_item;
typedef struct render_queue_clear render_queue_clear;
typedef struct render_queue_draw render_queue_draw;
typedef struct render_queue_target render_queue_target;
typedef struct render_queue_matrix render_queue_matrix;

enum render_queue_type
{
    render_queue_type_clear,
    render_queue_type_draw,
    render_queue_type_target,
    render_queue_type_projection,
    render_queue_type_view,
};

struct render_queue_item
{
    render_queue_type type;
    uint16 command_size;
};

struct render_queue_clear
{
    float clear_color[4];
    uint32 clear_flags;
};

struct render_queue_draw
{
    loaded_mesh* mesh;
    material* material;
    matrix4 transform;

    uint32 draw_element_offset;
    uint32 draw_element_count;
};

struct render_queue_target
{
    render_target* target;
};

struct render_queue_matrix
{
    matrix4 matrix;
};
    

static void
renderer_upload_uniform(shader_uniform* uniform, int count, unsigned char* data)
{
    switch(uniform->type)
    {
    case shader_data_float1:
	glUniformScalar(glUniform1fv, uniform, count, data);
	break;
    case shader_data_float2:
	glUniformScalar(glUniform2fv, uniform, count, data);
	break;
    case shader_data_float3:
	glUniformScalar(glUniform3fv, uniform, count, data);
	break;
    case shader_data_float4:
	glUniformScalar(glUniform4fv, uniform, count, data);
	break;
    case shader_data_integer1:
	glUniformScalar(glUniform1iv, uniform, count, data);
	break;
    case shader_data_integer2:
	glUniformScalar(glUniform2iv, uniform, count, data);
	break;
    case shader_data_integer3:
	glUniformScalar(glUniform3iv, uniform, count, data);
	break;
    case shader_data_integer4:
	glUniformScalar(glUniform4iv, uniform, count, data);
	break;
    case shader_data_matrix2:
	glUniformMatrix(glUniformMatrix2fv, uniform, count, data);
	break;
    case shader_data_matrix3:
	glUniformMatrix(glUniformMatrix3fv, uniform, count, data);
	break;
    case shader_data_matrix4:
	glUniformMatrix(glUniformMatrix4fv, uniform, count, data);
	break;
    case shader_data_sampler2d:
	glUniformScalar(glUniform1iv, uniform, count, data);
	break;
    default: break;
    }
}

void
renderer_apply_uniforms(shader_program* shader, shader_uniform_group* group)
{
    shader_reflection* info = &shader->info;

    int uniform_slot;
    for(uniform_slot = 0; uniform_slot < info->uniform_count; ++uniform_slot)
    {
	shader_uniform* uniform = (info->uniforms + uniform_slot);
	shader_uniform_data data = shader_uniform_get_data(group, uniform->name_hash);
	if(!data.size) continue;

	int count = data.size / uniform->size_per_element;
	renderer_upload_uniform(uniform, count, data.data);
    }
}

static input_layout*
mesh_layout_for_shader(loaded_mesh* mesh, shader_program* shader)
{
    input_layout* layout = 0;

    int shader_attributes = 0;
    int position_location = GL_CALL(glGetAttribLocation, shader->program, "in_vertex");
    int color_location	  = GL_CALL(glGetAttribLocation, shader->program, "in_color");
    int texcoord_location = GL_CALL(glGetAttribLocation, shader->program, "in_uv");
    int normal_location	  = GL_CALL(glGetAttribLocation, shader->program, "in_normal");
    int tangent_location  = GL_CALL(glGetAttribLocation, shader->program, "in_tangent");
    int binormal_location = GL_CALL(glGetAttribLocation, shader->program, "in_binormal");

    if(position_location != -1) VA_INCLUDE(shader_attributes, VA_POSITIONS_BIT);
    if(normal_location != -1)   VA_INCLUDE(shader_attributes, VA_NORMALS_BIT);
    if(texcoord_location != -1) VA_INCLUDE(shader_attributes, VA_TEXCOORDS_BIT);
    if(color_location != -1)    VA_INCLUDE(shader_attributes, VA_COLORS_BIT);
    if(tangent_location != -1)  VA_INCLUDE(shader_attributes, VA_TANGENTS_BIT);
    if(binormal_location != -1) VA_INCLUDE(shader_attributes, VA_BINORMALS_BIT);

    uint32 attribute_mask = (shader_attributes & mesh->attribute_mask);
    
    uint32 i;
    for_range(i, mesh->layout_count)
    {
	input_layout* check = (mesh->layouts + i);
	if(check->attribute_mask == attribute_mask)
	{
	    layout = check;
	    break;
	}
    }

    if(!layout)
    {
	assert(mesh->layout_count < MESH_MAX_INPUT_LAYOUTS);
	layout = (mesh->layouts + mesh->layout_count++);
	layout->attribute_mask = attribute_mask;

	GL_CALL(glGenVertexArrays, 1, &layout->vertex_array);
	GL_CALL(glBindVertexArray, layout->vertex_array);

	if(mesh->index_buffer)
	{
	    GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
	}

	if(VA_ISSET(attribute_mask, VA_POSITIONS_BIT))
	{
	    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *(mesh->vertex_buffer + vertex_attributes_positions));
	    GL_CALL(glEnableVertexAttribArray, position_location);
	    GL_CALL(glVertexAttribPointer, position_location, 3, GL_FLOAT, GL_FALSE, sizeof(vector3), 0);
	}

	if(VA_ISSET(attribute_mask, VA_NORMALS_BIT))
	{
	    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *(mesh->vertex_buffer + vertex_attributes_normals));
	    GL_CALL(glEnableVertexAttribArray, normal_location);
	    GL_CALL(glVertexAttribPointer, normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(vector3), 0);
	}

	if(VA_ISSET(attribute_mask, VA_TEXCOORDS_BIT))
	{
	    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *(mesh->vertex_buffer + vertex_attributes_texcoords));
	    GL_CALL(glEnableVertexAttribArray, texcoord_location);
	    GL_CALL(glVertexAttribPointer, texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(vector2), 0);
	}

	if(VA_ISSET(attribute_mask, VA_COLORS_BIT))
	{
	    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *(mesh->vertex_buffer + vertex_attributes_colors));
	    GL_CALL(glEnableVertexAttribArray, color_location);
	    GL_CALL(glVertexAttribPointer, color_location, 3, GL_FLOAT, GL_FALSE, sizeof(color), 0);
	}

	if(VA_ISSET(attribute_mask, VA_TANGENTS_BIT))
	{
	    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *(mesh->vertex_buffer + vertex_attributes_tangents));
	    GL_CALL(glEnableVertexAttribArray, tangent_location);
	    GL_CALL(glVertexAttribPointer, tangent_location, 3, GL_FLOAT, GL_FALSE, sizeof(vector3), 0);
	}

	if(VA_ISSET(attribute_mask, VA_BINORMALS_BIT))
	{
	    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *(mesh->vertex_buffer + vertex_attributes_normals));
	    GL_CALL(glEnableVertexAttribArray, binormal_location);
	    GL_CALL(glVertexAttribPointer, binormal_location, 3, GL_FLOAT, GL_FALSE, sizeof(vector3), 0);
	}
    }

    return layout;
}

static void
renderer_bind_mesh_buffers(loaded_mesh* mesh, shader_program* shader)
{
    input_layout* layout = mesh_layout_for_shader(mesh, shader);
    if(layout)
    {
	GL_CALL(glBindVertexArray, layout->vertex_array);
    }
}

render_queue
renderer_queue_create(uint32 capacity)
{
    render_queue queue = (render_queue){0};
    
    queue.queue_used	 = 0;
    queue.queue_capacity = capacity;

    queue.queue_items = malloc(capacity);

    queue.uniforms	      = shader_uniform_group_create(KB(1));
    queue.uniforms_per_object = shader_uniform_group_create(KB(1));

    return queue;
}

void
renderer_queue_set_render_size(render_queue* queue, int width, int height)
{
    queue->render_width = width;
    queue->render_height = height;
}

static inline void
renderer_queue_push_item(render_queue* queue, render_queue_type type,
			 void* command_data, uint16 command_size)
{
    uint32 push_size = sizeof(render_queue_item) + command_size;
    if((queue->queue_used + push_size) > queue->queue_capacity)
    {
	platform_log("render queue capacity full (%d/%d)\n",
		     queue->queue_used + push_size,
		     queue->queue_capacity);
	return;
    }
    
    render_queue_item* item = (render_queue_item*)(queue->queue_items + queue->queue_used);
    
    item->type         = type;
    item->command_size = command_size;

    uint32 push_data_offset = (queue->queue_used + sizeof(render_queue_item));
    
    platform_copy_memory((queue->queue_items + push_data_offset), command_data, command_size);

    queue->queue_used += push_size;
}

void
renderer_queue_push_projection(render_queue* queue, matrix4 projection)
{
    render_queue_matrix matrix;
    matrix.matrix = projection;

    renderer_queue_push_item(queue, render_queue_type_projection,
			     &matrix, sizeof(render_queue_matrix));
}

void
renderer_queue_push_view(render_queue* queue, matrix4 view)
{
    render_queue_matrix matrix;
    matrix.matrix = view;

    renderer_queue_push_item(queue, render_queue_type_view,
			   &matrix, sizeof(render_queue_matrix));
}

void
renderer_queue_push_clear(render_queue* queue, uint32 clear_flags, float clear_color[4])
{
    render_queue_clear clear;
    
    clear.clear_flags = clear_flags;
    
    clear.clear_color[0] = clear_color[0];
    clear.clear_color[1] = clear_color[1];
    clear.clear_color[2] = clear_color[2];
    clear.clear_color[3] = clear_color[3];

    renderer_queue_push_item(queue, render_queue_type_clear, &clear, sizeof(render_queue_clear));
}

void
renderer_queue_push_draw(render_queue* queue, loaded_mesh* mesh, material* material, matrix4 transform)
{
    render_queue_draw draw;
    draw.mesh = mesh;
    draw.material = material;
    draw.transform = transform;

    if(mesh->data.index_count)
    {
	draw.draw_element_offset = 0;
	draw.draw_element_count = mesh->data.index_count;
    }
    else
    {
	draw.draw_element_offset = 0;
	draw.draw_element_count = mesh->data.vertex_count;
    }
    
    renderer_queue_push_item(queue, render_queue_type_draw, &draw, sizeof(render_queue_draw));
}

void
renderer_queue_push_draw_elements(render_queue* queue, loaded_mesh* mesh,
				  material* material, matrix4 transform,
				  uint32 element_offset, uint32 element_count)
{
    render_queue_draw draw;
    draw.mesh	   = mesh;
    draw.material  = material;
    draw.transform = transform;

    draw.draw_element_offset = element_offset;
    draw.draw_element_count  = element_count;
    
    renderer_queue_push_item(queue, render_queue_type_draw, &draw, sizeof(render_queue_draw));
}

void
renderer_queue_push_target(render_queue* queue, render_target* target)
{
    render_queue_target set_target;
    set_target.target = target;
    
    renderer_queue_push_item(queue, render_queue_type_target,
			     &set_target, sizeof(render_queue_target));
}

void
renderer_queue_clear(render_queue* queue)
{
    queue->queue_used = 0;
}

static void
configure_for_transparency(bool32 transparency_enabled)
{
    if(transparency_enabled)
    {
	glDisable(GL_DEPTH_TEST);
		
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
		
	glDisable(GL_BLEND);
    }
}

void
renderer_queue_process(render_queue* queue)
{
    loaded_mesh* bound_mesh = 0;
    material* bound_material = 0;

    int rebind_uniforms = 0;

    uint32 queue_processed = 0;
    while(queue_processed < queue->queue_used)
    {
	uint8* queue_head = (queue->queue_items + queue_processed);
	render_queue_item* item = (render_queue_item*)queue_head;
	switch(item->type)
	{
	case render_queue_type_projection: {
	    render_queue_matrix* matrix =
		(render_queue_matrix*)(queue_head + sizeof(render_queue_item));

	    renderer_queue_set_projection(queue, matrix->matrix);

	    rebind_uniforms = 1;
	} break;
	case render_queue_type_view: {
	    render_queue_matrix* matrix =
		(render_queue_matrix*)(queue_head + sizeof(render_queue_item));

	    renderer_queue_set_view(queue, matrix->matrix);

	    rebind_uniforms = 1;
	} break;
	case render_queue_type_clear: {
	    render_queue_clear* clear =
		(render_queue_clear*)(queue_head + sizeof(render_queue_item));

	    glClearColor(clear->clear_color[0], clear->clear_color[1],
			 clear->clear_color[2], clear->clear_color[3]);
	    glClear(clear->clear_flags);

	} break;
	case render_queue_type_draw: {
	    render_queue_draw* draw =
		(render_queue_draw*)(queue_head + sizeof(render_queue_item));

	    int rebind_mesh = 0;
	    
	    material* material = draw->material;
	    shader_program* shader = material->shader;
	    if(material != bound_material)
	    {
		GL_CALL(glUseProgram, shader->program);
	    
		configure_for_transparency(shader->transparent);
	 
		rebind_uniforms = 1;

		material_apply(material, &queue->uniforms_per_object);
	    
		bound_material = material;
		rebind_mesh = 1;
	    }

	    if(rebind_uniforms)
	    {
		renderer_apply_uniforms(shader, &queue->uniforms);
		rebind_uniforms = 0;
	    }
	
	    loaded_mesh* mesh = draw->mesh;
	    if(mesh != bound_mesh || rebind_mesh)
	    {
		renderer_bind_mesh_buffers(mesh, shader);
		bound_mesh = mesh;
	    }

	    shader_uniform_set_data(&queue->uniforms_per_object, hash_string("world"),
				    draw->transform.data, sizeof(matrix4));

	    renderer_apply_uniforms(shader, &queue->uniforms_per_object);

	    // TODO: Make the choice about drawing arrays or elements earlier in the pipeline,
	    // remove the requirement for branching here.
	    
	    if(!mesh->index_buffer)
	    {
		glDrawArrays(GL_TRIANGLES, draw->draw_element_offset, draw->draw_element_count);
	    }
	    else
	    {
		glDrawElements(GL_TRIANGLES, draw->draw_element_count,
			       GL_UNSIGNED_INT, (void*)0);
	    }
	} break;
	case render_queue_type_target:
	{
	    render_queue_target* set_target =
		(render_queue_target*)(queue_head + sizeof(render_queue_item));

	    if(set_target->target)
	    {
		render_target_bind(set_target->target);

		GL_CALLC(glViewport, 0, 0, set_target->target->width, set_target->target->height);
	    }
	    else
	    {
		render_target_unbind();

		GL_CALLC(glViewport, 0, 0, queue->render_width, queue->render_height);
	    }

	} break;
	}

	queue_processed += (sizeof(render_queue_item) + item->command_size);
    }
}

void
renderer_queue_set_projection(render_queue* queue, matrix4 projection)
{
    shader_uniform_set_data(&queue->uniforms, hash_string("projection"),
			    projection.data, sizeof(matrix4));
}

void
renderer_queue_set_view(render_queue* queue, matrix4 view)
{
    shader_uniform_set_data(&queue->uniforms, hash_string("view"),
			    view.data, sizeof(matrix4));
}

#undef glUniformScalar
#undef glUniformMatrix
