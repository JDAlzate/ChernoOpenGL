#include <GLFW/glfw3.h>

int main(void)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* Window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (!Window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(Window);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(Window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLES);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.0f, 0.5f);
		glVertex2f(0.5f, -0.5f);
		glEnd();

		/* Swap front and back buffers */
		glfwSwapBuffers(Window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}