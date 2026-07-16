#ifndef DRAW_UTILS_H
#define DRAW_UTILS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>

#include "Object.h"

struct FunctionArgs
{
	std::string name{};
	Object::Type type{};
	std::vector<Object::Type> expectedArgs{};
};

// forward declarations for main.cpp
auto vertexSpec(const std::vector<float>& vertices)		  -> void;
auto updateBufferData(const std::vector<float>& vertices) -> void;

// forward declarations for interface.cpp
auto initializeImGui(GLFWwindow* window)																		 -> void;
auto getUserInput(std::vector<Object>& object)																	 -> void;
auto processInput(char inputBuffer[128], const std::vector<FunctionArgs>& function, std::vector<Object>& object) -> void;
auto showVariables(std::vector<Object>& object)																	 -> void;


void drawObjectLabels
(
	std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
);
void drawAxisLabels
(
	std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
);

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);

void buildAndRegisterObject(Object::Type type, const std::vector<float>& components, const glm::vec4& color, const std::array<int, 3>& pIDs = { -1, -1, -1 }, const std::array<int, 3>& pCompIndex = { -1, -1, -1 });
int generateObjectVertices(Object& obj, std::vector<float>& vertexData);

#endif
