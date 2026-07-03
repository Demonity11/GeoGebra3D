#ifndef CONTEXT_H
#define CONTEXT_H

#include "draw_utils.h"

namespace Context
{
	extern unsigned int VBO;
	extern unsigned int VAO;

	extern glm::vec3 cameraPos;
	extern glm::vec3 cameraTarget;
	extern glm::vec3 worldUp;

	extern float lastX;
	extern float lastY;

	extern float yaw;
	extern float pitch;

	extern float fov;

	extern bool isPressingRightClick;
	extern bool isFirstMouse;
	extern bool isEnterPressed;

	extern std::vector<float> vertexData;
	// Objects conteiner
	extern std::vector<Object> object;
	// store FunctionArgs which has name, type, and expected arguments
	extern std::vector<FunctionArgs> function;
	// store object symbols (default name)
	extern std::map<Object::Type, char> objectSymbols;
	// parent ID when the object has literal components
	inline constexpr int componentLiteral{ -2 };

	inline ImFont* spaceFont{ nullptr };

	constexpr float fontSize{ 16.0f };
}

#endif
