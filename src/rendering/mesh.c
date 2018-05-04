// mesh.c

#include "opengl.h"
#include "mesh.h"

// TODO: Use platform for memory allocation instead of stdlib.
#include <stdlib.h>
#include <math.h>

#include "../asset_import/obj_importer.c"

#define create_vertex(x, y, z, color, uv_x, uv_y) (vertex){{{{x, y, z}}}, color, {{{uv_x, uv_y}}}}
#define create_vertex_hash(x, y, z) ((int)(x * 1000.0f) & 0x3ff) |	\
    (((int)(y * 1000.0f) & 0x3ff) << 10) |				\
    (((int)(z * 1000.0f) & 0x3ff) << 20)

loaded_mesh
load_mesh(gl_functions* gl, mesh_data data, bool32 dynamic)
{
    loaded_mesh mesh = (loaded_mesh){0};
    mesh.data = data;
    
    GLuint* vertex_buffer = 0;
    size_t vertex_buffer_size = 0;

    GLenum usage = GL_STATIC_DRAW;

    if(dynamic)
    {
	usage = GL_DYNAMIC_DRAW;
    }

    if(data.triangles)
    {
	uint32* index_buffer = &mesh.index_buffer;
	gl->glGenBuffers(1, index_buffer);
	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *index_buffer);

	size_t index_buffer_size = (sizeof(uint32) * data.index_count);
	gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, data.triangles, usage);

	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    if(data.vertices.positions)
    {
	vertex_buffer = (mesh.vertex_buffer + vertex_attributes_positions);
	gl->glGenBuffers(1, vertex_buffer);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);
	
	vertex_buffer_size = (sizeof(vector3) * data.vertex_count);
	gl->glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size,
		      data.vertices.positions, usage);

	VA_INCLUDE(mesh.attribute_mask, VA_POSITIONS_BIT);
    }

    if(data.vertices.normals)
    {
	vertex_buffer = (mesh.vertex_buffer + vertex_attributes_normals);
	gl->glGenBuffers(1, vertex_buffer);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_size = (sizeof(vector3) * data.vertex_count);
	gl->glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size,
			  data.vertices.normals, usage);

	VA_INCLUDE(mesh.attribute_mask, VA_NORMALS_BIT);
    }

    if(data.vertices.texcoords)
    {
	vertex_buffer = (mesh.vertex_buffer + vertex_attributes_texcoords);
	gl->glGenBuffers(1, vertex_buffer);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_size = (sizeof(vector2) * data.vertex_count);
	gl->glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size,
			  data.vertices.texcoords, usage);
	
	VA_INCLUDE(mesh.attribute_mask, VA_TEXCOORDS_BIT);
    }

    if(data.vertices.colors)
    {
	vertex_buffer = (mesh.vertex_buffer + vertex_attributes_colors);
	gl->glGenBuffers(1, vertex_buffer);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_size = (sizeof(color) * data.vertex_count);
	gl->glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size,
			  data.vertices.colors, usage);

	VA_INCLUDE(mesh.attribute_mask, VA_COLORS_BIT);
    }

    if(data.vertices.tangents)
    {
	vertex_buffer = (mesh.vertex_buffer + vertex_attributes_tangents);
	gl->glGenBuffers(1, vertex_buffer);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_size = (sizeof(vector3) * data.vertex_count);
	gl->glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size,
			 data.vertices.tangents, usage);

	VA_INCLUDE(mesh.attribute_mask, VA_TANGENTS_BIT);
    }

    if(data.vertices.binormals)
    {
	vertex_buffer = (mesh.vertex_buffer + vertex_attributes_binormals);
	gl->glGenBuffers(1, vertex_buffer);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_size = (sizeof(vector3) * data.vertex_count);
	gl->glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size,
			 data.vertices.binormals, usage);

	VA_INCLUDE(mesh.attribute_mask, VA_BINORMALS_BIT);
    }

    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mesh;
}

