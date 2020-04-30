#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <math.h>
#include <fstream>


class Material {

  public:
    float shininess;
    glm::vec4 diffuse_coefficient;
    glm::vec4 specular_coefficient;
    glm::vec4 ambient_coefficient;

    std::string diffuseTexture;
    std::string specularTexture;
    std::string emissionTexture;
    std::string bumpTexture;
    std::string depthTexture;
    std::string displacementTexture;
    std::string skyboxTexture;
    
    bool hasDiffuseTexture = false;
    bool hasSpecularTexture = false;
    bool hasEmissionTexture = false;
    bool hasBumpTexture = false;
    bool hasDepthTexture = false;
    bool hasDisplacementTexture = false;

    Material(std::string material);
    Material(glm::vec4 ka, glm::vec4 kd, glm::vec4 ks, float p);
    Material();
    ~Material();

};

#endif
