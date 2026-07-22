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

auto getObjectComponents(std::vector<std::string>& args, std::vector<float>& vecComponents, std::array<int, 3>& pIDs, std::array<int, 3>& pCompIndex) -> void;
auto convertParametersToFloat(std::string& parameters, std::vector<float>& vecComponents)															  -> void;
auto compareObjectType(const std::string& component, Object::Type expectedType, const std::vector<Object>& object)									  -> bool;
auto searchObjectByID(int id, const std::vector<Object>& objectRef)																					  -> int;
auto searchObjectIndexByName(const std::string& objName, const std::vector<Object>& object)															  -> int;
auto nextFreeParentIndex(const std::array<int, 3>& pIDs)																							  -> int;
auto splitArgs(const std::string& argumentString)																									  -> std::vector<std::string>;
auto getStringFunctionType(Object::Type type)																										  -> std::string;
auto getObjectTypeFromString(const std::string& funcName)																							  -> Object::Type;
auto stripArg(std::string& arg)																														  -> void;
auto deleteObject(int objIndex, std::vector<Object>& object, std::vector<float>& vertexData)														  -> void;
auto updateObject(int objIndex, const Object& newObj, std::vector<Object>& object, std::vector<float>& vertexData)									  -> void;
bool rebuildObjectFromParents(Object& obj, const std::vector<Object>& object);
// return true if exist an object with the same type and components 
//auto scanForIdenticalObject(Object::Type type, const std::vector<float>& components, std::vector<Object>& object, int ignoreID = -1)				  -> bool;
auto getExpression(const Object& obj, const std::vector<Object>& object)																						  -> std::string;
std::string getRuntimeValueCompAsString(int id, const std::vector<Object>& objectRef);
auto getEquation(const Object& obj)																														  -> std::string;
auto intersectionLinePlane(glm::vec3 linePoint, glm::vec3 lineVector, glm::vec3 planeNormal, float d)												  -> glm::vec3;
glm::vec3 intersectionLinePlane(const Eval::Line& line, const glm::vec3& planeNormal);
auto intersectionLineLine(glm::vec3 ps, glm::vec3 vs, glm::vec3 pt, glm::vec3 vt)																	  -> glm::vec3;
glm::vec3 intersectionLineLine(const Eval::Line& lineS, const Eval::Line& lineT);
auto intersectionPlanePlane(glm::vec3 p1, glm::vec3 n1, glm::vec3 p2, glm::vec3 n2)																	  -> Eval::Line;
Eval::Line intersectionPlanePlane(Eval::Plane plane1, Eval::Plane plane2);
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

#endif
