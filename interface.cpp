#include "Window.h"
#include "draw_utils.h"
#include "utilities.h"
#include "objectCoords.h"
#include "Context.h"
#include "objectAssembling.h"
#include "Random.h"

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << vec.x << ", " << vec.y << ", " << vec.z;

	return os;
}

struct AutocompleteContext 
{
	bool textWasEdited = false;
	std::string textToInject{};
};

int AutocompleteCallback(ImGuiInputTextCallbackData* data) 
{
	AutocompleteContext* ctx{ (AutocompleteContext*)data->UserData };

	if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) 
	{
		ctx->textWasEdited = true;
	}

	if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways && !ctx->textToInject.empty())
	{
		data->DeleteChars(0, data->BufTextLen);
		data->InsertChars(0, ctx->textToInject.c_str());
		data->BufDirty = true;

		if (data->BufTextLen > 0 && data->Buf[data->BufTextLen - 1] == ')')
		{
			data->CursorPos = data->BufTextLen - 1;
		}

		ctx->textToInject = "";
	}

	return 0;
}

// initializes ImGui context
void initializeImGui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 

	if (io.WantCaptureMouse)

	io.Fonts->AddFontDefault();

	Context::spaceFont = io.Fonts->AddFontFromFileTTF("fonts/Inter_24pt-Bold.ttf", Context::fontSize);

	static_cast<void>(io);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

// captures user input through Dear ImGui interface
void getUserInput(std::vector<Object>& object)
{
	constexpr std::size_t bufferSize{ 128 };
	static char inputBuffer[bufferSize] = "";

	static ImGuiID activePopupID{ 0 };
	static int selectedIndex{ -1 };
	static bool showDropdown{ false };

	static AutocompleteContext context;
	static bool focusNextFrame{ false };

	ImGuiInputTextFlags inputFlags
	{ 
		ImGuiInputTextFlags_EnterReturnsTrue | 
		ImGuiInputTextFlags_CallbackEdit |
		ImGuiInputTextFlags_CallbackAlways
	};

	ImGuiID inputID{ ImGui::GetID("Input") };

	if (focusNextFrame)
	{
		ImGui::SetKeyboardFocusHere(0);
		focusNextFrame = false;
	}

	bool isEnterPressed{ ImGui::InputTextWithHint("Input", "input", inputBuffer, IM_COUNTOF(inputBuffer), inputFlags, AutocompleteCallback, &context) };

	bool isInputActive{ ImGui::IsItemActive() };
	bool isInputFocused{ ImGui::IsItemFocused() };

	if (isInputActive || isInputFocused) {
		activePopupID = inputID;
	}

	if (context.textWasEdited && activePopupID == inputID) {
		showDropdown = true;
	}

	std::string currentText(inputBuffer);
	std::vector<FunctionArgs> matches{};

	if (showDropdown && !currentText.empty() && activePopupID == inputID)
	{
		for (const auto& func : Context::function)
		{
			if (func.name.rfind(currentText, 0) == 0 && func.name != currentText)
				matches.push_back(func);
		}
	}

	if (!matches.empty() && activePopupID == inputID && showDropdown)
	{
		ImVec2 inputPosMin{ ImGui::GetItemRectMin() };
		ImVec2 inputPosMax{ ImGui::GetItemRectMax() };

		ImGui::SetNextWindowPos(ImVec2(inputPosMin.x, inputPosMax.y));
		ImGui::SetNextWindowSize(ImVec2(inputPosMax.x - inputPosMin.x, 0));

		ImGuiWindowFlags flags
		{
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_ChildWindow
		};

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
		
		if (ImGui::Begin("##AutocompletePopup", nullptr, flags))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) { selectedIndex++; }
			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) { selectedIndex--; }

			if (selectedIndex < 0) selectedIndex = static_cast<int>(matches.size() - 1);
			if (selectedIndex >= static_cast<int>(matches.size())) selectedIndex = 0;

			for (int i{ 0 }; i < static_cast<int>(matches.size()); ++i) 
			{
				const bool isSelected{ (i == selectedIndex) };

				const auto& match{ matches[i] };

				std::string func{ match.name + "(" };

				if (match.name == "Point")
					func += ")";

				for (std::size_t i{ 0 }; i < match.expectedArgs.size(); ++i)
				{
					const auto arg{ getStringFunctionType(match.expectedArgs[i]) };

					if (i < match.expectedArgs.size() - 1)
						func += arg + ", ";
					else
						func += arg + ")";
				}

				if (ImGui::Selectable(func.c_str(), isSelected))
				{
					context.textToInject = matches[i].name + "()";
					context.textWasEdited = false;
					showDropdown = false;
					selectedIndex = -1;
					focusNextFrame = true;
				}

				if (isSelected && ImGui::IsKeyPressed(ImGuiKey_Tab))
				{
					context.textToInject = matches[i].name + "()";
					context.textWasEdited = false;
					showDropdown = false;
					selectedIndex = -1;
					focusNextFrame = true;
				}
			}
		}

		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseClicked(0)) 
		{
			showDropdown = false;
			selectedIndex = -1;
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	if (isEnterPressed) 
	{
		showDropdown = false;
		selectedIndex = -1;

		processInput(inputBuffer, Context::function, object);

		ImGui::SetKeyboardFocusHere(-1);
	}

	ImGui::SeparatorText("Variables");
}

