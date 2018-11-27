#version 410

layout(location = 0) in vec2 texture_coord;

uniform sampler2D u_texture_0;
uniform ivec2 screen_size;

uniform float time;
uniform float frequency;

layout(location = 0) out vec4 out_color;

void main()
{
	vec2 new_coord = texture_coord;
	new_coord.x += sin(new_coord.y * frequency * 3.14 + time) / 100;
	out_color = texture(u_texture_0, new_coord);
}