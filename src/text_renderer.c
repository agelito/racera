// text_renderer.c

#include "text_renderer.h"

#include <stdlib.h>
#include <memory.h>

typedef struct text_draw_command {
    loaded_mesh*	mesh;
    material*		material;
    matrix4		transform;
    uint32		element_offset;
    uint32		element_count;
} text_draw_command;

text_renderer
text_renderer_create(uint32 text_capacity, uint32 draw_command_capacity)
{
    text_renderer renderer = (text_renderer){0};
    
    mesh_data text_mesh_data = (mesh_data){0};
    text_mesh_data.vertex_count = (text_capacity * 6);

    size_t position_data_size = (text_mesh_data.vertex_count * sizeof(vector3));
    text_mesh_data.vertices.positions = (vector3*)malloc(position_data_size);

    size_t texcoord_data_size = (text_mesh_data.vertex_count * sizeof(vector2));
    text_mesh_data.vertices.texcoords = (vector2*)malloc(texcoord_data_size);

    memset(text_mesh_data.vertices.positions, 0, position_data_size);
    memset(text_mesh_data.vertices.texcoords, 0, texcoord_data_size);

    renderer.text_buffer             = load_mesh(text_mesh_data, 1);
    renderer.text_buffer_capacity    = text_mesh_data.vertex_count;
    renderer.text_buffer_count       = 0;

    renderer.draw_command_buffer = malloc(draw_command_capacity);
    renderer.draw_command_buffer_capacity = draw_command_capacity;
    renderer.draw_command_buffer_size = 0;

    return renderer;
}

void
text_renderer_delete(text_renderer* renderer)
{
    unload_mesh(&renderer->text_buffer);
    
    renderer->text_buffer_capacity = 0;
    renderer->text_buffer_count = 0;
}

