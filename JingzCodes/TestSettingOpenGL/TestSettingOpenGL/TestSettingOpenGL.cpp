// TestSettingOpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "vgl.h"
#include "LoadShaders.h"

#include <vermilion.h>
#include <cmath>

enum VAO_IDs
{
	Triangles,
	Sphere,
	NumVAOs
};


enum Buffer_IDs
{
	Postion,
	UV,
	NumBuffers
};

enum Attrib_IDs
{
	vPosition = 0,
	uv,
};


GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

const GLuint NumVertices = 6;


GLuint baseTexture;
static const float black[] = { 0.0f,0.0f,0.0f,0.0f };
GLsizei TexWidth = 800, TexHeight = 600;
GLuint frameBuffer, texture1, texture2;


GLuint DisplayTexture_program;
GLuint GaussianBlur_program;

int BlurSize = 3;
float GaussianWeight1D[31];

void InitBaseScreenGeometry()
{
	GLfloat vertices[NumVertices][4] =
	{
		{ -1.0f, -1.0f,0.0f,1.0f }, {  1.0f, -1.0f,0.0f,1.0f }, { -1.0f,  1.0f,0.0f,1.0f },  // Triangle 1
		{  1.0f, -1.0f,0.0f,1.0f }, {  1.0f,  1.0f,0.0f,1.0f }, { -1.0f,  1.0f,0.0f,1.0f },   // Triangle 2
	};


	GLfloat uvs[NumVertices][2] =
	{
		{0.0f,1.0f},{1.0f,1.0f},{0.0f,0.0f},
		{1.0f,1.0f},{1.0f,0.0f},{0.0f,0.0f},
	};

	glBindVertexArray(VAOs[Triangles]);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Postion]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[UV]);
	glVertexAttribPointer(uv, 2, GL_FLOAT,
		GL_FALSE, 0, NULL);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(uvs), uvs, 0);
	glEnableVertexAttribArray(uv);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InitToDisplaySimpleTexture()
{

	ShaderInfo DisplayTextureShaders[] =
	{
		{GL_VERTEX_SHADER,"media/shaders/BlurDOF/DisplayTexture.vert"},
		{GL_FRAGMENT_SHADER,"media/shaders/BlurDOF/DisplayTexture.frag"},
		{GL_NONE,NULL}
	};

	DisplayTexture_program = LoadShaders(DisplayTextureShaders);
}


void InitGaussianBlur()
{

	ShaderInfo GaussianBlurShaders[] =
	{
		{GL_VERTEX_SHADER,"media/shaders/BlurDOF/GaussianBlur.vs.glsl"},
		{GL_FRAGMENT_SHADER,"media/shaders/BlurDOF/GaussianBlur.fs.glsl"},
		{GL_NONE,NULL}
	};

	GaussianBlur_program = LoadShaders(GaussianBlurShaders);

	auto count = BlurSize * 2 + 1;
	memset(GaussianWeight1D, 0, 31);



	for (int i =0;i<count;++i)
	{
		auto x = i - BlurSize;
		GaussianWeight1D[i] = std::exp((-1.0f*x*x) / (0.5f*BlurSize*BlurSize));
	}


	glCreateTextures(GL_TEXTURE_2D, 1, &texture1);
	glTextureStorage2D(texture1, 1, GL_RGBA32F, TexWidth, TexHeight);
	glBindImageTexture(0, texture1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glCreateTextures(GL_TEXTURE_2D, 1, &texture2);
	glTextureStorage2D(texture2, 1, GL_RGBA32F, TexWidth, TexHeight);
	glBindImageTexture(1, texture2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	glCreateFramebuffers(1, &frameBuffer);
}

void init(void)
{
	
	glGenVertexArrays(NumVAOs, VAOs);
	glCreateBuffers(NumBuffers, Buffers);

	InitBaseScreenGeometry();


	auto str = glGetError();
	InitToDisplaySimpleTexture();
	auto str1 = glGetError();
	InitGaussianBlur();
	auto str2 = glGetError();

	//vglImageData image;
	baseTexture = vglLoadTexture("media/test3.dds", 0, NULL);
	//vglUnloadImage(&image);





}

void DisplayTexture(GLuint inTexture)
{
	//DisplayTexture
	glUseProgram(DisplayTexture_program);
	glClearBufferfv(GL_COLOR, 0, black);
	glBindVertexArray(VAOs[Triangles]);
	glActiveTexture(GL_TEXTURE0);

	glUniform1i(glGetUniformLocation(GaussianBlur_program, "BaseTexture"), 0);
	glBindTexture(GL_TEXTURE_2D, inTexture);
	glEnable(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

void DrawGaussianBlur(GLuint inTexture, GLuint outTexture, GLuint frameBuffer)
{
	glUseProgram(GaussianBlur_program);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glNamedFramebufferTexture(frameBuffer, GL_COLOR_ATTACHMENT0, texture1, 0);

	glViewport(0, 0, TexWidth, TexWidth);
	glClearBufferfv(GL_COLOR, 0, black);
	glClearDepth(0.0f);

	//glGenerateMipmap(texture);
	
	glEnable(GL_TEXTURE_2D);


	glBindFragDataLocation(GaussianBlur_program, 0, "Color");

//	auto str8 = glGetError();
	glClearBufferfv(GL_COLOR, 0, black);
	glBindVertexArray(VAOs[Triangles]);
	//auto str5 = glGetError();

	auto tex0 = glGetUniformLocation(GaussianBlur_program, "inTexture");
	glBindTexture(GL_TEXTURE_2D, inTexture);
	glActiveTexture(GL_TEXTURE0);
	auto str24 = glGetError();




	auto uDirectionLoc = glGetUniformLocation(GaussianBlur_program, "uDirection");
	float dir[2] = { 1.0f / 800.0f, 0.0f };
	glUniform2fv(uDirectionLoc, 1, dir);
	//auto str23 = glGetError();

	auto uBlurSizeLoc = glGetUniformLocation(GaussianBlur_program, "uBlurSize");
	glUniform1i(uBlurSizeLoc, BlurSize);

	//
	//
	auto uWeightMapLoc = glGetUniformLocation(GaussianBlur_program, "WeightMap");
	glUniform1fv(uWeightMapLoc, BlurSize * 2 + 1, GaussianWeight1D);

	
	glEnable(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);


	glNamedFramebufferTexture(frameBuffer, GL_COLOR_ATTACHMENT0, texture2, 0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE0);
	dir[0] = 0.0f;
	dir[1] = 1.0f / 800.0f;
	glUniform2fv(uDirectionLoc, 1, dir);

	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

}

void display(void)
{

	DrawGaussianBlur(baseTexture,texture1, frameBuffer);
	
	DisplayTexture(texture2);
	
}


int main(int argc, char** argv)
{

	glfwInit();

	GLFWwindow* pWindow = glfwCreateWindow(800, 600, "Triangles", NULL, NULL);

	glfwMakeContextCurrent(pWindow);
	gl3wInit();

	init();
    
	while (!glfwWindowShouldClose(pWindow))
	{
		display();

		glfwSwapBuffers(pWindow);

		glfwPollEvents();
	}

	glfwDestroyWindow(pWindow);

	glfwTerminate();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
