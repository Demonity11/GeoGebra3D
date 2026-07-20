#include "evaluator.h"

RuntimeValue evaluator(const std::vector<Node>& nodes, std::vector<Object>& object, int nodeIdx)
{
    const Node& node{ nodes[nodeIdx] };

    if (node.type == Node::Literal)
    {
        std::optional<float> literal{ convertSVToFloat(node.content) };

        if (!literal.has_value())
        {
            return Context::RuntimeError{ "SEMANTICS::ERROR::LITERAL::INVALID_FLOAT_FORMAT\n" };
        }

        return *literal;
    }

    else if (node.type == Node::Function)
    {
        bool funcExist{ false };
        for (const auto& func : Context::function)
        {
            if (node.content == func.name) funcExist = true;
        }

        if (!funcExist)
        {
            return Context::RuntimeError{ "SEMANTICS::ERROR::FUNCTION::" + std::string(node.content) + "_DOES_NOT_EXIST\n" };
        }

        std::vector<RuntimeValue> args{};
        for (int childIdx : node.children)
        {
            if (childIdx == -1) break;
            
            RuntimeValue childVal{ evaluator(nodes, object, childIdx) };

            if (std::holds_alternative<Context::RuntimeError>(childVal))
            {
                return childVal;
            }

            args.push_back(childVal);
        }

        if (node.content == "Point")
        {
            return evaluatePointFunc(args);
        }
        else if (node.content == "Vector")
        {
            return evaluateVectorFunc(args);
        }
        else if (node.content == "Segment")
        {
            return evaluateSegmentFunc(args);
        }
        else if (node.content == "Line")
        {
            return evaluateLineFunc(args);
        }
        else if (node.content == "Plane")
        {
            return evaluatePlaneFunc(args);
        }

        return Context::RuntimeError{ "SEMANTICS::ERROR::FUNCTION_NOT_FOUND\n" };
    }

    else if (node.type == Node::Variable)
    {
        int objIdx{ searchObjectIndexByName(std::string(node.content), object) };
        if (objIdx == -1)
        {
            return Context::RuntimeError{ "SEMANTICS::ERROR::VARIABLE::" + std::string(node.content) + "_DOES_NOT_EXIST\n" };
        }

        Object& obj{ object[objIdx] };

        return assemblyVariable(obj, object);
    }

    return Context::RuntimeError{ "SEMANTICS::ERROR::UNKNOWN_NODE_TYPE\n" };
}

