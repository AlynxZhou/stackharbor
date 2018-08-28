#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

#include <stdlib.h>
#include <string.h>
#define LINE_LENGTH 512

GLuint make_shader(GLenum type, const char *const shader_src)
{
	GLuint shader = 0;
	GLint compiled = 0;
	// 创建 shader。
	if (!(shader = glCreateShader(type))) {
	    fprintf(stderr, "Shader Create Error.\n");
	    return 0;
	}
	// 加载 shader 内容。
	glShaderSource(shader, 1, &shader_src, NULL);
	// 开始编译。
	glCompileShader(shader);
	// 检查编译结果。
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	// 编译失败的话读取错误输出。
	if (!compiled) {
	    GLint info_len = 0;
	    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
	    if (info_len) {
		char *info = malloc(info_len * sizeof(*info));
		if (!info) {
			fprintf(stderr, "Malloc Error.\n");
			glDeleteShader(shader);
			return 0;
		}
		glGetShaderInfoLog(shader, info_len, NULL, info);
		fprintf(stderr, "Compile Error: %s\n", info);
		free(info);
	    }
	    glDeleteShader(shader);
	    return 0;
	}
	return shader;
}

GLuint load_shader(GLenum type, const char *const shader_path)
{
	GLuint result = 0;
	FILE *fp = NULL;
	size_t file_length = 0;
	char *file_content = NULL;
	char temp_line[LINE_LENGTH];
	// 读取文件不用解释了吧……
	if (!(fp = fopen(shader_path, "r"))) {
		fprintf(stderr, "Open file %s failed.\n", shader_path);
		return 0;
	}
	fseek(fp, 0l, SEEK_END);
	file_length = ftell(fp);
	rewind(fp);
	if (!(file_content = malloc(file_length))) {
		fprintf(stderr, "Malloc Error.\n");
		fclose(fp);
		return 0;
	}
	file_content[0] = '\0';
	while (fgets(temp_line, LINE_LENGTH, fp))
		strncat(file_content, temp_line, LINE_LENGTH);
	fclose(fp);
	result = make_shader(type, file_content);
	free(file_content);
	return result;
}

GLuint load_program(const char *const vshader_path, const char *const fshader_path)
{
	GLint linked = 0;
	GLuint vshader = 0;
	GLuint fshader = 0;
	GLuint program = 0;
	// 加载类型为 GL_VERTEX_SHADER。
	vshader = load_shader(GL_VERTEX_SHADER, vshader_path);
	// 加载类型为 GL_FRAGMENT_SHADER。
	fshader = load_shader(GL_FRAGMENT_SHADER, fshader_path);
	if (!vshader || !fshader)
		return 0;
	// 创建 Shader Program。
	if (!(program = glCreateProgram())) {
		fprintf(stderr, "Program Create Error.\n");
		return 0;
	}
	// 给 Program 添加 Shader。
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	// 将两个 Shader 链接成 Program。
	glLinkProgram(program);
	// 链接完成可以删除 Shader 了。
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	// 检查链接状态。
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	// 链接失败读取错误输出。
	if (!linked) {
		GLint info_len = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
		if (info_len) {
			char *info = malloc(info_len * sizeof(*info));
			if (!info) {
				fprintf(stderr, "Malloc Error.\n");
				glDeleteProgram(program);
				return 0;
			}
			glGetProgramInfoLog(program, info_len, NULL, info);
			fprintf(stderr, "Link Error: %s\n", info);
			free(info);
		}
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

int main(int argc, char *argv[])
{
	glfwInit();
	unsigned int width = 800;
	unsigned int height = 600;
	GLFWwindow *window = glfwCreateWindow(width, height, "learn-gles", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	glViewport(0, 0, width, height);

	unsigned int program = load_program("triangle.v.glsl", "triangle.f.glsl");

	const float vertices[] = {
		// 坐标	       颜色
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	unsigned int vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(window);
	}

	glDeleteBuffers(1, &buffer);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteProgram(program);

	glfwTerminate();
	return 0;
}
