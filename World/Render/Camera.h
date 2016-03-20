﻿



#pragma once
#ifndef Camera_h__
#define Camera_h__


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>
#include <glm/gtc/quaternion.hpp>
#include <tools\ray.h>

class Camera;
typedef std::shared_ptr<Camera> PCamera;

// Камера. Направление координатных осей:
// горизонтальная ось -x слева, +x справа;
// вертикальная ось -z вниз, +z вверх;
// ось глубины -y на себя, +y от себя.
class Camera
{
public:
  Camera();
  ~Camera();

  enum Type {
    ORTHO,
    SHADOW,
    PERSPECTIVE
  };

  const glm::mat4 & GetViewProject() const;

  const glm::mat4 & GetShadow() const;

  // Получить матрицу вида.
  const glm::mat4 &GetView() const;

  // Получить матрицу проекции.
  const glm::mat4 &GetProject() const;

  const glm::mat3 &GetDirection() const;

  void SetPos(const glm::vec3 &pos);

  void LookAt(const glm::vec3 & point);

  const glm::vec3 &GetPos() const;

  void Resize(const glm::uvec2 &size);

  // Повернуть камеру относительно текущей ориентации на заданный угол по трем осям.
  void Rotate(const glm::vec3 &angle);

  void SetRot(const glm::quat &quat);

  // Переместиться, относительно текущего положения и ориентации.
  void Move(const glm::vec3 &dist);

  void Update();

  glm::ray GetRay(const glm::vec2 &pos);

  void CalculateFrustum();

  bool BoxWithinFrustum(const glm::vec4 & min, const glm::vec4 & max) const;

  bool BoxWithinFrustum(const glm::vec3 & min, const glm::vec3 & max) const;

  void SetPerspective();
  void SetOrtho();
  void RebuildProjection();
  void SetShadow();
  Type type = PERSPECTIVE;

private:
  glm::quat mQuat;
  
  glm::mat4 mView;
  glm::mat4 mProjection;
  glm::mat4 mViewProjection;
  glm::mat4 mShadow;
  glm::mat3 mDirection;

  glm::vec3 mDir;
  glm::vec3 mPos;

  float m_clipMatrix[16];
  float m_frustum[6][4];

  float mFov = 45.0f;
  float mAspect = 1.0f;
  float mNear = 0.01f;
  float mFar = 2000.f;
  glm::vec2 mSize = {600, 600};

  bool changed = true;
};

#endif // Camera_h__
