// racera_game.c

#include "platform/platform.h"
#include "platform/opengl.h"

#include <stdlib.h>
#include <math.h>

#include "racera.h"

#include "math.c"
#include "rendering/renderer.c"
#include "font.c"
#include "ui.c"

#include "input/keyboard.c"
#include "input/mouse.c"

#include "profiler.h"

static void
game_initialize(game_state* state)
{
    PROFILER_BEGIN("initialize");
    PROFILER_BEGIN("opengl initialize");
    opengl_initialize();
    PROFILER_END();

    PROFILER_BEGIN("create renderer");
    state->render_queue = renderer_queue_create(KB(16));
    state->text_renderer = text_renderer_create(KB(16), KB(4));
    PROFILER_END();

    { // NOTE: Load shaders
	PROFILER_BEGIN("load shaders");
	platform_log("load shaders\n");
	
	char* vertex_shader = "shaders/simple.vert";
	state->textured =
	    load_shader(vertex_shader, "shaders/textured.frag", 0);
	state->colored =
	    load_shader(vertex_shader, "shaders/colored.frag", 1);
	state->text =
	    load_shader(vertex_shader, "shaders/text.frag", 1);
	state->visualize_normals =
	    load_shader(vertex_shader, "shaders/normals_visualize.frag", 0);
	state->visualize_colors =
	    load_shader(vertex_shader, "shaders/colors_visualize.frag", 0);
	state->visualize_texcoords =
	    load_shader(vertex_shader, "shaders/texcoords_visualize.frag", 0);
	state->postfx_test =
	    load_shader(vertex_shader, "shaders/postfx_test.frag", 0);

	PROFILER_END();
    }

    { // NOTE: Load Meshes
	PROFILER_BEGIN("load meshes");
	platform_log("load meshes\n");

	state->cube = load_mesh(mesh_create_cube(1.0f), 0);
	mesh_data_free(&state->cube.data);

	state->cup = load_mesh(obj_load_from_file("models/cup.obj"), 0);
	mesh_data_free(&state->cup.data);

	state->quad = load_mesh(mesh_create_quad(), 0);
	mesh_data_free(&state->quad.data);
	PROFILER_END();
    }

    { // NOTE: Load Textures
	PROFILER_BEGIN("load textures");
	platform_log("load textures\n");
	
	state->checker = load_texture(texture_create_checker(256, 256, 64), 1);
	texture_data_free(&state->checker.data);
	PROFILER_END();
    }

    { // NOTE: Load Fonts
	PROFILER_BEGIN("load fonts");
	platform_log("load fonts\n");
	
	state->deja_vu = load_font(font_create_from_file("fonts/DejaVu.fnt"));
	state->deja_vu_mono = load_font(font_create_from_file("fonts/DejaVu_Mono.fnt"));
	PROFILER_END();
    }

    { // NOTE: Load Materials
	PROFILER_BEGIN("load materials");
	platform_log("load materials\n");

	state->cup_material = material_create(&state->visualize_normals, KB(1));

	state->text_background = material_create(&state->colored, KB(1));
	material_set_color(&state->text_background, "color",
			   vector4_create(0.0f, 0.0f, 0.0f, 0.75f));

	state->postfx = material_create(&state->postfx_test, KB(1));
	PROFILER_END();
    }

    { // NOTE: Load terrain
	PROFILER_BEGIN("load terrain");
	platform_log("load terrain\n");
	
	texture_data heightmap_texture = texture_create_from_tga("heightmaps/heightmap.tga");
	state->terrain = terrain_create(4096.0f, 4096.0f, 400.0f, 1024.0f, 1024.0f, heightmap_texture);
	texture_data_free(&heightmap_texture);
	PROFILER_END();
    }
    
    state->camera_position = (vector3){{{-10.2f, 13.5f, -10.2f}}};
    state->camera_pitch_yaw_roll = vector3_create(-31.0f, -55.0f, 0.0f);

    state->scene_target = render_target_create(state->screen_width, state->screen_height, 1, 1);

    platform_log("rt: %d %d\n", state->screen_width, state->screen_height);
	
    state->initialized = 1;
    
    PROFILER_END();
}

