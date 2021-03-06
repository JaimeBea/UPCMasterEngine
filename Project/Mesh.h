#pragma once

#include "Math/float4x4.h"
#include <vector>

struct aiMesh;

class Mesh
{
public:
	void Load(const aiMesh* mesh);
	void Release();

	void Draw(const std::vector<unsigned>& materials, const float4x4& model_matrix) const;

public:
	unsigned vbo = 0;
	unsigned ebo = 0;
	unsigned vao = 0;
	unsigned num_vertices = 0;
	unsigned num_indices = 0;
	unsigned material_index = 0;
};

