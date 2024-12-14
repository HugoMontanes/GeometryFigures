#version 330

in vec3 front_color;
out vec4 fragment_color;

void main()
{
	fragment_color = vec4(front_color, 1.0);
};