void processInput(char inputBuffer[128], const std::vector<FunctionArgs>& function, std::vector<Object>& object)
{
	auto ss{ std::stringstream(inputBuffer) };
	auto inputText{ ss.str() };

	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(3,3,3)\nSegment(A,B)\nVector(A)\nLine(A,u)\nPlane(A,u)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nPlane(A,B,C)\nVector(A,B)\nVector(B,C)\nPoint(3,-2,-3)\nLine(A,D)\nCross(u,v)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nVector(A,B)\nVector(A,C)\nCross(u,v)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nPlane(A,B,C)\nPoint(-3,2,1)\nPoint(4,-2,3)\nLine(D,E)\nIntersect(r,p)\n") };
	static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nPlane(A,B,C)\nPoint(-3,2,1)\nPoint(4,-2,3)\nPoint(2,1,-3)\nPlane(D,E,F)\nIntersect(p,q)\nVector(A,B)\nVector(D,E)\nCross(u,v)\n") };

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

			Object::Type funcType{ getObjectTypeFromString(inputText.substr(0, funcOpenParenthesisPos)) };
			if (funcType == Object::Point && args.size() == 3)
			{
				std::vector<float> vecComponents{};
				convertParametersToFloat(parameters, vecComponents);

				if (vecComponents[0] == -9999.0f && vecComponents[2] == -9999.0f)
				{
					std::cerr << "ERROR::COULDNT_CONVERT\n";
					return;
				}
				else
				{
					if (!scanForIdenticalObject(func.type, vecComponents, object))
					{
						//draw(funcType, vecComponents, glm::vec4{ 0.0f, 0.2f, 0.5f, 1.0f });
						buildAndRegisterObject(funcType, vecComponents, Context::defaultColors[funcType]);
						inputBuffer[0] = '\0';
						return;
					}
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
						{
							//draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, pIDs, pCompIndex);
							buildAndRegisterObject(func.type, vecComponents, Context::defaultColors[func.type], pIDs, pCompIndex);
						}

					}

					else
					{
						getObjectComponents(args, vecComponents, pIDs, pCompIndex);

						if (!scanForIdenticalObject(func.type, vecComponents, object))
						{
							//draw(func.type, vecComponents, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, pIDs, pCompIndex);
							buildAndRegisterObject(func.type, vecComponents, Context::defaultColors[func.type], pIDs, pCompIndex);
						}
					}

					inputBuffer[0] = '\0';
				}
			}
		}
	}
}

