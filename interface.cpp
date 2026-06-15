#include "Window.h"
#include "draw_utils.h"
#include "Object.h"

// new
struct FunctionArgs
{
	std::string name{};
	Object::Type type{};
	std::vector<Object::Type> expectedArgs{};
};

// new
std::vector<FunctionArgs> function
{
	{"Point(",   Object::Point,   {}                              },
	{"Vector(",  Object::Vector,  {Object::Point,  Object::Point} },
	//{"Vector(",  Object::Vector,  {Object::Point}                 },
	{"Segment(", Object::Segment, {Object::Point,  Object::Point} },
	{"Plane(",   Object::Plane,   {Object::Vector, Object::Point} }
};

// new
std::map<Object::Type, char> objectSymbols
{
	{ Object::Vector,  'u' },
	{ Object::Point,   'A' },
	{ Object::Segment, 'f' },
	{ Object::Plane,   'p' },
};

constexpr int componentLiteral{ -2 };

void initializeImGui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); static_cast<void>(io);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void getUserInput()
{
	static char inputBuffer[128] = "";

	ImGui::Begin("Input");
	ImGui::InputTextWithHint("Input", "input", inputBuffer, IM_COUNTOF(inputBuffer));
	if (ImGui::Button("Enter"))
	{
		auto ss{ std::stringstream(inputBuffer) };
		auto inputText{ ss.str() };

		for (const auto& func : function)
		{
			auto funcOpenParenthesisPos{ inputText.find("(") };
			auto funcCloseParenthesisPos{ (inputText.rfind(")") != std::string::npos) ? inputText.rfind(")") : 0 };

			if (funcCloseParenthesisPos > funcOpenParenthesisPos)
			{
				std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

				std::vector<std::string> args{ splitArgs(parameters) };

				if (func.type == Object::Point && args.size() == 3)
				{
					std::vector<float> vecComponents{};
					extractComponents(parameters, vecComponents);

					draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.2f, 0.5f, 1.0f });
				}

				if (inputText.find(func.name) == 0 && args.size() == func.expectedArgs.size())
				{
					bool isObjectValid{ true };

					for (int index{ 0 }; index < args.size(); ++index)
					{
						if (func.expectedArgs.size() != args.size())
						{
							isObjectValid = false;
							break;
						}

						isObjectValid = compareObjectType(args[index], func.expectedArgs[index]);

						if (!isObjectValid)
						{
							isObjectValid = false;
							break;
						}
					}

					if (isObjectValid)
					{
						std::array<int, 3> pIDs{ -1, -1, -1 };
						std::array<int, 3> pCompIndex{ -1, -1, -1 };

						std::vector<float> vecComponents{ getObjectComponents(args, pIDs, pCompIndex) };

						draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, pIDs, pCompIndex);
					}
				}
			}
		}
	}

	ImGui::SeparatorText("Variables");

	for (int i{ 8 }; i < object.size(); ++i)
	{
		auto& obj{ object[i]};

		std::string headerText{ obj.getName() + " : " + getStringFunctionType(obj.getType())};

		if (ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_None))
		{
			char s{ 'A' };
			for (int increment{ 0 }; increment < obj.getComponents().size(); increment += 3)
			{
				ImGui::InputFloat3((obj.getName() + "::" + s).c_str(), obj.getComponentsPointer() + increment, "%.2f");

				if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
					updateObject(i, obj);

				++s;
			}

			ImGui::InputFloat4((obj.getName() + "::Color").c_str(), obj.getColorPointer(), "%.2f");

			if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
				updateObject(i, obj);

			if (ImGui::Button("Delete"))
			{
				vertexData = deleteObjectFromVertexData(i);
				deleteObject(i);
				updateBufferData(vertexData);
			}
		}
	}

	ImGui::End();
}

// new function to convert Object::Type to std::string
std::string getStringFunctionType(Object::Type type)
{
	switch (type)
	{
	case Object::Point:   return "Point";
	case Object::Vector:  return "Vector";
	case Object::Segment: return "Segment";
	case Object::Plane:   return "Plane";
	}
}

