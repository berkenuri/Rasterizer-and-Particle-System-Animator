#ifndef __CAMERA_CPP__
#define __CAMERA_CPP__

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <cmath>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <math.h>
#include <fstream>

#include "camera.h"
#include "ray.h"

Camera::Camera() {}

Camera::Camera(std::string v, glm::vec3 pos, float focal_length, float theta_val, float width, float height) {

  view = v;
  position = pos;
  d = focal_length;
  theta = theta_val;
  w = width;
  h = height;
  aspectRatio = w/h;

  // verticalAngle = 0.0f;
  // horizontalAngle = M_PI;

  // // Direction : Spherical coordinates to Cartesian coordinates conversion
  // direction = glm::vec3(
  //   cos(verticalAngle) * sin(horizontalAngle),
  //   sin(verticalAngle),
  //   cos(verticalAngle) * cos(horizontalAngle)
  // );
  // // Right vector
  // right = glm::vec3(
  //   sin(horizontalAngle - M_PI/2.0f),
  //   0,
  //   cos(horizontalAngle - M_PI/2.0f)
  // );
  // // Up vector : perpendicular to both direction and right
  // up = glm::cross(right, direction);

  if (v.compare("perspective") == 0) {

    t = focal_length * tan(theta_val / 2);
    b = -t;
    r = t * aspectRatio;
    l = -r;

  } else {

    t = h / 2;
    b = -t;
    r = w /2;
    l = -r;
  }

}

Camera::~Camera() {}

glm::vec3 Camera::direction() {
  return glm::vec3(
  cos(verticalAngle) * sin(horizontalAngle),
  sin(verticalAngle),
  cos(verticalAngle) * cos(horizontalAngle));
}

glm::vec3 Camera::right() {
  return glm::vec3(
  sin(horizontalAngle - M_PI/2.0f),
  0,
  cos(horizontalAngle - M_PI/2.0f));
}

glm::vec3 Camera::up() {
  return glm::cross(this->right(), this->direction());
}

Ray Camera::makePerspectiveViewRay(float i, float j) {

  float sigma = this->b + ((this->t - this->b) * (j + 0.5) / this->h);
  float tau = this->l + ((this->r - this->l) * (i + 0.5) / this->w);

  glm::vec3 target = (-this->d * glm::vec3(0.0f, 0.0f, 1.0f)) +
  (tau * glm::vec3(1.0f, 0.0f, 0.0f)) + (sigma * glm::vec3(0.0f, 1.0f, 0.0f));

  dir = glm::normalize(target);

  return Ray(position, dir);

}

Ray Camera::makeParallelViewRay(float i, float j) {

  float sigma = this->b + ((this->t - this->b) * (j + 0.5) / this->h);
  float tau = this->l + ((this->r - this->l) * (i + 0.5) / this->w);

  glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 origin = position + (tau * glm::vec3(1.0f, 0.0f, 0.0f)) +
  (sigma * glm::vec3(0.0f, 1.0f, 0.0f));

  return Ray(origin, direction);
}

std::string Camera::getCameraView() {
  return view;
}

#endif
