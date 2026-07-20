#include "utilities.h"
#include "draw_utils.h"
#include "objectCoords.h"
#include "Context.h"
#include "objectAssembling.h"

#include <iomanip>

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << vec.x << ", " << vec.y << ", " << vec.z;

	return os;
}

// convert Object::Type to std::string
std::string getStringFunctionType(Object::Type type)
{
	switch (type)
	{
	case Object::Point:   return "Point";
	case Object::Vector:  return "Vector";
	case Object::Segment: return "Segment";
	case Object::Line:    return "Line";
	case Object::Plane:   return "Plane";
	}

	return "???";
}

Object::Type getObjectTypeFromString(const std::string& funcName)
{
	if		(funcName == "Point")   return Object::Point;
	else if (funcName == "Vector")  return Object::Vector;
	else if (funcName == "Segment") return Object::Segment;
	else if (funcName == "Line")    return Object::Line;
	else if (funcName == "Plane")   return Object::Plane;

	return Object::Null;
}

// convert std::string parameters to float
void convertParametersToFloat(std::string& parameters, std::vector<float>& vecComponents)
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
			std::cerr << "ERROR::" << e.what() << "\n";
			vecComponents = { -9999.0f, -9999.0f, -9999.0f };
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
				std::cerr << "ERROR::" << e.what() << "\n";
				vecComponents = { -9999.0f, -9999.0f, -9999.0f };
				return;
			}

			break;
		}
	}
}

// compare if the given object has the specified type
bool compareObjectType(const std::string& objName, Object::Type expectedType, const std::vector<Object>& object)
{
	for (const auto& obj : object)
	{
		if (objName == obj.getName())
		{
			if (obj.getType() == expectedType)
				return true;

			return false;
		}
	}

	return false;
}

// removes the characters '(', ')', and ' ' from the argument
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

// split multiple arguments into separated ones.
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

// search object's index by name
int searchObjectIndexByName(const std::string& objName, const std::vector<Object>& object)
{
	//for (int index{ 0 }; index < object.size(); ++index)
	//{
	//	const auto& obj{ object[index] };

	//	if (obj.getName() == objName)
	//		return index;
	//}

	if (auto idx{ Context::symbolTable.find(objName) }; idx != Context::symbolTable.end())
	{
		return static_cast<int>(idx->second);
	}

	return -1;
}

// return the std::string arguments into components float vector
void getObjectComponents(std::vector<std::string>& args, std::vector<float>& vecComponents, std::array<int, 3>& pIDs, std::array<int, 3>& pCompIndex)
{
	for (int index{ 0 }; index < args.size(); ++index)
	{
		auto& arg{ args[index] };

		//stripArg(arg);

		int objIndex{ searchObjectIndexByName(arg, Context::object) };

		if (objIndex != -1)
		{
			int pIndex{ nextFreeParentIndex(pIDs) };

			pIDs[pIndex] = Context::object[objIndex].getID();
			pCompIndex[pIndex] = static_cast<int>(vecComponents.size());

			for (const auto comp : Context::object[objIndex].getComponents())
				vecComponents.push_back(comp);
		}

		else if (objIndex == -1)
		{
			int pIndex{ nextFreeParentIndex(pIDs) };

			pIDs[pIndex] = Context::componentLiteral;
			pCompIndex[pIndex] = static_cast<int>(vecComponents.size());

			convertParametersToFloat(arg, vecComponents);
		}
	}
}

// return the next free index in parentIDs array
int nextFreeParentIndex(const std::array<int, 3>& pIDs)
{
	for (int index{ 0 }; index < pIDs.size(); ++index)
		if (pIDs[index] == -1) 
			return index;

	return -1;
}

// return the object's index if it exists or return -1 if not
int searchObjectByID(int id, const std::vector<Object>& objectRef)
{
	for (int objIndex{ 0 }; objIndex < objectRef.size(); ++objIndex)
	{
		const auto& obj{ objectRef[objIndex] };

		if (obj.getID() == id)
			return objIndex;
	}

	return -1;
}

