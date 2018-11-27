#version 430
layout(lines) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;

uniform vec3 control_points[4];
uniform float surface_width;

uniform int no_of_instances;
uniform int generated_points_count;

uniform float time;
uniform float speed;
uniform float tilingFactor;

in int instance[2];

layout(location = 0) out vec2 v_tex_coord;

vec3 translateX(vec3 point, float t)
{
	return vec3(point.x + t, point.y, point.z);
}

vec3 bezier(float t)
{
	vec3 result = vec3(0);
	float c = 1;
	for (int i = 0; i < 4; i++)
	{
		if (i > 0)
			c *=  (3 - i + 1) / float(i);  
		result += c * pow((1 - t), 3 - i) * pow(t, i) * control_points[i]; 
	}
	return result;
}

vec3 get_curve_normal(float t)
{
	// Use the derivative to find out the tangent to the curve 
	vec3 tangent = -3 * pow((1 - t), 2) * control_points[0] + 3 * (3 * pow(t, 2) - 4 * t + 1) * control_points[1] +
					3 * (2 - 3 * t) * t * control_points[2] + 3 * pow(t, 2) * control_points[3];

	// Return a normal in the same plane
	return normalize(vec3(tangent.y, -tangent.x, tangent.z));
}

void emit_points(float t)
{
	// Get a normal to the curve in this point
	vec3 normal = get_curve_normal(t);

	// Offset along the normal
	vec3 offset = surface_width / 2.0f * normal;

	// Create one point on each side to build the surface
	float tex_u = t * tilingFactor + speed * time;
	gl_Position = Projection* View * vec4(bezier(t) - offset, 1);	v_tex_coord = vec2(tex_u, 0);	EmitVertex();
	gl_Position = Projection* View * vec4(bezier(t) + offset, 1);	v_tex_coord = vec2(tex_u, 1);	EmitVertex();
}

void main()
{
	if (instance[0] < no_of_instances)
	{
		// X points => X - 1 segments
		float step = 1.0 / (generated_points_count - 1);

		for (float t = 0.0f; t < 1.0f; t += step)
		{
			emit_points(t);
		}

		// Add the final point to the curve
		emit_points(1.0f);

		EndPrimitive();
	}
}