static void
control_camera(game_state* state)
{
    vector3 pitch_yaw_roll = state->camera_pitch_yaw_roll;
    pitch_yaw_roll.y -= (float)state->mouse.relative_x * 30.0f * state->time_frame;
    pitch_yaw_roll.x -= (float)state->mouse.relative_y * 30.0f * state->time_frame;
    if(pitch_yaw_roll.x < -90.0f) pitch_yaw_roll.x = -90.0f;
    if(pitch_yaw_roll.x > 90.0f) pitch_yaw_roll.x = 90.0f;
    state->camera_pitch_yaw_roll = pitch_yaw_roll;
	
    vector3 camera_movement = (vector3){0};
    if(keyboard_is_down(&state->keyboard, VKEY_W))
    {
	camera_movement.z += 250.0f * state->time_frame;
    }

    if(keyboard_is_down(&state->keyboard, VKEY_S))
    {
	camera_movement.z -= 250.0f * state->time_frame;
    }

    if(keyboard_is_down(&state->keyboard, VKEY_A))
    {
	camera_movement.x -= 250.0f * state->time_frame;
    }

    if(keyboard_is_down(&state->keyboard, VKEY_D))
    {
	camera_movement.x += 250.0f * state->time_frame;
    }

    matrix4 camera_rotation = matrix_rotation_pitch_yaw(pitch_yaw_roll.x, pitch_yaw_roll.y);
    camera_movement = vector3_matrix_multiply(camera_rotation, camera_movement);
    state->camera_position = vector3_add(state->camera_position, camera_movement);

    vector3 camera_forward = (vector3){{{0.0f, 0.0f, 3.0f}}};
    state->camera_forward = vector3_matrix_multiply(camera_rotation, camera_forward);
}