// creates a Object object
size_t createObject(Object obj, int vCount, const std::vector<float>& comp, const glm::vec4 color, uint8_t pCount, std::array<int, 3> pIDs, std::array<int, 3> pCompIndex)
{
	int offset{ 0 };
	int id{ Context::globalObjectIDCounter++ };

	if (!Context::object.empty())
	{
		int previousIndex{ static_cast<int>(Context::object.size()) - 1 };
		offset = Context::object[previousIndex].getOffset() + Context::object[previousIndex].getVertexCount();
	}

	obj.setID(id);
	obj.setOffset(offset);
	obj.setVertexCount(vCount);
	obj.setParentCount(pCount);
	obj.setComponents(comp);
	obj.setColor(color);

	if (pIDs[0] != -1)
	{
		obj.setParentIDs(pIDs);
		obj.setpCompIndex(pCompIndex);
	}

	Context::object.push_back(std::move(obj));

	return Context::object.size() - 1;
}

size_t createObject(Object obj, int vCount)
{
	int offset{ 0 };
	int id{ Context::globalObjectIDCounter++ };

	if (!Context::object.empty())
	{
		int previousIndex{ static_cast<int>(Context::object.size()) - 1 };
		offset = Context::object[previousIndex].getOffset() + Context::object[previousIndex].getVertexCount();
	}

	obj.setID(id);
	obj.setOffset(offset);
	obj.setVertexCount(vCount);

	Context::object.push_back(std::move(obj));

	return Context::object.size() - 1;
}

// delete a Object with a given index from Object's vector
void deleteObject(int objIndex, std::vector<Object>& object, std::vector<float>& vertexData)
{
	int parentID{ object[objIndex].getID() };
	std::vector<int> childIndex{};

	for (int i{ 0 }; i < static_cast<int>(object.size()); ++i)
	{
		const auto& obj{ object[i] };
		bool alreadyMarked{ false };

		for (int j{ 0 }; j < obj.getParentCount(); ++j)
		{
			if (alreadyMarked) break;

			int pID{ obj.getParentIDs()[j] };
			if (pID == parentID)
			{
				childIndex.push_back(i);
				alreadyMarked = true;
				continue;
			}

			if (pID == Context::componentLiteral || pID == -1) continue;

			int pIndex{ searchObjectByID(pID, object) };

			if (pIndex == -1) continue;

			for (int k{ 0 }; k < object[pIndex].getParentCount(); ++k)
			{
				if (object[pIndex].getParentIDs()[k] == parentID)
				{
					childIndex.push_back(i);
					alreadyMarked = true;
					break;
				}
			}
		}
	}

	childIndex.push_back(objIndex);

	std::sort(childIndex.begin(), childIndex.end(),
		[](const int a, const int b)
		{
			return a > b;
		}
	);

	for (auto i : childIndex)
	{
		const Object& obj{ object[i] };
		Context::symbolTable.erase(obj.getName());

		object.erase(object.begin() + i);
	}

	vertexData.clear();

	getEnvironmentVertices(vertexData);

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		Object& obj{ object[idx] };
		Context::symbolTable[obj.getName()] = idx;

		int newOffset = static_cast<int>(vertexData.size()) / 7;
		obj.setOffset(newOffset);

		//draw(obj.getType(), obj.getComponents(), obj.getColor(), obj.getParentIDs(), obj.getpCompIndex(), true);
		generateObjectVertices(obj, vertexData);
	}

	updateBufferData(vertexData);
}

