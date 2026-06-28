#include "Window.h"
#include "draw_utils.h"
#include "utilities.h"
#include "objectCoords.h"
#include "Context.h"

// initializes ImGui context
void initializeImGui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); static_cast<void>(io);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

// captures user input through Dear ImGui interface
void getUserInput(const std::vector<FunctionArgs>& function, std::vector<Object>& object)
{
	static char inputBuffer[128] = "";

	ImGui::Begin("Input");
	ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

	//static auto inputArray{ testInput("Point(0,1,0)\nVector(A)\nPlane(A,u)\nPoint(1,-2,3)\nPoint(2,2.5,-3)\nVector(B,C)\nLine(B, v)\nIntersect(p,r)\n") };
	static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nLine(A,B)\nPoint(3,3,5)\nPoint(3,3,7)\nLine(C,D)\nIntersect(r,s)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nLine(A,B)\nPoint(3,3,5)\nPoint(3,2,7)\nLine(C,D)\nIntersect(r,s)\n") };
	//static auto inputArray{ testInput("") };

	if (ImGui::InputTextWithHint("Input", "input", inputBuffer, IM_COUNTOF(inputBuffer), flags))
	{
		auto ss{ std::stringstream(inputBuffer) };
		auto inputText{ ss.str() };

		// types input faster for testing
		if (!inputArray.empty())
		{
			inputText = inputArray[0];
			inputArray.erase(inputArray.begin());
		}

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
					convertParametersToFloat(parameters, vecComponents);

					if (!scanForIdenticalObject(func.type, vecComponents, object))
					{
						draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.2f, 0.5f, 1.0f });
						inputBuffer[0] = '\0';
					}
				}

				else if (inputText.find(func.name) == 0 && args.size() == func.expectedArgs.size())
				{
					bool isObjectValid{ true };

					for (int index{ 0 }; index < args.size(); ++index)
					{
						if (func.expectedArgs.size() != args.size())
						{
							isObjectValid = false;
							break;
						}

						isObjectValid = compareObjectType(args[index], func.expectedArgs[index], object);

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

						std::vector<float> vecComponents{};

						if (args.size() == 1)
						{
							pIDs[0] = Context::componentLiteral;
							pCompIndex[0] = 0;

							vecComponents = { 0.0f, 0.0f, 0.0f };

							getObjectComponents(args, vecComponents, pIDs, pCompIndex);

							if (!scanForIdenticalObject(func.type, vecComponents, object))
								draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, pIDs, pCompIndex);
						}

						else
						{
							getObjectComponents(args, vecComponents, pIDs, pCompIndex);

							if (!scanForIdenticalObject(func.type, vecComponents, object))
								draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, pIDs, pCompIndex);
						}

						inputBuffer[0] = '\0';
					}
				}
			}
		}
	}

	ImGui::SeparatorText("Variables");

	for (int i{ 8 }; i < object.size(); ++i)
	{
		auto& obj{ object[i]};

		std::string headerText{ obj.getName() + ": " + getExpression(obj, object) + "###" + obj.getName() };

		if (ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_None))
		{
			if (obj.getType() == Object::Plane || obj.getType() == Object::Line) ImGui::Text(getEquation(obj).c_str());

			char s{ 'A' };
			for (int increment{ 0 }; increment < obj.getComponents().size(); increment += 3)
			{
				ImGuiInputFlags textFlags{};

				if (!obj.isMutable()) textFlags |= ImGuiInputTextFlags_ReadOnly;
					
				ImGui::InputFloat3((obj.getName() + "::" + s).c_str(), obj.getComponentsPointer() + increment, "%.2f", textFlags);

				if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
					updateObject(i, obj, object, Context::vertexData);

				++s;
			}

			ImGui::InputFloat4((obj.getName() + "::Color").c_str(), obj.getColorPointer(), "%.2f");

			if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
				updateObject(i, obj, object, Context::vertexData);

			std::string deleteText{ "Delete###" + std::to_string(obj.getID()) };

			if (ImGui::Button(deleteText.c_str()))
			{
				deleteObject(i, object, Context::vertexData);
			}
		}
	}

	ImGui::End();
}