void showVariables(std::vector<Object>& object, int out_selectedObjID)
{
	bool isSelectionChanged{ false };

	if (out_selectedObjID != -1)
	{
		if (Context::prevSelectedObjID != -1 && out_selectedObjID != Context::prevSelectedObjID)
		{
			isSelectionChanged = true;
		}

		else
		{
			Context::prevSelectedObjID = out_selectedObjID;
		}
	}

	for (size_t i{ 8 }; i < object.size(); ++i)
	{
		auto& obj{ object[i] };

		std::string headerText{ obj.getName() + ": " + getExpression(obj, object) + "###" + obj.getName() };

		const int currentID{ obj.getID() };
		const int currentIndex{ static_cast<int>(i) };

		if (out_selectedObjID == -1 && Context::prevSelectedObjID != -1)
		{
			if (currentID == Context::prevSelectedObjID)
			{
				ImGui::SetNextItemOpen(false);
				Context::prevSelectedObjID = out_selectedObjID;
				obj.setSelected(false);
				updateSelectedObjectColor(currentIndex, Context::object, Context::vertexData);
			}
		}

		if (isSelectionChanged && Context::prevSelectedObjID == currentID)
		{
			ImGui::SetNextItemOpen(false);
			isSelectionChanged = false;
			Context::prevSelectedObjID = out_selectedObjID;

			if (obj.isSelected())
			{
				obj.setSelected(false);
				updateSelectedObjectColor(currentIndex, Context::object, Context::vertexData);
			}
		}

		if (currentID == out_selectedObjID)
		{
			if (!obj.isSelected())
			{
				ImGui::SetNextItemOpen(true);
				obj.setSelected(true);
				updateSelectedObjectColor(currentIndex, Context::object, Context::vertexData);
			}
		}

		if (ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_None))
		{
			static ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_None;

			if (obj.getType() == Object::Plane || obj.getType() == Object::Line) ImGui::Text(getEquation(obj).c_str());

			char s{ 'A' };
			for (int increment{ 0 }; increment < obj.getComponents().size(); increment += 3)
			{
				ImGuiInputFlags textFlags{};

				if (!obj.isMutable()) textFlags |= ImGuiInputTextFlags_ReadOnly;

				ImGui::InputFloat3((obj.getName() + "::" + s).c_str(), obj.getComponentsPointer() + increment, "%.2f", textFlags);

				if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
					updateObject(static_cast<int>(i), obj, object, Context::vertexData);

				++s;
			}

			ImGui::ColorEdit4((obj.getName() + "::Color").c_str(), obj.getColorPointer(), ImGuiColorEditFlags_Float | colorFlags);

			if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
				updateObject(static_cast<int>(i), obj, object, Context::vertexData);

			std::string deleteText{ "Delete###" + std::to_string(obj.getID()) };

			if (ImGui::Button(deleteText.c_str()))
			{
				deleteObject(static_cast<int>(i), object, Context::vertexData);
				Context::prevSelectedObjID = -1;
				Context::selectedObjID = -1;
				break;
			}
		}
	}
}