// function to update objects
void updateObject(int objIndex, const Object& newObj, std::vector<Object>& object, std::vector<float>& vertexData)
{
	object[objIndex] = newObj;
	std::vector<int> toBeDeleted{};

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		auto& obj = object[idx];
		if (obj.getParentCount() > 0)
		{
			bool isIntersectionALive{ true };
			if (!obj.isMutable() && obj.getType() != Object::Vector) 
			{
				isIntersectionALive = recalculateIntersect(obj, object);
				if (!isIntersectionALive) toBeDeleted.push_back(static_cast<int>(idx));
				continue;
			}

			const std::array<int, 3>& currentParents{ obj.getParentIDs() };
			const std::array<int, 3>& currentOffsets{ obj.getpCompIndex() };

			for (int i{ 0 }; i < obj.getParentCount(); ++i)
			{
				if (auto pIndex = searchObjectByID(currentParents[i], object); pIndex != -1)
				{
					const std::vector<float>& parentComps{ object[pIndex].getComponents() };
					float* childCompsPtr{ obj.getComponentsPointer() };

					for (size_t j{ 0 }; j < parentComps.size(); ++j)
					{
						childCompsPtr[currentOffsets[i] + j] = parentComps[j];
					}
				}
			}
		}
	}

	if (!toBeDeleted.empty())
	{
		std::sort(toBeDeleted.begin(), toBeDeleted.end(),
			[](const int a, const int b)
			{
				return a > b;
			}
		);

		for (int index : toBeDeleted)
		{
			const Object& obj{ object[index] };
			Context::symbolTable.erase(obj.getName());

			deleteObject(index, object, vertexData);
		}
	}

	vertexData.clear();

	getEnvironmentVertices(vertexData);

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		Object& obj = object[idx];

		// update index for each object in symbolTable
		Context::symbolTable[obj.getName()] = idx;

		int newOffset = static_cast<int>(vertexData.size()) / 7;
		obj.setOffset(newOffset);
		
		//draw(obj.getType(), obj.getComponents(), obj.getColor(), obj.getParentIDs(), obj.getpCompIndex(), true);
		generateObjectVertices(obj, vertexData);
	}

	updateBufferData(vertexData);
}

void updateSelectedObjectColor(int objIndex, std::vector<Object>& object, std::vector<float>& vertexData)
{
	Object& obj{ object[objIndex] };

	std::vector<float>& comp{ obj.getComponents() };
	const size_t offset{ static_cast<size_t>(obj.getOffset() * 7) };
	const size_t vertexCount{ static_cast<size_t>(obj.getVertexCount() * 7) };
	const Object::Type type{ obj.getType() };

	glm::vec4 color{};

	if (obj.isSelected())
	{
		color = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (type == Object::Plane) color.w = 0.2f;
	}
	else
		color = obj.getColor();

	const float* colorPointer{ &color[0] };

	for (size_t i{ offset + 3 }; i < offset + vertexCount; i += 7)
	{
		for (size_t j{ 0 }; j < 4; ++j)
		{
			vertexData[i + j] = colorPointer[j];
		}
	}

	updateBufferData(vertexData);
}

bool scanForIdenticalObject(Object::Type type, const std::vector<float>& components, std::vector<Object>& object, int ignoreID)
{
	constexpr float epsilon{ 0.001f };

	for (auto& obj : object)
	{
		if (obj.getID() == ignoreID) continue;
		if (obj.getType() != type) continue;

		const auto& objComponents{ obj.getComponents() };
		if (objComponents.size() != components.size()) continue;

		bool isIdentical{ true };

		for (std::size_t i{ 0 }; i < components.size(); ++i)
		{
			if (glm::abs(objComponents[i] - components[i]) >= epsilon)
			{
				isIdentical = false;
				break;
			}
		}

		if (isIdentical)
			return true;
	}

	return false;
}

// return the content of each object
std::string getExpression(Object& obj, std::vector<Object>& object)
{
	auto type{ obj.getType() };

	if (type == Object::Point)
	{
		std::stringstream ss{};
		
		if (!obj.isMutable())
		{
			ss << "Intersect(";
			ss << object[searchObjectByID(obj.getParentIDs()[0], object)].getName() << ", ";
			ss << object[searchObjectByID(obj.getParentIDs()[1], object)].getName();
			ss << ")";

			return ss.str();
		}
		else 
		{
			ss << "Point";
			char parenthesis{ '(' };
			auto cSize{ obj.getComponents().size() };

			for (int i{ 0 }; i < cSize; ++i)
			{
				if (i % 3 == 0)
				{
					ss << parenthesis;
					parenthesis = (parenthesis == '(' ? ')' : '(');
				}

				if (i == cSize - 1) ss << obj.getComponents()[i] << parenthesis;

				else ss << obj.getComponents()[i] << ", ";
			}

			return ss.str();
		}

	}

	else
	{
		std::stringstream ss{};

		if (type == Object::Line)
		{
			if (!obj.isMutable())
			{
				ss << "Intersect(";
				ss << object[searchObjectByID(obj.getParentIDs()[0], object)].getName() << ", ";
				ss << object[searchObjectByID(obj.getParentIDs()[1], object)].getName();
				ss << ")";

				return ss.str();
			}
		}

		else if (type == Object::Vector && !obj.isMutable())
		{
			ss << "Cross(";
			ss << object[searchObjectByID(obj.getParentIDs()[0], object)].getName() << ", ";
			ss << object[searchObjectByID(obj.getParentIDs()[1], object)].getName();
			ss << ")";

			return ss.str();
		}

		ss << getStringFunctionType(type) << "(";

		auto comp{ obj.getComponents() };

		int pCount{ 0 };

		for (auto p : obj.getpCompIndex())
			if (p >= 0) ++pCount;

		for (int i{ 0 }; i < pCount; ++i)
		{
			auto pID{   obj.getParentIDs()[i] };
			auto start{ obj.getpCompIndex()[i] };

			if (pID >= 0)
			{
				auto parent{ object[searchObjectByID(pID, object)].getName() };

				ss << parent;

				if (i < pCount - 1) ss << ", ";
			}

			else ss << "(" << comp[start] << ", " << comp[start + 1] << ", " << comp[start + 2];

			if (i == pCount - 1) ss << ")";

			if (pID == -2) ss << "), ";
		}

		return ss.str();
	}

	return "";
}