void
update_mesh(gl_functions* gl, loaded_mesh* mesh, uint32 offset, uint32 count)
{
    GLuint* vertex_buffer = 0;
    size_t vertex_buffer_offset = 0;;
    size_t vertex_buffer_size = 0;

    if(mesh->index_buffer && mesh->data.triangles)
    {
	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);

	size_t index_buffer_offset = (offset * sizeof(uint32));
	size_t index_buffer_size = (count * sizeof(uint32));
	gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_offset, index_buffer_size,
			    mesh->data.triangles);
    }
    
    if(VA_ISSET(mesh->attribute_mask, VA_POSITIONS_BIT))
    {
	vertex_buffer = (mesh->vertex_buffer + vertex_attributes_positions);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_offset = (offset * sizeof(vector3));
	vertex_buffer_size = (count * sizeof(vector3));
	gl->glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_buffer_size,
			    mesh->data.vertices.positions);
    }

    if(VA_ISSET(mesh->attribute_mask, VA_NORMALS_BIT))
    {
	vertex_buffer = (mesh->vertex_buffer + vertex_attributes_normals);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_offset = (offset * sizeof(vector3));
	vertex_buffer_size = (count * sizeof(vector3));
	gl->glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_buffer_size,
			    mesh->data.vertices.normals);
    }

    if(VA_ISSET(mesh->attribute_mask, VA_TEXCOORDS_BIT))
    {
	vertex_buffer = (mesh->vertex_buffer + vertex_attributes_texcoords);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_offset = (offset * sizeof(vector2));
	vertex_buffer_size = (count * sizeof(vector2));
	gl->glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_buffer_size,
			    mesh->data.vertices.texcoords);
    }

    if(VA_ISSET(mesh->attribute_mask, VA_COLORS_BIT))
    {
	vertex_buffer = (mesh->vertex_buffer + vertex_attributes_colors);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_offset = (offset * sizeof(vector3));
	vertex_buffer_size = (count * sizeof(vector3));
	gl->glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_buffer_size,
			    mesh->data.vertices.colors);
    }

    if(VA_ISSET(mesh->attribute_mask, VA_TANGENTS_BIT))
    {
	vertex_buffer = (mesh->vertex_buffer + vertex_attributes_tangents);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_offset = (offset * sizeof(vector3));
	vertex_buffer_size = (count * sizeof(vector3));
	gl->glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_buffer_size,
			    mesh->data.vertices.tangents);
    }

    if(VA_ISSET(mesh->attribute_mask, VA_BINORMALS_BIT))
    {
	vertex_buffer = (mesh->vertex_buffer + vertex_attributes_binormals);
	gl->glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

	vertex_buffer_offset = (offset * sizeof(vector3));
	vertex_buffer_size = (count * sizeof(vector3));
	gl->glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_buffer_size,
			    mesh->data.vertices.binormals);
    }

    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

mesh_data
mesh_create_quad()
{
    mesh_data data = (mesh_data){0};
    data.vertex_count = 4;
    data.index_count = 6;

    real32 half_side = 0.5f;

    size_t position_data_size = (data.vertex_count * sizeof(vector3));
    vector3* positions = (vector3*)malloc(position_data_size);
    *(positions + 0) = vector3_create(-half_side, -half_side, 0.0f);
    *(positions + 1) = vector3_create( half_side, -half_side, 0.0f);
    *(positions + 2) = vector3_create(-half_side,  half_side, 0.0f);
    *(positions + 3) = vector3_create( half_side,  half_side, 0.0f);
    data.vertices.positions = positions;

    size_t texcoord_data_size = (data.vertex_count * sizeof(vector2));
    vector2* texcoords = (vector2*)malloc(texcoord_data_size);
    *(texcoords + 0) = vector2_create(0.0f, 0.0f);
    *(texcoords + 1) = vector2_create(1.0f, 0.0f);
    *(texcoords + 2) = vector2_create(0.0f, 1.0f);
    *(texcoords + 3) = vector2_create(1.0f, 1.0f);
    data.vertices.texcoords = texcoords;

    size_t triangle_data_size = (data.index_count * sizeof(uint32));
    uint32* triangles = (uint32*)malloc(triangle_data_size);

    *(triangles + 0) = 0;
    *(triangles + 1) = 1;
    *(triangles + 2) = 2;
    *(triangles + 3) = 2;
    *(triangles + 4) = 1;
    *(triangles + 5) = 3;

    data.triangles = triangles;

    return data;
}

