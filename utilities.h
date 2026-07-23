#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>

#include "Object.h"
#include "Context.h"
#include "evaluator.h"

//rewrite
bool compareRuntimeValue(Object::Type type, const RuntimeValue& components1, const RuntimeValue& components2);
bool scanForIdenticalObject(Object::Type type, const RuntimeValue& components, const std::vector<Object>& object, int ignoreID = -1);

auto searchObjectByID(int id, const std::vector<Object>& objectRef)																					  -> int;
auto searchObjectIndexByName(const std::string& objName, const std::vector<Object>& object)															  -> int;
auto getStringFunctionType(Object::Type type)																										  -> std::string;
auto getObjectTypeFromString(const std::string& funcName)																							  -> Object::Type;
auto deleteObject(int objIndex, std::vector<Object>& object, std::vector<float>& vertexData)														  -> void;
auto updateObject(int objIndex, const Object& newObj, std::vector<Object>& object, std::vector<float>& vertexData)									  -> void;
bool rebuildObjectFromParents(Object& obj, const std::vector<Object>& object);
auto getExpression(const Object& obj, const std::vector<Object>& object)																						  -> std::string;
std::string extractPName(const Object& obj);
std::string getRuntimeValueCompAsString(int id, const std::vector<Object>& objectRef);
auto getEquation(const Object& obj)																														  -> std::string;
RuntimeValue intersectionLinePlane(const Eval::Line& line, const Eval::Plane& plane); // Eval::IPoint or RuntimeError
RuntimeValue intersectionLineLine(const Eval::Line& lineS, const Eval::Line& lineT); // Eval::IPoint or RuntimeError
RuntimeValue intersectionPlanePlane(const Eval::Plane& plane1, const Eval::Plane& plane2); // Eval::ILine or RuntimeError
auto recalculateIntersect(Object& obj, const std::vector<Object>& object)																					  -> bool;
auto testInput(std::string input)																													  -> std::vector<std::string>;

bool projectWorldToScreen
(
	const glm::vec3& worldPos,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize,
	glm::vec2& outScreenPos
);

auto getSelectedObjectID(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, std::vector<Object>& object) -> int;
auto updateSelectedObjectColor(int objIndex, std::vector<Object>& object, std::vector<float>& vertexData) -> void;

size_t createObject(Object obj, int vCount, const RuntimeValue& comp, const glm::vec4& color, int pCount, const std::array<int, 3>& pIDs = { -1, -1, -1 });
size_t createObject(Object obj, int vCount);

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);

Object::Type duduceRuntimeValueType(const RuntimeValue& value);
std::optional<glm::vec3> extractPoint(const RuntimeValue& val);
std::optional<Eval::Line> extractLine(const RuntimeValue& val);

#endif