// return the equations of planes and lines
std::string getEquation(Object& obj)
{
	Object::Type type{ obj.getType() };

	if (type == Object::Plane)
	{
		std::array<glm::vec3, 3> plane{ assemblyPlane(obj, Context::object) };

		glm::vec3 normal{ plane[1] - plane[0] };
		glm::vec3 point{ plane[2] };

		float d{ -normal.x * point.x - normal.y * point.y - normal.z * point.z };

		std::stringstream ss{};

		std::string sign{};
		auto normalPointer{ &normal[0] };
		char comp{ 'x' };

		for (int i{ 0 }; i < 3; ++i)
		{
			float value = normalPointer[i];

			if (ss.str().length() == 0 && value != 0.0f)
			{
				if (value == 1.0f) ss << comp;

				else if (value == -1.0f)
				{
					sign = "-";
					ss << sign << comp;
				}

				else ss << value << comp;

				++comp;

				continue;
			}

			if (value != 0.0f)
			{
				sign = (value > 0.0f ? " + " : " - ");

				if (value < 0.0f) value = -value;

				if (value == 1.0f) ss << sign << comp;

				else ss << sign << value << comp;
			}

			++comp;
		}

		if (d != 0.0f)
		{
			sign = (d > 0.0f ? " + " : " - ");

			if (d < 0.0f) d = -d;

			ss << sign << d << " = 0";
		}

		else ss << " = 0";

		return ss.str();
	}

	else if (type == Object::Line)
	{
		std::array<glm::vec3, 3> line{ assemblyLine(obj) };

		glm::vec3 point{ line[0] };
		glm::vec3 dVector{ line[2] - line[1] };

		std::stringstream ss{};

		ss << "P = (" << point.x << ", " << point.y << ", " << point.z << ") + t(" << dVector.x << ", " << dVector.y << ", " << dVector.z << ")";

		return ss.str();
	}

	return "";
}

// return the intersection of a plane and a line, if it exist
// return (-9999,-9999,-9999) if not
glm::vec3 intersectionLinePlane(glm::vec3 linePoint, glm::vec3 lineVector, glm::vec3 planeNormal, float d)
{
	float divisor{ glm::dot(planeNormal, lineVector) };
	float t{};

	// t = -(a.x1 + b.y1 + c.z1 + d) / n.v
	// where
	// A(x1, y1, z1) is a point of the line
	// v is the direction vector of the line
	// n = (a, b, c) is the normal vector of the plane
	// 
	// d = -(a.x0 + b.y0 + c.z0)
	// B(x0, y0, z0) is a point of the plane

	const float epsilon{ 0.001f };
	if (glm::abs(divisor) < epsilon)
	{
		if (glm::abs(glm::dot(planeNormal, linePoint) + d) < epsilon)
			return linePoint;

		return glm::vec3(-9999.0f, -9999.0f, -9999.0f);
	}
	else	
		t = -(glm::dot(planeNormal, linePoint) + d) / divisor;

	glm::vec3 intersection{ linePoint + t * lineVector };

	return intersection;
}