int generateObjectVertices(Object& obj, std::vector<float>& vertexData)
{
	using Context::object;

	constexpr float scale{ 0.1f };

	// object data
	Object::Type type{ obj.getType() };
	const std::array<int, 3>& pIDs{ obj.getParentIDs() };
	const std::array<int, 3>& pCompIndex{ obj.getpCompIndex() };
	const std::vector<float>& comp{ obj.getComponents() };
	const glm::vec4& color{ obj.getColor() };

	int vCount{ 0 };

	// intersection
	if (type == Object::Point && pIDs[0] != -1)
	{
		Intersect intersect{ gatherPlaneLine(obj, object) };
		glm::vec3 intersection{ assemblyIntersectPoint(intersect) };
		std::vector components{ intersection.x, intersection.y, intersection.z };

		intersection *= scale;

		constexpr float radius{ 0.005f };

		if (scanForIdenticalObject(type, components, object, obj.getID()))
		{
			std::cout << components[0] << ", " << components[1] << ", " << components[2] << "\n";
			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
			return -1;
		}

		obj.setMutable(false);
		obj.setComponents(components);
		obj.setColor({ 0.7f, 0.3f, 0.0f, 1.0f });

		vCount = getSphereVertices(intersection, color, radius, vertexData);
	}

	else if (type == Object::Point)
	{
		glm::vec3 point{ comp[0], comp[1], comp[2] };

		point *= scale;

		constexpr float radius{ 0.005f };

		// getSphereVertices create 120960 new floats => 120960 / 7 = 17280 vertices, where 7 = number of components
		vCount = getSphereVertices(point, color, radius, vertexData);
	}

	else if (type == Object::Vector)
	{
		std::array<glm::vec3, 2> vector{ assemblyVector(obj, object) };

		auto [vecOrigin, vecHead] = vector;

		constexpr float epsilon{ 0.001f };
		if (glm::length(vecHead - vecOrigin) < epsilon)
			return -1;

		vecOrigin *= scale;
		vecHead *= scale;

		constexpr float radius{ 0.0015f };
		constexpr float coneRadius{ radius * 4.0f };

		const glm::vec3 direction{ glm::normalize(vecHead - vecOrigin) };
		const float cilinderLength{ glm::length(vecHead - vecOrigin) };
		constexpr float coneHeight{ 0.025f };

		glm::vec3 newVecHead{ vecOrigin + (cilinderLength - coneHeight) * direction };

		int vCountCilinder{};
		int vCountCone{};

		if (comp.size() > 6) 
			obj.setMutable(false); // if the object is the cross product, then it cannot be mutable

		// getCilinderVertices creates 144 new vertices 
		vCountCilinder = getCilinderVertices(vecOrigin, newVecHead, color, radius, vertexData);
		vCountCone = getConeVertices(direction, vecHead, color, coneRadius, coneHeight, vertexData);

		vCount = vCountCilinder + vCountCone;
	}

	else if (type == Object::Segment)
	{
		int startA{ pCompIndex[0] };
		int startB{ pCompIndex[1] };

		glm::vec3 pointA{ comp[startA], comp[startA + 1], comp[startA + 2] };
		glm::vec3 pointB{ comp[startB], comp[startB + 1], comp[startB + 2] };

		pointA *= scale;
		pointB *= scale;

		const float radius{ 0.0015f };

		vCount = getCilinderVertices(pointA, pointB, color, radius, vertexData);
	}

	else if
	(
		auto pType0{ object[searchObjectByID(pIDs[0], object)].getType() };

		type == Object::Line && pType0 == Object::Plane
	)
	{
		Intersect intersect{ gatherPlaneLine(obj, object) };

		auto [p1, p2] = intersect.points;
		auto [n1, n2] = intersect.vectors;

		std::array<glm::vec3, 2> intersection{ intersectionPlanePlane(p1, n1, p2, n2) };

		constexpr float epsilon{ 0.001f };
		if (glm::length(intersection[1]) < epsilon)
		{
			std::cerr << "Intersection doesn't exist.\n";
			return -1;
		}

		std::vector components
		{
			intersection[0].x, intersection[0].y, intersection[0].z,
			intersection[1].x, intersection[1].y, intersection[1].z
		};

		intersection[0] *= scale;
		intersection[1] *= scale;

		const float radius{ 0.0015f };

		if (scanForIdenticalObject(type, components, object, obj.getID()))
		{
			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
			return -1;
		}

		obj.setMutable(false);
		obj.setColor({ 0.7f, 0.3f, 0.0f, 1.0f });
		obj.setComponents(components);
		
		vCount = getLineVertices(intersection[0], { 0.0f, 0.0f, 0.0f }, intersection[1], color, radius, vertexData);
	}

	else if (type == Object::Line)
	{
		std::array<glm::vec3, 3> line{ assemblyLine(obj) };

		auto [point, dVectorOrigin, dVectorHead] = line;

		point *= scale;
		dVectorOrigin *= scale;
		dVectorHead *= scale;

		constexpr float radius{ 0.0015f };

		vCount = getLineVertices(point, dVectorOrigin, dVectorHead, color, radius, vertexData);
	}

	else if (type == Object::Plane)
	{
		std::array<glm::vec3, 3> plane{ assemblyPlane(obj, object) };

		auto [normalOrigin, normalHead, point] = plane;

		normalOrigin *= scale;
		normalHead *= scale;
		point *= scale;

		// getPlaneVertices create 6 new vertices
		vCount = getPlaneVertices(normalOrigin, normalHead, point, color, vertexData);
	}

	return vCount;
}

