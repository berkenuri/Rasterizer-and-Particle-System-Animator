#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "GLInclude.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <math.h>
#include <fstream>

#include "ray.h"


class Camera {

public:

  std::string view;
  glm::vec3 position;
  float d;
  float theta;
  float l;
  float r;
  float t;
  float b;
  float aspectRatio;
  float w;
  float h;

  float verticalAngle = 0.0f;
  float horizontalAngle = M_PI;
  glm::vec3 dir;
  glm::mat4 projectionMatrix;

  glm::vec3 direction();
  glm::vec3 up();
  glm::vec3 right();

  Camera();
  Camera(std::string view, glm::vec3 pos, float focal_length, float theta_val, float width, float height);
  ~Camera();

  Ray makePerspectiveViewRay(float i, float j);
  Ray makeParallelViewRay(float i, float j);
  std::string getCameraView();


};

#endif
