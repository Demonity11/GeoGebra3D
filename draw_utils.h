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

enum class funcType
{
	Vector,
	Point,
	Segment,
	Plane
};

struct ObjectMetadata
{
	int offset{};
	int vertexCount{};
	unsigned int primitiveType{};
	funcType type{};
	std::string name{};
	std::vector<float> components{};
	glm::vec4 color{};
};

// forward declarations for main.cpp
auto vertexSpec(const std::vector<float>& vertices)																   -> void;
auto updateBufferData(const std::vector<float>& vertices)														   -> void;

extern std::vector<float> vertexData;

// forward declarations for draw.cpp
auto getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, float radius, std::vector<float>& vertexData) -> void;
auto getRingsVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, std::vector<float>& vertexData)				   -> void;
auto getSphereVertices(glm::vec3 translation, glm::vec4 color, float radius, std::vector<float>& vertexData)	   -> void;
auto getGridVertices()																							   -> void;
auto getEnvironmentVertices(bool firstRun = false)																   -> void;
void getNewCoordSystem(glm::vec3& direction, glm::vec3& right, glm::vec3& up);
void getPlaneVertices(glm::vec3 normalP0, glm::vec3 normalP, glm::vec3 point, glm::vec4 color, std::vector<float>& vertexData);
void deleteObject(int objIndex);

//extern std::map<int, ObjectMetadata> objInfo;
extern std::map<std::string, int> symbolTable_legacy;

// forward declarations for interface.cpp
auto initializeImGui(GLFWwindow* window)												  -> void;
auto getUserInput()																		  -> void;
auto extractComponents(std::string& parameters, std::vector<float>& vecComponents)		  -> void;
void stripArg(std::string& arg);
std::vector<std::string> splitArgs(const std::string& argumentString);
void getObjectComponents(std::vector<std::string>& args, std::vector<float>& vecComponents, std::array<int, 3>& pIDs, std::array<int, 3>& pCompIndex);
int searchObjectID(const std::string& objName);

//extern std::map<std::string, int> symbolTable;

#endif
