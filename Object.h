#ifndef OBJECT_H
#define OBJECT_H

#include <array>
#include "draw_utils.h"

class Object
{
public:
	enum Type
	{
		Point,
		Vector,
		Segment,
		Plane
	};

	Object(std::string name, Type type, unsigned int primitive)
		: m_name{ name }
		, m_type { type }
		, m_primitive{ primitive }
	{ }

	int						  getID() const          { return m_id; }
	const std::string&		  getName() const        { return m_name; }
	Type				      getType() const        { return m_type; }
	unsigned int		      getPrimitive() const   { return m_primitive; }
	int				          getOffset() const      { return m_offset; }
	int					      getVertexCount() const { return m_vertexCount; }
	std::vector<float>&		  getComponents()		 { return m_components; }
	const glm::vec4&          getColor() const		 { return m_color; }
	const std::array<int, 3>& getParentIDs() const   { return m_parentIDs; }
	uint8_t			          getParentCount() const { return m_parentCount; }
	const std::array<int, 3>& getpCompIndex() const  { return m_pCompIndex; }


	void setID(int id)										 { m_id = id; }
	void setName(const std::string& name)					 { m_name = name; }
	void setOffset(int offset)								 { m_offset = offset; }
	void setVertexCount(int vertexCount)					 { m_vertexCount = vertexCount; }
	void setComponents(const std::vector<float>& components) { m_components = components; }
	void setColor(const glm::vec4& color)					 { m_color = color; }
	void setParentIDs(const std::array<int, 3>& parentIDs)   { m_parentIDs = parentIDs; }
	void setParentCount(uint8_t parentCount)				 { m_parentCount = parentCount; }
	void setpCompIndex(const std::array<int, 3>& pCompIndex) { m_pCompIndex = pCompIndex; }

	float* getComponentsPointer() { return m_components.data(); }
	float* getColorPointer()	  { return &m_color[0]; }

private:
	int m_id{};
	std::string m_name{};
	Type m_type{};
	unsigned int m_primitive{};

	int m_offset{};
	int m_vertexCount{};

	std::vector<float> m_components{};
	glm::vec4 m_color{};

	std::array<int, 3> m_parentIDs{ -1, -1, -1 }; // default values to a object that has no parent
	std::array<int, 3> m_pCompIndex{ -1, -1, -1 };
	uint8_t m_parentCount{}; // max parent count. for example: a vector can have up to two parents
};

// forward declarations for functions that I don't know where to put yet

int searchObjectByID(int id, const std::vector<Object>& objectRef);
void createObject(Object obj, int vCount, const std::vector<float>& comp, const glm::vec4 color, uint8_t pCount, std::array<int, 3> pIDs = { -1, -1, -1 }, std::array<int, 3> pCompIndex = { -1, -1, -1 });
void draw(Object::Type type, std::vector<float>& vecComponents, glm::vec4 color, std::array<int, 3> pIDs = { -1, -1, -1 }, std::array<int, 3> pCompIndex = {-1, -1, -1}, bool update = false);
bool compareObjectType(const std::string& component, Object::Type expectedType);
std::string getStringFunctionType(Object::Type type);
std::vector<float> deleteObjectFromVertexData(int objIndex);
void updateObject(int objIndex, const Object& newObj);
int searchObjectIndex(const std::string& objName);
int nextFreeParentID(const std::array<int, 3>& pIDs);

extern std::vector<Object> object;

#endif

