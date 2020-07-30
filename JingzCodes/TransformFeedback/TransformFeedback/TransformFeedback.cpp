//jingz
// 1.先使用glBeginTransformFeedback将模型顶点数据画在geometry_vbo中;
// 2.特效的update_vs中，从geometry_vbo获取模型每3个顶点组成一个几何面和特效进行相交测试并根据时间来更新位置;
// 3.有两个vbo,分别用于生成特效点，并用tik-tok结构读取写入TransformFeedback的数据缓存着；在对应的fs执行过程中会绘制结果出来
//
#include "pch.h"

#include "vapp.h"
#include "vutils.h"

#include "LoadShaders.h"

#include "vmath.h"

#include "vbm.h"
#include <stdio.h>

BEGIN_APP_DECLARATION(TransformFeedbackExample)

//Override functions from base class
virtual void Initialize(const char * title);
virtual void Display(bool auto_redraw);
virtual void Finalize(void);
virtual void Resize(int width, int height);

//Member variables
float aspect;
GLuint update_program;
GLuint vao[2];
GLuint vbo[2];
GLuint xfb;

GLuint render_program;
GLuint geometry_vbo;
GLuint render_vao;
GLuint render_model_matrix_loc;
GLuint render_projection_matrix_loc;


GLuint geometry_tex;

GLuint model_matrix_loc;
GLuint projection_matrix_loc;
GLuint triangle_count_loc;
GLuint time_step_loc;

VBObject object;

END_APP_DECLARATION();

//main函数和主循环
DEFINE_APP(TransformFeedbackExample, "TransformFeedback Example")

const int point_count = 5000;
static unsigned int seed = 0x13371337;

static inline float random_float()
{
	float res;

	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

static vmath::vec3 random_vector(float minmag = 0.0f, float maxmag = 1.0f)
{
	vmath::vec3 randomVec(random_float()*2.0f - 1.0f, random_float()*2.0f - 1.0f, random_float()*2.0f - 1.0f);
	randomVec = normalize(randomVec);
	randomVec *= (random_float()*(maxmag - minmag) + minmag);

	return randomVec;
}

void TransformFeedbackExample::Initialize(const char * title)
{
	int i, j;

	base::Initialize(title);

	////////////////////////////////////
	/* update_program */
	////////////////////

	ShaderInfo DisplayTextureShaders[] =
	{
		{ GL_VERTEX_SHADER,"media/shaders/TransformFeedback/TransformFeedback.vert" },
		{ GL_FRAGMENT_SHADER,"media/shaders/TransformFeedback/TransformFeedback.frag" },
		{ GL_NONE,NULL }
	};

	update_program = LoadShaders(DisplayTextureShaders);

	static const char * varyings[] =
	{
		"position_out","velocity_out"
	};

	glTransformFeedbackVaryings(update_program, 2, varyings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(update_program);
	glUseProgram(update_program);

	model_matrix_loc = glGetUniformLocation(update_program, "model_matrix");
	projection_matrix_loc = glGetUniformLocation(update_program, "projection_matrix");
	triangle_count_loc = glGetUniformLocation(update_program, "triangle_count");
	time_step_loc = glGetUniformLocation(update_program, "time_step");


	////////////////////////////////////
	/* render_program */
	////////////////////
	ShaderInfo RenderShaders[] =
	{
		{ GL_VERTEX_SHADER,"media/shaders/TransformFeedback/render.vert" },
		{ GL_FRAGMENT_SHADER,"media/shaders/TransformFeedback/blue.frag" },
		{ GL_NONE,NULL }
	};

	render_program = LoadShaders(RenderShaders);

	static const char * varyings2[] = 
	{
		"world_space_position"
	};

	glTransformFeedbackVaryings(render_program, 1, varyings2, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(render_program);
	glUseProgram(render_program);

	render_model_matrix_loc = glGetUniformLocation(render_program, "model_matrix");
	render_projection_matrix_loc = glGetUniformLocation(render_program, "projection_matrix");


	////////////////////////////////////
	/* init */
	////////////////////

	glGenVertexArrays(2, vao);
	glGenBuffers(2, vbo);


	for (i = 0;i<2;i++)
	{
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, vbo[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, point_count*(sizeof(vmath::vec4)+sizeof(vmath::vec3)),NULL,GL_DYNAMIC_COPY );//Copy-数据从一个OpenGL缓冲区读取,然后作为顶点数据,用于渲染

		if (i==0)
		{
			struct buffer_t
			{
				vmath::vec4 position;
				vmath::vec3 velocity;
			}* buffer = (buffer_t*)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,GL_WRITE_ONLY);

			for (j = 0;j<point_count;++j)
			{
				buffer[j].velocity = random_vector();
				buffer[j].position = vmath::vec4(
					buffer[j].velocity + vmath::vec3(-0.5f, 40.f, 0.0f), 1.0f);

				buffer[j].velocity = vmath::vec3(buffer[j].velocity[0], buffer[j].velocity[1] * 0.3f,
					buffer[j].velocity[2] * 0.3f);

			}//for

			glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
		}//if


		glBindVertexArray(vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4) + sizeof(vmath::vec3), NULL);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4) + sizeof(vmath::vec3), (GLvoid*)sizeof(vmath::vec4));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

	//创建纹理缓冲区，并绑定到纹理管理对象
	glGenBuffers(1, &geometry_vbo);
	glGenTextures(1, &geometry_tex);

	glBindBuffer(GL_TEXTURE_BUFFER, geometry_vbo);
	glBufferData(GL_TEXTURE_BUFFER, 1024 * 2014 * sizeof(vmath::vec4), NULL, GL_DYNAMIC_COPY);
	glBindTexture(GL_TEXTURE_BUFFER, geometry_tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, geometry_vbo);// 需要将纹理缓冲区对象绑定到纹理上才有用,绑定时候可以设置访问格式


	//将数组输入
	glGenVertexArrays(1, &render_vao);
	glBindVertexArray(render_vao);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);//但是只有gl_Position数据项
	glEnableVertexAttribArray(0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	object.LoadFromVBM("media/armadillo_low.vbm", 0, 1, 2);
}


