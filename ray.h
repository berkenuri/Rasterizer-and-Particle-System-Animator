#ifndef __RAY_H__
#define __RAY_H__

#include "GLInclude.h"

class Ray {

  public:

    glm::vec3 origin;
    glm::vec3 direction;
    float length = 1.0e30f;

    Ray();
    Ray(const Ray& other_Ray);
    Ray(glm::vec3 ori, glm::vec3 dir);
    ~Ray();

    glm::vec3 getOrigin();
    glm::vec3 getDirection();
    float getLength();
    glm::vec3 calculatePoint(float t);
    float getXComponent();


};
#endif
