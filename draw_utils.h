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

// forward declarations for main.cpp
void vertexSpec(const std::vector<float>& vertices);
void updateBufferData(const std::vector<float>& vertices);
void getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec3 color, std::vector<float>& vertexData);
void drawRings(glm::vec3 p0, glm::vec3 p, glm::vec3 color, std::vector<float>& vertexData);
void drawCilinder();

extern std::vector<float> vertexData;

// forward declarations for interface.cpp

std::vector<float> isComponentsConvertible(std::string parameters);

#endif
