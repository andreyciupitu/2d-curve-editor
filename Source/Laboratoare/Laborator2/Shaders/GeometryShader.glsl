#version 430
layout(lines) in;
//TODO 
//prima data generati o curba (cu line strip), apoi o suprafata de rotatie/translatie cu triangle_strip
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 control_p1, control_p2, control_p3, control_p4;
uniform int no_of_instances;


in int instance[2];


vec3 rotateY(vec3 point, float u)
{
	float x = point.x * cos(u) - point.z *sin(u);
	float z = point.x * sin(u) + point.z *cos(u);
	return vec3(x, point.y, z);
}

vec3 translateX(vec3 point, float t)
{
	return vec3(point.x + t, point.y, point.z);
}

vec3 bezier(float t)
{
	return control_p1 * pow((1 - t), 3) + control_p2 * 3 * t * pow((1 - t), 2) + control_p3 * 3 * pow(t, 2) * (1 - t) + control_p4 * pow(t, 3);
}

//TODO - incercati sa creati si o curba Hermite/Bspline

void main()
{

	if (instance[0] < no_of_instances)
	{
		//TODO 
		//in loc sa emiteti varfuri reprezentand punctele de control, emiteti varfuri care sa aproximeze curba Bezier
		float step = 1.0 / 30.0f;
		for (float t = 0; t <= 1; t += step)
		{
			gl_Position = Projection* View * vec4(bezier(t), 1);	EmitVertex();
			gl_Position = Projection* View * vec4(translateX(bezier(t), 1.0f), 1);	EmitVertex();
		}
		EndPrimitive();
	}
}
