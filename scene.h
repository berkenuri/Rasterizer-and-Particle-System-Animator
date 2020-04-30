#ifndef __SCENE_H__
#define __SCENE_H__

#include "GLInclude.h"

// STL
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <math.h>
#include <fstream>
#include <vector>

#include "object.h"
#include "camera.h"
#include "light.h"


class Scene {

  public:

    Camera camera = Camera();
    std::vector<std::shared_ptr<Object>> objects;
    std::vector<std::shared_ptr<Light>> lights;

    GlobalAmbient globalAmbient = GlobalAmbient();
    std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
    std::vector<std::shared_ptr<PointLight>> pointLights;
    std::vector<std::shared_ptr<SpotLight>> spotLights;

    glm::mat4 viewMatrix;

    std::vector<float> vertexPositions;
    std::vector<float> normalPositions;
    std::vector<float> texturePositions;

    bool fog = false;
    bool dissection = false;
    bool hasSky = false;

    Scene();
    ~Scene();

    void addObject(std::shared_ptr<Object> object);
    void addLight(std::shared_ptr<Light> light);
    void addGlobalAmbient(GlobalAmbient& glAmb);
    void addDirectionalLight(std::shared_ptr<DirectionalLight> light);
    void addPointLight(std::shared_ptr<PointLight> light);
    void addSpotLight(std::shared_ptr<SpotLight> light);
    void addCamera(Camera& cam);
    void rayTracer(std::unique_ptr<glm::vec4[]> & frame, int pixelX, int pixelY);
    glm::vec4 trace(Ray& ray, int bounce_Count);

};

#endif
