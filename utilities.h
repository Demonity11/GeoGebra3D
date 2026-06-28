#ifndef UTILITIES_H
#define UTILITIES_H

#include "draw_utils.h"

#include <iostream>

auto getObjectComponents(std::vector<std::string>& args, std::vector<float>& vecComponents, std::array<int, 3>& pIDs, std::array<int, 3>& pCompIndex) -> void;
auto convertParametersToFloat(std::string& parameters, std::vector<float>& vecComponents)															  -> void;
auto compareObjectType(const std::string& component, Object::Type expectedType, const std::vector<Object>& object)									  -> bool;
auto searchObjectByID(int id, const std::vector<Object>& objectRef)																					  -> int;
auto searchObjectIndexByName(const std::string& objName, const std::vector<Object>& object)															  -> int;
auto nextFreeParentIndex(const std::array<int, 3>& pIDs)																							  -> int;
auto splitArgs(const std::string& argumentString)																									  -> std::vector<std::string>;
auto getStringFunctionType(Object::Type type)																										  -> std::string;
auto stripArg(std::string& arg)																														  -> void;
auto deleteObjectFromVertexData(int objIndex, std::vector<float>& vertexData, std::vector<Object>& object)											  -> std::vector<float>;
auto deleteObject(int objIndex, std::vector<Object>& object, std::vector<float>& vertexData)														  -> void;
auto updateObject(int objIndex, const Object& newObj, std::vector<Object>& object, std::vector<float>& vertexData)									  -> void;
auto scanForIdenticalObject(Object::Type type, const std::vector<float>& components, std::vector<Object>& object)									  -> bool;
auto getExpression(Object& obj, std::vector<Object>& object)																						  -> std::string;
auto getEquation(Object& obj)																														  -> std::string;
auto intersectionLinePlane(glm::vec3 linePoint, glm::vec3 lineVector, glm::vec3 planeNormal, float d)												  -> glm::vec3;
auto intersectionLineLine(glm::vec3 ps, glm::vec3 vs, glm::vec3 pt, glm::vec3 vt)																	  -> glm::vec3;
auto intersectionPlanePlane(glm::vec3 p1, glm::vec3 n1, glm::vec3 p2, glm::vec3 n2)																	  -> std::array<glm::vec3, 2>;
auto recalculateIntersect(const Object& obj, std::vector<Object>& object)																		  -> bool;
auto testInput(std::string input)																													  -> std::vector<std::string>;

void createObject(Object obj, int vCount, const std::vector<float>& comp, const glm::vec4 color, uint8_t pCount, std::array<int, 3> pIDs = { -1, -1, -1 }, std::array<int, 3> pCompIndex = { -1, -1, -1 });

#endif
