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

enum class funcType
{
	Vector,
	Point,
	Segment,
	Plane
};

// forward declarations for main.cpp
auto vertexSpec(const std::vector<float>& vertices)																   -> void;
auto updateBufferData(const std::vector<float>& vertices)														   -> void;

// forward declarations for draw.cpp
auto getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, float radius, std::vector<float>& vertexData) -> void;
auto getRingsVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, std::vector<float>& vertexData)				   -> void;
auto getSphereVertices(glm::vec3 translation, glm::vec4 color, float radius, std::vector<float>& vertexData)	   -> void;
auto getGridVertices()																							   -> void;
auto drawCilinder()																								   -> void;
auto addNewObject(int vertexCount, unsigned int primitive, funcType type, std::string name)						   -> void;

struct ObjectMetadata
{
	int offset{};
	int vertexCount{};
	unsigned int primitiveType{};
	funcType type{};
	std::string name{};
};

extern std::vector<float> vertexData;
extern std::map<int, ObjectMetadata> objInfo;
extern std::map<std::string, int> symbolTable;


// forward declarations for interface.cpp
auto initializeImGui(GLFWwindow* window)												  -> void;
auto getUserInput()																		  -> void;
auto extractComponents(std::string& parameters, std::vector<float>& vecComponents)		  -> void;
auto draw(funcType type, const std::vector<float>& vecComponents, const glm::vec4& color) -> void;

#endif