// return the intersection of a line and a line, if it exist
// return (-9999,-9999,-9999) if not
glm::vec3 intersectionLineLine(glm::vec3 ps, glm::vec3 vs, glm::vec3 pt, glm::vec3 vt)
{
	// s: P = ps + s * vs
	// t: P = pt + t * vt

	const float epsilon{ 0.001f }; // for float comparison purposes
	auto cross{ glm::cross(vs, vt) };

	bool isParallel{ true };
	for (int i{ 0 }; i < 3; ++i)
	{
		if (glm::abs(cross[i]) >= epsilon)
		{
			isParallel = false;
			break;
		}
	}

	auto w0{ pt - ps };
	
	bool isSuperimposed{ false };
	if (isParallel)
	{
		auto separationCross{ glm::cross(w0, vt) };

		isSuperimposed = true;
		for (int i{ 0 }; i < 3; ++i)
		{
			if (glm::abs(separationCross[i]) >= epsilon)
			{
				isSuperimposed = false;
				break;
			}
		}
	}

	if (isSuperimposed)
		return ps;

	auto crossDot{ glm::dot(cross, w0) };

	if (glm::abs(crossDot) >= epsilon)
		return glm::vec3(-9999.0f, -9999.0f, -9999.0f); // intersection doesn't exist

	float a{ glm::dot(vs, vs) };
	float b{ glm::dot(vs, vt) };
	float c{ glm::dot(vt, vt) };
	float d{ glm::dot(w0, vs) };
	float e{ glm::dot(w0, vt) };

	float D{ a * (-c) + b * b };
	float Ds{ -d * c + e * b };

	float s{ Ds / D };
	
	glm::vec3 intersection{ ps + s * vs };

	return intersection;
}

std::array<glm::vec3, 2> intersectionPlanePlane(glm::vec3 p1, glm::vec3 n1, glm::vec3 p2, glm::vec3 n2)
{
	const float epsilon{ 0.001f };
	auto dVec{ glm::cross(n1, n2) };

	float d1{ -glm::dot(n1, p1) };
	float d2{ -glm::dot(n2, p2) };

	std::array<glm::vec3, 2> intersection{};

	// parallel planes
	bool isSuperimposed{ false };
	bool isParallel{ false };
	if (glm::length(dVec) < epsilon)
	{
		isParallel = true;

		float pointTest{ glm::dot(n2, p1) + d2 };
		if (glm::abs(pointTest) < epsilon)
		{
			isSuperimposed = true;
			isParallel = false;
		}
	}

	if (isSuperimposed)
	{
		std::cerr << "The planes are the same. Handle later.\n";
		return intersection;
	}
	else if (isParallel)
	{
		std::cerr << "The intersection doesn't exist. Handle later.\n";
		return intersection;
	}

	if (glm::abs(dVec.x) >= glm::abs(dVec.y) && glm::abs(dVec.x) >= glm::abs(dVec.z))
	{
		float D{ dVec.x };
		float Dy{ -d1 * n2.z + d2 * n1.z };
		float Dz{ -n1.y * d2 + n2.y * d1 };

		float x{ 0.0f };
		float y{ Dy / D };
		float z{ Dz / D };

		intersection[0] = glm::vec3(x, y, z);
	}
	else if (glm::abs(dVec.y) >= glm::abs(dVec.z))
	{
		float D{ dVec.y };
		float Dx{ -n1.z * d2 + n2.z * d1 };
		float Dz{ -d1 * n2.x + d2 * n1.x }; 

		float x{ Dx / D };
		float y{ 0.0f };
		float z{ Dz / D };

		intersection[0] = glm::vec3(x, y, z);
	}
	else
	{
		float D{ dVec.z };
		float Dx{ -d1 * n2.y + d2 * n1.y };
		float Dy{ -n1.x * d2 + n2.x * d1 };

		float x{ Dx / D };
		float y{ Dy / D };
		float z{ 0.0f };

		intersection[0] = glm::vec3(x, y, z);
	}

	intersection[1] = dVec;

	return intersection;
}

std::vector<std::string> testInput(std::string input)
{
	std::vector<std::string> inputArray{};

	std::string str{};
	for (auto c : input)
	{
		if (c == '\n')
		{
			inputArray.push_back(str);
			str.clear();
		}
		else
		{
			str += c;
		}
	}

	return inputArray;
}

