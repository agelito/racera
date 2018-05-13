#ifndef RENDERTARGET_H_INCLUDED
#define RENDERTARGET_H_INCLUDED

#include "../platform/platform.h"

#include "texture.h"

typedef struct render_target {
    int width;
    int height;

    uint32 framebuffer;

    int textures_color_max;

    loaded_texture created_color;
    loaded_texture created_depth;
    
    loaded_texture* textures_color;
    loaded_texture texture_depth;
} render_target;

render_target
render_target_create(int width, int height, int create_color, int create_depth);

void
render_target_attach_color(render_target* target, int index, loaded_texture* texture);

void
render_target_attach_depth(render_target* target, loaded_texture* texture);

void
render_target_bind(render_target* target);

void
render_target_unbind();

void
render_target_destroy(render_target* target);

#endif // RENDERTARGET_H_INCLUDED
