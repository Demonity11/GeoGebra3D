#include "Window.h"
#include "Shader.h"
#include "draw_utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <vector>

void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);

// interface.cpp
void initializeImGui(GLFWwindow* window);
void getUserInput();

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

std::vector<float> vertexData;

int main()
{
	constexpr int width{ 960 };
	constexpr int height{ 540 };
	std::string title{ "GeoGebra3D" };

	Window window{ width, height, title };
	window.init();

	Shader shader{ "shader.vert", "shader.frag" };

	glfwSetMouseButtonCallback(window.getWindow(), mouse_button_callback);
	glfwSetCursorPosCallback(window.getWindow(), mouse_cursor_callback);

	drawCilinder();
	vertexSpec(vertexData);

	//for (int i{ 0 }; i < vertexData.size(); ++i)
	//{
	//	std::cout << vertexData[i] << " ";
	//}

	//std::cout << "\n\n" << vertexData.size();

	initializeImGui(window.getWindow());

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		processInput(window.getWindow());

		// ImGui stuff
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		window.clear(0.6f, 0.3f, 0.0f, 1.0f);

		shader.use();

		glm::mat4 view{ glm::lookAt(cameraPos, cameraTarget, worldUp) };

		glm::mat4 projection{};
		projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

		glm::mat4 model{ 1.0f };

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, vertexData.size() / 6);

		// render ImGui here
		ImGui::ShowDemoWindow();
		getUserInput();

		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window.getWindow());
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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

	const float radius{ 3.0f };

	cameraPos.x = cameraTarget.x + radius * cos(yaw) * cos(pitch);
	cameraPos.y = cameraTarget.y + radius * sin(pitch);
	cameraPos.z = cameraTarget.z + radius * sin(yaw) * cos(pitch);
}

void vertexSpec(const std::vector<float>& vertices)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	int stride{ 6 * sizeof(float) };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
}

void updateBufferData(const std::vector<float>& vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
}

void getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec3 color, float radius, std::vector<float>& vertexData)
{
	glm::vec3 direction{ p - p0 };

	float length{ glm::length(direction) };

	if (length == 0.0f)
		return;

	direction = glm::normalize(direction);

	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

	// changes worldUp accordingly with the direction vector, because the cross product between vectors with the same direction is a null vector 
	float cosTheta = glm::dot(direction, worldUp);
	if (glm::abs(cosTheta) > 0.999f)
	{
		if (cosTheta > 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

		if (cosTheta < 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 right{ glm::normalize(glm::cross(direction, worldUp)) };
	glm::vec3 up{ glm::cross(right, direction) };

	const float linesDensity{ 360.0f / 72.0f };
	for (float angle{ 0.0f }; angle <= 360.0f; angle += linesDensity)
	{
		float rad{ glm::radians(angle) };

		glm::vec3 a{ p0 + radius * cos(rad) * right + radius * sin(rad) * up };
		glm::vec3 b{ a + length * direction };

		vertexData.push_back(a.x);
		vertexData.push_back(a.y);
		vertexData.push_back(a.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);

		vertexData.push_back(b.x);
		vertexData.push_back(b.y);
		vertexData.push_back(b.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
	}

	//std::cout << "direction: " << direction.x << ", " << direction.y << ", " << direction.z << "\n";
	//std::cout << "right: " << right.x << ", " << right.y << ", " << right.z << "\n";
	//std::cout << "up: " << up.x << ", " << up.y << ", " << up.z << "\n";
	//std::cout << "color: " << color.x << ", " << color.y << ", " << color.z << "\n";
}

void drawRings(glm::vec3 p0, glm::vec3 p, glm::vec3 color, std::vector<float>& vertexData)
{
	glm::vec3 direction{ p - p0 };

	float ringWidth{ glm::length(direction) };

	if (ringWidth == 0.0f)
		return;

	direction = glm::normalize(direction);

	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

	// changes worldUp accordingly with the direction vector, because the cross product between vectors with the same direction is a null vector 
	float cosTheta = glm::dot(direction, worldUp);
	if (glm::abs(cosTheta) > 0.999f)
	{
		if (cosTheta > 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

		if (cosTheta < 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 right{ glm::normalize(glm::cross(direction, worldUp)) };
	glm::vec3 up{ glm::cross(right, direction) };

	const float linesDensity{ 360.0f / 72.0f };
	const float ringRadius{ 0.002f };
	float stride{ 0.1f };
	const int ringCount{ 20 };

	for (int i{ 0 }; i < ringCount; ++i)
	{
		p0 += direction * stride;

		for (float angle{ 0.0f }; angle <= 360.0f; angle += linesDensity)
		{
			float rad{ glm::radians(angle) };

			glm::vec3 a{ p0 + ringRadius * cos(rad) * right + ringRadius * sin(rad) * up };
			glm::vec3 b{ a + ringWidth * direction };

			vertexData.push_back(a.x);
			vertexData.push_back(a.y);
			vertexData.push_back(a.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);

			vertexData.push_back(b.x);
			vertexData.push_back(b.y);
			vertexData.push_back(b.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
		}
	}
}

void drawCilinder()
{
	std::vector axisVertices
	{
		-1.0f, 0.0f, 0.0f,
		 1.0f, 0.0f, 0.0f,

		 0.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,

		 0.0f, 0.0f, -1.0f,
		 0.0f, 0.0f,  1.0f
	};

	std::vector axisColors
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	for (int v{ 0 }, c{ 0 }; v < axisVertices.size(); v += 6, c += 3)
	{
		glm::vec3 a{ axisVertices[v], axisVertices[v + 1], axisVertices[v + 2] };
		glm::vec3 b(axisVertices[v + 3], axisVertices[v + 4], axisVertices[v + 5]);
		glm::vec3 color{ axisColors[c], axisColors[c + 1], axisColors[c + 2] };

		getCilinderVertices(a, b, color, 0.001f ,vertexData);
	}

	float ringWidth{ 0.002f };
	std::vector ringVertices
	{
		-1.0f, 0.0f, 0.0f,
		-1.0f +ringWidth, 0.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f + ringWidth, 0.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f + ringWidth
	};

	glm::vec3 ringColor{ 0.0f, 0.0f, 0.0f };

	for (int v{ 0 }; v < axisVertices.size(); v += 6)
	{
		glm::vec3 a{ ringVertices[v], ringVertices[v + 1], ringVertices[v + 2] };
		glm::vec3 b(ringVertices[v + 3], ringVertices[v + 4], ringVertices[v + 5]);

		drawRings(a, b, ringColor, vertexData);
	}
}