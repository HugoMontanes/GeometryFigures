#version 330

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform mat4 normal_matrix;

layout (location = 0) in vec3 vertex_coordinates;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

out vec3 fragment_position;
out vec3 fragment_normal;
out vec3 base_color;

void main()
{
    // Transform vertex position to view space
    vec4 position = model_view_matrix * vec4(vertex_coordinates, 1.0);
    fragment_position = position.xyz;
    
    // Transform normal to view space
    fragment_normal = normalize((normal_matrix * vec4(vertex_normal, 0.0)).xyz);
    
    // Pass color to fragment shader
    base_color = vertex_color;
    
    // Set final position
    gl_Position = projection_matrix * position;
}