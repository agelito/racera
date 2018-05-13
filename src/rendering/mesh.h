#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#include "math.h"
#include "heightmap.h"

#define MESH_MAX_VERTEX_BUFFERS 8
#define MESH_MAX_INPUT_LAYOUTS 16

typedef enum vertex_attributes vertex_attributes;

enum vertex_attributes
{
    vertex_attributes_positions,
    vertex_attributes_normals,
    vertex_attributes_texcoords,
    vertex_attributes_colors,
    vertex_attributes_tangents,
    vertex_attributes_binormals,
    vertex_attributes_count
};

#define VA_POSITIONS_BIT (1 << vertex_attributes_positions)
#define VA_NORMALS_BIT   (1 << vertex_attributes_normals)
#define VA_TEXCOORDS_BIT (1 << vertex_attributes_texcoords)
#define VA_COLORS_BIT    (1 << vertex_attributes_colors)
#define VA_TANGENTS_BIT  (1 << vertex_attributes_tangents)
#define VA_BINORMALS_BIT (1 << vertex_attributes_binormals)

#define VA_ISSET(mask, bits) ((mask & bits) == bits)
#define VA_INCLUDE(mask, bits) (mask |= bits)
#define VA_EXCLUDE(mask, bits) (mask ^= bits)

typedef struct color color;
typedef struct vertex_data vertex_data;
typedef struct mesh_data mesh_data;
typedef struct input_layout input_layout;
typedef struct loaded_mesh loaded_mesh;

struct color
{
    float r;
    float g;
    float b;
};

struct vertex_data
{
    vector3* positions;
    vector3* normals;
    vector2* texcoords;
    vector3* tangents;
    vector3* binormals;
    color* colors;
};

struct mesh_data
{
    uint32 index_count;
    uint32* triangles;
    uint32 vertex_count;
    vertex_data vertices;
};

struct input_layout
{
    uint32 attribute_mask;
    uint32 vertex_array;
};

struct loaded_mesh
{
    uint32 attribute_mask;
    uint32 layout_count;
    input_layout layouts[MESH_MAX_INPUT_LAYOUTS];
    uint32 vertex_buffer[MESH_MAX_VERTEX_BUFFERS];
    uint32 index_buffer;
    mesh_data data;
};

loaded_mesh
load_mesh(mesh_data data, bool32 dynamic);

void
unload_mesh(loaded_mesh* mesh);

void
update_mesh(loaded_mesh* mesh, uint32 offset, uint32 count);

mesh_data
mesh_create_quad();

mesh_data
mesh_create_triangle(float side);

mesh_data
mesh_create_circle(float radius, int subdivisions);

mesh_data
mesh_create_cube(float side);

mesh_data
mesh_create_plane_xz(float side, int subdivisions);

mesh_data
mesh_create_from_heightmap(heightmap heightmap, float world_width, float world_height,
			   int heightmap_x, int heightmap_y,
			   int heightmap_w,  int heightmap_h,
			   int resolution_w, int resolution_h,
			   float height_scale);

void
mesh_data_free(mesh_data* data);

void
mesh_generate_tangents(mesh_data* data);

void
mesh_generate_normals(mesh_data* data, bool32 from_center);

#endif // MESH_H_INCLUDED