void
text_renderer_push_text(text_renderer* renderer, char* text, loaded_font* font,
			real32 font_size, shader_program* shader, matrix4 transform)
{
    text_draw_command draw;
    draw.mesh      = &renderer->text_buffer;
    draw.transform = transform;
    
    int32 cursor_x = 0;
    int32 cursor_y = 0;

    vector3* vertex_positions =
	(renderer->text_buffer.data.vertices.positions + renderer->text_buffer_count);
    vector2* vertex_texcoords =
	(renderer->text_buffer.data.vertices.texcoords + renderer->text_buffer_count);

    uint32 submit_vertex_count = 0;

    real32 one_over_page_width = 0.0f;
    real32 one_over_page_height = 0.0f;
    real32 one_over_size = (1.0f / font->data.size);

    material* configured_material = 0;
    
    char character, previous = 0;
    while((character = *text++) != 0)
    {
	// TODO: Make sure the text vertex buffer isn't overflowed.
	
	font_kerning* kern = font_get_kerning(&font->data, (uint32)previous, (uint32)character);
	font_character* fc = font_get_character(&font->data, (uint32)character);
	if(fc != 0)
	{
	    material* page_material = (font->materials + fc->page);
	    if(page_material != configured_material)
	    {
		loaded_texture* page_texture = (font->textures + fc->page);
	    
		if(submit_vertex_count)
		{
		    draw.element_offset = renderer->text_buffer_count;
		    draw.element_count  = submit_vertex_count;

		    if(renderer->draw_command_buffer_size < renderer->draw_command_buffer_capacity)
		    {
			text_draw_command* write_command =
			    (text_draw_command*)(renderer->draw_command_buffer +
						 renderer->draw_command_buffer_size);

			*write_command = draw;
			
			renderer->draw_command_buffer_size += sizeof(text_draw_command);
		    }
		    else
		    {
			platform_log("[error]: text_renderer command buffer full.\n");
		    }

		    renderer->text_buffer_count += submit_vertex_count;

		    submit_vertex_count = 0;
		}

		one_over_page_width = (1.0f / page_texture->data.width);
		one_over_page_height = (1.0f / page_texture->data.height);
		
		configured_material = page_material;
		configured_material->shader = shader;
		
		draw.material = page_material;
	    }

	    int32 base_x = cursor_x;
	    int32 base_y = cursor_y + font->data.baseline;

	    int32 x_min = base_x + fc->offset_x;
	    int32 x_max = x_min + fc->source_w;
	    int32 y_max = base_y - fc->offset_y;
	    int32 y_min = y_max - fc->source_h;

	    if(kern != 0)
	    {
		x_min += kern->amount;
		x_max += kern->amount;
	    }

	    real32 left   = (real32)x_min * one_over_size * font_size;
	    real32 right  = (real32)x_max * one_over_size * font_size;
	    
	    real32 bottom = (real32)y_min * one_over_size * font_size;
	    real32 top    = (real32)y_max * one_over_size * font_size;

	    *(vertex_positions + 0) = vector3_create(left, bottom, 0.0f);
	    *(vertex_positions + 1) = vector3_create(right, bottom, 0.0f);
	    *(vertex_positions + 2) = vector3_create(left, top, 0.0f);
	    *(vertex_positions + 3) = vector3_create(right, bottom, 0.0f);
	    *(vertex_positions + 4) = vector3_create(right, top, 0.0f);
	    *(vertex_positions + 5) = vector3_create(left, top, 0.0f);
	    vertex_positions += 6;

	    real32 uv_x0 = one_over_page_width * fc->source_x;
	    real32 uv_x1 = one_over_page_width * (fc->source_x + fc->source_w);

	    real32 uv_y1 = (one_over_page_height * fc->source_y);
	    real32 uv_y0 = (one_over_page_height * (fc->source_y + fc->source_h));

	    *(vertex_texcoords + 0) = vector2_create(uv_x0, uv_y0);
	    *(vertex_texcoords + 1) = vector2_create(uv_x1, uv_y0);
	    *(vertex_texcoords + 2) = vector2_create(uv_x0, uv_y1);
	    *(vertex_texcoords + 3) = vector2_create(uv_x1, uv_y0);
	    *(vertex_texcoords + 4) = vector2_create(uv_x1, uv_y1);
	    *(vertex_texcoords + 5) = vector2_create(uv_x0, uv_y1);
	    vertex_texcoords += 6;

	    submit_vertex_count += 6;

	    cursor_x += fc->advance;
	}
	else
	{
	    if(character == '\n')
	    {
		cursor_x = 0;
		cursor_y -= font->data.line_height;
	    }
	}

	previous = character;
    }

    if(submit_vertex_count)
    {
	draw.element_offset = renderer->text_buffer_count;
	draw.element_count  = submit_vertex_count;	

	if(renderer->draw_command_buffer_size < renderer->draw_command_buffer_capacity)
	{
	    text_draw_command* write_command =
		(text_draw_command*)(renderer->draw_command_buffer +
				     renderer->draw_command_buffer_size);

	    *write_command = draw;
	    
	    renderer->draw_command_buffer_size += sizeof(text_draw_command);
	}
	else
	{
	    platform_log("[error]: text_renderer command buffer full.\n");
	}
	
	renderer->text_buffer_count += submit_vertex_count;
	submit_vertex_count = 0;
    }
}

void
text_renderer_prepare(text_renderer* renderer)
{
    if(renderer->text_buffer_count)
    {
	update_mesh(&renderer->text_buffer, 0, renderer->text_buffer_count);
    }
}

void
text_renderer_draw(text_renderer* renderer, render_queue* queue)
{
    uint32 command_read = 0;
    while(command_read < renderer->draw_command_buffer_size)
    {
	text_draw_command* command =
	    (text_draw_command*)(renderer->draw_command_buffer + command_read);

	renderer_queue_push_draw_elements(queue, command->mesh, command->material, command->transform,
					  command->element_offset, command->element_count);

	command_read += sizeof(text_draw_command);
    }
}

void
text_renderer_clear(text_renderer* renderer)
{
    renderer->draw_command_buffer_size = 0;
    renderer->text_buffer_count = 0;
}
