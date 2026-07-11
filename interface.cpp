#include "Window.h"
#include "draw_utils.h"
#include "utilities.h"
#include "objectCoords.h"
#include "Context.h"

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
	
	ImGui::PushFont(Context::spaceFont, 16.0f);
	ImGui::Begin("InputWindow");

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

	for (int i{ 8 }; i < object.size(); ++i)
	{
		auto& obj{ object[i]};

		std::string headerText{ obj.getName() + ": " + getExpression(obj, object) + "###" + obj.getName() };

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
					updateObject(i, obj, object, Context::vertexData);

				++s;
			}

			//ImGui::InputFloat4((obj.getName() + "::Color").c_str(), obj.getColorPointer(), "%.2f");
			ImGui::ColorEdit4((obj.getName() + "::Color").c_str(), obj.getColorPointer(), ImGuiColorEditFlags_Float | colorFlags);

			if (ImGui::IsItemDeactivatedAfterEdit()) // saves the changes
				updateObject(i, obj, object, Context::vertexData);

			std::string deleteText{ "Delete###" + std::to_string(obj.getID()) };

			if (ImGui::Button(deleteText.c_str()))
			{
				deleteObject(i, object, Context::vertexData);
			}
		}
	}

	ImGui::PopFont();
	ImGui::End();
}

