#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "GLInclude.h"

#include "ray.h"
#include "material.h"

class Light {

  public:

    glm::vec3 position;

    Light();
    Light(glm::vec3 pos);
    Light(const Light& other_Light);
    ~Light();

    glm::vec3 getLightPosition();
    glm::vec4 colorShading(glm::vec3 surfacePoint, glm::vec3 surfaceNormal, glm::vec4 surfaceColor, Material material);

};

class GlobalAmbient : public Light {

  public:
    glm::vec4 ambientIntensity;

    GlobalAmbient();
    ~GlobalAmbient();
    GlobalAmbient(glm::vec4 ambIntensity);

};

class DirectionalLight : public Light {

  public:
    glm::vec3 position;
    //glm::vec3 direction;
    //glm::vec4 ambientIntensity;
    glm::vec4 diffuseIntensity = glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
    glm::vec4 specularIntensity = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
    glm::vec4 color;

    DirectionalLight();
    ~DirectionalLight();
    DirectionalLight(glm::vec3 pos, glm::vec4 col);
};

class PointLight : public Light {

  public:
    glm::vec3 position;
    //glm::vec4 ambientIntensity;
    glm::vec4 diffuseIntensity = glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
    glm::vec4 specularIntensity = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
    float ac;
    float al;
    float aq;
    glm::vec4 color;

    PointLight();
    ~PointLight();
    PointLight(glm::vec3 pos, glm::vec4 col);

};

class SpotLight : public Light {

  public:
    glm::vec3 position;
    glm::vec3 direction;
    float cutOffAngle;
    float outerCutOffAngle;
    //glm::vec4 ambientIntensity;
    glm::vec4 diffuseIntensity = glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
    glm::vec4 specularIntensity = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
    float ac;
    float al;
    float aq;
    glm::vec4 color;

    SpotLight();
    ~SpotLight();
    SpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec4 col, float innerAngle, float outerAngle);

};

#endif