// draw objects such as Points, Vectors, Planes, etc.
void draw(Object::Type type, std::vector<float>& vecComponents, glm::vec4 color, std::array<int, 3> pIDs, std::array<int, 3> pCompIndex, bool update)
{
	const float scale{ 0.1f };

	// intersection
	if (type == Object::Point && pIDs[0] != -1)
	{
		std::array<Object, 2> args{};

		for (int i{ 0 }; i < pIDs.size(); ++i)
		{
			if (pIDs[i] >= 0)
			{
				int index{ searchObjectByID(pIDs[i], Context::object) };
				const auto& obj{ Context::object[index] };

				args[i] = obj;
			}
		}

		int lineCount{ 0 };

		std::array<glm::vec3, 2> points{};
		std::array<glm::vec3, 2> vectors{};
		std::array<Object::Type, 2> types{};

		for (int i{ 0 }; i < args.size(); ++i)
		{
			const auto& arg{ args[i] };

			if (arg.getType() == Object::Line)
			{
				int parentIndex1{ searchObjectByID(arg.getParentIDs()[0], Context::object) };
				auto comp{ Context::object[parentIndex1].getComponents() };

				points[i] = { comp[0], comp[1], comp[2] };

				int parentIndex2{ searchObjectByID(arg.getParentIDs()[1], Context::object) };
				comp = Context::object[parentIndex2].getComponents();

				if (Context::object[parentIndex2].getType() == Object::Vector)
					vectors[i] = {comp[3] - comp[0], comp[4] - comp[1], comp[5] - comp[2]};

				else if (Context::object[parentIndex2].getType() == Object::Point)
					vectors[i] = {comp[0] - points[i].x, comp[1] - points[i].y, comp[2] - points[i].z};

				++lineCount;
				types[i] = Object::Line;
			}

			else if (arg.getType() == Object::Plane)
			{
				int parentIndex1 = searchObjectByID(arg.getParentIDs()[0], Context::object);
				auto comp{ Context::object[parentIndex1].getComponents() };

				points[i] = { comp[0], comp[1], comp[2] };

				int parentIndex2{ searchObjectByID(arg.getParentIDs()[1], Context::object) };
				comp = Context::object[parentIndex2].getComponents();

				points[i] = {comp[3] - comp[0], comp[4] - comp[1], comp[5] - comp[2]};

				types[i] = Object::Plane;
			}
		}

		glm::vec3 intersection{};

		if (lineCount == 2)
		{
			intersection = intersectionLineLine(points[0], vectors[0], points[1], vectors[1]);
		}

		else
		{
			float d{};
			if (types[0] == Object::Plane) 
			{
				d = -glm::dot(points[0], vectors[0]);
				intersection = intersectionLinePlane(points[1], vectors[1], points[0], d);
			}
			else 
			{
				d = -glm::dot(points[1], vectors[1]);
				intersection = intersectionLinePlane(points[0], vectors[0], points[1], d);
			}
		}

		if (intersection == glm::vec3(-9999.0f, -9999.0f, -9999.0f))
		{
			std::cerr << "Intersection doesn't exist. Handle later.\n";
			return;
		}

		std::vector components{ intersection.x, intersection.y, intersection.z };

		intersection *= scale;

		const float radius{ 0.005f };
		color = { 0.7f, 0.3f, 0.0f, 1.0f };

		if (update)
		{
			getSphereVertices(intersection, color, radius, Context::vertexData);
			return;
		}

		if (scanForIdenticalObject(type, components, Context::object))
		{
			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
			return;
		}

		getSphereVertices(intersection, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Point, GL_LINES };
		obj.setMutable(false);
		createObject(std::move(obj), 17280, components, color, 2, pIDs, pCompIndex);
			
		updateBufferData(Context::vertexData);
	}

	else if (type == Object::Point)
	{
		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };

		point *= scale;

		const float radius{ 0.005f };

		if (update)
		{
			getSphereVertices(point, color, radius, Context::vertexData);
			return;
		}

		// getSphereVertices create 120960 new floats => 120960 / 7 = 17280 vertices, where 7 = number of components
		getSphereVertices(point, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Point, GL_LINES };
		createObject(std::move(obj), 17280, vecComponents, color, 0, pIDs);

		updateBufferData(Context::vertexData);
		}

	else if (type == Object::Vector)
	{
		int startA{ pCompIndex[0] };
		int startB{ pCompIndex[1] };

		glm::vec3 pointA{};
		glm::vec3 pointB{};

		pointA = { vecComponents[startA], vecComponents[startA + 1], vecComponents[startA + 2] };
		pointB = { vecComponents[startB], vecComponents[startB + 1], vecComponents[startB + 2] };

		pointA *= scale;
		pointB *= scale;

		const float radius{ 0.0015f };

		if (update)
		{
			getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);
			return;
		}

		// getCilinderVertices creates 144 new vertices 
		getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Vector, GL_LINES};
		createObject(std::move(obj), 144, vecComponents, color, 2, pIDs, pCompIndex);

		updateBufferData(Context::vertexData);
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
			getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);
			return;
		}
		// getCilinderVertices create 144 new vertices
		getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Segment, GL_LINES };
		createObject(obj, 144, vecComponents, color, 2, pIDs, pCompIndex);

		updateBufferData(Context::vertexData);
	}

	else if (type == Object::Line)
	{
		int startPoint { pCompIndex[0] };
		int startVector{ pCompIndex[1] };

		glm::vec3 point{ vecComponents[startPoint],  vecComponents[startPoint + 1],  vecComponents[startPoint + 2] };
		glm::vec3 dVectorP0{};
		glm::vec3 dVectorP{};

		if (vecComponents.size() == 6)
		{
			dVectorP0 = { vecComponents[startPoint],  vecComponents[startPoint + 1],  vecComponents[startPoint + 2] };
			dVectorP  = { vecComponents[startVector], vecComponents[startVector + 1], vecComponents[startVector + 2] };
		}

		else if (vecComponents.size() == 9)
		{
			dVectorP0 = { vecComponents[startVector],     vecComponents[startVector + 1], vecComponents[startVector + 2] };
			dVectorP  = { vecComponents[startVector + 3], vecComponents[startVector + 4], vecComponents[startVector + 5] };
		}

		point     *= scale;
		dVectorP0 *= scale;
		dVectorP  *= scale;

		const float radius{ 0.0015f };

		if (update)
		{
			getLineVertices(point, dVectorP0, dVectorP, color, radius, Context::vertexData);
			return;
		}
		// getCilinderVertices create 144 new vertices
		getLineVertices(point, dVectorP0, dVectorP, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Line, GL_LINES };
		createObject(obj, 144, vecComponents, color, 2, pIDs, pCompIndex);

		updateBufferData(Context::vertexData);
	}

	else if (type == Object::Plane)
	{
		int startPoint{ pCompIndex[0] };
		int startNormal{ pCompIndex[1] };

		glm::vec3 normalP0{ vecComponents[startNormal], vecComponents[startNormal + 1], vecComponents[startNormal + 2] };
		glm::vec3 normalP{ vecComponents[startNormal + 3], vecComponents[startNormal + 4], vecComponents[startNormal + 5] };
		glm::vec3 point{ vecComponents[startPoint], vecComponents[startPoint + 1], vecComponents[startPoint + 2] };

		normalP0 *= scale;
		normalP *= scale;
		point *= scale;

		color.w = 0.2f;

		if (update)
		{
			getPlaneVertices(normalP0, normalP, point, color, Context::vertexData);
			return;
		}

		// getCilinderVertices create 6 new vertices
		getPlaneVertices(normalP0, normalP, point, color, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Plane, GL_TRIANGLES };
		createObject(obj, 6, vecComponents, color, 3, pIDs, pCompIndex);

		updateBufferData(Context::vertexData);
	}
}
