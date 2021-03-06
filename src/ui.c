// ui.c

#include "text_renderer.h"

void
ui_draw_label(game_state* state, vector2 position, char* text, real32 size, real32 border,
	      loaded_font* font)
{
    matrix4 text_transform = matrix_translate(position.x, position.y, 0.0f);

    vector2 text_size = font_measure_text(&font->data, size, text);

    float background_x = position.x + (text_size.x * 0.5f);

    // TODO: Something is wrong with text measurement. Probably miscalculation
    // related to baseline and offsets.
    float background_y = position.y - (text_size.y * 0.5f) + (size * 0.75f);
    
    matrix4 text_background_translate =
	matrix_translate(background_x, background_y, 0.0f);
    matrix4 text_background_scale =
	matrix_scale(text_size.x + border, text_size.y + border, 1.0f);
    matrix4 text_background_transform =
	matrix_multiply(text_background_translate, text_background_scale);

    renderer_queue_push_draw(&state->render_queue, &state->quad,
			     &state->text_background, text_background_transform);
    text_renderer_push_text(&state->text_renderer, text, font,
			     size, &state->text, text_transform);
}