static inline int min(int a, int b)
{
	return a < b ? a : b;
}

void TransformFeedbackExample::Display(bool auto_redraw)
{
	static int frame_count = 0;

	float t = float(app_time() & 0x3FFFF) / float(0x3FFFF);

	static float q = 0.0f;

	static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
	static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
	static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);

	
	vmath::mat4 model_matrix( vmath::scale(0.3f)*
		vmath::rotate(t*360.0f,0.0f,1.0f,0.0f)*
		vmath::rotate(t*360.0f*3.0f,0.0f,0.0f,1.0f)
	);
	vmath::mat4 projection_matrix(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5000.0f)
		*vmath::translate(0.0f, 0.0f, -100.0f)
	);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(render_program);
	glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
	glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
	
	glBindVertexArray(render_vao);
	
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, geometry_vbo);

	glBeginTransformFeedback(GL_TRIANGLES);
	object.Render();
	glEndTransformFeedback();



	glUseProgram(update_program);
	model_matrix = vmath::mat4::identity();
	glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);
	glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);
	glUniform1i(triangle_count_loc, object.GetVertexCount() / 3);

	if (t>q)
	{
		glUniform1f(time_step_loc, (t - q)*2000.0f);
	}

	q = t;

	if ((frame_count & 1)!=0 )
	{
		glBindVertexArray(vao[1]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[0]);
	}
	else
	{
		glBindVertexArray(vao[0]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[1]);
	}

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, min(point_count, (frame_count >> 3)));
	glEndTransformFeedback();

	glBindVertexArray(0);

	frame_count++;

	base::Display();

}

void TransformFeedbackExample::Finalize(void)
{
	glUseProgram(0);
	glDeleteProgram(update_program);
	glDeleteVertexArrays(2, vao);
	glDeleteBuffers(2, vbo);
}

void TransformFeedbackExample::Resize(int width, int height)
{
	glViewport(0, 0, width, height);

	aspect = float(height) / float(width);
}