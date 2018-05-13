// render_target.c

#include "render_target.h"

#include "../platform/opengl.h"

#include <stdlib.h>

render_target
render_target_create(int width, int height, int create_color, int create_depth)
{
    render_target target = (render_target){0};

    GLint textures_color_max = 0;
    GL_CALLC(glGetIntegerv, GL_MAX_COLOR_ATTACHMENTS, &textures_color_max);

    target.textures_color_max = textures_color_max;
    
    target.width = width;
    target.height = height;

    target.textures_color = (loaded_texture*)malloc(sizeof(loaded_texture) * textures_color_max);

    if(create_color)
    {
	texture_data data = (texture_data){0};
	data.width = width;
	data.height = height;
	data.components = 4;
    
	loaded_texture texture_color = load_texture(data, 0);
	*(target.textures_color + 0) = texture_color;
	target.created_color = texture_color;
    }
    
    if(create_depth)
    {
	loaded_texture texture_depth = load_texture_depth(width, height);
	target.texture_depth = texture_depth;
	target.created_depth = texture_depth;
    }

    GLuint framebuffer;
    GL_CALL(glGenFramebuffers, 1, &framebuffer);
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, (GLuint)framebuffer);

    if(create_color)
    {
	GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, (*target.textures_color).handle, 0);

	GLenum status = GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
	    platform_log("[error] render target: %d\n", status);
	}
    }

    if(create_depth)
    {
	GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, target.created_depth.handle, 0);

	GLenum status = GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
	    platform_log("[error] render target: %d\n", status);
	}
    }

    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

    target.framebuffer = framebuffer;

    return target;
}

void
render_target_attach_color(render_target* target, int index, loaded_texture* texture)
{
    loaded_texture* attached_color = (target->textures_color + index);
    *attached_color = *texture;

    int color_slot = GL_COLOR_ATTACHMENT0 + index;
    
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER,  target->framebuffer);
    GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, color_slot,
		GL_TEXTURE_2D, attached_color->handle, 0);

    GLenum status = GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
	platform_log("[error] render target: %d\n", status);
    }

    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void
render_target_attach_depth(render_target* target, loaded_texture* texture)
{
    target->texture_depth = *texture;
    
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, target->framebuffer);
    GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
	    GL_TEXTURE_2D, texture->handle, 0);
    
    GLenum status = GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
	platform_log("[error] render target: %d\n", status);
    }
    
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void
render_target_bind(render_target* target)
{
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, target->framebuffer);
}

void
render_target_unbind()
{
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void
render_target_destroy(render_target* target)
{
    if(target->framebuffer)
    {
	GL_CALL(glDeleteFramebuffers, 1, &target->framebuffer);
	target->framebuffer = 0;
    }

    unload_texture(&target->created_color);

    free(target->textures_color);
    target->textures_color = 0;

    unload_texture(&target->created_depth);
    target->texture_depth = (loaded_texture){0};

    target->width = 0;
    target->height = 0;
}
