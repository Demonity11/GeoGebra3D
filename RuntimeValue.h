#ifndef RUNTIME_VALUE_H
#define RUNTIME_VALUE_H

#include <variant>
#include <string>
#include <glm/glm.hpp>

namespace Context
{
	struct RuntimeError
	{
		std::string message{};
	};
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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

#endif