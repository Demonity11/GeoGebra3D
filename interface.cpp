#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "draw_utils.h"

static char inputBuffer[128] = "";
std::string inputText{};
std::stringstream ss{};

std::vector<std::string> functions
{
	"Vector(",
	"Point("
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
			auto funcOpenParenthesisPos{ functions[0].length() - 1};
			auto funcCloseParenthesisPos{ inputText.find(")") };

			if (funcCloseParenthesisPos > funcOpenParenthesisPos)
			{
				std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

				std::vector<float> vecComponents{ isComponentsConvertible(parameters) };

				if (vecComponents.size() == 3)
					draw(funcType::Vector, vecComponents, glm::vec3(1.0f, 1.0f, 0.2f));
			}
		}

		if (inputText.find(functions[1]) == 0)
		{
			auto funcOpenParenthesisPos{ functions[1].length() - 1 };
			auto funcCloseParenthesisPos{ inputText.find(")") };

			if (funcCloseParenthesisPos > funcOpenParenthesisPos)
			{
				std::string parameters{ inputText.substr(funcOpenParenthesisPos + 1, funcCloseParenthesisPos - funcOpenParenthesisPos - 1) };

				std::vector<float> vecComponents{ isComponentsConvertible(parameters) };

				if (vecComponents.size() == 3)
					draw(funcType::Point, vecComponents, glm::vec3(0.0f, 0.0f, 0.0f));
			}
		}
	}

	ImGui::End();
}

// returns a empty vector if something goes wrong
std::vector<float> isComponentsConvertible(std::string parameters)
{
	auto commaPos{ parameters.find(",") };
	std::vector<float> vecComponents{};

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
			return std::vector<float>{};
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
				return std::vector<float>{};
			}

			break;
		}
	}

	return vecComponents;
}

void draw(funcType type, const std::vector<float>& vecComponents, const glm::vec3& color)
{
	const float scale{ 0.1f };

	if (type == funcType::Vector)
	{
		glm::vec3 vector{ vecComponents[0], vecComponents[1], vecComponents[2] };

		vector *= scale;

		getCilinderVertices(glm::vec3(0.0f, 0.0f, 0.0f), vector, color, 0.0015f, vertexData);

		updateBufferData(vertexData);
	}

	if (type == funcType::Point)
	{
		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };

		point *= scale;

		getSphereVertices(point, color, 0.005f, vertexData);

		updateBufferData(vertexData);
	}
}