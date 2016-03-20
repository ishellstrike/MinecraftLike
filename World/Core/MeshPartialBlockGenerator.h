



#pragma once
#include <vector>
#include "..\Render\TemplateMesh.h"
#include "..\Render\Vertex.h"
#include <rapidjson\document.h>

class MeshPartialBlockGenerator
{
public:
	using VertexType = VertexVTN;

	enum Side
	{
		EMPTY = 0,

		FRONT = 1 << 0,
		RIGHT = 1 << 1,
		BACK = 1 << 2,
		LEFT = 1 << 3,
		TOP = 1 << 4,
		BOTTOM = 1 << 5,

		ALL = 0x3F,
	};

public:
	MeshPartialBlockGenerator();

	PMesh<VertexType> Create(Side side, const std::array<char, 8> &neib);

	void SetTexture(int side, const std::string &texture);

	void JsonLoad(const rapidjson::Value & val);

private:
	// ������ ��������� ������� ��� ������ ���� � ������.
	std::vector<glm::vec4> mTextures;
};