void extractComponents(std::string& parameters, std::vector<float>& vecComponents)
{
	auto commaPos{ parameters.find(",") };

	while (parameters.find(",") != std::string::npos)
	{
		int startComp{ 0 };

		// convert the extracted string component into float
		try
		{
			vecComponents.push_back(std::stof(parameters.substr(startComp, commaPos - startComp)));
		}

		catch (const std::invalid_argument& e)
		{
			std::cerr << "ERROR::STRING_IS_NOT_A_NUMBER\n";
			return;
		}

		parameters = parameters.substr(commaPos + 1, parameters.length() - 1);

		commaPos = parameters.find(",");

		if (commaPos == std::string::npos)
		{
			try
			{
				vecComponents.push_back(std::stof(parameters));
			}

			catch (const std::invalid_argument& e)
			{
				std::cerr << "ERROR::STRING_IS_NOT_A_NUMBER\n";
				return;
			}

			break;
		}
	}
}

// new function for comparing objects
bool compareObjectType(const std::string& component, Object::Type expectedType)
{
	for (const auto& obj : object)
	{
		if (component == obj.getName())
		{
			if (obj.getType() == expectedType)
				return true;

			return false;
		}
	}

	return false;
}

void stripArg(std::string& arg)
{
	std::string newArg{};

	for (char c : arg)
	{
		if (c != '(' && c != ')' && c != ' ')
			newArg += c;
	}

	arg = newArg;
}

std::vector<std::string> splitArgs(const std::string& argumentString)
{
	std::vector<std::string> args{};
	std::string currentArg{};
	int parenthesisCount{ 0 };

	for (char c : argumentString)
	{
		if (c == '(')
			++parenthesisCount;
		if (c == ')')
			--parenthesisCount;

		if (c == ',' && parenthesisCount == 0)
		{
			stripArg(currentArg);

			args.push_back(currentArg);
			currentArg.clear();
		}
		else
		{
			currentArg += c;
		}
	}

	if (!currentArg.empty())
	{
		stripArg(currentArg);
		args.push_back(currentArg);
	}

	return args;
}

// new function search object's index
int searchObjectIndex(const std::string& objName)
{
	for (int index{ 0 }; index < object.size(); ++index)
	{
		const auto& obj{ object[index] };

		if (obj.getName() == objName)
			return index;
	}

	return -1;
}

std::vector<float> getObjectComponents(std::vector<std::string>& args, std::array<int, 3>& pIDs, std::array<int, 3>& pCompIndex)
{
	std::vector<float> vecComponents{};

	for (int index{ 0 }; index < args.size(); ++index)
	{
		auto& arg{ args[index] };

		stripArg(arg);
		std::cout << arg << "\n";

		int objIndex{ searchObjectIndex(arg) };

		if (objIndex != -1)
		{
			pIDs[index] = object[objIndex].getID();
			pCompIndex[index] = static_cast<int>(vecComponents.size());

			for (const auto comp : object[objIndex].getComponents())
				vecComponents.push_back(comp);
		}

		else if (objIndex == -1)
		{
			extractComponents(arg, vecComponents);
			pIDs[index] = componentLiteral;
			pCompIndex[index] = static_cast<int>(vecComponents.size());
		}
	}

	return vecComponents;
}