void
game_update_and_render(game_state* state)
{
    profiler_frame* frame_stats = state->frame_stats;
    
    PROFILER_BEGIN("game update & render");
    if(!state->initialized)
    {
	game_initialize(state);
    }

    PROFILER_BEGIN("game update");

    renderer_queue_set_render_size(&state->render_queue, state->screen_width, state->screen_height);

    if(state->screen_width != state->scene_target.width ||
       state->screen_height != state->scene_target.height)
    {
	render_target_destroy(&state->scene_target);
	state->scene_target = render_target_create(state->screen_width, state->screen_height, 1, 1);

	platform_log("rt: %d %d\n", state->screen_width, state->screen_height);
    }

    if(keyboard_is_pressed(&state->keyboard, VKEY_P))
    {
	state->display_profiler = !state->display_profiler;
    }

    if(keyboard_is_pressed(&state->keyboard, VKEY_ESCAPE))
    {
	state->should_quit = 1;
    }

    control_camera(state);

    vector3 pointer_location = vector3_add(state->camera_position, state->camera_forward);
    if(keyboard_is_pressed(&state->keyboard, VKEY_Q) && state->created_cube_count < MAX_CUBES)
    {
	*(state->created_cube_positions + state->created_cube_count++) = pointer_location;
    }

    if(keyboard_is_pressed(&state->keyboard, VKEY_Z) && state->created_cube_count > 0)
    {
	state->created_cube_count -= 1;
    }

    PROFILER_END();
    PROFILER_BEGIN("render scene");

    float right = (float)state->screen_width * 0.5f;
    float top = (float)state->screen_height * 0.5f;
    matrix4 projection = matrix_perspective(80.0f, right / top, 0.1f, 2000.0f);
    renderer_queue_push_projection(&state->render_queue, projection);

    matrix4 view = matrix_look_fps(state->camera_position, state->camera_pitch_yaw_roll.x,
				   state->camera_pitch_yaw_roll.y);
    renderer_queue_push_view(&state->render_queue, view);

    renderer_queue_push_target(&state->render_queue, &state->scene_target);

    renderer_queue_push_clear(&state->render_queue, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			      (float[4]){0.1f, 0.1f, 0.1f, 1.0f});

    { // NOTE: Draw scene
	terrain_render(&state->render_queue, &state->terrain);
	
	int i;
	for(i = 0; i < state->created_cube_count; i++)
	{
	    vector3 position = *(state->created_cube_positions + i);
	    matrix4 transform = matrix_translate(position.x, position.y, position.z);
	
	    renderer_queue_push_draw(&state->render_queue, &state->cup, &state->cup_material, transform);
	}

	renderer_queue_process(&state->render_queue);
	renderer_queue_clear(&state->render_queue);
    }
    PROFILER_END();
    PROFILER_BEGIN("render ui");

    renderer_queue_push_target(&state->render_queue, 0);

    projection = matrix_orthographic((real32)state->screen_width,
				     (real32)state->screen_height, 1.0f, 100.0f);
    renderer_queue_push_projection(&state->render_queue, projection);

    vector3 eye = (vector3){{{0.0f, 0.0f, -1.0f}}};
    vector3 at = (vector3){0};
    vector3 up = (vector3){{{0.0f, 1.0f, 0.0f}}};
	
    view = matrix_look_at(eye, at, up);
    renderer_queue_push_view(&state->render_queue, view);

    { // NOTE: Draw UI
	matrix4 fullscreen_quad = matrix_scale(state->screen_width, state->screen_height, 1.0f);

	vector4 fx_params = vector4_create(state->time_wall, 1.0f, 16.0f, 0.0f);
	material_set_named_value(&state->postfx, "fx_params", &fx_params, material_data_type_vector4,
	 			 sizeof(vector4));
	material_set_texture(&state->postfx, "main_texture", &state->scene_target.created_color);
	
	renderer_queue_push_draw(&state->render_queue, &state->quad,
				 &state->postfx, fullscreen_quad);
	

	vector2 text_position = vector2_create((real32)state->screen_width * -0.48f,
						(real32)state->screen_height * 0.45f);

	PROFILER_BEGIN("draw profiler");
	char stats_text[256];
	platform_format(stats_text, 256,
			"[Profiler] dt: %.4f fps: %.2f frame: %-5d index: %-3d entries: %-4d memory: %.02f MB overhead: %05.02f%%\n",
			frame_stats->frame_delta, frame_stats->frames_per_second,
			frame_stats->frame_count, frame_stats->frame_index, frame_stats->entry_count,
			(real32)frame_stats->memory_overhead / 1024 / 1024,
	                frame_stats->time_overhead_percent * 100.0f);
	ui_draw_label(state, text_position, stats_text, 14.0f, 10.0f, &state->deja_vu_mono);
	text_position.y -= 22.0f;

	if(state->display_profiler)
	{
	    uint32 entry_index;
	    for(entry_index = 0; entry_index < frame_stats->entry_count; ++entry_index)
	    {
		PROFILER_BEGIN("draw profiler entry");
		profiler_entry* entry = (frame_stats->entries + entry_index);

		char label[256];
		platform_format(label, 256, "[%s:%d]", entry->file, entry->line);
	    
		char entry_text[256];
		platform_format(entry_text, 256,
				"%-40s %-30s avg %-10llu min %-10llu max %-10llu tot %-10llu cnt %10d prc %05.02f%% prt %05.02f%%\n",
				label, entry->label, entry->avg, entry->min, entry->max,
				entry->tot, entry->cnt, entry->prc * 100.0f, entry->prt * 100.0f);

		ui_draw_label(state, text_position, entry_text, 14.0f, 10.0f, &state->deja_vu_mono);

		text_position.y -= 22.0f;
		PROFILER_END();
	    }
	}
	PROFILER_END();

	text_position.y -= 10.0f;

	char camera_text[256];
	platform_format(camera_text, 256, "camera p: %.2f %.2f %.2f\ncamera r: %.2f %.2f",
			state->camera_position.x, state->camera_position.y, state->camera_position.z,
			state->camera_pitch_yaw_roll.x, state->camera_pitch_yaw_roll.y);
	ui_draw_label(state, text_position, camera_text, 18.0f, 10.0f, &state->deja_vu_mono);

	text_position.y -= 34.0f;

	text_renderer_prepare(&state->text_renderer);
	text_renderer_draw(&state->text_renderer, &state->render_queue);
	text_renderer_clear(&state->text_renderer);
    
	renderer_queue_process(&state->render_queue);
	renderer_queue_clear(&state->render_queue);
    }
    PROFILER_END();

    PROFILER_END();
}

