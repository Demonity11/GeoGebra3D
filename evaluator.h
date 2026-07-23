#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <charconv>
#include <vector>
#include <optional>
#include <array>
#include <string_view>

#include "Context.h"
#include "parser.h"

class Object;

void printRuntimeValue(const RuntimeValue& value);

RuntimeValue evaluator(const std::vector<Node>& nodes, const std::vector<Object>& object, int nodeIdx = 0);
std::optional<float> convertSVToFloat(std::string_view sv);
//RuntimeValue assemblyVariable(const Object& obj, const std::vector<Object>& object);

RuntimeValue evaluatePointFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluateVectorFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes);
RuntimeValue evaluateCrossFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes);
RuntimeValue evaluateSegmentFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes);
RuntimeValue evaluateLineFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes);
RuntimeValue evaluatePlaneFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes);
RuntimeValue evaluateIntersectFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes);
RuntimeValue evaluateIntersectFunc(const std::vector<RuntimeValue>& args);

std::array<int, 3> findParentsIDs(const std::vector<Node>& nodes);
Object::Type deduceTypeByIdentifierName(std::string_view func);

#endif