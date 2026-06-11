#include "Window.h"
#include "draw_utils.h"

static char inputBuffer[128] = "";
std::string inputText{};
std::stringstream ss{};

std::vector<std::string> functions
{
	"Vector(",
	"Point(",
	"Segment(",
	"Plane("
};

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
	ImGui::Begin("Input");
	ImGui::InputText("Input", inputBuffer, IM_COUNTOF(inputBuffer));
	if (ImGui::Button("Enter"))
	{
		ss = std::stringstream(inputBuffer);
		inputText = ss.str();

		if (inputText.find(functions[0]) == 0)
		{
			auto funcOpenParenthesisPos{ inputText.find("(") };
			auto funcCloseParenthesisPos{ (inputText.rfind(")") != std::string::npos) ? inputText.rfind(")") : 0 };

			if (funcCloseParenthesisPos > funcOpenParenthesisPos)
			{
				std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

				std::vector<std::string> args{ splitArgs(parameters) };

				bool isObjectValid{ false };

				for (auto& arg : args)
				{
					isObjectValid = compareObjectType(arg, funcType::Point);

					if (!isObjectValid)
						break;
				}

				if (isObjectValid)
				{
					std::vector<float> vecComponents{ getObjectComponents(args) };

					int componentsCount{ static_cast<int>(vecComponents.size()) };

					if (componentsCount == 6 || componentsCount == 3)
						draw(funcType::Vector, vecComponents, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				}
			}
		}

		else if (inputText.find(functions[1]) == 0)
		{
			auto funcOpenParenthesisPos{ inputText.find("(") };
			auto funcCloseParenthesisPos{ (inputText.rfind(")") != std::string::npos) ? inputText.rfind(")") : 0 };

			if (funcCloseParenthesisPos > funcOpenParenthesisPos)
			{
				std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

				std::vector<float> vecComponents{};
				extractComponents(parameters, vecComponents);

				if (vecComponents.size() == 3)
					draw(funcType::Point, vecComponents, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			}
		}

		else if (inputText.find(functions[2]) == 0)
		{
			auto funcOpenParenthesisPos{ inputText.find("(") };
			auto funcCloseParenthesisPos{ (inputText.rfind(")") != std::string::npos) ? inputText.rfind(")") : 0 };

			if (funcCloseParenthesisPos > funcOpenParenthesisPos)
			{
				std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

				std::vector<std::string> args{ splitArgs(parameters) };

				bool isObjectValid{ false };

				for (auto& arg : args)
				{
					isObjectValid = compareObjectType(arg, funcType::Point);

					if (!isObjectValid)
						break;
				}

				if (isObjectValid)
				{
					std::vector<float> vecComponents{ getObjectComponents(args) };

					int componentsCount{ static_cast<int>(vecComponents.size()) };

					if (componentsCount == 6)
						draw(funcType::Segment, vecComponents, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				}
			}
		}

		else if (inputText.find(functions[3]) == 0)
		{

		}
	}

	ImGui::End();
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

bool compareObjectType(const std::string& component, funcType expectedType)
{
	int objID{ searchObjectID(component) };

	if (objID != -1)
	{
		if (objInfo[objID].type != expectedType)
			return false;
	}

	return true;
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
			vecComponents.push_back(objInfo[objID].components[0]);
			vecComponents.push_back(objInfo[objID].components[1]);
			vecComponents.push_back(objInfo[objID].components[2]);
		}

		else if (objID == -1)
		{
			extractComponents(arg, vecComponents);
		}
	}

	return vecComponents;
}

void draw(funcType type, const std::vector<float>& vecComponents, const glm::vec4& color)
{
	const float scale{ 0.1f };

	static char vecSymbol{ 'u' };
	static char pointSymbol{ 'A' };
	static char segmentSymbol{ 'f' };

	if (type == funcType::Vector)
	{
		glm::vec3 pointA{ 0.0f, 0.0f, 0.0f };
		glm::vec3 pointB{ vecComponents[0], vecComponents[1], vecComponents[2] };

		if (vecComponents.size() == 6)
		{
			pointA = glm::vec3(vecComponents[0], vecComponents[1], vecComponents[2]);
			pointB = glm::vec3(vecComponents[3], vecComponents[4], vecComponents[5]);
		}

		pointA *= scale;
		pointB *= scale;

		const float radius{ 0.0015f };

		getCilinderVertices(pointA, pointB, color, radius, vertexData);

		// getCilinderVertices create 1008 new vertices
		addNewObject(144, GL_LINES, funcType::Vector, std::string(1, vecSymbol), vecComponents);

		++vecSymbol;

		if (vecSymbol == 'x')
			vecSymbol = 'a';

		updateBufferData(vertexData);
	}

	else if (type == funcType::Point)
	{
		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };

		point *= scale;

		const float radius{ 0.005f };

		getSphereVertices(point, color, radius, vertexData);

		// getSphereVertices create 120960 new vertices => 120960 / 7 = 17280, where 7 = number of components
		addNewObject(17280, GL_LINES, funcType::Point, std::string(1, pointSymbol), vecComponents);

		++pointSymbol;

		updateBufferData(vertexData);
	}

	else if (type == funcType::Segment)
	{
		glm::vec3 pointA{ vecComponents[0], vecComponents[1], vecComponents[2] };
		glm::vec3 pointB{ vecComponents[3], vecComponents[4], vecComponents[5] };

		pointA *= scale;
		pointB *= scale;

		const glm::vec3 origin{ 0.0f, 0.0f, 0.0f };
		const float radius{ 0.0015f };

		getCilinderVertices(pointA, pointB, color, radius, vertexData);

		// getCilinderVertices create 1008 new vertices
		addNewObject(144, GL_LINES, funcType::Segment, std::string(1, segmentSymbol), vecComponents);

		++segmentSymbol;

		updateBufferData(vertexData);
	}
}