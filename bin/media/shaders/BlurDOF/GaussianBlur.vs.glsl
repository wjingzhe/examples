#version 450 core

//precision highp float;

in vec4 inPosition;
in vec2 uv;

out vec2 vTexCoord;

void
main()
{
	vTexCoord = uv;
    gl_Position = inPosition;
}