#include "Window.h"
#include "draw_utils.h"
#include "Object.h"

// legacy
//struct FuncArgs
//{
//	std::string name{};
//	funcType type{};
//	std::vector<funcType> expectedArgs{};
//};

// legacy
//std::vector<FuncArgs> functions
//{
//	{"Point(",   funcType::Point,   {}                                  },
//	{"Vector(",  funcType::Vector,  {funcType::Point,  funcType::Point} },
//	{"Vector(",  funcType::Vector,  {funcType::Point}                   },
//	{"Segment(", funcType::Segment, {funcType::Point,  funcType::Point} },
//	{"Plane(",   funcType::Plane,   {funcType::Vector, funcType::Point} }
//};

// legacy
//std::map<funcType, char> objSymbols
//{
//	{ funcType::Vector,  'u' },
//	{ funcType::Point,   'A' },
//	{ funcType::Segment, 'f' },
//	{ funcType::Plane,   'p' },
//};

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
	{"Vector(",  Object::Vector,  {Object::Point}                 },
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

// new symbolTable
std::map<std::string, int> symbolTable{};

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

		// ============= legacy ================
		//for (const auto& func : functions)
		//{
		//	auto funcOpenParenthesisPos{ inputText.find("(") };
		//	auto funcCloseParenthesisPos{ (inputText.rfind(")") != std::string::npos) ? inputText.rfind(")") : 0 };

		//	if (funcCloseParenthesisPos > funcOpenParenthesisPos)
		//	{
		//		std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

		//		std::vector<std::string> args{ splitArgs(parameters) };

		//		if (func.type == funcType::Point && args.size() == 3)
		//		{
		//			std::vector<float> vecComponents{};
		//			extractComponents(parameters, vecComponents);

		//			draw_legacy(func.type, vecComponents, glm::vec4{ 0.0f, 0.2f, 0.5f, 1.0f });
		//		}

		//		if (inputText.find(func.name) == 0 && args.size() == func.expectedArgs.size())
		//		{
		//			bool isObjectValid{ true };

		//			for (int index{ 0 }; index < args.size(); ++index)
		//			{
		//				if (func.expectedArgs.size() != args.size())
		//				{
		//					isObjectValid = false;
		//					break;
		//				}

		//				isObjectValid = compareObjectType_legacy(args[index], func.expectedArgs[index]);

		//				if (!isObjectValid)
		//				{
		//					isObjectValid = false;
		//					break;
		//				}
		//			}

		//			if (isObjectValid)
		//			{
		//				std::vector<float> vecComponents{ getObjectComponents(args) };

		//				int componentsCount{ static_cast<int>(vecComponents.size()) };

		//				draw_legacy(func.type, vecComponents, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
		//			}
		//		}
		//	}
		//}
		// ============= legacy ================

		// ============= new ================
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
						std::vector<float> vecComponents{ getObjectComponents(args) };

						int componentsCount{ static_cast<int>(vecComponents.size()) };

						draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
					}
				}
			}
		}
		// ============= new ================
	}

	ImGui::SeparatorText("Variables");

	// =============== legacy ================
	//for(int i{ 8 }; i < objInfo.size(); ++i)
	//{
	//	auto& obj{ objInfo[i] };

	//	std::string headerText{ obj.name + " : " + getStringFuncType(obj.type) };

	//	if (ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_None))
	//	{
	//		char s{ 'A' };
	//		for (int increment{ 0 }; increment < obj.components.size(); increment += 3)
	//		{
	//			ImGui::InputFloat3((obj.name + "::" + s).c_str(), &obj.components[increment], "%.2f");

	//			if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
	//				updateObject_legacy(i, obj);

	//			++s;
	//		}

	//		ImGui::InputFloat4((obj.name + "::Color").c_str(), &obj.color[0], "%.2f");

	//		if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
	//			updateObject_legacy(i, obj);

	//		if (ImGui::Button("Delete"))
	//		{
	//			vertexData = deleteObjectFromVertexData_legacy(symbolTable[obj.name]);
	//			updateObjectName(obj.type, true);
	//			deleteObjectRegister(i);
	//			updateBufferData(vertexData);
	//		}
	//	}
	//}
	// =============== legacy ================

	// =============== new ================
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

			//if (ImGui::Button("Delete"))
			//{
			//	vertexData = deleteObjectFromVertexData_legacy(symbolTable[obj.name]);
			//	updateObjectName(obj.type, true);
			//	deleteObjectRegister(i);
			//	updateBufferData(vertexData);
			//}
		}
	}
	// =============== new ================

	ImGui::End();
}

