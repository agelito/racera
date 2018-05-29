// texcoords3_visualize.frag

varying vec3 uv;

void main()
{
    uv.x = 0.5 + (uv.x * 0.5f);
    uv.y = 0.5 + (uv.y * 0.5f);
    uv.z = 0.5 + (uv.z * 0.5f);
    gl_FragColor = vec4(uv.x, uv.y, uv.z, 1.0);
}
