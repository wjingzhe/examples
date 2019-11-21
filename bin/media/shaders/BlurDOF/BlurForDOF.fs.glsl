#version 450 core

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
//	float tempDepth = g_TextureDepth.Load(uvec3(tempX*g_RenderTargetSize.x, tempY*g_RenderTargetSize.y, 0)).x;
float tempDepth = 1.0f;
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
	
	
	vec4 color = vec4(0.0);
	
	if(abs(PosW.x)<=0.05f)
	{
		OutColor = texture2D(g_TextureClear, vTexCoord.xy);
	}
	else
	{
		OutColor = mix(texture2D(g_TextureClear, vTexCoord.xy),texture2D(g_TextureBlur, vTexCoord.xy),(abs(PosW.x)-0.05f)/0.05f);
	}
}