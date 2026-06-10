#include "Window.h"
#include "draw_utils.h"

static char inputBuffer[128] = "";
std::string inputText{};
std::stringstream ss{};

std::vector<std::string> functions
{
	"Vector(",
	"Point(",
	"Segment("
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

				std::vector<float> vecComponents{};
				extractComponents(parameters, vecComponents);

				if (vecComponents.size() == 3)
					draw(funcType::Vector, vecComponents, glm::vec3(1.0f, 1.0f, 0.2f));
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
					draw(funcType::Point, vecComponents, glm::vec3(0.0f, 0.0f, 0.0f));
			}
		}

		else if (inputText.find(functions[2]) == 0)
		{
			auto funcOpenParenthesisPos{ inputText.find("(") };
			auto funcCloseParenthesisPos{ (inputText.rfind(")") != std::string::npos) ? inputText.rfind(")") : 0 };

			if (funcCloseParenthesisPos > funcOpenParenthesisPos)
			{
				std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

				funcOpenParenthesisPos = parameters.find("(");
				funcCloseParenthesisPos = parameters.find(")");

				std::string parametersPointA{};
				std::string parametersPointB{};

				if (funcCloseParenthesisPos > funcOpenParenthesisPos)
				{
					parametersPointA = parameters.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1);
					parameters = parameters.substr(funcCloseParenthesisPos + 1, parameters.length() - 1);
				}

				funcOpenParenthesisPos = parameters.find("(");
				funcCloseParenthesisPos = parameters.find(")");

				if (parameters.substr(funcCloseParenthesisPos, parameters.length() - 1).rfind("(") != std::string::npos
			   	 || parameters.substr(funcCloseParenthesisPos, parameters.length() - 1).rfind(")") != std::string::npos)
				{
					funcCloseParenthesisPos = 0;
				}

				if (funcCloseParenthesisPos > funcOpenParenthesisPos)
				{
					parametersPointB = parameters.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1);
				}

				std::vector<float> vecComponents{};
				extractComponents(parametersPointA, vecComponents);
				extractComponents(parametersPointB, vecComponents);

				std::cout << vecComponents.size() << "\n";
				if (vecComponents.size() == 6)
					draw(funcType::Segment, vecComponents, glm::vec3(0.0f, 0.0f, 0.0f));
			}
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

void draw(funcType type, const std::vector<float>& vecComponents, const glm::vec3& color)
{
	const float scale{ 0.1f };

	if (type == funcType::Vector)
	{
		glm::vec3 vector{ vecComponents[0], vecComponents[1], vecComponents[2] };

		vector *= scale;

		const glm::vec3 origin{ 0.0f, 0.0f, 0.0f };
		const float radius{ 0.0015f };

		getCilinderVertices(origin, vector, color, radius, vertexData);

		updateBufferData(vertexData);
	}

	else if (type == funcType::Point)
	{
		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };

		point *= scale;

		const float radius{ 0.005f };

		getSphereVertices(point, color, radius, vertexData);

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

		updateBufferData(vertexData);
	}
}