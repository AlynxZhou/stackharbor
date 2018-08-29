#version 300 es
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
out vec3 pass_color;
void main()
{
	gl_Position = vec4(pos, 1.0f);
	pass_color = color;
}
