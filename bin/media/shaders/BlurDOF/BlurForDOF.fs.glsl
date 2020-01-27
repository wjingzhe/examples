#version 330 core

precision highp float;

in vec2 vTexCoord;
out vec4 OutColor;


uniform vec2 g_RenderTargetSize;
uniform float g_radioWtoH;
uniform float g_NearZ = 1;
uniform float g_RangZ = 1000;
uniform float g_tanHalfFovY;

uniform float minRadius = 50.0f;


uniform mat4x4 g_mViewInv;

uniform sampler2D g_TextureClear;
uniform sampler2D g_TextureBlur;
uniform sampler2D g_TextureDepth;

vec3 ConvertScreenToNdc(vec2 screenXY)
{
	float tempX = screenXY.x - 0.5f;
	float tempY = screenXY.y - 0.5f;
	float tempDepth = texture2D(g_TextureDepth,vec2(tempX,tempY)).x;
//float tempDepth = 1.0f;
	//float ndcX = tempX / g_RenderTargetSize.x - 1.0f;
	//float ndcY = -tempY / g_RenderTargetSize.y + 1.0f;

	return vec3(tempX, tempY, tempDepth);
}


vec3 ConvertNdcToProj(vec3 ndcPos3)
{
	vec3 projPos3;
	projPos3.x = ndcPos3.x*g_radioWtoH;
	projPos3.y = ndcPos3.y;
	projPos3.z = ndcPos3.z;
	return projPos3;
}


vec3 ConvertProjToView(vec3 projPos3)
{
	float viewdDepth = g_NearZ*g_RangZ / (g_RangZ - projPos3.z);
	vec3 viewPos3;
	float k = viewdDepth*g_tanHalfFovY;
	viewPos3.xy = projPos3.xy*k;
	viewPos3.z = viewdDepth;
	return viewPos3;
}



void main()
{
//gl_TexCoord[0].st

	vec3 ndcPos3 = ConvertScreenToNdc(vTexCoord.xy);
	vec3 projPos3 = ConvertNdcToProj(ndcPos3);
	vec3 viewPos3 = ConvertProjToView(projPos3);
	vec4 PosW = g_mViewInv*vec4(viewPos3,1.0f);
	
	vec2 uv = vec2(vTexCoord.x,1-vTexCoord.y);
	float tempDepth = texture2D(g_TextureDepth,uv).x;
	//float tempDepth = g_NearZ*g_RangZ / (g_RangZ - texture2D(g_TextureDepth,uv).x);
	
	vec4 color = vec4(0.0);
	int j = 0;
	
	vec2 uv2 = uv+vec2(1.0f/g_RenderTargetSize.x,0.0f);
	vec2 uv4 = uv+vec2(1.0f/g_RenderTargetSize.x,1.0f/g_RenderTargetSize.y);
	vec2 uv8 = uv+vec2(1.0f/g_RenderTargetSize.x,-1.0f/g_RenderTargetSize.y);
		
	vec2 uv6 = uv+vec2(0,1.0f/g_RenderTargetSize.y);
	vec2 uv7 = uv+vec2(0,-1.0f/g_RenderTargetSize.y);

	vec2 uv9 = uv+vec2(-1.0f/g_RenderTargetSize.x,0.0f);
	vec2 uv3 = uv+vec2(-1.0f/g_RenderTargetSize.x,1.0f/g_RenderTargetSize.y);
	vec2 uv5 = uv+vec2(-1.0f/g_RenderTargetSize.x,-1.0f/g_RenderTargetSize.y);


	//if(texture2D(g_TextureDepth,uv).x>tempDepth)
	{
	//	color+=
	}

	//if()
	
	//OutColor = vec4(texture2D(g_TextureDepth,uv).x,texture2D(g_TextureDepth,uv).x,texture2D(g_TextureDepth,uv).x,1.0f);
	//OutColor = texture2D(g_TextureDepth,vec2(vTexCoord.x,1.0f-vTexCoord.y)).rrrr;
	
	if(tempDepth<0.166f)
	{
		OutColor = texture2D(g_TextureClear, vec2(vTexCoord.x,1.0f-vTexCoord.y));
	}
	else
	{
	//	OutColor = texture2D(g_TextureBlur, vec2(vTexCoord.x,1.0f-vTexCoord.y));
		OutColor = mix(texture2D(g_TextureBlur, vec2(vTexCoord.x,1.0f-vTexCoord.y)),texture2D(g_TextureClear, vec2(vTexCoord.x,1.0f-vTexCoord.y)),tempDepth);
	}
}