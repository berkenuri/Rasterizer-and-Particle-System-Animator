#ifndef __RAY_CPP__
#define __RAY_CPP__

#include "ray.h"

Ray::Ray() {}

Ray::~Ray() {}

Ray::Ray(const Ray& other_Ray) {
  origin = other_Ray.origin;
  direction = other_Ray.direction;
  length = other_Ray.length;
}

Ray::Ray(glm::vec3 ori, glm::vec3 dir) {
  origin = ori;
  direction = dir;
}

glm::vec3 Ray::getOrigin() {
  return origin;
}

glm::vec3 Ray::getDirection() {
  return glm::normalize(direction);
}

float Ray::getLength() {
  return length;
}

glm::vec3 Ray::calculatePoint(float t) {
  return origin + direction * t;
}

float Ray::getXComponent() {
  return direction.x;
}

#endif