// new architecture for drawing things
void draw(Object::Type type, std::vector<float>& vecComponents, glm::vec4 color, std::array<int, 3> pIDs, std::array<int, 3> pCompIndex, bool update)
{
	const float scale{ 0.1f };

	if (type == Object::Point)
	{
		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };

		point *= scale;

		const float radius{ 0.005f };

		if (update)
		{
			getSphereVertices(point, color, radius, vertexData);
			return;
		}

		// getSphereVertices create 120960 new float => 120960 / 7 = 17280 vertices, where 7 = number of components
		getSphereVertices(point, color, radius, vertexData);

		Object obj{ std::string(1, objectSymbols[type]++), Object::Point, GL_LINES };
		createObject(std::move(obj), 17280, vecComponents, color, 0, pIDs);

		updateBufferData(vertexData);
		}

	else if (type == Object::Vector)
	{
		int startA{ pCompIndex[0] };
		int startB{ pCompIndex[1] };

		glm::vec3 pointA{};
		glm::vec3 pointB{};

		//if (startB == -1)
		//{
		//	pCompIndex[1] = 3;

		//	int start{ startA };

		//	std::cout << "passei aqui1\n";

		//	pointA = { 0.0f, 0.0f, 0.0f };
		//	pointB = { vecComponents[start], vecComponents[start + 1], vecComponents[start + 2] };

		//	std::cout << "passei aqui2\n";

		//	vecComponents.clear();

		//	vecComponents.push_back(pointA.x);
		//	vecComponents.push_back(pointA.y);
		//	vecComponents.push_back(pointA.z);

		//	vecComponents.push_back(pointB.x);
		//	vecComponents.push_back(pointB.y);
		//	vecComponents.push_back(pointB.z);
		//}

		if (startA != -1 && startB != -1)
		{
			pointA = { vecComponents[startA], vecComponents[startA + 1], vecComponents[startA + 2] };
			pointB = { vecComponents[startB], vecComponents[startB + 1], vecComponents[startB + 2] };
		}

		pointA *= scale;
		pointB *= scale;

		const float radius{ 0.0015f };

		if (update)
		{
			getCilinderVertices(pointA, pointB, color, radius, vertexData);
			return;
		}

		// getCilinderVertices creates 144 new vertices 
		getCilinderVertices(pointA, pointB, color, radius, vertexData);

		Object obj{ std::string(1, objectSymbols[type]++), Object::Vector, GL_LINES};
		createObject(std::move(obj), 144, vecComponents, color, 2, pIDs, pCompIndex);

		updateBufferData(vertexData);
	}

	else if (type == Object::Segment)
	{
		int startA{ pCompIndex[0] };
		int startB{ pCompIndex[1] };

		glm::vec3 pointA{ vecComponents[startA], vecComponents[startA + 1], vecComponents[startA + 2] };
		glm::vec3 pointB{ vecComponents[startB], vecComponents[startB + 1], vecComponents[startB + 2] };

		pointA *= scale;
		pointB *= scale;

		const float radius{ 0.0015f };

		if (update)
		{
			getCilinderVertices(pointA, pointB, color, radius, vertexData);
			return;
		}
		// getCilinderVertices create 144 new vertices
		getCilinderVertices(pointA, pointB, color, radius, vertexData);

		Object obj{ std::string(1, objectSymbols[type]++), Object::Segment, GL_LINES };
		createObject(obj, 144, vecComponents, color, 2, pIDs, pCompIndex);

		updateBufferData(vertexData);
	}

	else if (type == Object::Plane)
	{
		int startNormal{ pCompIndex[0] };
		int startPoint{ pCompIndex[1] };

		glm::vec3 normalP0{ vecComponents[startNormal], vecComponents[startNormal + 1], vecComponents[startNormal + 2] };
		glm::vec3 normalP{ vecComponents[startNormal + 3], vecComponents[startNormal + 4], vecComponents[startNormal + 5] };
		glm::vec3 point{ vecComponents[startPoint], vecComponents[startPoint + 1], vecComponents[startPoint + 2] };

		normalP0 *= scale;
		normalP *= scale;
		point *= scale;

		color.w = 0.2f;

		if (update)
		{
			getPlaneVertices(normalP0, normalP, point, color, vertexData);
			return;
		}

		// getCilinderVertices create 6 new vertices
		getPlaneVertices(normalP0, normalP, point, color, vertexData);

		Object obj{ std::string(1, objectSymbols[type]++), Object::Plane, GL_TRIANGLES };
		createObject(obj, 6, vecComponents, color, 3, pIDs, pCompIndex);

		updateBufferData(vertexData);
	}
}
