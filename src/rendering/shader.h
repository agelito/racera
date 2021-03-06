#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

typedef struct shader_uniform shader_uniform;
typedef struct shader_reflection shader_reflection;
typedef struct shader_uniform_data shader_uniform_data;
typedef struct shader_program shader_program;

typedef enum
{
    shader_data_float1,
    shader_data_float2,
    shader_data_float3,
    shader_data_float4,
    shader_data_integer1,
    shader_data_integer2,
    shader_data_integer3,
    shader_data_integer4,
    shader_data_matrix2,
    shader_data_matrix3,
    shader_data_matrix4,
    shader_data_sampler2d,
    shader_data_unknown,
} shader_data_type;

struct shader_uniform
{
    unsigned int name_hash;
    char* name;
    int location;
    size_t size_per_element;
    shader_data_type type;
};

struct shader_uniform_data
{
    size_t size;
    unsigned char* data;
};

struct shader_reflection
{
    int uniform_count;
    shader_uniform* uniforms;
};

struct shader_program
{
    int source_hash;
    GLuint vertex;
    GLuint fragment;
    GLuint program;
    shader_reflection info;
    bool32 transparent;
};

typedef struct uniform_data_location_list
{
    int count;
    struct uniform_data_location* locations;
    struct uniform_data_location_list* next;
} uniform_data_location_list;

typedef struct
{
    uniform_data_location_list uniform_list;
    
    size_t data_capacity;
    size_t data_reserved;
    unsigned char* data;
} shader_uniform_group;

shader_program
load_shader(char* vertex_path, char* fragment_path, bool32 transparent);

size_t
shader_data_type_size(shader_data_type type, int count);

shader_reflection
shader_reflect(shader_program* shader);

shader_uniform_group
shader_uniform_group_create(size_t data_capacity);

void
shader_uniform_set_data(shader_uniform_group* uniform_group, unsigned int name_hash,
			 void* data, size_t data_size);

shader_uniform_data
shader_uniform_get_data(shader_uniform_group* uniform_group, unsigned int name_hash);

#endif // SHADER_H_INCLUDED
