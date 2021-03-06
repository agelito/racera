#ifndef RACERA_H_INCLUDED
#define RACERA_H_INCLUDED

#include "math.h"
#include "font.h"
#include "profiler.h"
#include "text_renderer.h"
#include "rendering/renderer.h"

#include "terrain.h"

#include "input/mouse.h"
#include "input/keyboard.h"

#define MAX_CUBES 2048

typedef struct game_state game_state;

struct game_state
{
    int should_quit;

    real32 time_wall;
    real32 time_frame;
    
    mouse_state mouse;
    keyboard_state keyboard;

    int screen_width;
    int screen_height;
    
    int initialized;

    shader_program textured;
    shader_program colored;
    shader_program text;
    shader_program visualize_colors;
    shader_program visualize_normals;
    shader_program visualize_texcoords;
    shader_program sky_shader;
    shader_program postfx_test;

    render_queue render_queue;

    text_renderer text_renderer;

    loaded_mesh cube;
    loaded_mesh quad;
    loaded_mesh cup;
    loaded_mesh skybox;

    loaded_texture checker;

    loaded_font deja_vu;
    loaded_font deja_vu_mono;

    material cup_material;
    material text_background;
    material sky_material;
    material postfx;

    terrain terrain;

    vector3 camera_position;
    vector3 camera_pitch_yaw_roll;
    vector3 camera_forward;
    
    int created_cube_count;
    vector3 created_cube_positions[MAX_CUBES];

    render_target scene_target;

    bool32 display_profiler;
    profiler_frame* frame_stats;
};

void
game_update_and_render(struct game_state* state);

#endif // RACERA_H_INCLUDED
