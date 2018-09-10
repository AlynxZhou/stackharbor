#version 300 es
precision mediump float;
in vec3 pass_color;
out vec4 frag_color;
void main()
{
	frag_color = vec4(pass_color, 1.0f);
}
