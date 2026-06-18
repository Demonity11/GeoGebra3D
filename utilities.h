#ifndef UTILITIES_H
#define UTILITIES_H

#include "draw_utils.h"

#include <iostream>

auto getObjectComponents(std::vector<std::string>& args, std::vector<float>& vecComponents, std::array<int, 3>& pIDs, std::array<int, 3>& pCompIndex) -> void;
auto convertParametersToFloat(std::string& parameters, std::vector<float>& vecComponents)															  -> void;
auto compareObjectType(const std::string& component, Object::Type expectedType)																		  -> bool;
auto searchObjectByID(int id, const std::vector<Object>& objectRef)																					  -> int;
auto searchObjectIndexByName(const std::string& objName)																							  -> int;
auto nextFreeParentIndex(const std::array<int, 3>& pIDs)																							  -> int;
auto splitArgs(const std::string& argumentString)																									  -> std::vector<std::string>;
auto getStringFunctionType(Object::Type type)																										  -> std::string;
auto stripArg(std::string& arg)																														  -> void;
auto deleteObjectFromVertexData(int objIndex)																										  -> std::vector<float>;
auto deleteObject(int objIndex)																														  -> void;
auto updateObject(int objIndex, const Object& newObj)																								  -> void;
auto scanForIdenticalObject(Object::Type type, const std::vector<float>& components)																  -> bool;
auto getEquation(Object& obj)																														  -> std::string;

void createObject(Object obj, int vCount, const std::vector<float>& comp, const glm::vec4 color, uint8_t pCount, std::array<int, 3> pIDs = { -1, -1, -1 }, std::array<int, 3> pCompIndex = { -1, -1, -1 });

#endif
