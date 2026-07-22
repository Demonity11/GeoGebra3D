#ifndef OBJECT_ASSEMBLING_H
#define OBJECT_ASSEMBLING_H

#include <array>
#include <vector>
#include <glm/glm.hpp>

#include "Object.h"

struct Intersect
{
	std::array<glm::vec3, 2> points{};
	std::array<glm::vec3, 2> vectors{};
	std::array<Object::Type, 2> types{};

	int lineCount{};
	int planeCount{};
};

Intersect gatherPlaneLine(const Object& obj, const std::vector<Object>& object);
glm::vec3 assemblyIntersectPoint(const Intersect& intersect);
//std::array<glm::vec3, 2> assemblyVector(Object& obj, const std::vector<Object>& object);
//std::array<glm::vec3, 3> assemblyLine(Object& obj);
//std::array<glm::vec3, 3> assemblyPlane(Object& obj, const std::vector<Object>& object);

#endif
