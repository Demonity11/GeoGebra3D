#include "utilities.h"
#include "draw_utils.h"
#include "objectCoords.h"
#include "Context.h"

#include <format>
#include <iomanip>

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
	for (int index{ 0 }; index < object.size(); ++index)
	{
		const auto& obj{ object[index] };

		if (obj.getName() == objName)
			return index;
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
void createObject(Object obj, int vCount, const std::vector<float>& comp, const glm::vec4 color, uint8_t pCount, std::array<int, 3> pIDs, std::array<int, 3> pCompIndex)
{
	int offset{ 0 };
	int id{ static_cast<int>(Context::object.size()) };

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

	for (const auto& obj : Context::object)
	{
		std::cout << obj.getName() << "::" << obj.getID() << "::" << static_cast<int>(obj.getParentCount()) << "\n";
	}

	std::cout << "\n";
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

			std::cout << object[pIndex].getName() << '\n';

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
		std::cout << i << " ";
		object.erase(object.begin() + i);
	}

	std::cout << "\n";

	vertexData.clear();

	getEnvironmentVertices(vertexData);

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		auto& obj = object[idx];

		int newOffset = static_cast<int>(vertexData.size()) / 7;
		obj.setOffset(newOffset);

		draw(obj.getType(), obj.getComponents(), obj.getColor(), obj.getParentIDs(), obj.getpCompIndex(), true);
	}

	updateBufferData(vertexData);
	std::cout << "executei\n";
}

// delete objects from vertexData
std::vector<float> deleteObjectFromVertexData(int objIndex, std::vector<float>& vertexData, std::vector<Object>& object)
{
	if (vertexData.empty())
		return {};

	// 7 is the number of components for each vertice
	// 3 position components + 4 color values
	const auto& obj{ object[objIndex] };

	int offset{ obj.getOffset() * 7 };
	int floatCount{ obj.getVertexCount() * 7 };

	std::vector<float> newVertexData{};

	newVertexData.reserve(vertexData.size() - floatCount);

	for (int i{ 0 }; i < vertexData.size(); ++i)
	{
		if (i >= offset && i < offset + floatCount)
			continue;

		newVertexData.push_back(vertexData[i]);
	}

	return newVertexData;
}

// new function to update objects
void updateObject(int objIndex, const Object& newObj, std::vector<Object>& object, std::vector<float>& vertexData)
{
	object[objIndex] = newObj;

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		auto& obj = object[idx];
		if (obj.getParentCount() > 0)
		{
			// intersection has parents which causes the program to crash when updating the color of the intersection
			// this prevents this crash
			if (obj.getType() == Object::Point) continue;

			std::array<int, 3> currentParents = obj.getParentIDs();
			std::array<int, 3> currentOffsets = obj.getpCompIndex();

			for (int i{ 0 }; i < obj.getParentCount(); ++i)
			{
				if (auto pIndex = searchObjectByID(currentParents[i], object); pIndex != -1)
				{
					const auto& parentComps = object[pIndex].getComponents();
					float* childCompsPtr = obj.getComponentsPointer();

					for (size_t j{ 0 }; j < parentComps.size(); ++j)
					{
						childCompsPtr[currentOffsets[i] + j] = parentComps[j];
					}
				}
			}
		}
	}

	vertexData.clear();

	getEnvironmentVertices(vertexData);

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		auto& obj = object[idx];

		int newOffset = static_cast<int>(vertexData.size()) / 7;
		obj.setOffset(newOffset);

		draw(obj.getType(), obj.getComponents(), obj.getColor(), obj.getParentIDs(), obj.getpCompIndex(), true);
	}

	updateBufferData(vertexData);
}

// return true if exist an object with the same type and components 
bool scanForIdenticalObject(Object::Type type, const std::vector<float>& components, std::vector<Object>& object)
{
	const float epsilon{ 0.001f };

	for (auto& obj : object)
	{
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
	auto type{ obj.getType() };

	if (type == Object::Plane)
	{
		int startPoint{ obj.getpCompIndex()[0] };
		int startNormal{ obj.getpCompIndex()[1] };

		glm::vec3 normal
		{
			obj.getComponents()[startNormal + 3] - obj.getComponents()[startNormal],
			obj.getComponents()[startNormal + 4] - obj.getComponents()[startNormal + 1],
			obj.getComponents()[startNormal + 5] - obj.getComponents()[startNormal + 2]
		};

		glm::vec3 point{ obj.getComponents()[startPoint], obj.getComponents()[startPoint + 1], obj.getComponents()[startPoint + 2] };

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
		auto comp{ obj.getComponents() };

		glm::vec3 dVector{};
		glm::vec3 point{ comp[0], comp[1], comp[2] };

		if (comp.size() == 6)
			dVector = { comp[3] - comp[0], comp[4] - comp[1], comp[5] - comp[2] };

		else if (comp.size() == 9)
			dVector = { comp[6] - comp[3], comp[7] - comp[4], comp[8] - comp[5] };

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

	bool isSuperimposed{ false };
	if (isParallel)
	{
		isSuperimposed = true;

		auto tStart{ (ps[0] - pt[0]) / vt[0] };
		for (int i{ 1 }; i < 3; ++i)
		{
			auto t{ (ps[i] - pt[i]) / vt[i] };

			if (glm::abs(tStart - t) >= epsilon)
			{
				isSuperimposed = false;
				break;
			}
		}
	}

	if (isSuperimposed)
		return ps;

	auto w0{ pt - ps };

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