// return true if the intersection persists
// return false if the intersection ceases to exist
bool recalculateIntersect(Object& obj, std::vector<Object>& object)
{
	Object::Type type{ obj.getType() };
	std::array<int, 3> pIDs{ obj.getParentIDs() };

	if (type == Object::Line)
	{
		Intersect intersect{ gatherPlaneLine(obj, object) };

		auto [p1, p2] = intersect.points;
		auto [n1, n2] = intersect.vectors;

		std::array<glm::vec3, 2> intersection{ intersectionPlanePlane(p1, n1, p2, n2) };

		constexpr float epsilon{ 0.001f };
		if (glm::length(intersection[1]) < epsilon)
		{
			std::cerr << "Intersection doesn't exist.\n";
			return false;
		}

		obj.getComponents()[0] = intersection[0].x;
		obj.getComponents()[1] = intersection[0].y;
		obj.getComponents()[2] = intersection[0].z;
		obj.getComponents()[3] = intersection[1].x;
		obj.getComponents()[4] = intersection[1].y;
		obj.getComponents()[5] = intersection[1].z;
	}

	else if (type == Object::Point)
	{
		Intersect intersect{ gatherPlaneLine(obj, object) };

		glm::vec3 intersection{ assemblyIntersectPoint(intersect) };

		if (intersection == glm::vec3(-9999.0f, -9999.0f, -9999.0f))
		{
			std::cerr << "Intersection doesn't exist.\n";
			return false;
		}

		obj.getComponents()[0] = intersection[0];
		obj.getComponents()[1] = intersection[1];
		obj.getComponents()[2] = intersection[2];
	}

	return true;
}

bool projectWorldToScreen
(
	const glm::vec3& worldPos,			  
	const glm::mat4& viewMatrix,			  
	const glm::mat4& projectionMatrix,	  
	const glm::mat4& modelMatrix,
	const glm::vec2& viewportPos,  
	const glm::vec2& viewportSize,	        
		  glm::vec2& outScreenPos
)
{
	glm::vec4 clipSpacePos{ projectionMatrix * viewMatrix * modelMatrix * glm::vec4(worldPos, 1.0f) };
	float wc{ clipSpacePos.w };
	
	if (wc <= 0.0f)
	{
		return false;
	}

	glm::vec3 ndc{ glm::vec3(clipSpacePos) / clipSpacePos.w };

	if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f)
	{
		return false;
	}

	outScreenPos.x = ((ndc.x + 1.0f) * 0.5f) * viewportSize.x + viewportPos.x;
	outScreenPos.y = ((-ndc.y + 1.0f) * 0.5f) * viewportSize.y + viewportPos.y;

	return true;
}

