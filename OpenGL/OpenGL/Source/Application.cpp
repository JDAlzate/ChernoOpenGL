#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/** @returns whether the window was successfully created */
static bool InitialSetup(GLFWwindow** outWindow)
{
	/* Initialize the library */
	if (!glfwInit() || !outWindow)
	{
		return false;
	}

	/* Create a windowed mode window and its OpenGL context */
	*outWindow = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (!outWindow)
	{
		glfwTerminate();
		return false;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(*outWindow);

	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return false;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;
	return true;
}

static void InitializeVertexBuffer()
{
	// Generate a buffer and store its id - this will be our vertex buffer
	unsigned int bufferId;
	glGenBuffers(1, &bufferId);

	// Select the buffer - every operation will now affect this specific buffer
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);

	// Define the data that I want to bind to the buffer
	constexpr float positions[6] = {
		-0.5f, -0.5f,
		 0.0f,  0.5f,
		 0.5f, -0.5f
	};

	// Specify the data that the buffer has - in this case that means an array of positions that will not change and will be drawn to the screen
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

	// Specify how the data held by the buffer is supposed to be interpreted
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
	glEnableVertexAttribArray(0); // Enable this vertex attribute (tell the GPU that these values should be taken into consideration
}

static unsigned int CompileShader(const unsigned int type, const std::string& source)
{
	const unsigned int shaderId = glCreateShader(type);
	const char* rawSource = source.c_str();

	glShaderSource(shaderId, 1, &rawSource, nullptr);
	glCompileShader(shaderId);

	int result;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

		const std::unique_ptr<char[]> message(new char[length]);
		glGetShaderInfoLog(shaderId, length, &length, message.get());

		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;

		glDeleteShader(shaderId);
		return 0;
	}

	return shaderId;
}

static unsigned int CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
	const unsigned int programId = glCreateProgram();

	const unsigned int vertexShaderId = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	const unsigned int fragmentShaderId = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);
	glValidateProgram(programId);

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	return programId;
}

int main(void)
{
	GLFWwindow* window;
	if (!InitialSetup(&window))
	{
		return -1;
	}

	// Could probably have a better name
	InitializeVertexBuffer();

	const std::string vertexShaderSource = R"(
	#version 330 core

	layout(location = 0) in vec4 position;

	void main()
	{
		gl_Position = position;
	}
	)";

	const std::string fragmentShaderSource = R"(
	#version 330 core

	layout(location = 0) out vec4 color;

	void main()
	{
		color = vec4(1.0, 0.0, 0.0, 1.0);
	}
	)";

	const unsigned int shaderProgramId = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	glUseProgram(shaderProgramId);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw triangles with the specified first index and index count. This won't do anything just yet because we haven't
		// specified what kind of data the current buffer has and we haven't specified the shaders
		glDrawArrays(GL_TRIANGLES, 0, 6);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgramId);

	glfwTerminate();
	return 0;
}