// postfx_test.frag

uniform sampler2D main_texture;
uniform vec4 fx_params;

varying vec2 uv;

void main()
{
    uv.x += cos(uv.y * fx_params.z + fx_params.x) * fx_params.w;
    uv.y += sin(uv.x * fx_params.z + fx_params.x) * fx_params.w;
    
    gl_FragColor = texture2D(main_texture, uv);
}