mesh_data
mesh_create_triangle(float side)
{
    mesh_data data = (mesh_data){0};
    data.vertex_count = 3;

    float half_side = side * 0.5f;

    size_t position_data_size = (data.vertex_count * sizeof(vector3));
    vector3* positions = (vector3*)malloc(position_data_size);
    *(positions + 0) = (vector3){{{0.0f, half_side, 0.0f}}};
    *(positions + 1) = (vector3){{{-half_side, -half_side, 0.0f}}};
    *(positions + 2) = (vector3){{{half_side, -half_side, 0.0f}}};
    data.vertices.positions = positions;
    
    size_t texcoord_data_size = (data.vertex_count * sizeof(vector2));
    vector2* texcoords = (vector2*)malloc(texcoord_data_size);
    *(texcoords + 0) = (vector2){{{0.5f, 1.0f}}};
    *(texcoords + 1) = (vector2){{{0.0f, 0.0f}}};
    *(texcoords + 2) = (vector2){{{1.0f, 0.0f}}};
    data.vertices.texcoords = texcoords;

    return data;
}

mesh_data
mesh_create_circle(float radius, int subdivisions)
{
    mesh_data data = (mesh_data){0};

    int segments = (subdivisions * 4);
    data.vertex_count = (segments * 3);

    size_t position_data_size = (sizeof(vector3) * data.vertex_count);
    vector3* positions = (vector3*)malloc(position_data_size);
    data.vertices.positions = positions;

    size_t texcoord_data_size = (sizeof(vector2) * data.vertex_count);
    vector2* texcoords = (vector2*)malloc(texcoord_data_size);
    data.vertices.texcoords = texcoords;

    vector3 center = (vector3){{{0.0f, 0.0f, 0.0f}}};

    float segment_step = MATH_PIOVER2 / subdivisions;

    int segment, vertex_index = 0;
    for(segment = 0; segment < subdivisions; ++segment)
    {
	float circle_x1 = cos(segment * segment_step) * radius;
	float circle_y1 = sin(segment * segment_step) * radius;

	float circle_x2 = cos((segment + 1) * segment_step) * radius;
	float circle_y2 = sin((segment + 1) * segment_step) * radius;

	// side 0
	vector3 segment0_0 = vector3_create(circle_x1, circle_y1, 0.0f);
	vector3 segment1_0 = vector3_create(circle_x2, circle_y2, 0.0f);

	*(positions + vertex_index+0) = center;
	*(positions + vertex_index+1) = segment0_0;
	*(positions + vertex_index+2) = segment1_0;

	*(texcoords + vertex_index+0) = vector2_create(0.5f, 0.5f);
	*(texcoords + vertex_index+1) =
	    vector2_scale(vector2_create(1.0f + circle_x1, 1.0f + circle_y1), 0.5f);
	*(texcoords + vertex_index+2) =
	    vector2_scale(vector2_create(1.0f + circle_x2, 1.0f + circle_y2), 0.5f);
	vertex_index += 3;

	// side 1
	vector3 segment0_1 = vector3_create(circle_x1, -circle_y1, 0.0f);
	vector3 segment1_1 = vector3_create(circle_x2, -circle_y2, 0.0f);
	
	*(positions + vertex_index+0) = center;
	*(positions + vertex_index+1) = segment1_1;
	*(positions + vertex_index+2) = segment0_1;

	*(texcoords + vertex_index+0) = vector2_create(0.5f, 0.5f);
	*(texcoords + vertex_index+2) =
	    vector2_scale(vector2_create(1.0f + circle_x1, 1.0f + -circle_y1), 0.5f);
	*(texcoords + vertex_index+1) =
	    vector2_scale(vector2_create(1.0f + circle_x2, 1.0f + -circle_y2), 0.5f);
	vertex_index += 3;

	// side 2
	vector3 segment0_2 = vector3_create(-circle_x1, circle_y1, 0.0f);
	vector3 segment1_2 = vector3_create(-circle_x2, circle_y2, 0.0f);

	*(positions + vertex_index+0) = center;
	*(positions + vertex_index+1) = segment1_2;
	*(positions + vertex_index+2) = segment0_2;

	*(texcoords + vertex_index+0) = vector2_create(0.5f, 0.5f);
	*(texcoords + vertex_index+2) =
	    vector2_scale(vector2_create(1.0f + -circle_x1, 1.0f + circle_y1), 0.5f);
	*(texcoords + vertex_index+1) =
	    vector2_scale(vector2_create(1.0f + -circle_x2, 1.0f + circle_y2), 0.5f);
	vertex_index += 3;

	// side 3
	vector3 segment0_3 = vector3_create(-circle_x1, -circle_y1, 0.0f);
	vector3 segment1_3 = vector3_create(-circle_x2, -circle_y2, 0.0f);

	*(positions + vertex_index+0) = center;
	*(positions + vertex_index+1) = segment0_3;
	*(positions + vertex_index+2) = segment1_3;

	*(texcoords + vertex_index+0) = vector2_create(0.5f, 0.5f);
	*(texcoords + vertex_index+1) =
	    vector2_scale(vector2_create(1.0f + -circle_x1, 1.0f + -circle_y1), 0.5f);
	*(texcoords + vertex_index+2) =
	    vector2_scale(vector2_create(1.0f + -circle_x2, 1.0f + -circle_y2), 0.5f);
	vertex_index += 3;
    }

    return data;
}

