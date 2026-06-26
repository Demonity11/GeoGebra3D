#include "Context.h"

namespace Context
{
	unsigned int VBO{};
	unsigned int VAO{};

	glm::vec3 cameraPos{ 0.0f, 0.5f,  3.0f };
	glm::vec3 cameraTarget{ 0.0f, 0.0f,  0.0f };
	glm::vec3 worldUp{ 0.0f, 1.0f,  0.0f };

	float lastX{ 400.0f };
	float lastY{ 300.0f };

	float yaw{};
	float pitch{};

	float fov{ 45.0f };

	bool isPressingRightClick{ false };
	bool isFirstMouse{ true };
	bool isEnterPressed{ false };

	std::vector<float> vertexData{};
	// Objects conteiner
	std::vector<Object> object{};

	// store FunctionArgs which has name, type, and expected arguments
	std::vector<FunctionArgs> function
	{
		{"Point(",     Object::Point,   {}											  },
		{"Vector(",    Object::Vector,  {Object::Point,  Object::Point}				  },
		{"Vector(",    Object::Vector,  {Object::Point}								  },
		{"Segment(",   Object::Segment, {Object::Point,  Object::Point}				  },
		{"Line(",	   Object::Line,    {Object::Point,  Object::Point}				  },
		{"Line(",      Object::Line,    {Object::Point,  Object::Vector}			  },
		{"Plane(",     Object::Plane,   {Object::Point,  Object::Vector}			  },
		{"Intersect(", Object::Point,   {Object::Line,   Object::Plane }              }
		//{"Plane(",   Object::Plane,   {Object::Point, Object::Point, Object::Point} }
	};

	// store object symbols (default name)
	std::map<Object::Type, char> objectSymbols
	{
		{ Object::Vector,  'u' },
		{ Object::Point,   'A' },
		{ Object::Segment, 'f' },
		{ Object::Plane,   'p' },
		{ Object::Line,    'r' }
	};
}