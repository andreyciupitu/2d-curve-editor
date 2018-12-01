#version 430
layout(location = 0) in vec2 texture_coord;

uniform sampler2D texture_1;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 bright_color;

void main()
{
	vec3 magic_stuff = vec3(0.2126, 0.7152, 0.0722);

	vec4 color = texture(texture_1, texture_coord);

	if (color.a < 0.75)
	{
		discard;
	}
	out_color = color;

	// Get post processing data
	// Convert to grayscale & get brigthness value 
	float brightness = dot(out_color.rgb, magic_stuff);

	// Store the values above the threshold in the second buffer
    if(brightness > 0.7f)
        bright_color = out_color;
    else
        bright_color = vec4(0.0, 0.0, 0.0, 1.0);
}