#include <string.h>

mesh_data
mesh_create_cube(float side)
{
    mesh_data data = (mesh_data){0};
    data.vertex_count = 36;

    size_t position_data_size = (sizeof(vector3) * data.vertex_count);
    vector3* positions = (vector3*)malloc(position_data_size);
    data.vertices.positions = positions;

    size_t texcoord_data_size = (sizeof(vector2) * data.vertex_count);
    vector2* texcoords = (vector2*)malloc(texcoord_data_size);
    data.vertices.texcoords = texcoords;
    
    memset(positions, 0, position_data_size);

    int vertex_index = 0;
    float half_side = side * 0.5f;

    vector2 uv_quad[4] = {
	{{{0.0f, 0.0f}}}, // BOTTOM LEFT
	{{{0.0f, 1.0}}},  // TOP LEFT
	{{{1.0f, 1.0f}}}, // TOP RIGHT
	{{{1.0f, 0.0f}}}  // BOTTOM RIGHT
    };

    vector3 sides[8] = {
	// [0]TOP RIGHT FRONT
	{{{half_side, half_side, half_side}}},
	// [1]TOP RIGHT BACK
	{{{half_side, half_side, -half_side}}},
	// [2]BOTTOM RIGHT FRONT
	{{{half_side, -half_side, half_side}}},
	// [3]BOTTOM RIGHT BACK
	{{{half_side, -half_side, -half_side}}},
	// [4]TOP LEFT FRONT
	{{{-half_side, half_side, half_side}}},
	// [5]TOP LEFT BACK
	{{{-half_side, half_side, -half_side}}},
	// [6]BOTTOM LEFT FRONT
	{{{-half_side, -half_side, half_side}}},
	// [7]BOTTOM LEFT BACK
	{{{-half_side, -half_side, -half_side}}}
    };

    // Right Side
    *(positions + vertex_index+0) = *(sides + 0);
    *(positions + vertex_index+1) = *(sides + 1);
    *(positions + vertex_index+2) = *(sides + 2);
    *(positions + vertex_index+3) = *(sides + 2);
    *(positions + vertex_index+4) = *(sides + 1);
    *(positions + vertex_index+5) = *(sides + 3);

    *(texcoords + vertex_index+0) = *(uv_quad + 1);
    *(texcoords + vertex_index+1) = *(uv_quad + 2);
    *(texcoords + vertex_index+2) = *(uv_quad + 0);
    *(texcoords + vertex_index+3) = *(uv_quad + 0);
    *(texcoords + vertex_index+4) = *(uv_quad + 2);
    *(texcoords + vertex_index+5) = *(uv_quad + 3);

    vertex_index += 6;

    // Left Side
    *(positions + vertex_index+0) = *(sides + 5);
    *(positions + vertex_index+1) = *(sides + 4);
    *(positions + vertex_index+2) = *(sides + 6);
    *(positions + vertex_index+3) = *(sides + 6);
    *(positions + vertex_index+4) = *(sides + 7);
    *(positions + vertex_index+5) = *(sides + 5);
    
    *(texcoords + vertex_index+0) = *(uv_quad + 1);
    *(texcoords + vertex_index+1) = *(uv_quad + 2);
    *(texcoords + vertex_index+2) = *(uv_quad + 3);
    *(texcoords + vertex_index+3) = *(uv_quad + 3);
    *(texcoords + vertex_index+4) = *(uv_quad + 0);
    *(texcoords + vertex_index+5) = *(uv_quad + 1);

    vertex_index += 6;
    
    // Top Side
    *(positions + vertex_index+0) = *(sides + 4);
    *(positions + vertex_index+1) = *(sides + 5);
    *(positions + vertex_index+2) = *(sides + 1);
    *(positions + vertex_index+3) = *(sides + 1);
    *(positions + vertex_index+4) = *(sides + 0);
    *(positions + vertex_index+5) = *(sides + 4);

    *(texcoords + vertex_index+0) = *(uv_quad + 1);
    *(texcoords + vertex_index+1) = *(uv_quad + 0);
    *(texcoords + vertex_index+2) = *(uv_quad + 3);
    *(texcoords + vertex_index+3) = *(uv_quad + 3);
    *(texcoords + vertex_index+4) = *(uv_quad + 2);
    *(texcoords + vertex_index+5) = *(uv_quad + 1);

    vertex_index += 6;
    
    // Bottom Side
    *(positions + vertex_index+0) = *(sides + 6);
    *(positions + vertex_index+1) = *(sides + 3);
    *(positions + vertex_index+2) = *(sides + 7);
    *(positions + vertex_index+3) = *(sides + 3);
    *(positions + vertex_index+4) = *(sides + 6);
    *(positions + vertex_index+5) = *(sides + 2);

    *(texcoords + vertex_index+0) = *(uv_quad + 0);
    *(texcoords + vertex_index+1) = *(uv_quad + 2);
    *(texcoords + vertex_index+2) = *(uv_quad + 1);
    *(texcoords + vertex_index+3) = *(uv_quad + 2);
    *(texcoords + vertex_index+4) = *(uv_quad + 0);
    *(texcoords + vertex_index+5) = *(uv_quad + 3);

    vertex_index += 6;

    // Front Side
    *(positions + vertex_index+0) = *(sides + 6);
    *(positions + vertex_index+1) = *(sides + 4);
    *(positions + vertex_index+2) = *(sides + 2);
    *(positions + vertex_index+3) = *(sides + 4);
    *(positions + vertex_index+4) = *(sides + 0);
    *(positions + vertex_index+5) = *(sides + 2);

    *(texcoords + vertex_index+0) = *(uv_quad + 3);
    *(texcoords + vertex_index+1) = *(uv_quad + 2);
    *(texcoords + vertex_index+2) = *(uv_quad + 0);
    *(texcoords + vertex_index+3) = *(uv_quad + 2);
    *(texcoords + vertex_index+4) = *(uv_quad + 1);
    *(texcoords + vertex_index+5) = *(uv_quad + 0);

    vertex_index += 6;
    
    // Back Side
    *(positions + vertex_index+0) = *(sides + 7);
    *(positions + vertex_index+1) = *(sides + 1);
    *(positions + vertex_index+2) = *(sides + 5);
    *(positions + vertex_index+3) = *(sides + 1);
    *(positions + vertex_index+4) = *(sides + 7);
    *(positions + vertex_index+5) = *(sides + 3);

    *(texcoords + vertex_index+0) = *(uv_quad + 0);
    *(texcoords + vertex_index+1) = *(uv_quad + 2);
    *(texcoords + vertex_index+2) = *(uv_quad + 1);
    *(texcoords + vertex_index+3) = *(uv_quad + 2);
    *(texcoords + vertex_index+4) = *(uv_quad + 0);
    *(texcoords + vertex_index+5) = *(uv_quad + 3);

    vertex_index += 6;

    return data;
}

