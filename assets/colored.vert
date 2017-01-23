// colored.vert

attribute vec3 in_vertex;
attribute vec3 in_color;
attribute vec2 in_uv;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 tint_color;

varying vec3 color;

void main()
{
    color = (in_color * tint_color);

    mat4 world_view_projection = projection * view * world;
    gl_Position = world_view_projection * vec4(in_vertex, 1.0);
}
