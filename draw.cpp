#include "draw_utils.h"
#include "Window.h"

void getNewCoordSystem(glm::vec3& direction, glm::vec3& right, glm::vec3& up)
{
	direction = glm::normalize(direction);

	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

	// changes worldUp accordingly with the direction vector, because the cross product between vectors with the same direction is a null vector 
	float cosTheta = glm::dot(direction, worldUp);
	if (glm::abs(cosTheta) > 0.999f)
	{
		if (cosTheta > 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

		if (cosTheta < 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, -1.0f);
	}

	right = glm::normalize(glm::cross(direction, worldUp));
	up = glm::cross(right, direction);
}

void getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, float radius, std::vector<float>& vertexData)
{
	glm::vec3 direction{ p - p0 };

	float length{ glm::length(direction) };

	if (length == 0.0f)
		return;

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	const float linesDensity{ 360.0f / 72.0f };

	// this loop does 72 * 14 = 1008 pushbacks
	for (float angle{ 0.0f }; angle < 360.0f; angle += linesDensity)
	{
		float rad{ glm::radians(angle) };

		glm::vec3 a{ p0 + radius * cos(rad) * right + radius * sin(rad) * up };
		glm::vec3 b{ a + length * direction };

		vertexData.push_back(a.x);
		vertexData.push_back(a.y);
		vertexData.push_back(a.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(b.x);
		vertexData.push_back(b.y);
		vertexData.push_back(b.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);
	}
}

void getRingsVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, std::vector<float>& vertexData)
{
	glm::vec3 direction{ p - p0 };

	float ringWidth{ glm::length(direction) };

	if (ringWidth == 0.0f)
		return;

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	const float linesDensity{ 360.0f / 72.0f };
	const float ringRadius{ 0.002f };
	float stride{ 0.1f };
	const int ringCount{ 20 };

	// this loop does 20 * 72 * 14 = 20160 pushbacks
	for (int i{ 0 }; i < ringCount; ++i)
	{
		p0 += direction * stride;

		for (float angle{ 0.0f }; angle < 360.0f; angle += linesDensity)
		{
			float rad{ glm::radians(angle) };

			glm::vec3 a{ p0 + ringRadius * cos(rad) * right + ringRadius * sin(rad) * up };
			glm::vec3 b{ a + ringWidth * direction };

			vertexData.push_back(a.x);
			vertexData.push_back(a.y);
			vertexData.push_back(a.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);

			vertexData.push_back(b.x);
			vertexData.push_back(b.y);
			vertexData.push_back(b.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);
		}
	}
}

void getSphereVertices(glm::vec3 translation, glm::vec4 color, float radius, std::vector<float>& vertexData)
{
	const float deltaPhi{ 180.f / 60.0f };

	const float deltaTheta{ 360.0f / 72.0f };

	// this loop does 60 * 72 * 28 = 120960 pushbacks
	for (float i{ 0.0f }; i <= 180.0f - deltaPhi; i += deltaPhi)
	{
		float phi{ glm::radians(i) };
		float phiNext{ glm::radians(i + deltaPhi) };

		for (float j{ 0.0f }; j <= 360.0f - deltaTheta; j += deltaTheta)
		{
			float theta{ glm::radians(j) };
			float thetaNext{ glm::radians(j + deltaTheta) };

			glm::vec3 p0
			{
				translation.x + radius * sin(phi) * cos(theta),
				translation.y + radius * cos(phi),
				translation.z + radius * sin(phi) * sin(theta)
			};

			glm::vec3 pRight
			{
				translation.x + radius * sin(phi) * cos(thetaNext),
				translation.y + radius * cos(phi),
				translation.z + radius * sin(phi) * sin(thetaNext)
			};

			glm::vec3 pBottom
			{
				translation.x + radius * sin(phiNext) * cos(theta),
				translation.y + radius * cos(phiNext),
				translation.z + radius * sin(phiNext) * sin(theta)
			};

			vertexData.push_back(p0.x);
			vertexData.push_back(p0.y);
			vertexData.push_back(p0.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);

			vertexData.push_back(pRight.x);
			vertexData.push_back(pRight.y);
			vertexData.push_back(pRight.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);

			vertexData.push_back(p0.x);
			vertexData.push_back(p0.y);
			vertexData.push_back(p0.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);

			vertexData.push_back(pBottom.x);
			vertexData.push_back(pBottom.y);
			vertexData.push_back(pBottom.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);
		}
	}
}

