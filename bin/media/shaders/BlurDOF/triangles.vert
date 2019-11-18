
#version 400 core

layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 uv;

varying vec2 vTexCoord;

void
main()
{
	vTexCoord = uv;
    gl_Position = vPosition;
}
