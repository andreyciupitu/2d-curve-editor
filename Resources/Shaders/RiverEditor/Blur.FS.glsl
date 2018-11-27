#version 410

layout(location = 0) in vec2 texture_coord;

uniform sampler2D u_texture_0;
uniform ivec2 screen_size;

layout(location = 0) out vec4 out_color;

vec4 blur(sampler2D texture, int blurRadius)
{
	vec2 texelSize = 1.0f / screen_size;
	vec4 sum = vec4(0);
	for(int i = -blurRadius; i <= blurRadius; i++)
	{
		for(int j = -blurRadius; j <= blurRadius; j++)
		{
			sum += texture(texture, texture_coord + vec2(i, j) * texelSize);
		}
	}
		
	float samples = pow((2 * blurRadius + 1), 2);
	return sum / samples;
}

void main()
{
	out_color = blur(u_texture_0, 3);
}