void buildAndRegisterObject(Object::Type type, const std::vector<float>& components, const glm::vec4& color, const std::array<int, 3>& pIDs, const std::array<int, 3>& pCompIndex)
{
	if (scanForIdenticalObject(type, components, Context::object))
	{
		std::cerr << "Object already exist.\n";
		return;
	}

	int pCount{ 0 };
	for (size_t i{ 0 }; i < std::size(pIDs); ++i)
	{
		if (pIDs[i] != -1 && pIDs[i] != Context::componentLiteral)
			++pCount;
	}

	glm::vec4 finalColor{ color };

	// plane random color
	//if (type == Object::Plane)
	//{
	//	float r{ Random::get(0.0f, 1.0f) };
	//	float g{ Random::get(0.0f, 1.0f) };
	//	float b{ Random::get(0.0f, 1.0f) };

	//	finalColor = { r, g, b, 0.2f };
	//}

	if (type == Object::Plane)
	{
		float r{}, g{}, b{};
		do {
			r = static_cast<float>(Random::get(0, 10)) * 0.1f;
			g = static_cast<float>(Random::get(0, 10)) * 0.1f;
			b = static_cast<float>(Random::get(0, 10)) * 0.1f;
		} while (r < 0.2f && g > 0.4f && b > 0.7f); // Re-sorteia se ficar muito próximo do ciano do fundo!

		finalColor = { r, g, b, 0.2f };
	}

	Object obj{ std::string(1, Context::objectSymbols[type]++), type, Context::primitives[type], components, finalColor, pIDs, pCompIndex, pCount};
	int vCount{ generateObjectVertices(obj, Context::vertexData) };

	if (vCount == -1)
	{
		std::cerr << "ERROR::FAILED_TO_GENERATE_VERTICES\n";
		return;
	}

	createObject(std::move(obj), vCount);

	updateBufferData(Context::vertexData);
}

