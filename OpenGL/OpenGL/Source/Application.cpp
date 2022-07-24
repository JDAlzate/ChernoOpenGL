#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak()
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

struct ShaderProgramSource
{
	std::string vertexSource;
	std::string fragmentSource;
};

static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
	while (const GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

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

	glfwSwapInterval(1);

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
		GLCall(glGenBuffers(1, &vertexBufferId));

		// Bind the buffer index to the array buffer slot
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId));

		// Specify the data that the buffer has - in this case that means an array of positions that will not change and will be drawn to the screen
		GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

		// Specify how the data held by the buffer is supposed to be interpreted
		GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr));
		GLCall(glEnableVertexAttribArray(0)); // Enable this vertex attribute (tell the GPU that these values should be taken into consideration
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
		GLCall(glGenBuffers(1, &indexBufferId));

		// Bind the buffer index to the element array buffer slot
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId));

		// Specify the indices data that the buffer has
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * sizeof(unsigned int), indices, GL_STATIC_DRAW));
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
	GLCall(const unsigned int shaderId = glCreateShader(type));
	const char* rawSource = source.c_str();

	GLCall(glShaderSource(shaderId, 1, &rawSource, nullptr));
	GLCall(glCompileShader(shaderId));

	int result;
	GLCall(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length));

		const std::unique_ptr<char[]> message(new char[length]);
		GLCall(glGetShaderInfoLog(shaderId, length, &length, message.get()));

		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;

		GLCall(glDeleteShader(shaderId));
		return 0;
	}

	return shaderId;
}

static unsigned int CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
	GLCall(const unsigned int programId = glCreateProgram());

	const unsigned int vertexShaderId = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	const unsigned int fragmentShaderId = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	
	GLCall(glAttachShader(programId, vertexShaderId));
	GLCall(glAttachShader(programId, fragmentShaderId));

	GLCall(glLinkProgram(programId));
	GLCall(glValidateProgram(programId));

	GLCall(glDeleteShader(vertexShaderId));
	GLCall(glDeleteShader(fragmentShaderId));

	return programId;
}

int main()
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
	GLCall(glUseProgram(shaderProgramId));

	GLCall(const int location = glGetUniformLocation(shaderProgramId, "u_Color"));
	ASSERT(location != -1);

	float redValue = 0.f;
	float redValueIncrement = 0.05f;
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		GLCall(glUniform4f(location, redValue, 0.3f, 0.8f, 1.0f));
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		if (redValue > 1.f)
		{
			redValueIncrement = -0.05f;
		}
		else if (redValue < 0.f)
		{
			redValueIncrement = 0.05f;
		}

		redValue += redValueIncrement;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	GLCall(glDeleteProgram(shaderProgramId));

	glfwTerminate();
	return 0;
}