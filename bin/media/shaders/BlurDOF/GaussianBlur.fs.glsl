#version 450 core

//precision mediump float;

in vec2 vTexCoord;


uniform vec2 uDirection = vec2(0.1f,0);
uniform int uBlurSize=5;

uniform sampler2D inTexture;

uniform float WeightMap[32];

out vec4 OutColor;

void main()
{
	
	vec4 pixVal = vec4(0.0,0.0,0.0,1.0f);
	float total = 0.0;
	
	for (int i = -uBlurSize; i <= uBlurSize; ++i)
	{
		vec2 uvOffset = i*uDirection;
		
		float g = WeightMap[i + uBlurSize];
		
		pixVal += g * texture(inTexture,(uvOffset+vTexCoord));
		total+= g;
	}

	//OutColor = vec4(pixVal.xyz/total,texture(inTexture,vTexCoord).a);
	OutColor = pixVal/total;
	//OutColor = vec4(1,0,0,1);
}