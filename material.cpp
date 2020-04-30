#ifndef __MATERIAL_CPP__
#define __MATERIAL_CPP__

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <math.h>
#include <fstream>

#include "material.h"

Material::Material(std::string material) {

  //texture = "";

  if (material.compare("shiny") == 0) {

    shininess = 9;
    diffuse_coefficient = glm::vec4(0.9f, 0.75f, 0.8f, 0);
    specular_coefficient = glm::vec4(0.9f, 0.8f, 0.7f, 0);


  } else {

    shininess = 1;
    diffuse_coefficient = glm::vec4(0.4f, 0.3f, 0.4f, 0);
    specular_coefficient = glm::vec4(0.2f, 0.2f, 0.1f, 0);

  }

}

Material::Material(glm::vec4 ka, glm::vec4 kd, glm::vec4 ks, float p/*, std::string t*/) {
  ambient_coefficient = ka;
  diffuse_coefficient = kd;
  specular_coefficient = ks;
  shininess = p;
  //texture = t;
}

Material::Material() {
  ambient_coefficient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  diffuse_coefficient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  specular_coefficient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  shininess = 0.0f;
  //texture = "";

}

Material::~Material() {}

#endif