mesh_data
mesh_create_plane_xz(float side, int subdivisions)
{
    mesh_data data = (mesh_data){0};
    data.vertex_count = (subdivisions * subdivisions) * 6;

    size_t position_data_size = (sizeof(vector3) * data.vertex_count);
    vector3* positions = (vector3*)malloc(position_data_size);
    data.vertices.positions = positions;

    size_t texcoord_data_size = (sizeof(vector2) * data.vertex_count);
    vector2* texcoords = (vector2*)malloc(texcoord_data_size);
    data.vertices.texcoords = texcoords;

    float half_side = side * 0.5f;
    float quad_side = side / subdivisions;

    vector3 corners[4] = {
	{{{0.0f, 0.0f, 1.0f}}},
	{{{1.0f, 0.0f, 1.0f}}},
	{{{0.0f, 0.0f, 0.0f}}},
	{{{1.0f, 0.0f, 0.0f}}}
    };

    int vertex_index = 0;
    
    int x, y;
    for(y = 0; y < subdivisions; y++)
    {
	float z0 = (y * quad_side) - half_side;
	float z1 = ((y + 1) * quad_side) - half_side;

	corners[0].z = z1;
	corners[1].z = z1;
	corners[2].z = z0;
	corners[3].z = z0;
	
	for(x = 0; x < subdivisions; x++)
	{
	    float x0 = (x * quad_side) - half_side;
	    float x1 = ((x + 1) * quad_side) - half_side;
	
	    corners[0].x = x0;
	    corners[1].x = x1;
	    corners[2].x = x0;
	    corners[3].x = x1;
	    
	    *(positions + vertex_index+0) = *(corners + 2);
	    *(positions + vertex_index+1) = *(corners + 3);
	    *(positions + vertex_index+2) = *(corners + 0);
	    *(positions + vertex_index+3) = *(corners + 0);
	    *(positions + vertex_index+4) = *(corners + 3);
	    *(positions + vertex_index+5) = *(corners + 1);

	    *(texcoords + vertex_index+0) = vector2_create(x0, z0);
	    *(texcoords + vertex_index+1) = vector2_create(x1, z0);
	    *(texcoords + vertex_index+2) = vector2_create(x0, z1);
	    *(texcoords + vertex_index+3) = vector2_create(x0, z1);
	    *(texcoords + vertex_index+4) = vector2_create(x1, z0);
	    *(texcoords + vertex_index+5) = vector2_create(x1, z1);

	    vertex_index += 6;
	}
    }

    return data;
}

