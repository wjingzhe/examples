// TestSettingOpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "vgl.h"
#include "LoadShaders.h"

#include <vermilion.h>

enum VAO_IDs
{
	Triangles,
	UVs,
	NumVAOs
};


enum Buffer_IDs
{
	ArrayBuffers,
	UVBuffers,
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


void init(void)
{
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Triangles]);

	GLfloat vertices[NumVertices][4] =
	{
		{ -0.90f, -0.90f,0.0f,1.0f }, {  0.85f, -0.90f,0.0f,1.0f }, { -0.90f,  0.85f,0.0f,1.0f },  // Triangle 1
		{  0.90f, -0.85f,0.0f,1.0f }, {  0.90f,  0.90f,0.0f,1.0f }, { -0.85f,  0.90f,0.0f,1.0f },   // Triangle 2
	};


	GLfloat uvs[NumVertices][2] = 
	{
		{1.0f,0.0f},{0.0f,1.0f},{0.0f,0.0f},
		{0.0f,0.0f},{1.0f,1.0f},{1.0f,0.0f}
	};


	glCreateBuffers(NumBuffers, Buffers);
	

	ShaderInfo shaders[] =
	{
		{GL_VERTEX_SHADER,"media/shaders/BlurDOF/triangles.vert"},
		{GL_FRAGMENT_SHADER,"media/shaders/BlurDOF/triangles.frag"},
		{GL_NONE,NULL}
	};

	vglImageData image;
	baseTexture = vglLoadTexture("media/test3.dds", 0, &image);


	GLuint program = LoadShaders(shaders);
	glUseProgram(program);



	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffers]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);


	glBindBuffer(GL_ARRAY_BUFFER, Buffers[UVBuffers]);
	glVertexAttribPointer(uv, 2, GL_FLOAT,
		GL_FALSE, 0, NULL);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(uvs), uvs, 0);
	glEnableVertexAttribArray(uv);

	auto temp = glGetUniformLocation(program, "BaseTexture");
	glUniform1i(temp, 0);
	glActiveTexture(GL_TEXTURE0);
	vglUnloadImage(&image);

}

void display(void)
{
	static const float black[] = { 0.0f,0.0f,0.0f,0.0f };

	glClearBufferfv(GL_COLOR, 0, black);
	glBindVertexArray(VAOs[Triangles]);
	glBindTexture(GL_TEXTURE_2D, baseTexture);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
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
