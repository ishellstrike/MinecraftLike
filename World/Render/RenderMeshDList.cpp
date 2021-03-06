



#include "RenderMeshDList.h"

#include <assert.h>
#include <gl/glew.h>

#include <string>

RenderMeshDList::RenderMeshDList()
{
  for (auto &i : mEnabled)
  {
    i = false;
  }
}


RenderMeshDList::~RenderMeshDList()
{
  if (mCreated)
  {
    glDeleteLists(mList, 1);
  }
}

void RenderMeshDList::SetAttribute(const std::vector<Attribute> &attribute, const std::vector<int> &locations)
{
  mVertexSize = 0;
  for (const auto &attr : attribute)
  {
    if (std::string("vertex") == attr.name)
    {
      mAttribute[ATTRIBUTE_VERTEX] = attr;
      mEnabled[ATTRIBUTE_VERTEX] = true;
    }
    if (std::string("texture") == attr.name)
    {
      mAttribute[ATTRIBUTE_TEXTURE] = attr;
      mEnabled[ATTRIBUTE_TEXTURE] = true;
    }
    mVertexSize += attr.size;
  }
}

void RenderMeshDList::Send(const float *vertex, size_t vertexCount, const size_t *index, size_t indexCount)
{
  assert(vertex && "vertex pointer is null");
  assert(index && "index pointer is null");
  if (!mCreated)
  {
    mList = glGenLists(1);
    mCreated = true;
  }

  // ������� ������ ������ � ��������, ����� ������������ ���������� ������.
  // ��� ����� ����� ������� � ������������������ �������.

  glNewList(mList, GL_COMPILE);
  glBegin(GL_TRIANGLES);
  for (size_t i = 0; i < indexCount; ++i)
  {
    if (mEnabled[ATTRIBUTE_TEXTURE])
    {
      glTexCoord2fv(&vertex[(mVertexSize * index[i] + mAttribute[ATTRIBUTE_TEXTURE].offset) / sizeof(float)]);
    }
    if (mEnabled[ATTRIBUTE_VERTEX])
    {
      glVertex3fv(&vertex[(mVertexSize * index[i] + mAttribute[ATTRIBUTE_VERTEX].offset) / sizeof(float)]);
    }
  }
  glEnd();
  glEndList();
}

void RenderMeshDList::Draw() const
{
  if (mCreated)
  {
    glCallList(mList);
  }
}

