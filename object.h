#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "GLInclude.h"

#include "ray.h"
#include "material.h"
#include "objParser.h"

class Object {

  public:

    glm::vec3 position;
    glm::vec4 color;
    Material material = Material();

    GLuint diffuseTextureID;
    GLuint specularTextureID;
    GLuint emissionTextureID;
    GLuint bumpTextureID;
    GLuint depthTextureID;
    GLuint displacementTextureID;
    GLuint skyboxTextureID;

    bool isLocalLightSource = false;
    bool isSkyBox = false;
    
    std::vector<std::string> faces;

    mesh meshes;
    float verticesCount = 0.0f;

    glm::vec3 scale;
    glm::vec3 translate;
    float rotationAroundX;
    float rotationAroundY;
    float rotationAroundZ;

    glm::mat4 modelMatrix;

    Object();
    Object(glm::vec3 pos);
    ~Object();

    virtual glm::vec3 getPosition();
    virtual glm::vec3 getNormal(Ray& Ray);
    virtual glm::vec4 getColor();
    virtual Material getMaterial();
    virtual bool intersected(Ray& Ray);
    virtual float intersection(Ray& Ray);
    virtual glm::vec3 getIntersectionCoordinate(Ray& Ray);
    virtual bool isPlane();

};

class Plane : public Object {

  public:

    glm::vec3 normal;

    Plane();
    Plane(const Plane& other_Plane);
    Plane(glm::vec3 pos, glm::vec3 n, glm::vec4 c, Material& m);
    ~Plane();

    glm::vec3 getPosition();
    glm::vec3 getNormal(Ray& Ray);
    glm::vec4 getColor();
    Material getMaterial();
    bool intersected(Ray& Ray);
    float intersection(Ray& Ray);
    glm::vec3 getIntersectionCoordinate(Ray& Ray);
    bool isPlane();

};

class Sphere : public Object {

  public:

    float radius;

    float numVertices;
    float numIndices;
    std::vector<int> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    void init(int);
    float toRadians(float degrees);

    Sphere();
    Sphere(const Sphere& other_Sphere);
    Sphere(glm::vec3 pos, float r, glm::vec4 c, Material& m);
    ~Sphere();

    Sphere(int prec);

    float getRadius();
    glm::vec3 getCenter();
    glm::vec4 getColor();
    Material getMaterial();
    bool intersected(Ray& Ray);
    float intersection(Ray& Ray);
    glm::vec3 getIntersectionCoordinate(Ray& Ray);
    glm::vec3 getNormal(Ray& Ray);
    bool isPlane();

};

#endif