mesh_data
mesh_create_from_heightmap(heightmap heightmap,
			   float world_x, float world_y, float world_width, float world_height,
			   int heightmap_x, int heightmap_y, int heightmap_w,  int heightmap_h,
			   int resolution_w, int resolution_h)
{
    int width_minus_one  = resolution_w - 1;
    int height_minus_one = resolution_h - 1;
    
    mesh_data data = (mesh_data){0};
    data.vertex_count = (resolution_w * resolution_h);
    data.index_count  = (width_minus_one * height_minus_one) * 6;

    size_t position_data_size = (sizeof(vector3) * data.vertex_count);
    vector3* positions = (vector3*)malloc(position_data_size);
    data.vertices.positions = positions;

    size_t texcoord_data_size = (sizeof(vector2) * data.vertex_count);
    vector2* texcoords = (vector2*)malloc(texcoord_data_size);
    data.vertices.texcoords = texcoords;

    size_t triangles_data_size = (sizeof(uint32) * data.index_count);
    uint32* triangles = (uint32*)malloc(triangles_data_size);
    data.triangles = triangles;

    vector3 center_offset = vector3_create(world_width * 0.5f, 0.0f, world_height * 0.5f);

    float one_over_heightmap_w = 1.0f / heightmap.width;
    float one_over_heightmap_h = 1.0f / heightmap.height;

    int x, y;
    for(y = 0; y < resolution_h; ++y)
    {
	float v = (float)y / height_minus_one;

	float heightmap_sample_y = (float)heightmap_y + heightmap_h * v;
	float v_h = heightmap_sample_y * one_over_heightmap_h;

	for(x = 0; x < resolution_w; ++x)
	{
	    float u = (float)x / width_minus_one;

	    float heightmap_sample_x =(float)heightmap_x + heightmap_w * u;
	    float u_h = heightmap_sample_x * one_over_heightmap_w;
	    
	    float height_y = heightmap_sample(&heightmap, u_h, v_h);
	    vector3 position = vector3_create(u * world_width, height_y, v * world_height);

	    position.x += world_x;
	    position.z += world_y;

	    int vertex = x + y * resolution_w;
	    positions[vertex] = vector3_subtract(position, center_offset);
	    texcoords[vertex] = vector2_create(u, v);
	}
    }

    int vertex_index = 0;
    for(y = 0; y < height_minus_one; ++y)
    {
	for(x = 0; x < width_minus_one; ++x)
	{
	    triangles[vertex_index + 0] =  x       +  y      * resolution_w;
	    triangles[vertex_index + 1] = (x + 1)  +  y      * resolution_w;
	    triangles[vertex_index + 2] =  x       + (y + 1) * resolution_w;
	    triangles[vertex_index + 3] =  x       + (y + 1) * resolution_w;
	    triangles[vertex_index + 4] = (x + 1)  +  y      * resolution_w;
	    triangles[vertex_index + 5] = (x + 1)  + (y + 1) * resolution_w;

	    vertex_index += 6;
	}
    }

    return data;
}