// legacy function to convert funcType to std::string
//std::string getStringFuncType(funcType type)
//{
//	switch (type)
//	{
//	case funcType::Point:   return "Point";
//	case funcType::Vector:  return "Vector";
//	case funcType::Segment: return "Segment";
//	case funcType::Plane:   return "Plane";
//	}
//}

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

// legacy function for comparing objects
//bool compareObjectType_legacy(const std::string& component, funcType expectedType)
//{
//	int objID{ searchObjectID(component) };
//
//	if (objID != -1)
//	{
//		if (objInfo[objID].type != expectedType)
//			return false;
//	}
//
//	return true;
//}

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

int searchObjectID(const std::string& objName)
{
	for (const auto& [name, id] : symbolTable)
	{
		if (objName == name)
			return id;
	}

	return -1; // if object is not found
}

std::vector<float> getObjectComponents(std::vector<std::string>& args)
{
	std::vector<float> vecComponents{};

	for (auto& arg : args)
	{
		stripArg(arg);
		std::cout << arg << "\n";


		int objID{ searchObjectID(arg) };

		if (objID != -1)
		{
			for (const auto comp : objInfo[objID].components)
				vecComponents.push_back(comp);

		}

		else if (objID == -1)
		{
			extractComponents(arg, vecComponents);
		}
	}

	return vecComponents;
}

