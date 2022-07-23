#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderProgramSource
{
	std::string vertexSource;
	std::string fragmentSource;
};

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

static void InitializeBuffers()
{
	// Vertex buffer definition
	{
		// Define the data that I want to bind to the buffer
		constexpr float positions[] = {
			-0.5f, -0.5f,
			 0.5f, -0.5f,
			 0.5f,  0.5f,
			-0.5f,  0.5f
		};

		// Generate a buffer and store its id - this will be our vertex buffer
		unsigned int vertexBufferId;
		glGenBuffers(1, &vertexBufferId);

		// Bind the buffer index to the array buffer slot
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

		// Specify the data that the buffer has - in this case that means an array of positions that will not change and will be drawn to the screen
		glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

		// Specify how the data held by the buffer is supposed to be interpreted
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
		glEnableVertexAttribArray(0); // Enable this vertex attribute (tell the GPU that these values should be taken into consideration
	}

	// Index buffer definition
	{
		// Define the indices I want to bind to the buffer
		constexpr unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
		};

		// Generate a buffer and store its id - this will be our index buffer
		unsigned int indexBufferId;
		glGenBuffers(1, &indexBufferId);

		// Bind the buffer index to the element array buffer slot
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);

		// Specify the indices data that the buffer has
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * sizeof(unsigned int), indices, GL_STATIC_DRAW);


	}
}

static ShaderProgramSource ParseShader(const std::string& filePath)
{
	std::ifstream stream(filePath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};
	ShaderType type = ShaderType::NONE;

	std::string line;
	std::stringstream stringStreams[2];

	while (std::getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else if (type != ShaderType::NONE)
		{
			stringStreams[(int)type] << line << '\n';
		}
	}

	return { stringStreams[0].str(), stringStreams[1].str() };
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
	InitializeBuffers();

	const ShaderProgramSource source = ParseShader("Resources/Shaders/Basic.shader");
	const unsigned int shaderProgramId = CreateShaderProgram(source.vertexSource, source.fragmentSource);

	glUseProgram(shaderProgramId);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw the triangles specified by the vertex buffer and the index buffer
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgramId);

	glfwTerminate();
	return 0;
}