// draw objects such as Points, Vectors, Planes, etc.
//void draw(Object::Type type, std::vector<float>& vecComponents, glm::vec4 color, std::array<int, 3> pIDs, std::array<int, 3> pCompIndex, bool update)
//{
//	using Context::object;
//
//	constexpr float scale{ 0.1f };
//
//	// intersection
//	if (type == Object::Point && pIDs[0] != -1)
//	{
//		Intersect intersect{ gatherPlaneLine(pIDs, object) };
//
//		glm::vec3 intersection{ assemblyIntersectPoint(intersect) };
//
//		std::vector components{ intersection.x, intersection.y, intersection.z };
//
//		intersection *= scale;
//
//		const float radius{ 0.005f };
//		if (color == glm::vec4{0.0f, 0.0f, 0.0f, 1.0f})
//			color = { 0.7f, 0.3f, 0.0f, 1.0f };
//
//		int vCountSphere{};
//
//		if (update)
//		{
//			vCountSphere = getSphereVertices(intersection, color, radius, Context::vertexData);
//			return;
//		}
//
//		if (scanForIdenticalObject(type, components, object))
//		{
//			std::cout << components[0] << ", " << components[1] << ", " << components[2] << "\n";
//			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
//			return;
//		}
//
//		vCountSphere = getSphereVertices(intersection, color, radius, Context::vertexData);
//
//		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Point, GL_LINES };
//		obj.setMutable(false);
//		createObject(std::move(obj), vCountSphere, components, color, 2, pIDs, pCompIndex);
//			
//		updateBufferData(Context::vertexData);
//	}
//
//	else if (type == Object::Point)
//	{
//		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };
//
//		point *= scale;
//
//		constexpr float radius{ 0.005f };
//
//		int vCountSphere{};
//
//		if (update)
//		{
//			vCountSphere = getSphereVertices(point, color, radius, Context::vertexData);
//			return;
//		}
//
//		// getSphereVertices create 120960 new floats => 120960 / 7 = 17280 vertices, where 7 = number of components
//		vCountSphere = getSphereVertices(point, color, radius, Context::vertexData);
//
//		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Point, GL_LINES };
//		createObject(std::move(obj), vCountSphere, vecComponents, color, 0, pIDs);
//
//		updateBufferData(Context::vertexData);
//	}
//
//	else if (type == Object::Vector)
//	{
//		std::array<glm::vec3, 2> vector{ assemblyVector(vecComponents, pIDs, pCompIndex, object) };
//
//		auto [vecOrigin, vecHead] = vector;
//
//		constexpr float epsilon{ 0.001f };
//		if (glm::length(vecHead - vecOrigin) < epsilon)
//			return;
//
//		vecOrigin *= scale;
//		vecHead *= scale;
//
//		constexpr float radius{ 0.0015f };
//		constexpr float coneRadius{ radius * 4.0f };
//
//		const glm::vec3 direction{ glm::normalize(vecHead - vecOrigin) };
//		const float cilinderLength{ glm::length(vecHead - vecOrigin) };
//		constexpr float coneHeight{ 0.025f };
//
//		glm::vec3 newVecHead{ vecOrigin + (cilinderLength - coneHeight) * direction };
//
//		int vCountCilinder{};
//		int vCountCone{};
//
//		if (update)
//		{
//			vCountCilinder = getCilinderVertices(vecOrigin, newVecHead, color, radius, Context::vertexData);
//			vCountCone     = getConeVertices(direction, vecHead, color, coneRadius, coneHeight, Context::vertexData);
//			return;
//		}
//
//		// getCilinderVertices creates 144 new vertices 
//		vCountCilinder = getCilinderVertices(vecOrigin, newVecHead, color, radius, Context::vertexData);
//		vCountCone	   = getConeVertices(direction, vecHead, color, coneRadius, coneHeight, Context::vertexData);
//
//		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Vector, GL_LINES};
//		if (vecComponents.size() > 6) obj.setMutable(false); // if the object is the cross product, then it cannot be mutable
//		createObject(std::move(obj), vCountCilinder + vCountCone, vecComponents, color, 2, pIDs, pCompIndex);
//
//		updateBufferData(Context::vertexData);
//	}
//
//	else if (type == Object::Segment)
//	{
//		int startA{ pCompIndex[0] };
//		int startB{ pCompIndex[1] };
//
//		glm::vec3 pointA{ vecComponents[startA], vecComponents[startA + 1], vecComponents[startA + 2] };
//		glm::vec3 pointB{ vecComponents[startB], vecComponents[startB + 1], vecComponents[startB + 2] };
//
//		pointA *= scale;
//		pointB *= scale;
//
//		const float radius{ 0.0015f };
//
//		int vCountCilinder{};
//
//		if (update)
//		{
//			vCountCilinder = getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);
//			return;
//		}
//
//		vCountCilinder = getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);
//
//		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Segment, GL_LINES };
//		createObject(obj, vCountCilinder, vecComponents, color, 2, pIDs, pCompIndex);
//
//		updateBufferData(Context::vertexData);
//	}
//
//	else if 
//		(
//		auto pType0{ object[searchObjectByID(pIDs[0], object)].getType() }; 
//		type == Object::Line && pType0 == Object::Plane
//		)
//	{
//		Intersect intersect{ gatherPlaneLine(pIDs, object) };
//
//		auto [p1, p2] = intersect.points;
//		auto [n1, n2] = intersect.vectors;
//
//		std::array<glm::vec3, 2> intersection{ intersectionPlanePlane(p1, n1, p2, n2) };
//
//		constexpr float epsilon{ 0.001f };
//		if (glm::length(intersection[1]) < epsilon)
//		{
//			std::cerr << "Intersection doesn't exist.\n";
//			return;
//		}
//
//		std::vector components
//		{
//			intersection[0].x, intersection[0].y, intersection[0].z,
//			intersection[1].x, intersection[1].y, intersection[1].z
//		};
//
//		intersection[0] *= scale;
//		intersection[1] *= scale;
//
//		const float radius{ 0.0015f };
//		if (color == glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f })
//			color = { 0.7f, 0.3f, 0.0f, 1.0f };
//
//		int vCountLine{};
//
//		if (update)
//		{
//			vCountLine = getLineVertices(intersection[0], {0.0f, 0.0f, 0.0f}, intersection[1], color, radius, Context::vertexData);
//			return;
//		}
//
//		if (scanForIdenticalObject(type, components, object))
//		{
//			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
//			return;
//		}
//
//		vCountLine = getLineVertices(intersection[0], { 0.0f, 0.0f, 0.0f }, intersection[1], color, radius, Context::vertexData);
//
//		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Line, GL_LINES };
//		obj.setMutable(false);
//		createObject(std::move(obj), vCountLine, components, color, 2, pIDs, pCompIndex);
//
//		updateBufferData(Context::vertexData);
//	}
//
//	else if (type == Object::Line)
//	{
//		std::array<glm::vec3, 3> line{ assemblyLine(vecComponents, pCompIndex) };
//
//		auto [point, dVectorOrigin, dVectorHead] = line;
//
//		point     *= scale;
//		dVectorOrigin *= scale;
//		dVectorHead *= scale;
//
//		constexpr float radius{ 0.0015f };
//
//		int vCountLine{};
//
//		if (update)
//		{
//			vCountLine = getLineVertices(point, dVectorOrigin, dVectorHead, color, radius, Context::vertexData);
//			return;
//		}
//		
//		vCountLine = getLineVertices(point, dVectorOrigin, dVectorHead, color, radius, Context::vertexData);
//
//		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Line, GL_LINES };
//		createObject(obj, vCountLine, vecComponents, color, 2, pIDs, pCompIndex);
//
//		updateBufferData(Context::vertexData);
//	}
//
//	else if (type == Object::Plane)
//	{
//		int pIndex1{ searchObjectByID(pIDs[1], object) };
//		Object::Type pType1{ object[pIndex1].getType() };
//
//		std::array<glm::vec3, 3> plane{ assemblyPlane(vecComponents, pIDs, pCompIndex, object) };
//
//		auto [normalOrigin, normalHead, point] = plane;
//
//		normalOrigin *= scale;
//		normalHead *= scale;
//		point *= scale;
//
//		color.w = 0.2f;
//		int vCountPlane{};
//
//		if (update)
//		{
//			vCountPlane = getPlaneVertices(normalOrigin, normalHead, point, color, Context::vertexData);
//			return;
//		}
//
//		// getPlaneVertices create 6 new vertices
//		vCountPlane = getPlaneVertices(normalOrigin, normalHead, point, color, Context::vertexData);
//
//		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Plane, GL_TRIANGLES };
//		createObject(obj, vCountPlane, vecComponents, color, 3, pIDs, pCompIndex);
//
//		updateBufferData(Context::vertexData);
//	}
//}

