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
	const std::vector<float>&  getComponents() const { return m_components; }
	const glm::vec4& getColor() const				 { return m_color; }
	const std::array<int, 3>& getParentIDs() const   { return m_parentIDs; }
	uint8_t			          getParentCount() const { return m_parentCount; }

	void setID(int id)										 { m_id = id; }
	void setName(const std::string& name)					 { m_name = name; }
	void setOffset(int offset)								 { m_offset = offset; }
	void setVertexCount(int vertexCount)					 { m_vertexCount = vertexCount; }
	void setComponents(const std::vector<float>& components) { m_components = components; }
	void setColor(const glm::vec4& color)					 { m_color = color; }
	void setParentIDs(const std::array<int, 3>& parentIDs)   { m_parentIDs = parentIDs; }
	void setParentCount(uint8_t parentCount)				 { m_parentCount = parentCount; }

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
	uint8_t m_parentCount{}; // max parent count. for example: a vector can have up to two parents
};

// forward declarations for functions that I don't know where to put yet

int searchObject(int id, const std::vector<Object>& objectRef);
void createObject(Object obj, int vCount, const std::vector<float>& comp, const glm::vec4 color, uint8_t pCount, std::array<int, 3> pIDs = { -1, -1, -1 });
void draw(Object::Type type, const std::vector<float>& vecComponents, glm::vec4 color, std::array<int, 3> pIDs = { -1, -1, -1 }, bool update = false);
bool compareObjectType(const std::string& component, Object::Type expectedType);
std::string getStringFunctionType(Object::Type type);
std::vector<float> deleteObjectFromVertexData(int objIndex);
void updateObject(int objIndex, const Object& newObj);

extern std::vector<Object> object;

#endif

