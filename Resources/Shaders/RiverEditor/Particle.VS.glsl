#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;

uniform float delta_time;
uniform vec3 fall_speed;
uniform float decay_radius;

struct Particle
{
	vec4 position;
	vec4 speed;
	vec4 iposition;
	vec4 ispeed;
};

layout(std430, binding = 0) buffer particles {
	Particle data[];
};

// Rand in [0, 1)
float rand(vec2 co)
{
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	vec3 pos = data[gl_VertexID].position.xyz;
	vec3 spd = data[gl_VertexID].speed.xyz;

	pos = pos + spd * delta_time + fall_speed * delta_time * delta_time/2.0f;
	spd = spd + fall_speed * delta_time;

	if(abs(pos.y) > (decay_radius + rand(pos.xy) * length(spd)))
	{
		pos = data[gl_VertexID].iposition.xyz;
		spd = data[gl_VertexID].ispeed.xyz;
	}
	
	if(abs(pos.x) > (decay_radius + rand(pos.xy) * length(spd)))
	{
		pos = data[gl_VertexID].iposition.xyz;
		spd = data[gl_VertexID].ispeed.xyz;
	}
	data[gl_VertexID].position.xyz =  pos;
	data[gl_VertexID].speed.xyz =  spd;

	gl_Position = Model * vec4(pos, 1);
}