// new architecture for drawing things
void draw(Object::Type type, const std::vector<float>& vecComponents, glm::vec4 color, std::array<int, 3> pIDs, bool update)
{
	const float scale{ 0.1f };

	if (type == Object::Vector)
	{
		glm::vec3 pointA{ 0.0f, 0.0f, 0.0f };
		glm::vec3 pointB{ vecComponents[0], vecComponents[1], vecComponents[2] };

		std::vector<float> newComponents{};

		if (vecComponents.size() == 6)
		{
			pointA = glm::vec3(vecComponents[0], vecComponents[1], vecComponents[2]);
			pointB = glm::vec3(vecComponents[3], vecComponents[4], vecComponents[5]);
		}
		else
		{
			newComponents.push_back(pointA.x);
			newComponents.push_back(pointA.y);
			newComponents.push_back(pointA.z);

			newComponents.push_back(pointB.x);
			newComponents.push_back(pointB.y);
			newComponents.push_back(pointB.z);
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
		createObject(std::move(obj), 144, (newComponents.size() > 0 ? newComponents : vecComponents), color, 2, pIDs);

		updateBufferData(vertexData);
	}

	else if (type == Object::Point)
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

	else if (type == Object::Segment)
	{
		glm::vec3 pointA{ vecComponents[0], vecComponents[1], vecComponents[2] };
		glm::vec3 pointB{ vecComponents[3], vecComponents[4], vecComponents[5] };

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
		createObject(obj, 144, vecComponents, color, 2, pIDs);

		updateBufferData(vertexData);
	}

	else if (type == Object::Plane)
	{
		glm::vec3 normalP0{ 0.0f, 0.0f, 0.0f };
		glm::vec3 normalP{ vecComponents[0], vecComponents[1], vecComponents[2] };
		glm::vec3 point{ vecComponents[3], vecComponents[4], vecComponents[5] };

		if (vecComponents.size() == 9) // get back here later.
		{
			normalP0 = glm::vec3(vecComponents[0], vecComponents[1], vecComponents[2]);
			normalP = glm::vec3(vecComponents[3], vecComponents[4], vecComponents[5]);
			point = glm::vec3(vecComponents[6], vecComponents[7], vecComponents[8]);
		}

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
		createObject(obj, 6, vecComponents, color, 3, pIDs);

		updateBufferData(vertexData);
	}
}

// legacy function for drawing things
//void draw_legacy(funcType type, const std::vector<float>& vecComponents, glm::vec4 color, bool update)
//{
//	const float scale{ 0.1f };
//
//	if (type == funcType::Vector)
//	{
//		glm::vec3 pointA{ 0.0f, 0.0f, 0.0f };
//		glm::vec3 pointB{ vecComponents[0], vecComponents[1], vecComponents[2] };
//
//		std::vector<float> newComponents{};
//
//		if (vecComponents.size() == 6)
//		{
//			pointA = glm::vec3(vecComponents[0], vecComponents[1], vecComponents[2]);
//			pointB = glm::vec3(vecComponents[3], vecComponents[4], vecComponents[5]);
//		}
//		else
//		{
//			newComponents.push_back(pointA.x);
//			newComponents.push_back(pointA.y);
//			newComponents.push_back(pointA.z);
//
//			newComponents.push_back(pointB.x);
//			newComponents.push_back(pointB.y);
//			newComponents.push_back(pointB.z);
//		}
//
//		pointA *= scale;
//		pointB *= scale;
//
//		const float radius{ 0.0015f };
//
//		if (update)
//		{
//			getCilinderVertices(pointA, pointB, color, radius, vertexData);
//			return;
//		}
//
//		getCilinderVertices(pointA, pointB, color, radius, vertexData);
//
//		// getCilinderVertices create 1008 new vertices
//		addNewObject(144, GL_LINES, funcType::Vector, std::string(1, updateObjectName(type)), (newComponents.size() > 0 ? newComponents : vecComponents), color);
//
//		updateBufferData(vertexData);
//	}
//
//	else if (type == funcType::Point)
//	{
//		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };
//
//		point *= scale;
//
//		const float radius{ 0.005f };
//
//		if (update)
//		{
//			getSphereVertices(point, color, radius, vertexData);
//			return;
//		}
//
//		getSphereVertices(point, color, radius, vertexData);
//
//		// getSphereVertices create 120960 new vertices => 120960 / 7 = 17280, where 7 = number of components
//		addNewObject(17280, GL_LINES, funcType::Point, std::string(1, updateObjectName(type)), vecComponents, color);
//
//		updateBufferData(vertexData);
//	}
//
//	else if (type == funcType::Segment)
//	{
//		glm::vec3 pointA{ vecComponents[0], vecComponents[1], vecComponents[2] };
//		glm::vec3 pointB{ vecComponents[3], vecComponents[4], vecComponents[5] };
//
//		pointA *= scale;
//		pointB *= scale;
//
//		const float radius{ 0.0015f };
//
//		if (update)
//		{
//			getCilinderVertices(pointA, pointB, color, radius, vertexData);
//			return;
//		}
//
//		getCilinderVertices(pointA, pointB, color, radius, vertexData);
//
//		// getCilinderVertices create 1008 new vertices
//		addNewObject(144, GL_LINES, funcType::Segment, std::string(1, updateObjectName(type)), vecComponents, color);
//
//		updateBufferData(vertexData);
//	}
//
//	else if (type == funcType::Plane)
//	{
//		glm::vec3 normalP0{ 0.0f, 0.0f, 0.0f };
//		glm::vec3 normalP { vecComponents[0], vecComponents[1], vecComponents[2] };
//		glm::vec3 point{ vecComponents[3], vecComponents[4], vecComponents[5] };
//
//		if (vecComponents.size() == 9) // get back here later.
//		{
//			normalP0 = glm::vec3(vecComponents[0], vecComponents[1], vecComponents[2]);
//			normalP =  glm::vec3(vecComponents[3], vecComponents[4], vecComponents[5]);
//			point = glm::vec3(vecComponents[6], vecComponents[7], vecComponents[8]);
//		}
//
//		normalP0 *= scale;
//		normalP  *= scale;
//		point    *= scale;
//
//		color.w = 0.2f;
//
//		if (update)
//		{
//			getPlaneVertices(normalP0, normalP, point, color, vertexData);
//			return;
//		}
//
//		getPlaneVertices(normalP0, normalP, point, color, vertexData);
//
//		addNewObject(6, GL_TRIANGLES, funcType::Plane, std::string(1, updateObjectName(type)), vecComponents, color);
//
//		updateBufferData(vertexData);
//	}
//}

//char updateObjectName(funcType type, bool isObjectDeleted)
//{
//	if (isObjectDeleted)
//	{
//		return --objSymbols[type];
//	}
//	
//	//if (objID != -1)
//	//{
//	//	for (auto& [id, obj] : objInfo)
//	//	{
//	//		if (obj.name.c_str() == 'a')
//	//	}
//	//}
//
//	if (objSymbols[type] + 1 == 'x' && type == funcType::Vector)
//	{
//		objSymbols[type] = 'a';
//		return objSymbols[type];
//	}
//
//	return objSymbols[type]++;
//}
