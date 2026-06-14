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
std::vector<float> deleteObjectFromVertexData_legacy(int objID);
void updateObject_legacy(int objID, const ObjectMetadata& newObj);
void deleteObjectRegister(int objID);

extern std::vector<float> vertexData;

// forward declarations for draw.cpp
auto getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, float radius, std::vector<float>& vertexData) -> void;
auto getRingsVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, std::vector<float>& vertexData)				   -> void;
auto getSphereVertices(glm::vec3 translation, glm::vec4 color, float radius, std::vector<float>& vertexData)	   -> void;
auto getGridVertices()																							   -> void;
auto getEnvironmentVertices()																					   -> void;
auto addNewObject(int vertexCount, unsigned int primitive, funcType type, std::string name, const std::vector<float>& components, const glm::vec4 color) -> void;
void getNewCoordSystem(glm::vec3& direction, glm::vec3& right, glm::vec3& up);
void getPlaneVertices(glm::vec3 normalP0, glm::vec3 normalP, glm::vec3 point, glm::vec4 color, std::vector<float>& vertexData);

extern std::map<int, ObjectMetadata> objInfo;
extern std::map<std::string, int> symbolTable_legacy;

// forward declarations for interface.cpp
auto initializeImGui(GLFWwindow* window)												  -> void;
auto getUserInput()																		  -> void;
auto extractComponents(std::string& parameters, std::vector<float>& vecComponents)		  -> void;
auto draw_legacy(funcType type, const std::vector<float>& vecComponents, glm::vec4 color, bool update = false) -> void;
void stripArg(std::string& arg);
int searchObjectID(const std::string& objName);
std::vector<std::string> splitArgs(const std::string& argumentString);
std::vector<float> getObjectComponents(std::vector<std::string>& args);
bool compareObjectType_legacy(const std::string& component, funcType expectedType);
std::string getStringFuncType(funcType type);
char updateObjectName(funcType type, bool isObjectDeleted = false);

extern std::map<funcType, char> objSymbols;
extern std::map<std::string, int> symbolTable;

#endif
