#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <variant>
#include <charconv>

#include "utilities.h"
#include "objectAssembling.h"
#include "Context.h"
#include "parser.h"

namespace Eval
{
	struct Vector
	{
		glm::vec3 origin{};
		glm::vec3 head{};
	};

	struct Segment
	{
		glm::vec3 A{};
		glm::vec3 B{};
	};

	struct Line
	{
		glm::vec3 point{};
		glm::vec3 dVecOrigin{};
		glm::vec3 dVecHead{};
	};

	struct Plane
	{
		glm::vec3 point{};
		glm::vec3 normalOrigin{};
		glm::vec3 normalHead{};
	};
}

using RuntimeValue = std::variant<float, glm::vec3, Eval::Vector, Eval::Segment, Eval::Line, Eval::Plane, Context::RuntimeError>;

void printRuntimeValue(const RuntimeValue& value);

RuntimeValue evaluator(const std::vector<Node>& nodes, std::vector<Object>& object, int nodeIdx = 0);
std::optional<float> convertSVToFloat(std::string_view sv);
RuntimeValue assemblyVariable(Object& obj, const std::vector<Object>& object);

RuntimeValue evaluatePointFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluateVectorFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluateSegmentFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluateLineFunc(const std::vector<RuntimeValue>& args);
RuntimeValue evaluatePlaneFunc(const std::vector<RuntimeValue>& args);

#endif