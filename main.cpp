#include "Window.h"
#include "Shader.h"
#include "draw_utils.h"
#include "Object.h"

void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int VBO{};
unsigned int VAO{};

glm::vec3 cameraPos   { 0.0f, 0.5f,  3.0f };
glm::vec3 cameraTarget{ 0.0f, 0.0f,  0.0f };
glm::vec3 worldUp     { 0.0f, 1.0f,  0.0f };

float lastX{ 400.0f };
float lastY{ 300.0f };

float yaw{};
float pitch{};

float fov{ 45.0f };

bool isPressingRightClick{ false };
bool isFirstMouse{ true };

std::vector<float> vertexData
{  // plane positions      // colors
	 1.0f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
	 1.0f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
	-1.0f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.1f,

	 1.0f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
	-1.0f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
	-1.0f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.1f
};

    // id     // offset, vertexCount, primitive
std::map<int, ObjectMetadata> objInfo{};

// legacy symbolTable
//std::map<std::string, int> symbolTable_legacy{};

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

	// new architecture
	createObject({ "GRID_PLANE", Object::Plane, GL_TRIANGLES }, static_cast<int>(vertexData.size()) / 7, {}, glm::vec4(0.0f, 0.0f, 0.0f, 0.1f), 0); // add plane

	// legacy
	//addNewObject(static_cast<int>(vertexData.size()) / 7, GL_TRIANGLES, funcType::Plane, "GRID_PLANE", {}, glm::vec4(0.0f, 0.0f, 0.0f, 0.1f)); 

	getEnvironmentVertices();
	vertexSpec(vertexData);

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

		glm::mat4 view{ glm::lookAt(cameraPos, cameraTarget, worldUp) };

		glm::mat4 projection{};
		projection = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

		glm::mat4 model{ 1.0f };

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat4("model", model);

		glBindVertexArray(VAO);

		// new
		std::vector<TransparentItem> transparentQueue{};
		for (auto const& obj : object)
		{
			transparentQueue.push_back({ obj.getID(), obj.getColor().w});
		}

		// legacy
		//std::vector<int> transparentQueue{};
		//for (auto const& [id, obj] : objInfo)
		//{
		//	transparentQueue.push_back(id);
		//}

		// new
		std::sort(transparentQueue.begin(), transparentQueue.end(),
			[](const TransparentItem a, const TransparentItem b)
			{
				return a.alpha > b.alpha;
			}
		);

		// legacy
		//std::sort(transparentQueue.begin(), transparentQueue.end(),
		//	[](const int a, const int b) 
		//	{
		//		return objInfo[a].color.w > objInfo[b].color.w;
		//	}
		//);

		// new
		for (auto const& t : transparentQueue)
		{
			const auto& obj{ object[t.objIndex] };

			glDrawArrays(obj.getPrimitive(), obj.getOffset(), obj.getVertexCount());
		}

		// legacy
		//for (auto const id : transparentQueue)
		//{
		//	const auto& obj{ objInfo[id] };

		//	glDrawArrays(obj.primitiveType, obj.offset, obj.vertexCount);
		//}

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

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= static_cast<float>(yoffset) * 1.3f;

	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}

void vertexSpec(const std::vector<float>& vertices)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	int stride{ 7 * sizeof(float) };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
}

void updateBufferData(const std::vector<float>& vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
}

// new function to delete objects from vertexData
std::vector<float> deleteObjectFromVertexData(int objIndex)
{
	if (vertexData.empty())
		return {};

	// 7 is the number of components for each vertice
	// 3 position components + 4 color values
	const auto& obj{ object[objIndex] };

	int offset{ obj.getOffset() * 7 };
	int floatCount{ obj.getVertexCount() * 7 };

	std::vector<float> newVertexData{};

	newVertexData.reserve(vertexData.size() - floatCount);

	for (int i{ 0 }; i < vertexData.size(); ++i)
	{
		if (i >= offset && i < offset + floatCount)
			continue;

		newVertexData.push_back(vertexData[i]);
	}

	return newVertexData;
}

// legacy function to delete objects from vertexData
//std::vector<float> deleteObjectFromVertexData_legacy(int objID)
//{
//	if (vertexData.empty())
//		return {};
//
//	// 7 is the number of components for each vertice
//	// 3 position components + 4 color values
//	int offset{ objInfo[objID].offset * 7 };
//	int floatCount{ objInfo[objID].vertexCount * 7 };
//
//	std::vector<float> newVertexData{};
//
//	newVertexData.reserve(vertexData.size() - floatCount);
//
//	for (int i{ 0 }; i < vertexData.size(); ++i)
//	{
//		if (i >= offset && i < offset + floatCount)
//			continue;
//
//		newVertexData.push_back(vertexData[i]);
//	}
//
//	return newVertexData;
//}

//void deleteObjectRegister(int objID)
//{
//	symbolTable_legacy.erase(objInfo[objID].name);
//	objInfo.erase(objID);
//}

// new function to update objects
void updateObject(int objIndex, const Object& newObj)
{
	//if (objInfo.find(objID) == objInfo.end())
	//	return;

	vertexData = deleteObjectFromVertexData(objIndex);

	symbolTable.erase(object[objIndex].getName());

	object[objIndex] = newObj;
	const auto& obj{ object[objIndex] };

	symbolTable[obj.getName()] = obj.getID();

	draw(obj.getType(), obj.getComponents(), obj.getColor(), obj.getParentIDs(), true);
	updateBufferData(vertexData);
}

// legacy function to update objects
//void updateObject_legacy(int objID, const ObjectMetadata& newObj)
//{
//	if (objInfo.find(objID) == objInfo.end())
//		return;
//
//	vertexData = deleteObjectFromVertexData(objID);
//	symbolTable_legacy.erase(objInfo[objID].name);
//	
//	objInfo[objID] = newObj;
//	const auto& obj{ objInfo[objID] };
//
//	symbolTable_legacy[obj.name] = objID;
//
//	draw_legacy(obj.type, obj.components, obj.color, true);
//	updateBufferData(vertexData);
//}