void getPlaneVertices(glm::vec3 normalP0, glm::vec3 normalP, glm::vec3 point, glm::vec4 color, std::vector<float>& vertexData)
{
	glm::vec3 direction{ normalP - normalP0 };

	float length{ glm::length(direction) };

	if (length == 0.0f)
		return;

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	//glm::mat3 R{ right, direction, up };

	std::vector planeVertices
	{
		 1.0f, 0.0f, -1.0f,
		 1.0f, 0.0f,  1.0f,
		-1.0f, 0.0f, -1.0f,

		 1.0f, 0.0f,  1.0f,
		-1.0f, 0.0f,  1.0f,
		-1.0f, 0.0f, -1.0f
	};

	for (int i{ 0 }; i < planeVertices.size(); i += 3)
	{
		glm::vec3 p
		{
			planeVertices[i] * right.x + planeVertices[i + 2] * up.x + point.x,
			planeVertices[i] * right.y + planeVertices[i + 2] * up.y + point.y,
			planeVertices[i] * right.z + planeVertices[i + 2] * up.z + point.z
		};

		vertexData.push_back(p.x);
		vertexData.push_back(p.y);
		vertexData.push_back(p.z);

		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);
	}
}

void getGridVertices()
{
	glm::vec3 p0Horizontal{ -1.0f, 0.0f, -1.0f };
	glm::vec3 pHorizontal {  1.0f, 0.0f, -1.0f };

	glm::vec3 p0Vertical  { -1.0f, 0.0f, -1.0f };
	glm::vec3 pVertical   { -1.0f, 0.0f,  1.0f };

	glm::vec4 color{ 0.0f, 0.0f, 0.0f, 0.5f };

	const float stride{ 0.1f };
	const int lineCount{ 21 };

	// this loop does 21 * 28 = 588 pushbacks
	for (int i{ 0 }; i < lineCount; ++i)
	{
		vertexData.push_back(p0Horizontal.x);
		vertexData.push_back(p0Horizontal.y);
		vertexData.push_back(p0Horizontal.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(pHorizontal.x);
		vertexData.push_back(pHorizontal.y);
		vertexData.push_back(pHorizontal.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(p0Vertical.x);
		vertexData.push_back(p0Vertical.y);
		vertexData.push_back(p0Vertical.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(pVertical.x);
		vertexData.push_back(pVertical.y);
		vertexData.push_back(pVertical.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		p0Horizontal.z += stride;
		pHorizontal.z += stride;

		p0Vertical.x += stride;
		pVertical.x += stride;
	}
}

void drawCilinder()
{
	std::vector axisVertices
	{
		-1.0f, 0.0f, 0.0f,
		 1.0f, 0.0f, 0.0f,

		 0.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,

		 0.0f, 0.0f, -1.0f,
		 0.0f, 0.0f,  1.0f
	};

	std::vector axisColors
	{
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f
	};

	char axis{ 'X' };

	std::vector<float> axisPos{ 0.0f, 0.5f, 0.0f };

	// this loop does 3 * 1008 = 3024 pushbacks
	for (int v{ 0 }, c{ 0 }; v < axisVertices.size(); v += 6, c += 4)
	{
		glm::vec3 a{ axisVertices[v], axisVertices[v + 1], axisVertices[v + 2] };
		glm::vec3 b(axisVertices[v + 3], axisVertices[v + 4], axisVertices[v + 5]);
		glm::vec4 color{ axisColors[c], axisColors[c + 1], axisColors[c + 2], axisColors[c + 3] };

		getCilinderVertices(a, b, color, 0.001f, vertexData);
		
		addNewObject(144, GL_LINES, funcType::Segment, std::string(1, axis) + "_AXIS", axisPos, color);

		++axis;
	}


	float ringWidth{ 0.002f };
	std::vector ringVertices
	{
		-1.0f, 0.0f, 0.0f,
		-1.0f + ringWidth, 0.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f + ringWidth, 0.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f + ringWidth
	};

	glm::vec4 ringColor{ 0.0f, 0.0f, 0.0f, 1.0f };

	axis = 'X';

	// this loop does 3 * 20160 = 60480 pushbacks
	for (int v{ 0 }; v < axisVertices.size(); v += 6)
	{
		glm::vec3 a{ ringVertices[v], ringVertices[v + 1], ringVertices[v + 2] };
		glm::vec3 b(ringVertices[v + 3], ringVertices[v + 4], ringVertices[v + 5]);

		getRingsVertices(a, b, ringColor, vertexData);

		addNewObject(2880, GL_LINES, funcType::Segment, std::string(1, axis) + "_AXIS_RINGS", axisPos, ringColor);

		++axis;
	}


	// this execution does 21 * 28 = 588 pushbacks
	getGridVertices();
	addNewObject(84, GL_LINES, funcType::Segment, "GRID_LINES", {}, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
}

void addNewObject(int vertexCount, unsigned int primitive, funcType type, std::string name, const std::vector<float>& components, const glm::vec4 color)
{
	int offset{ 0 };

	if (!objInfo.empty())
	{
		int previousId{ static_cast<int>(objInfo.size()) - 1 };
		offset = objInfo[previousId].offset + objInfo[previousId].vertexCount;
	}

	int objID{ static_cast<int>(objInfo.size()) };

	objInfo[objID] = ObjectMetadata
	{
		offset,
		vertexCount,
		primitive,
		type,
		name,
		components,
		color
	};

	symbolTable[name] = objID;

	for (const auto& [name, id] : symbolTable)
	{
		std::cout << name << "::" << id << "::" << objInfo[id].components.size() << "\n";
	}

	std::cout << "\n\n";
}