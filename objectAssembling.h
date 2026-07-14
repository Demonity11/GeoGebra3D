#ifndef OBJECT_ASSEMBLING_H
#define OBJECT_ASSEMBLING_H

#include "draw_utils.h"

struct Intersect
{
	std::array<glm::vec3, 2> points{};
	std::array<glm::vec3, 2> vectors{};
	std::array<Object::Type, 2> types{};

	int lineCount{};
	int planeCount{};
};

Intersect gatherPlaneLine(const std::array<int, 3>& pIDs, std::vector<Object>& object);
glm::vec3 assemblyIntersectPoint(const Intersect& intersect);
std::array<glm::vec3, 2> assemblyVector(const std::vector<float>& comp, std::array<int, 3> pIDs, const std::array<int, 3>& pCompIndex, const std::vector<Object>& object);
//std::array<glm::vec3, 2> assemblyVector(Object& obj, const std::vector<Object>& object);
std::array<glm::vec3, 3> assemblyLine(const std::vector<float>& comp, const std::array<int, 3>& pCompIndex);
std::array<glm::vec3, 3> assemblyLine(Object& obj);
std::array<glm::vec3, 3> assemblyPlane(const std::vector<float>& comp, std::array<int, 3> pIDs, const std::array<int, 3>& pCompIndex, const std::vector<Object>& object);

#endif
