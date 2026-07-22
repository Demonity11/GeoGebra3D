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
RuntimeValue evaluateVectorFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluateCrossFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluateSegmentFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluateLineFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluatePlaneFunc(const std::vector<RuntimeValue>& args);

std::array<int, 3> findParentsIDs(const std::vector<Node>& nodes);

#endif