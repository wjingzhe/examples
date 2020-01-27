#version 330 core

in vec4 PositionH;
in vec2 uv;
out vec2 vTexCoord;

void main()
{
	vTexCoord = uv;
    gl_Position = PositionH;
}