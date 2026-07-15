#include "objectAssembling.h"
#include "utilities.h"

Intersect gatherPlaneLine(const Object& obj, std::vector<Object>& object)
{
	const std::array<int, 3>& pIDs{ obj.getParentIDs() };

	std::array<Object, 2> args{};

	for (int i{ 0 }; i < pIDs.size(); ++i)
	{
		if (pIDs[i] >= 0)
		{
			int index{ searchObjectByID(pIDs[i], object) };
			const auto& obj{ object[index] };

			args[i] = obj;
		}
	}
	
	Intersect intersect{};

	for (size_t i{ 0 }; i < args.size(); ++i)
	{
		const auto& arg{ args[i] };

		if (arg.getType() == Object::Line)
		{
			int parentIndex1{ searchObjectByID(arg.getParentIDs()[0], object) };
			auto comp{ object[parentIndex1].getComponents() };

			intersect.points[i] = { comp[0], comp[1], comp[2] };

			int parentIndex2{ searchObjectByID(arg.getParentIDs()[1], object) };
			comp = object[parentIndex2].getComponents();

			if (object[parentIndex2].getType() == Object::Vector)
				intersect.vectors[i] = { comp[3] - comp[0], comp[4] - comp[1], comp[5] - comp[2] };

			else if (object[parentIndex2].getType() == Object::Point)
				intersect.vectors[i] = { comp[0] - intersect.points[i].x, comp[1] - intersect.points[i].y, comp[2] - intersect.points[i].z };

			++intersect.lineCount;
			intersect.types[i] = Object::Line;
		}

		else if (arg.getType() == Object::Plane)
		{
			int parentIndex1 = searchObjectByID(arg.getParentIDs()[0], object);
			auto comp{ object[parentIndex1].getComponents() };

			intersect.points[i] = { comp[0], comp[1], comp[2] };

			int parentIndex2{ searchObjectByID(arg.getParentIDs()[1], object) };
			if (object[parentIndex2].getType() == Object::Point)
			{
				glm::vec3 A{ intersect.points[i] };

				comp = object[parentIndex2].getComponents();
				glm::vec3 B{ comp[0], comp[1], comp[2] };

				int parentIndex3{ searchObjectByID(arg.getParentIDs()[2], object) };
				comp = object[parentIndex3].getComponents();
				glm::vec3 C{ comp[0], comp[1], comp[2] };

				glm::vec3 u{ B - A };
				glm::vec3 v{ C - A };

				glm::vec3 normal{ glm::cross(u, v) };

				intersect.vectors[i] = normal;
			}

			else
			{
				comp = object[parentIndex2].getComponents();

				intersect.vectors[i] = { comp[3] - comp[0], comp[4] - comp[1], comp[5] - comp[2] };
			}

			++intersect.planeCount;
			intersect.types[i] = Object::Plane;
		}
	}

	return intersect;
}

glm::vec3 assemblyIntersectPoint(const Intersect& intersect)
{
	glm::vec3 intersection{};

	if (intersect.lineCount == 2)
	{
		intersection = intersectionLineLine(intersect.points[0], intersect.vectors[0], intersect.points[1], intersect.vectors[1]);
	}

	else
	{
		float d{};
		if (intersect.types[0] == Object::Plane)
		{
			d = -glm::dot(intersect.points[0], intersect.vectors[0]);
			intersection = intersectionLinePlane(intersect.points[1], intersect.vectors[1], intersect.vectors[0], d);
		}
		else
		{
			d = -glm::dot(intersect.points[1], intersect.vectors[1]);
			intersection = intersectionLinePlane(intersect.points[0], intersect.vectors[0], intersect.vectors[1], d);
		}
	}

	if (intersection == glm::vec3(-9999.0f, -9999.0f, -9999.0f))
	{
		std::cerr << "Intersection doesn't exist. Handle later.\n";
	}

	return intersection;
}