void
mesh_data_free(mesh_data* data)
{
    if(data->triangles)
    {
	free(data->triangles);
	data->triangles = 0;
    }
    
    if(data->vertices.positions)
    {
	free(data->vertices.positions);
	data->vertices.positions = 0;
    }

    if(data->vertices.normals)
    {
	free(data->vertices.normals);
	data->vertices.normals = 0;
    }

    if(data->vertices.texcoords)
    {
	free(data->vertices.texcoords);
	data->vertices.texcoords = 0;
    }

    if(data->vertices.colors)
    {
	free(data->vertices.colors);
	data->vertices.colors = 0;
    }

    if(data->vertices.tangents)
    {
	free(data->vertices.tangents);
	data->vertices.tangents = 0;
    }

    if(data->vertices.binormals)
    {
	free(data->vertices.binormals);
	data->vertices.binormals = 0;
    }
}

void
mesh_generate_tangents(mesh_data* data)
{
    if(data->vertices.tangents)
	free(data->vertices.tangents);
    
    if(data->vertices.binormals)
	free(data->vertices.binormals);

    data->vertices.tangents = 0;
    data->vertices.binormals = 0;
    
    if(!data->vertices.positions)
    {
	platform_log("can't generate tangents without positions.\n");
	return;
    }
    
    if(!data->vertices.texcoords)
    {
	platform_log("can't generate tangents without texcoords.\n");
	return;
    }

    size_t tangent_workset_size = sizeof(vector3) * data->vertex_count;
    vector3* tangents = (vector3*)malloc(tangent_workset_size * 2);
    vector3* binormals = (tangents + data->vertex_count);

    uint32 workset_reset;
    for_range(workset_reset, data->vertex_count)
    {
	*(tangents + workset_reset) = vector3_create(0.0f, 0.0f, 0.0f);
	*(binormals + workset_reset) = vector3_create(0.0f, 0.0f, 0.0f);
    }

    uint32 triangle_count;
    if(data->triangles)
    {
	triangle_count = data->index_count / 3;
    }
    else
    {
	triangle_count = data->vertex_count / 3;
    }
    
    uint32 triangle;
    for_range(triangle, triangle_count)
    {
	int vertex_index0;
	int vertex_index1;
	int vertex_index2;
	
	if(data->triangles)
	{
	    uint32* triangle_start =
		(data->triangles + (triangle * 3));
	    vertex_index0 = *(triangle_start + 0);
	    vertex_index1 = *(triangle_start + 1);
	    vertex_index2 = *(triangle_start + 2);
	}
	else
	{
	    int triangle_start = triangle * 3;
	    vertex_index0 = triangle_start + 0;
	    vertex_index1 = triangle_start + 1;
	    vertex_index2 = triangle_start + 2;
	}

	vector3 vertex_position0 = *(data->vertices.positions + vertex_index0);
	vector3 vertex_position1 = *(data->vertices.positions + vertex_index1);
	vector3 vertex_position2 = *(data->vertices.positions + vertex_index2);

	vector3 position_delta1 = vector3_subtract(vertex_position1, vertex_position0);
	vector3 position_delta2 = vector3_subtract(vertex_position2, vertex_position0);

	vector2 vertex_texcoord0 = *(data->vertices.texcoords + vertex_index0);
	vector2 vertex_texcoord1 = *(data->vertices.texcoords + vertex_index1);
	vector2 vertex_texcoord2 = *(data->vertices.texcoords + vertex_index2);

	vector2 texcoord_delta1 = vector2_subtract(vertex_texcoord1, vertex_texcoord0);
	vector2 texcoord_delta2 = vector2_subtract(vertex_texcoord2, vertex_texcoord0);

	real32 r = 1.0f / (texcoord_delta1.x * texcoord_delta2.y -
			   texcoord_delta1.y * texcoord_delta2.x);
	
	vector3 tangent = vector3_subtract(vector3_scale(position_delta1, texcoord_delta2.y),
					   vector3_scale(position_delta2, texcoord_delta1.y));
	tangent = vector3_scale(tangent, r);
	
	vector3 binormal = vector3_subtract(vector3_scale(position_delta2, texcoord_delta1.x),
					     vector3_scale(position_delta1, texcoord_delta2.x));
	binormal = vector3_scale(binormal, r);

	*(tangents + vertex_index0) = vector3_add(tangent, *(tangents + vertex_index0));
	*(tangents + vertex_index1) = vector3_add(tangent, *(tangents + vertex_index1));
	*(tangents + vertex_index2) = vector3_add(tangent, *(tangents + vertex_index2));
	
	*(binormals + vertex_index0) = vector3_add(binormal, *(binormals + vertex_index0));
	*(binormals + vertex_index1) = vector3_add(binormal, *(binormals + vertex_index1));
	*(binormals + vertex_index2) = vector3_add(binormal, *(binormals + vertex_index2));
    }

    size_t tangents_output_size = sizeof(vector3) * data->vertex_count;
    vector3* tangents_result = (vector3*)malloc(tangents_output_size);

    size_t binormal_output_size = sizeof(vector3) * data->vertex_count;
    vector3* binormals_result = (vector3*)malloc(binormal_output_size);

    uint32 vertex;
    for_range(vertex, data->vertex_count)
    {
	vector3 tangent = *(tangents + vertex);
	tangent = vector3_normalize(tangent);
	
	vector3 binormal = *(binormals + vertex);
	binormal = vector3_normalize(binormal);

	*(tangents_result + vertex) = tangent;
	*(binormals_result + vertex) = binormal;
    }

    data->vertices.tangents = tangents_result;
    data->vertices.binormals = binormals_result;

    free(tangents);
}

