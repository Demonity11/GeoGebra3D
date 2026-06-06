#include "Window.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
void vertexSpec();

unsigned int VBO{};
unsigned int VAO{};

glm::vec3 cameraPos   { 0.0f, 0.5f,  3.0f };
glm::vec3 cameraTarget{ 0.0f, 0.0f, 0.0f };
glm::vec3 worldUp     { 0.0f, 1.0f,  0.0f };

float lastX{ 400.0f };
float lastY{ 300.0f };

float yaw{};
float pitch{};

bool isPressingRightClick{ false };
bool isFirstMouse{ true };

int main()
{
	constexpr int width{ 800 };
	constexpr int height{ 600 };
	std::string title{ "GeoGebra3D" };

	Window window{ width, height, title };
	window.init();

	Shader shader{ "shader.vert", "shader.frag" };

	glfwSetMouseButtonCallback(window.getWindow(), mouse_button_callback);
	glfwSetCursorPosCallback(window.getWindow(), mouse_cursor_callback);

	vertexSpec();

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		processInput(window.getWindow());

		window.clear(0.6f, 0.3f, 0.0f, 1.0f);

		shader.use();

		glm::mat4 view{ glm::lookAt(cameraPos, cameraTarget, worldUp) };

		//view = glm::rotate(view, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		//view = glm::rotate(view, pitch, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 projection{};
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		glm::mat4 model{ 1.0f };

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 6);

		glfwSwapBuffers(window.getWindow());
		glfwPollEvents();
	}
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
		glfwSetWindowShouldClose(window, true);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			isPressingRightClick = true;
			isFirstMouse = true;
		}

		if (action == GLFW_RELEASE)
			isPressingRightClick = false;
	}
}

void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!isPressingRightClick)
		return;

	if (isFirstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		isFirstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = ypos - lastY;

	lastX = xpos;
	lastY = ypos;

	const float sensitivity{ 0.01f };

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;

	if (pitch < -89.0f)
		pitch = -89.0f;

	const float radius{ 2.0f };

	cameraPos.x = cameraTarget.x + radius * cos(yaw) * cos(pitch);
	cameraPos.y = cameraTarget.y + radius * sin(pitch);
	cameraPos.z = cameraTarget.z + radius * sin(yaw) * cos(pitch);

	std::cout << "x = " << cameraPos.x << ", y = " << cameraPos.y << ", z = " << cameraPos.z << "\n";
}

void vertexSpec()
{
	std::vector vertices
	{
		-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

		 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

		 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	int stride{ 6 * sizeof(float) };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
}