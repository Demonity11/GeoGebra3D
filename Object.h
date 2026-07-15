#ifndef OBJECT_H
#define OBJECT_H

#include <array>
#include "draw_utils.h"

class Object
{
public:
	enum Type
	{
		Null,
		Point,
		Vector,
		Segment,
		Line,
		Plane
	};

	Object(std::string name, Type type, unsigned int primitive)
		: m_name{ name }
		, m_type { type }
		, m_primitive{ primitive }
	{ }

	Object
	(
		std::string name, 
		Type type, 
		unsigned int primitive, 
		const std::vector<float>& components, 
		const glm::vec4& color, 
		const std::array<int, 3>& pIDs, 
		const std::array<int, 3>& pCompIndex,
		int pCount
	)
		: m_name{ name }
		, m_type{ type }
		, m_primitive{ primitive }
		, m_components{ components }
		, m_color { color }
		, m_parentIDs { pIDs }
		, m_pCompIndex { pCompIndex }
		, m_parentCount { pCount }
	{
	}

	Object()
		: m_id{ -1 }
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
	int					      getParentCount() const { return m_parentCount; }
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

	bool isMutable() const		 { return m_isMutable; }
	bool isSelected() const		 { return m_isSelected; }

	void setMutable(bool status) { m_isMutable = status; }
	void setSelected(bool isSelected) { m_isSelected = isSelected; }

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
	int m_parentCount{}; // max parent count. for example: a vector can have up to two parents

	bool m_isMutable{ true };
	bool m_isSelected{ false };
};

#endif