std::array<glm::vec3, 2> assemblyVector(Object& obj, const std::vector<Object>& object)
{
	// object data
	const std::array<int, 3>& pIDs{ obj.getParentIDs() };
	const std::array<int, 3>& pCompIndex{ obj.getpCompIndex() };
	const std::vector<float>& comp{ obj.getComponents() };

	std::array<glm::vec3, 2> vector{};

	Object::Type pType1{ object[searchObjectByID(pIDs[1], object)].getType() };

	if (pType1 == Object::Point)
	{
		int startOrigin{ pCompIndex[0] };
		int startHead{ pCompIndex[1] };

		glm::vec3 vecOrigin{ comp[startOrigin], comp[startOrigin + 1], comp[startOrigin + 2] };
		glm::vec3 vecHead{ comp[startHead], comp[startHead + 1], comp[startHead + 2] };

		vector[0] = vecOrigin;
		vector[1] = vecHead;
	}

	// when the vector is a cross vector
	else if (pType1 == Object::Vector)
	{
		int startU{ pCompIndex[0] };
		int startV{ pCompIndex[1] };

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

		vector[0] = glm::vec3(0.0f, 0.0f, 0.0f);
		vector[1] = cross;
	}

	return vector;
}

std::array<glm::vec3, 3> assemblyLine(Object& obj)
{
	const std::vector<float>& comp{ obj.getComponents() };
	const std::array<int, 3>& pCompIndex{ obj.getpCompIndex() };

	std::array<glm::vec3, 3> line{};
	
	if (!obj.isMutable())
	{
		line[0] = { comp[0], comp[1], comp[2] };
		line[1] = { 0.0f, 0.0f, 0.0f };
		line[2] = { comp[3], comp[4], comp[5] };

		return line;
	}

	int startPoint{ pCompIndex[0] };
	int startVector{ pCompIndex[1] };

	glm::vec3 point{ comp[startPoint],  comp[startPoint + 1],  comp[startPoint + 2] };
	glm::vec3 dVectorOrigin{};
	glm::vec3 dVectorHead{};

	if (comp.size() == 6)
	{
		dVectorOrigin = { comp[startPoint],  comp[startPoint + 1],  comp[startPoint + 2] };
		dVectorHead = { comp[startVector], comp[startVector + 1], comp[startVector + 2] };
	}

	else if (comp.size() == 9)
	{
		dVectorOrigin = { comp[startVector],   comp[startVector + 1], comp[startVector + 2] };
		dVectorHead = { comp[startVector + 3], comp[startVector + 4], comp[startVector + 5] };
	}

	line[0] = point;
	line[1] = dVectorOrigin;
	line[2] = dVectorHead;

	return line;
}

std::array<glm::vec3, 3> assemblyPlane(Object& obj, const std::vector<Object>& object)
{
	// object data
	const std::array<int, 3>& pIDs{ obj.getParentIDs() };
	const std::array<int, 3>& pCompIndex{ obj.getpCompIndex() };
	const std::vector<float>& comp{ obj.getComponents() };

	Object::Type pType1{ object[searchObjectByID(pIDs[1], object)].getType() };

	std::array<glm::vec3, 3> plane{};

	if (pType1 == Object::Vector)
	{
		int startPoint{ pCompIndex[0] };
		int startNormal{ pCompIndex[1] };

		glm::vec3 normalP0{ comp[startNormal], comp[startNormal + 1], comp[startNormal + 2] };
		glm::vec3 normalP{ comp[startNormal + 3], comp[startNormal + 4], comp[startNormal + 5] };
		glm::vec3 point{ comp[startPoint], comp[startPoint + 1], comp[startPoint + 2] };

		plane[0] = normalP0;
		plane[1] = normalP;
		plane[2] = point;
	}

	else if (pType1 == Object::Point)
	{
		int startPointA{ pCompIndex[0] };
		int startPointB{ pCompIndex[1] };
		int startPointC{ pCompIndex[2] };

		glm::vec3 A{ comp[startPointA], comp[startPointA + 1], comp[startPointA + 2] };
		glm::vec3 B{ comp[startPointB], comp[startPointB + 1], comp[startPointB + 2] };
		glm::vec3 C{ comp[startPointC], comp[startPointC + 1], comp[startPointC + 2] };

		glm::vec3 u{ B - A };
		glm::vec3 v{ C - A };

		glm::vec3 normal{ glm::cross(u, v) };

		plane[0] = glm::vec3(0.0f, 0.0f, 0.0f);
		plane[1] = normal;
		plane[2] = A;
	}

	return plane;
}