std::optional<float> convertSVToFloat(std::string_view sv)
{
    float value{};

    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

    if (ec == std::errc{})
    {
        return value;
    }

    return {};
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void printRuntimeValue(const RuntimeValue& value)
{
    std::visit(overloaded
        {
        [](float f)
        {
            std::cout << "Literal: " << f << '\n';
        },
        [](glm::vec3 point)
        {
            std::cout << "Point: (" << point << ")\n";
        },
        [](Eval::Vector vector)
        {
            std::cout << "Vector Origin:\t(" << vector.origin << ")\n";
            std::cout << "Vector Head:\t(" << vector.head << ")\n";
        },
        [](Eval::Segment segment)
        {
            std::cout << "Segment A:\t(" << segment.A << ")\n";
            std::cout << "Segment B:\t(" << segment.B << ")\n";
        },
        [](Eval::Line line)
        {
            std::cout << "Point:\t(" << line.point << ")\n";
            std::cout << "Direction Vector Origin:\t(" << line.dVecOrigin << ")\n";
            std::cout << "Direction Vector Head:\t(" << line.dVecHead << ")\n";
        },
        [](Eval::Plane plane)
        {
            std::cout << "Point:\t(" << plane.point << ")\n";
            std::cout << "Normal Vector Origin:\t(" << plane.normalOrigin << ")\n";
            std::cout << "Normal Vector Head:\t(" << plane.normalHead << ")\n";
        },
        [](Context::RuntimeError error)
        {
            std::cerr << error.message << '\n';
        }

        }, value);
}

RuntimeValue assemblyVariable(Object& obj, const std::vector<Object>& object)
{
    Object::Type type{ obj.getType() };
    const std::vector<float>& comp{ obj.getComponents() };

    if (type == Object::Point)
    {
        glm::vec3 point{ comp[0], comp[1], comp[2] };
        return point;
    }
    else if (type == Object::Vector)
    {
        Eval::Vector vector{};
        std::array<glm::vec3, 2> temp{ assemblyVector(obj, object) };

        vector.origin = temp[0];
        vector.head = temp[1];

        return vector;
    }
    else if (type == Object::Segment)
    {
        std::array<int, 3> pCompIndex{ obj.getpCompIndex() };
        int startA{ pCompIndex[0] };
        int startB{ pCompIndex[1] };

        Eval::Segment segment
        {
            { comp[startA], comp[startA + 1], comp[startA + 2] },
            { comp[startB], comp[startB + 1], comp[startB + 2] }
        };

        return segment;
    }
    else if (type == Object::Line)
    {
        Eval::Line line{};
        std::array<glm::vec3, 3> temp{ assemblyLine(obj) };

        line.point = temp[0];
        line.dVecOrigin = temp[1];
        line.dVecHead = temp[2];

        return line;
    }
    else if (type == Object::Plane)
    {
        Eval::Plane plane{};
        std::array<glm::vec3, 3> temp{ assemblyPlane(obj, object) };

        plane.normalOrigin = temp[0];
        plane.normalHead = temp[1];
        plane.point = temp[2];

        return plane;
    }

    return Context::RuntimeError{ "???" };
}

RuntimeValue evaluatePointFunc(const std::vector<RuntimeValue>& args)
{
    if (args.size() == 3 &&
        std::holds_alternative<float>(args[0]) &&
        std::holds_alternative<float>(args[1]) &&
        std::holds_alternative<float>(args[2])
        )
    {
        float x{ std::get<float>(args[0]) };
        float y{ std::get<float>(args[1]) };
        float z{ std::get<float>(args[2]) };

        glm::vec3 point{ x, y, z };

        return point;
    }

    else if (args.size() == 1 && std::holds_alternative<glm::vec3>(args[0]))
    {
        return std::get<glm::vec3>(args[0]);
    }

    return Context::RuntimeError{ "SEMANTICS::ERROR::POINT::INVALID_ARGUMENTS_OVERLOAD\n" };
}

RuntimeValue evaluateVectorFunc(const std::vector<RuntimeValue>& args)
{
    if (args.size() == 1 && std::holds_alternative<Eval::Vector>(args[0]))
    {
        return args[0];
    }

    else if (args.size() == 2 &&
        std::holds_alternative<glm::vec3>(args[0]) &&
        std::holds_alternative<glm::vec3>(args[1])
        )
    {
        Eval::Vector vector
        {
            std::get<glm::vec3>(args[0]),
            std::get<glm::vec3>(args[1])
        };

        return vector;
    }

    return Context::RuntimeError{ "SEMANTICS::ERROR::VECTOR::INVALID_ARGUMENTS_OVERLOAD\n" };
}

RuntimeValue evaluateSegmentFunc(const std::vector<RuntimeValue>& args)
{
    if (args.size() == 1 && std::holds_alternative<Eval::Segment>(args[0]))
    {
        return args[0];
    }

    else if (args.size() == 2 &&
        std::holds_alternative<glm::vec3>(args[0]) &&
        std::holds_alternative<glm::vec3>(args[1])
        )
    {
        Eval::Segment segment
        {
            std::get<glm::vec3>(args[0]),
            std::get<glm::vec3>(args[1])
        };

        return segment;
    }

    return Context::RuntimeError{ "SEMANTICS::ERROR::VECTOR::INVALID_ARGUMENTS_OVERLOAD\n" };
}

RuntimeValue evaluateLineFunc(const std::vector<RuntimeValue>& args)
{
    if (args.size() == 1 && std::holds_alternative<Eval::Line>(args[0]))
    {
        return args[0];
    }

    else if (args.size() == 2 &&
        std::holds_alternative<glm::vec3>(args[0]) &&
        std::holds_alternative<Eval::Vector>(args[1])
        )
    {
        glm::vec3 point{ std::get<glm::vec3>(args[0]) };
        Eval::Vector vector{ std::get<Eval::Vector>(args[1]) };

        Eval::Line line{ point, vector.origin, vector.head };

        return line;
    }

    else if (args.size() == 2 &&
        std::holds_alternative<glm::vec3>(args[0]) &&
        std::holds_alternative<glm::vec3>(args[1])
        )
    {
        glm::vec3 point{ std::get<glm::vec3>(args[0]) };
        Eval::Vector vector{ point, std::get<glm::vec3>(args[1]) };

        Eval::Line line{ point, vector.origin, vector.head };

        return line;
    }

    return Context::RuntimeError{ "SEMANTICS::ERROR::LINE::INVALID_ARGUMENTS_OVERLOAD\n" };
}

RuntimeValue evaluatePlaneFunc(const std::vector<RuntimeValue>& args)
{
    if (args.size() == 2 &&
        std::holds_alternative<glm::vec3>(args[0]) &&
        std::holds_alternative<Eval::Vector>(args[1])
        )
    {
        glm::vec3 point{ std::get<glm::vec3>(args[0]) };
        Eval::Vector vector{ std::get<Eval::Vector>(args[1]) };

        Eval::Plane plane{ point, vector.origin, vector.head };

        return plane;
    }

    else if (args.size() == 3 &&
        std::holds_alternative<glm::vec3>(args[0]) &&
        std::holds_alternative<glm::vec3>(args[1]) &&
        std::holds_alternative<glm::vec3>(args[2])
        )
    {
        glm::vec3 A{ std::get<glm::vec3>(args[0]) };
        glm::vec3 B{ std::get<glm::vec3>(args[1]) };
        glm::vec3 C{ std::get<glm::vec3>(args[2]) };

        glm::vec3 u{ B - A };
        glm::vec3 v{ C - A };

        Eval::Plane plane{ A, A, glm::cross(u, v) };

        return plane;
    }

    return Context::RuntimeError{ "SEMANTICS::ERROR::PLANE::INVALID_ARGUMENTS_OVERLOAD\n" };
}