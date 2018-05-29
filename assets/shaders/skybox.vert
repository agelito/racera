// simple.vert

attribute vec3 in_vertex;

uniform mat4 view;
uniform mat4 projection;

varying vec3 eye_dir;
varying vec3 uv;

void main()
{
    mat4 inverse_projection = inverse(projection);
    mat3 inverse_view = transpose(mat3(view));

    vec4 position = vec4(in_vertex, 1.0f);
    vec3 unprojected = (inverse_projection * position).xyz;

    eye_dir = inverse_view * unprojected;
    uv = normalize(eye_dir);
    
    gl_Position = vec4(in_vertex, 1.0);
}
