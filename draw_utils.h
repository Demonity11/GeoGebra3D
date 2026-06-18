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

// parent ID when the object has literal components
constexpr int componentLiteral{ -2 };
// Objects conteiner
extern std::vector<Object> object;

// forward declarations for main.cpp
auto vertexSpec(const std::vector<float>& vertices)		  -> void;
auto updateBufferData(const std::vector<float>& vertices) -> void;

extern std::vector<float> vertexData;

// forward declarations for interface.cpp
auto initializeImGui(GLFWwindow* window)																					-> void;
auto getUserInput()																											-> void;
void draw(Object::Type type, std::vector<float>& vecComponents, glm::vec4 color, std::array<int, 3> pIDs = { -1, -1, -1 }, std::array<int, 3> pCompIndex = { -1, -1, -1 }, bool update = false);

#endif