void
mesh_generate_normals(mesh_data* data, bool32 from_center)
{
    if(data->vertices.normals)
	free(data->vertices.normals);

    data->vertices.normals = 0;
    
    if(!data->vertices.positions)
    {
	platform_log("can't generate normals without positions.\n");
	return;
    }

    int vertex_count = data->vertex_count;

    vector3* normals = (vector3*)malloc(sizeof(vector3) * vertex_count);

    int vertex_index;
    for_range(vertex_index, vertex_count)
    {
	*(normals + vertex_index) = vector3_create(0.0f, 0.0f, 0.0f);

	if(from_center)
	{
	    vector3 position = *(data->vertices.positions + vertex_index);
	    *(normals + vertex_index) = vector3_normalize(position);
	}
    }

    if(!from_center)
    {
	int triangle_count;
	if(data->triangles)
	{
	    triangle_count = data->index_count / 3;
	}
	else
	{
	    triangle_count = data->vertex_count / 3;
	}

	int triangle;
	for_range(triangle, triangle_count)
	{
	    int vertex_index0;
	    int vertex_index1;
	    int vertex_index2;
	
	    if(data->triangles)
	    {
		uint32* triangle_start =
		    (data->triangles + (triangle * 3));
		vertex_index0 = *(triangle_start + 0);
		vertex_index1 = *(triangle_start + 1);
		vertex_index2 = *(triangle_start + 2);
	    }
	    else
	    {
		int triangle_start = triangle * 3;
		vertex_index0 = triangle_start + 0;
		vertex_index1 = triangle_start + 1;
		vertex_index2 = triangle_start + 2;
	    }

	    vector3 position0 = *(data->vertices.positions + vertex_index0);
	    vector3 position1 = *(data->vertices.positions + vertex_index1);
	    vector3 position2 = *(data->vertices.positions + vertex_index2);

	    vector3 delta0 = vector3_subtract(position1, position0);
	    vector3 delta1 = vector3_subtract(position2, position0);

	    vector3 normal = vector3_cross(delta1, delta0);
	    normal = vector3_normalize(normal);

	    vector3* normal0 = (normals + vertex_index0);
	    *normal0 = vector3_add(*normal0, normal);

	    vector3* normal1 = (normals + vertex_index1);
	    *normal1 = vector3_add(*normal1, normal);

	    vector3* normal2 = (normals + vertex_index2);
	    *normal2 = vector3_add(*normal2, normal);
	}

	uint32 vertex_index;
	for_range(vertex_index, data->vertex_count)
	{
	    vector3 normal = *(normals + vertex_index);
	    *(normals + vertex_index) = vector3_normalize(normal);
	}
    }

    data->vertices.normals = normals;
}

#undef create_vertex