void processInput(char inputBuffer[128], const std::vector<FunctionArgs>& function, std::vector<Object>& object)
{
	auto ss{ std::stringstream(inputBuffer) };
	auto inputText{ ss.str() };

	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(3,3,3)\nSegment(A,B)\nVector(A)\nLine(A,u)\nPlane(A,u)\n") };
	static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nPlane(A,B,C)\n") };
	//static auto inputArray{ testInput("Point(1,1,1)\nPoint(2,2,2)\nPoint(3,-1,2)\nVector(A,B)\nVector(A,C)\nCross(u,v)\n") };

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
						draw(funcType, vecComponents, glm::vec4{ 0.0f, 0.2f, 0.5f, 1.0f });
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

				vectors[i] = {comp[3] - comp[0], comp[4] - comp[1], comp[5] - comp[2]};

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
				intersection = intersectionLinePlane(points[1], vectors[1], vectors[0], d);
			}
			else 
			{
				d = -glm::dot(points[1], vectors[1]);
				intersection = intersectionLinePlane(points[0], vectors[0], vectors[1], d);
			}
		}

		if (intersection == glm::vec3(-9999.0f, -9999.0f, -9999.0f))
		{
			std::cerr << "Intersection doesn't exist. Handle later.\n";
		}

		std::vector components{ intersection.x, intersection.y, intersection.z };

		intersection *= scale;

		const float radius{ 0.005f };
		if (color == glm::vec4{0.0f, 0.0f, 0.0f, 1.0f})
			color = { 0.7f, 0.3f, 0.0f, 1.0f };

		int vCountSphere{};

		if (update)
		{
			vCountSphere = getSphereVertices(intersection, color, radius, Context::vertexData);
			return;
		}

		if (scanForIdenticalObject(type, components, Context::object))
		{
			std::cout << components[0] << ", " << components[1] << ", " << components[2] << "\n";
			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
			return;
		}

		vCountSphere = getSphereVertices(intersection, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Point, GL_LINES };
		obj.setMutable(false);
		createObject(std::move(obj), vCountSphere, components, color, 2, pIDs, pCompIndex);
			
		updateBufferData(Context::vertexData);
	}

	else if (type == Object::Point)
	{
		glm::vec3 point{ vecComponents[0], vecComponents[1], vecComponents[2] };

		point *= scale;

		const float radius{ 0.005f };

		int vCountSphere{};

		if (update)
		{
			vCountSphere = getSphereVertices(point, color, radius, Context::vertexData);
			return;
		}

		// getSphereVertices create 120960 new floats => 120960 / 7 = 17280 vertices, where 7 = number of components
		vCountSphere = getSphereVertices(point, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Point, GL_LINES };
		createObject(std::move(obj), vCountSphere, vecComponents, color, 0, pIDs);

		updateBufferData(Context::vertexData);
		}

	else if (type == Object::Vector)
	{
		int pIndex1{ searchObjectByID(pIDs[1], Context::object) };
		int pType1{ Context::object[pIndex1].getType() };

		std::array<glm::vec3, 2> comp{};

		if (pType1 == Object::Point)
		{
			int startA{ pCompIndex[0] };
			int startB{ pCompIndex[1] };

			glm::vec3 pointA{ vecComponents[startA], vecComponents[startA + 1], vecComponents[startA + 2] };
			glm::vec3 pointB{ vecComponents[startB], vecComponents[startB + 1], vecComponents[startB + 2] };

			comp[0] = pointA;
			comp[1] = pointB;
		}
		
		else if (pType1 == Object::Vector)
		{
			int startU{ pCompIndex[0] };
			int startV{ pCompIndex[1] };

			glm::vec3 u
			{
				vecComponents[startU + 3] - vecComponents[startU],
				vecComponents[startU + 4] - vecComponents[startU + 1],
				vecComponents[startU + 5] - vecComponents[startU + 2]
			};

			glm::vec3 v
			{
				vecComponents[startV + 3] - vecComponents[startV],
				vecComponents[startV + 4] - vecComponents[startV + 1],
				vecComponents[startV + 5] - vecComponents[startV + 2]
			};

			glm::vec3 cross{ glm::cross(u, v) };

			comp[0] = glm::vec3(0.0f, 0.0f, 0.0f);
			comp[1] = cross;
		}

		auto [pointA, pointB] = comp;

		constexpr float epsilon{ 0.001f };

		if (glm::length(pointB - pointA) < epsilon)
			return;

		pointA *= scale;
		pointB *= scale;

		const float cilinderLength{ glm::length(pointB - pointA) };

		constexpr float radius{ 0.0015f };
		constexpr float coneRadius{ radius * 4.0f };

		const glm::vec3 direction{ glm::normalize(pointB - pointA) };
		constexpr float coneHeight{ 0.025f };
		
		auto newPointB{ pointA + (cilinderLength - coneHeight) * direction };

		int vCountCilinder{};
		int vCountCone{};

		if (update)
		{
			vCountCilinder = getCilinderVertices(pointA, newPointB, color, radius, Context::vertexData);
			vCountCone     = getConeVertices(direction, pointB, color, coneRadius, coneHeight, Context::vertexData);
			return;
		}

		// getCilinderVertices creates 144 new vertices 
		vCountCilinder = getCilinderVertices(pointA, newPointB, color, radius, Context::vertexData);
		vCountCone	   = getConeVertices(direction, pointB, color, coneRadius, coneHeight, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Vector, GL_LINES};
		if (pType1 == Object::Vector) obj.setMutable(false); // if the object is the cross product, then it cannot be mutable
		createObject(std::move(obj), vCountCilinder + vCountCone, vecComponents, color, 2, pIDs, pCompIndex);

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

		int vCountCilinder{};

		if (update)
		{
			vCountCilinder = getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);
			return;
		}

		vCountCilinder = getCilinderVertices(pointA, pointB, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Segment, GL_LINES };
		createObject(obj, vCountCilinder, vecComponents, color, 2, pIDs, pCompIndex);

		updateBufferData(Context::vertexData);
	}

	else if (type == Object::Line && Context::object[searchObjectByID(pIDs[0], Context::object)].getType() == Object::Plane)
	{
		auto plane1{ Context::object[searchObjectByID(pIDs[0], Context::object)] };
		auto plane2{ Context::object[searchObjectByID(pIDs[1], Context::object)] };

		// plane 1 components extraction
		int startP1{ plane1.getpCompIndex()[0] };
		int startN1{ plane1.getpCompIndex()[1] };

		auto comp1{ plane1.getComponents() };

		glm::vec3 p1{ comp1[startP1], comp1[startP1 + 1], comp1[startP1 + 2] };
		glm::vec3 n1{ comp1[startN1 + 3] - comp1[startN1], comp1[startN1 + 4] - comp1[startN1 + 1], comp1[startN1 + 5] - comp1[startN1 + 2] };

		// plane 2 components extraction
		int startP2{ plane2.getpCompIndex()[0] };
		int startN2{ plane2.getpCompIndex()[1] };

		auto comp2{ plane2.getComponents() };

		glm::vec3 p2{ comp2[startP2], comp2[startP2 + 1], comp2[startP2 + 2] };
		glm::vec3 n2{ comp2[startN2 + 3] - comp2[startN2], comp2[startN2 + 4] - comp2[startN2 + 1], comp2[startN2 + 5] - comp2[startN2 + 2] };

		auto intersection{ intersectionPlanePlane(p1, n1, p2, n2) };

		if (glm::length(intersection[1]) < 0.001f)
		{
			std::cerr << "Intersection doesn't exist.\n";
			return;
		}

		std::vector components
		{
			intersection[0].x, intersection[0].y, intersection[0].z,
			intersection[1].x, intersection[1].y, intersection[1].z
		};

		intersection[0] *= scale;
		intersection[1] *= scale;

		const float radius{ 0.0015f };
		if (color == glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f })
			color = { 0.7f, 0.3f, 0.0f, 1.0f };

		int vCountLine{};

		if (update)
		{
			vCountLine = getLineVertices(intersection[0], {0.0f, 0.0f, 0.0f}, intersection[1], color, radius, Context::vertexData);
			return;
		}

		if (scanForIdenticalObject(type, components, Context::object))
		{
			std::cerr << "INTERSECTION::ALREADY::EXISTS\n";
			return;
		}

		vCountLine = getLineVertices(intersection[0], { 0.0f, 0.0f, 0.0f }, intersection[1], color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Line, GL_LINES };
		obj.setMutable(false);
		createObject(std::move(obj), vCountLine, components, color, 2, pIDs, pCompIndex);

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

		int vCountLine{};

		if (update)
		{
			vCountLine = getLineVertices(point, dVectorP0, dVectorP, color, radius, Context::vertexData);
			return;
		}
		
		vCountLine = getLineVertices(point, dVectorP0, dVectorP, color, radius, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Line, GL_LINES };
		createObject(obj, vCountLine, vecComponents, color, 2, pIDs, pCompIndex);

		updateBufferData(Context::vertexData);
	}

	else if (type == Object::Plane)
	{
		int pIndex1{ searchObjectByID(pIDs[1], Context::object) };
		Object::Type pType1{ Context::object[pIndex1].getType() };

		std::array<glm::vec3, 3> comp{};

		if (pType1 == Object::Vector)
		{
			int startPoint{ pCompIndex[0] };
			int startNormal{ pCompIndex[1] };

			glm::vec3 normalP0{ vecComponents[startNormal], vecComponents[startNormal + 1], vecComponents[startNormal + 2] };
			glm::vec3 normalP{ vecComponents[startNormal + 3], vecComponents[startNormal + 4], vecComponents[startNormal + 5] };
			glm::vec3 point{ vecComponents[startPoint], vecComponents[startPoint + 1], vecComponents[startPoint + 2] };

			normalP0 *= scale;
			normalP  *= scale;
			point    *= scale;

			comp[0] = normalP0;
			comp[1] = normalP;
			comp[2] = point;
		}

		else if (pType1 == Object::Point)
		{
			int startPointA{ pCompIndex[0] };
			int startPointB{ pCompIndex[1] };
			int startPointC{ pCompIndex[2] };

			glm::vec3 A{ vecComponents[startPointA], vecComponents[startPointA + 1], vecComponents[startPointA + 2] };
			glm::vec3 B{ vecComponents[startPointB], vecComponents[startPointB + 1], vecComponents[startPointB + 2] };
			glm::vec3 C{ vecComponents[startPointC], vecComponents[startPointC + 1], vecComponents[startPointC + 2] };

			glm::vec3 u{ B - A };
			glm::vec3 v{ C - A };

			glm::vec3 normal{ glm::cross(u, v) };

			A	   *= scale;
			normal *= scale;

			comp[0] = glm::vec3(0.0f, 0.0f, 0.0f);
			comp[1] = normal;
			comp[2] = A;
		}

		color.w = 0.2f;

		int vCountPlane{};

		auto [planeNormal0, planeNormal, planePoint] = comp;

		if (update)
		{
			vCountPlane = getPlaneVertices(planeNormal0, planeNormal, planePoint, color, Context::vertexData);
			return;
		}

		// getPlaneVertices create 6 new vertices
		vCountPlane = getPlaneVertices(planeNormal0, planeNormal, planePoint, color, Context::vertexData);

		Object obj{ std::string(1, Context::objectSymbols[type]++), Object::Plane, GL_TRIANGLES };
		createObject(obj, vCountPlane, vecComponents, color, 3, pIDs, pCompIndex);

		updateBufferData(Context::vertexData);
	}
}

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
			int pIndex1{ obj.getParentIDs()[1] };
			int pType1{ object[searchObjectByID(pIndex1, object)].getType() };

			if (pType1 == Object::Vector)
			{
				int startU{ obj.getpCompIndex()[0] };
				int startV{ obj.getpCompIndex()[1] };

				glm::vec3 u
				{
					comp[startU + 3] - comp[startU],
					comp[startU + 4] - comp[startU + 1],
					comp[startU + 5] - comp[startU + 2]
				};

				glm::vec3 v
				{
					comp[startV + 3] - comp[startV],
					comp[startV + 4] - comp[startV + 1],
					comp[startV + 5] - comp[startV + 2]
				};

				glm::vec3 cross{ glm::cross(u, v) };

				targetWorldPos = std::move(cross);
			}

			else
			{
				int startB{ obj.getpCompIndex()[1] };
				targetWorldPos = { comp[startB], comp[startB + 1], comp[startB + 2] };
			}

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