void drawObjectLabels
(
	std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
)
{
	ImDrawList* drawList{ ImGui::GetBackgroundDrawList() };

	constexpr float scale{ 0.1f };

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		auto& obj{ object[idx] };
		auto type{ obj.getType() };

		if (type == Object::Line || type == Object::Plane)
			continue;

		glm::vec3 targetWorldPos{};
		auto comp{ obj.getComponents() };

		if (type == Object::Vector)
		{
			std::array<glm::vec3, 2> vector{ assemblyVector(obj, object) };

			targetWorldPos = vector[1];

			targetWorldPos *= scale;
			targetWorldPos += glm::vec3{ 0.0f, 0.03f, 0.0f };
		}

		else if (type == Object::Point)
		{
			targetWorldPos = { comp[0], comp[1], comp[2] };
			targetWorldPos *= scale;
			targetWorldPos += glm::vec3{ 0.0f, 0.015f, 0.0f };
		}

		else if (type == Object::Segment)
		{
			targetWorldPos = { (comp[0] + comp[3]) * 0.5f, (comp[1] + comp[4]) * 0.5f, (comp[2] + comp[5]) * 0.5f };
			targetWorldPos *= scale;
		}

		glm::vec2 screenPos{};

		if (projectWorldToScreen(targetWorldPos, viewMatrix, projectionMatrix, modelMatrix, viewportPos, viewportSize, screenPos))
		{
			std::string label{ obj.getName() };

			float textWidth{ Context::spaceFont->CalcTextSizeA(Context::fontSize, FLT_MAX, 0.0f, label.c_str()).x };
			ImVec2 adjustedPos{ screenPos.x - (textWidth * 0.5f), screenPos.y };

			drawList->AddText(Context::spaceFont, Context::fontSize, ImVec2{ adjustedPos.x + 1.0f, adjustedPos.y + 1.0f }, ImColor{ 0, 0, 0, 255 }, label.c_str());
			drawList->AddText(Context::spaceFont, Context::fontSize, adjustedPos, ImColor{ 0, 80, 255, 255 }, label.c_str());
		}
	}
}

