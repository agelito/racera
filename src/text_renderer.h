#ifndef TEXTRENDERER_H_INCLUDED
#define TEXTRENDERER_H_INCLUDED

#include "platform/platform.h"

#include "rendering/renderer.h"

#include "font.h"

typedef struct text_renderer
{
    loaded_mesh text_buffer;
    uint32 text_buffer_capacity;
    uint32 text_buffer_count;

    uint32 draw_command_buffer_size;
    uint32 draw_command_buffer_capacity;
    void* draw_command_buffer;
} text_renderer;

text_renderer
text_renderer_create(uint32 text_capacity, uint32 draw_command_capacity);

void
text_renderer_delete(text_renderer* renderer);

void
text_renderer_push_text(text_renderer* renderer, char* text, loaded_font* font,
			real32 font_size, shader_program* shader, matrix4 transform);

void
text_renderer_prepare(text_renderer* renderer);

void
text_renderer_draw(text_renderer* renderer, render_queue* queue);

void
text_renderer_clear(text_renderer* renderer);

#endif // TEXTRENDERER_H_INCLUDED
