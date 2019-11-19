#version 330 core

uniform sampler2D BaseTexture;

in vec2 vTexCoord;

out vec4 fColor;

void main()
{
    fColor = texture(BaseTexture, vTexCoord);
}
