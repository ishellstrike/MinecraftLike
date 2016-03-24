﻿



#pragma once
#ifndef Vertex_h__
#define Vertex_h__

#include <glm/glm.hpp>
#include <vector>
#include "VertexTools.h"

// Конфигурация атрибута.
struct Attribute
{
  size_t size;
  size_t offset;
  const char *name;
};


// Сформирует структуру вершины и статическую информацию 
// об атрибуте для каждой компоненты вершины.
VERTEX
(
  (VertexVTN)
  (glm::vec3, vertex)
  (glm::vec2, texture)
  (glm::vec3, normal)
  (float, ao)
)

VERTEX
(
	(VertexVTC)
	(glm::vec4, vertex)
	(glm::vec2, texture)
	(glm::u8vec4, color)
)

#endif // Vertex_h__

