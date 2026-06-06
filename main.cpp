#include "Window.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

void processInput(GLFWwindow* window);
void vertexSpec();

unsigned int VBO{};
unsigned int VAO{};

glm::vec3 cameraPos   { 0.0f, 0.0f,  3.0f };
glm::vec3 cameraTarget{ 0.0f, 0.0f, -1.0f };
glm::vec3 worldUp     { 0.0f, 1.0f,  0.0f };

float deltaTime{ 0.0f };
float lastTime{ 0.0f };

int main()
{
	constexpr int width{ 800 };
	constexpr int height{ 600 };
	std::string title{ "GeoGebra3D" };

	Window window{ width, height, title };
	window.init();

	Shader shader{ "shader.vert", "shader.frag" };

	vertexSpec();

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		processInput(window.getWindow());

		window.clear(1.0f, 0.0f, 0.0f, 1.0f);

		shader.use();

		glm::mat4 view{ glm::lookAt(cameraPos, cameraPos + cameraTarget, worldUp) };

		float currentFrame{ static_cast<float>(glfwGetTime()) };
		deltaTime = currentFrame - lastTime;
		lastTime = currentFrame;

		glm::mat4 projection{};
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		glm::mat4 model{ 1.0f };

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 2);

		glfwSwapBuffers(window.getWindow());
		glfwPollEvents();
	}
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
		glfwSetWindowShouldClose(window, true);
}

void vertexSpec()
{
	std::vector vertices
	{
		-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
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