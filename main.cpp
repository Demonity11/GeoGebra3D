#include "Window.h"
#include "Shader.h"
#include "draw_utils.h"
#include "objectCoords.h"
#include "utilities.h"
#include "Context.h"

void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

struct TransparentItem
{
	int objIndex{};
	float alpha{};
};

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
	glfwSetScrollCallback(window.getWindow(), mouse_scroll_callback);

	getEnvironmentVertices(Context::vertexData, true);
	vertexSpec(Context::vertexData);

	initializeImGui(window.getWindow());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		processInput(window.getWindow());

		// ImGui stuff
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		window.clear(0.0f, 0.5f, 0.8f, 1.0f);

		shader.use();

		glm::mat4 view{ glm::lookAt(Context::cameraPos, Context::cameraTarget, Context::worldUp) };

		glm::mat4 projection{};
		projection = glm::perspective(glm::radians(Context::fov), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

		glm::mat4 model{ 1.0f };

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat4("model", model);

		glBindVertexArray(Context::VAO);

		std::vector<TransparentItem> transparentQueue{};

		for (int i{ 0 }; i < Context::object.size(); ++i)
		{
			const auto& obj{ Context::object[i] };

			transparentQueue.push_back({ i, obj.getColor().w });
		}

		std::sort(transparentQueue.begin(), transparentQueue.end(),
			[](const TransparentItem a, const TransparentItem b)
			{
				return a.alpha > b.alpha;
			}
		);

		for (auto const& t : transparentQueue)
		{
			const auto& obj{ Context::object[t.objIndex] };

			glDrawArrays(obj.getPrimitive(), obj.getOffset(), obj.getVertexCount());
		}

		// render ImGui here
		ImGui::ShowDemoWindow();
		getUserInput(Context::object);

		int width{};
		int height{};
		glfwGetWindowSize(window.getWindow(), &width, &height);

		glm::vec2 viewportPos{ 0.0f, 0.0f }; 
		glm::vec2 viewportSize{ static_cast<float>(width), static_cast<float>(height) };

		drawObjectLabels(Context::object, view, projection, model, viewportPos, viewportSize);
		drawAxisLabels(Context::object, view, projection, model, viewportPos, viewportSize);

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

	//if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	//{
	//	isPressingRightClick = true;
	//}

	//if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	//{
	//	isPressingRightClick = false;
	//}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			Context::isPressingRightClick = true;
			Context::isFirstMouse = true;
		}

		if (action == GLFW_RELEASE)
			Context::isPressingRightClick = false;
	}
}

void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!Context::isPressingRightClick)
		return;


	if (Context::isFirstMouse)
	{
		Context::lastX = static_cast<float>(xpos);
		Context::lastY = static_cast<float>(ypos);
		Context::isFirstMouse = false;
	}

	//std::cout << "xpos: " << xpos << "\n";
	//std::cout << "ypos: " << ypos << "\n";

	float xoffset = static_cast<float>(xpos) - Context::lastX;
	float yoffset = static_cast<float>(ypos) - Context::lastY;

	Context::lastX = static_cast<float>(xpos);
	Context::lastY = static_cast<float>(ypos);

	const float sensitivity{ 0.01f };

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	Context::yaw += xoffset;
	Context::pitch += yoffset;

	if (Context::pitch > 89.0f)
		Context::pitch = 89.0f;

	if (Context::pitch < -89.0f)
		Context::pitch = -89.0f;

	const float radius{ 3.0f };

	Context::cameraPos.x = Context::cameraTarget.x + radius * cos(Context::yaw) * cos(Context::pitch);
	Context::cameraPos.y = Context::cameraTarget.y + radius * sin(Context::pitch);
	Context::cameraPos.z = Context::cameraTarget.z + radius * sin(Context::yaw) * cos(Context::pitch);
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Context::fov -= static_cast<float>(yoffset) * 1.3f;

	if (Context::fov < 1.0f)
		Context::fov = 1.0f;
	if (Context::fov > 45.0f)
		Context::fov = 45.0f;
}

void vertexSpec(const std::vector<float>& vertices)
{
	glGenVertexArrays(1, &Context::VAO);
	glBindVertexArray(Context::VAO);

	glGenBuffers(1, &Context::VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Context::VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	int stride{ 7 * sizeof(float) };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
}

void updateBufferData(const std::vector<float>& vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, Context::VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
}
