#include "utilities.h"
#include "draw_utils.h"
#include "objectCoords.h"

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

// compare if the given object has the specified type
bool compareObjectType(const std::string& objName, Object::Type expectedType)
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
int searchObjectIndexByName(const std::string& objName)
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

		int objIndex{ searchObjectIndexByName(arg) };

		if (objIndex != -1)
		{
			int pIndex{ nextFreeParentIndex(pIDs) };

			std::cout << "pIndex: " << pIndex << "\n";

			pIDs[pIndex] = object[objIndex].getID();
			pCompIndex[pIndex] = static_cast<int>(vecComponents.size());

			for (const auto comp : object[objIndex].getComponents())
				vecComponents.push_back(comp);
		}

		else if (objIndex == -1)
		{
			int pIndex{ nextFreeParentIndex(pIDs) };

			std::cout << "pIndex: " << pIndex << "\n";

			pIDs[pIndex] = componentLiteral;
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
	int id{ static_cast<int>(object.size()) };

	if (!object.empty())
	{
		int previousIndex{ static_cast<int>(object.size()) - 1 };
		offset = object[previousIndex].getOffset() + object[previousIndex].getVertexCount();
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

	object.push_back(std::move(obj));
}

// delete a Object with a given index from Object's vector
void deleteObject(int objIndex)
{
	for (std::vector<Object>::iterator it = object.begin(); it != object.end();)
	{
		if (it->getID() == objIndex)
			it = object.erase(it);
		else
			++it;
	}
}

// delete objects from vertexData
std::vector<float> deleteObjectFromVertexData(int objIndex)
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
void updateObject(int objIndex, const Object& newObj)
{
	object[objIndex] = newObj;

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		auto& obj = object[idx];
		if (obj.getParentCount() > 0)
		{
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

	getEnvironmentVertices();

	for (size_t idx{ 8 }; idx < object.size(); ++idx)
	{
		auto& obj = object[idx];

		int newOffset = static_cast<int>(vertexData.size()) / 7;
		obj.setOffset(newOffset);

		draw(obj.getType(), obj.getComponents(), obj.getColor(), obj.getParentIDs(), obj.getpCompIndex(), true);
	}

	updateBufferData(vertexData);
}