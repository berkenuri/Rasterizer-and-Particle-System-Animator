#ifndef __SCENE_CPP__
#define __SCENE_CPP__

// STL
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <math.h>
#include <fstream>
#include <vector>

// GLM
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

#include "scene.h"

Scene::Scene() {}
Scene::~Scene() {}

void Scene::addObject(std::shared_ptr<Object> object) {
  objects.push_back(object);
}

void Scene::addLight(std::shared_ptr<Light> light) {
  lights.push_back(light);
}

void Scene::addGlobalAmbient(GlobalAmbient& glAmb) {
  globalAmbient = glAmb;
}


void Scene::addDirectionalLight(std::shared_ptr<DirectionalLight> light) {
  directionalLights.push_back(light);
}

void Scene::addPointLight(std::shared_ptr<PointLight> light) {
  pointLights.push_back(light);
}

void Scene::addSpotLight(std::shared_ptr<SpotLight> light) {
  spotLights.push_back(light);
}

void Scene::addCamera(Camera& cam) {
  camera = cam;
}

void Scene::rayTracer(std::unique_ptr<glm::vec4[]> & frame, int pixelX, int pixelY) {

  int x = -1;
  int y = -1;

  for (int i = 0; i < pixelX*pixelY; ++i) {

    x++;

    if (i % (pixelX) == 0) {
      y++;
      x = 0;
    }

    Ray ray = Ray();

    if (camera.getCameraView().compare("perspective") == 0) {
      ray = camera.makePerspectiveViewRay(x, y);
    } else {
      ray = camera.makeParallelViewRay(x, y);
    }

    frame[i] = trace(ray, 5);

  }
}

glm::vec4 Scene::trace(Ray& ray, int bounce_Count) {

glm::vec4 color(0, 0, 0, 0);
glm::vec4 reflectedColor(0, 0, 0, 0);

if(bounce_Count < 0) {
  return color;
}

  float t_min = 1.0e30f;
  int closest = -1;
  float shade = 1;

  for (int j = 0; j < objects.size(); j++) {

    if (objects[j]->intersected(ray)) {

      if (objects[j]->intersection(ray) < t_min) {
        t_min = objects[j]->intersection(ray);
        closest = j;
      }
    }
  }


  if (closest > -1) {

    for (int k = 0; k < lights.size(); k++) {

      Ray shadowRay = Ray(objects[closest]->getIntersectionCoordinate(ray),
      glm::normalize(lights[k]->getLightPosition() - objects[closest]->getIntersectionCoordinate(ray)));

      float length = glm::length(lights[k]->getLightPosition() - objects[closest]->getIntersectionCoordinate(ray));

      for (int j = 0; j < objects.size(); j++) {

        if (objects[j]->intersected(shadowRay) && j != closest &&
        objects[j]->intersection(shadowRay) < length) {
          shade *= 0.2;
        }

      }

      glm::vec3 reflectedRayDirection = ray.getDirection() -
      (2 * glm::dot(ray.getDirection(), objects[closest]->getNormal(ray))
      * objects[closest]->getNormal(ray));

      Ray reflected = Ray(objects[closest]->getIntersectionCoordinate(ray),
      reflectedRayDirection);

      if (objects[closest]->getMaterial().shininess > 1) {
        reflectedColor = trace(reflected, bounce_Count - 1) * 0.3;
      }

      color += lights[k]->colorShading(objects[closest]->getIntersectionCoordinate(ray),
      objects[closest]->getNormal(ray), objects[closest]->getColor(), objects[closest]->getMaterial())
      + reflectedColor;
    }

    return color * shade;

  } else {
    return glm::vec4(0, 0, 0, 0);
  }

}

#endif
