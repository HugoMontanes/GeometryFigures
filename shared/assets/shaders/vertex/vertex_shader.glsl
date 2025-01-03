#version 330

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform mat4 normal_matrix;

layout (location = 0) in vec3 vertex_coordinates;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

out vec3 front_color;

void main()
{
	vec4 normal = normal_matrix * vec4(vertex_normal, 0.0);
	vec4 position = model_view_matrix * vec4(vertex_coordinates, 1.0);
	gl_Position = projection_matrix * position;
	front_color = vertex_color;
};