int getSelectedObjectID(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, std::vector<Object>& object)
{
	int closestIndex{ -1 };
	float closestT{ FLT_MAX };
	float closestTEffective{ FLT_MAX };
	constexpr float epsilon{ 0.005f };
	constexpr float scale{ 0.1f };

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		Object& obj{ object[idx] };
		Object::Type type{ obj.getType() };
		auto& comp{ obj.getComponents() };
		
		if (type == Object::Point)
		{
			glm::vec3 targetPoint{ comp[0], comp[1], comp[2] };
			targetPoint *= scale;

			glm::vec3 v{ targetPoint - rayOrigin };
			float T{ glm::dot(v, rayDirection) };
			
			if (T < 0.0f) continue;

			glm::vec3 closestPointOnRay{ rayOrigin + T * rayDirection };

			float distance{ glm::length(targetPoint - closestPointOnRay) };

			if (distance < epsilon)
			{
				float bias{ obj.isMutable() ? 0.06f : 0.08f };
				float tEffective{ T - bias };

				if (tEffective < closestTEffective)
				{
					closestT = T;
					closestTEffective = tEffective;
					closestIndex = static_cast<int>(idx);
				}
			}
		}

		else if (type == Object::Vector || type == Object::Segment || type == Object::Line)
		{
			constexpr float epsilon_0{ 0.001f };

			glm::vec3 point{};
			glm::vec3 vecOrigin{};
			glm::vec3 vecHead{};

			if (type == Object::Line)
			{
				std::array<glm::vec3, 3> line{ assemblyLine(obj) };

				point = line[0];
				vecOrigin = line[1];
				vecHead = line[2];
			}

			else if (type == Object::Vector)
			{
				std::array<glm::vec3, 2> vector{ assemblyVector(obj, object) };

				point = vector[0];
				vecOrigin = vector[0];
				vecHead = vector[1];
			}

			else if (type == Object::Segment)
			{
				point = { comp[0], comp[1], comp[2] };
				vecOrigin = { comp[0], comp[1], comp[2] };
				vecHead = { comp[3], comp[4], comp[5] };
			}

			point *= scale;
			vecOrigin *= scale;
			vecHead *= scale;

			glm::vec3 vecDirection{ vecHead - vecOrigin };
			float lineLength{ glm::length(vecDirection) };

			if (lineLength < 0.0001f) continue;

			glm::vec3 normalizedVecDirection{ vecDirection / lineLength };

			glm::vec3 w0{ rayOrigin - point };
			glm::vec3 normalizedRayDirection{ glm::normalize(rayDirection) };

			float b{ glm::dot(normalizedRayDirection, normalizedVecDirection) };
			float d{ glm::dot(normalizedRayDirection, w0) };
			float e{ glm::dot(normalizedVecDirection, w0) };

			float D{ 1.0f - b * b };
			if (glm::abs(D) < epsilon_0) continue;

			float s{ (e - b * d) / D };
			float t{ b * s - d };

			if (type == Object::Vector || type == Object::Segment)
			{
				if (s < 0.0f || s > lineLength)
				{
					if (s < 0.0f)
						s = 0.0f;
					else if (s > lineLength)
						s = lineLength;

					t = glm::dot((point + s * normalizedVecDirection) - rayOrigin, normalizedRayDirection);
				}
			}

			if (t < 0.0f) continue;

			glm::vec3 pRay{ rayOrigin + t * normalizedRayDirection };
			glm::vec3 pVec{ point + s * normalizedVecDirection };

			float distance{ glm::length(pRay - pVec) };

			if (distance < epsilon)
			{
				float bias{ 0.02f };
				if (type == Object::Line && !obj.isMutable())
				{
					bias = 0.05f; 
				}

				float tEffective{ t - bias };
				if (tEffective < closestTEffective)
				{
					closestT = t;
					closestTEffective = tEffective;
					closestIndex = static_cast<int>(idx);
				}
			}
		}

		else if (type == Object::Plane)
		{
			std::array<glm::vec3, 3> plane{ assemblyPlane(obj, object) };

			auto [normalOrigin, normalHead, point] = plane;

			normalOrigin *= scale;
			normalHead *= scale;
			point *= scale;

			glm::vec3 direction{ normalHead - normalOrigin };
			glm::vec3 planeNormal{ glm::normalize(direction) };

			glm::vec3 right{};
			glm::vec3 up{};
			getNewCoordSystem(direction, right, up);

			float divisor{ glm::dot(planeNormal, rayDirection) };
			float d{ -glm::dot(planeNormal, point) };

			constexpr float epsilon{ 0.001f };
			float t{ -1.0f };
			bool hasIntersect{ false };
			if (glm::abs(divisor) < epsilon)
			{
				if (glm::abs(glm::dot(planeNormal, rayOrigin) + d) < epsilon)
				{
					float t = 0.0f;
					hasIntersect = true;
				}

				else
					continue;
			}
			else
			{
				t = -(glm::dot(planeNormal, rayOrigin) + d) / divisor;
				if (t >= 0.0f)
				{
					hasIntersect = true;
				}
			}

			if (hasIntersect)
			{
				glm::vec3 intersectionPoint{ rayOrigin + t * rayDirection };
				glm::vec3 relativePos{ intersectionPoint - point };

				float uCoord{ glm::dot(relativePos, right) };
				float vCoord{ glm::dot(relativePos, up) };

				if (glm::abs(uCoord) <= 1.0f && glm::abs(vCoord) <= 1.0f)
				{
					float bias{ 0.0f };
					float tEffective{ t - bias };

					if (tEffective < closestTEffective)
					{
						closestT = t;
						closestTEffective = tEffective;
						closestIndex = static_cast<int>(idx);
					}
				}
			}
		}
	}

	if (closestIndex > 0)
		return object[closestIndex].getID();

	return -1;
}
