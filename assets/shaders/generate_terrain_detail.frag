// generate_terrain_detail.frag

uniform sampler2D heightmap;
uniform vec4 heightmap_st;

varying vec2 uv;

void main()
{
    float u = heightmap_st.y + uv.x * heightmap_st.x;
    float v = heightmap_st.w + uv.y * heightmap_st.z;

    float height = texture2D(heightmap, vec2(u, v)).r;
    vec4 detail_channels;
    
    if(height > 0.75)
    {
	detail_channels = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else if(height > 0.5)
    {
	vec4 stone_light = vec4(0.66, 0.66, 0.66, 1.0);
	vec4 stone_dark = vec4(0.5, 0.5, 0.5, 1.0);

	float stone_t = 1.0 - ((height - 0.5) / 0.25);
	detail_channels = mix(stone_light, stone_dark, stone_t);
    }
    else if(height > 0.12)
    {
	vec4 grass_light = vec4(0.15, 0.95, 0.2, 1.0);
	vec4 grass_deep = vec4(0.25, 0.85, 0.4, 1.0);

	float grass_t = 1.0 - ((height - 0.12) / 0.38);

	detail_channels = mix(grass_light, grass_deep, grass_t);
    }
    else if(height > 0.1)
    {
	vec4 sand_light = vec4(0.75, 0.69, 0.5, 1.0);
	vec4 sand_dark = vec4(0.65, 0.59, 0.4, 1.0);

	float sand_t = 1.0 - ((height - 0.1) / 0.02);

	detail_channels = mix(sand_light, sand_dark, sand_t);
    }
    else
    {
	vec4 sea_light = vec4(0.1, 0.4, 0.6, 1.0);
	vec4 sea_deep = vec4(0.05, 0.1,0.9, 1.0);

	float depth = 1.0 - (height / 0.1);

	detail_channels = mix(sea_light, sea_deep, depth);
    }
    
    gl_FragColor = detail_channels;
}