void drawAxisLabels
(
	std::vector<Object>& object,
	const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix,
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,
	const glm::vec2& viewportSize
)
{
	// ring start vertices
	std::vector ringVertices
	{
		-1.0f,  0.0f,  0.0f,
		 1.0f,  0.0f,  0.0f,

		 0.0f, -1.0f,  0.0f,
		 0.0f,  1.0f,  0.0f,
		  
		 0.0f,  0.0f, -1.0f,
		 0.0f,  0.0f,  1.0f,
	};

	std::vector<ImColor> axisColors
	{
		{ 255, 0, 0, 255 },
		{ 0, 255, 0, 255 },
		{ 0, 0, 255, 255 }
	};

	ImDrawList* drawList{ ImGui::GetBackgroundDrawList() };

	constexpr float stride{ 0.1f };
	constexpr int ringCount{ 21 };
	constexpr float axisSpacing{ -0.0085f };

	bool isZeroDrawn{ false };
	bool isColorWhite{ true };

	for (size_t ringStart{ 0 }, c{ 0 }; ringStart < ringVertices.size(); ringStart += 6, ++c)
	{
		glm::vec3 ringPos{ ringVertices[ringStart], ringVertices[ringStart + 1], ringVertices[ringStart + 2] };
		glm::vec3 direction{ glm::normalize(glm::vec3
		(
			ringVertices[ringStart + 3] - ringVertices[ringStart],
			ringVertices[ringStart + 4] - ringVertices[ringStart + 1],
			ringVertices[ringStart + 5] - ringVertices[ringStart + 2]
		)) };

		if (ringStart == 0) ringPos  += glm::vec3{ 0.0f, axisSpacing, axisSpacing };
		if (ringStart == 6) ringPos  += glm::vec3{ axisSpacing, 0.0f, axisSpacing };
		if (ringStart == 12) ringPos += glm::vec3{ axisSpacing, axisSpacing, 0.0f };

		ImColor color{};

		for (int ring{ 0 }, count{ -10 }; ring < ringCount; ++ring, ++count)
		{
			if (count == 0 && isZeroDrawn)
			{
				ringPos += direction * stride;
				continue;
			}

			glm::vec2 screenPos{};

			if (projectWorldToScreen(ringPos, viewMatrix, projectionMatrix, modelMatrix, viewportPos, viewportSize, screenPos))
			{
				if (count == 0 && isColorWhite) 
				{
					isZeroDrawn = true;
					isColorWhite = false;
					color = ImColor{ 255, 255, 255, 255 };
				}
				else
				{
					color = axisColors[c];
				}

				std::string label{ std::to_string(count) };

				float textWidth{ Context::spaceFont->CalcTextSizeA(Context::fontSize, FLT_MAX, 0.0f, label.c_str()).x };
				ImVec2 adjustedPos{ screenPos.x - (textWidth * 0.5f), screenPos.y };

				drawList->AddText(Context::spaceFont, Context::fontSize, ImVec2{ adjustedPos.x + 1.0f, adjustedPos.y + 1.0f }, ImColor{ 0, 0, 0, 255 }, label.c_str());
				drawList->AddText(Context::spaceFont, Context::fontSize, adjustedPos, color, label.c_str());
			}

			ringPos += direction * stride;
		}
	}
}