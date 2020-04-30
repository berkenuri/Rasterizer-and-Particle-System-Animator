////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Contains main function to create a window and run engine that
///        repeatedly generates a framebuffer and displays it.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
#include <SOIL2/SOIL2.h>

#include "CompileShaders.h"
#include "objParser.h"
#include "ray.h"
#include "object.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "scene.h"

#include "particlesystem.h"
#include "random.h"

// STL
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <cmath>
#include <array>
#include <math.h>
#include <vector>

#include "GLInclude.h"

Scene scene = Scene();
Scene sky = Scene();

State particle;
std::vector<std::shared_ptr<ParticleSystem>> pSystems;
int iterator = -1;
float startFrame = 0.0f;
bool animation = false;


float speed = 1; // units / second

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Global variables - avoid these

// Window
int g_width{1360};
int g_height{768};
int g_window{0};
GLuint g_program{0};
GLuint skybox_program{0};
//GLuint animation_program{0};
GLuint g_vao{0}; ///< Vertex Array Object
GLuint g_vbo[3]; ///< Vertex Buffer Object
GLuint s_vao{0};
GLuint s_vbo[3];
std::unique_ptr<glm::vec4[]> g_frame{nullptr}; ///< Framebuffer

// Frame rate
const unsigned int FPS = 60;
float g_frameRate{0.f};
std::chrono::high_resolution_clock::time_point g_frameTime{
    std::chrono::high_resolution_clock::now()};
float g_delay{0.f};
float g_framesPerSecond{0.f};

////////////////////////////////////////////////////////////////////////////////
// Functions

void
setupMaterialProperties(const std::string& _filename, std::shared_ptr<Object> object) {

  std::ifstream ifs;
  ifs.open(_filename);

  if (!ifs) {
        std::cout << "Unable to open the material file";
  }

  std::string line;

  while(ifs) {

    getline(ifs, line);
    std::istringstream iss(line);

    std::string tag;
    iss >> tag;

    if (tag.compare("Ka") == 0) {

      iss >> object->material.ambient_coefficient[0] >>
      object->material.ambient_coefficient[1] >>
      object->material.ambient_coefficient[2];

    } else if (tag.compare("Kd") == 0) {

      iss >> object->material.diffuse_coefficient[0] >>
      object->material.diffuse_coefficient[1] >>
      object->material.diffuse_coefficient[2];

    } else if (tag.compare("Ks") == 0) {

      iss >> object->material.specular_coefficient[0] >>
      object->material.specular_coefficient[1] >>
      object->material.specular_coefficient[2];

    } else if (tag.compare("Ns") == 0) {

      iss >> object->material.shininess;

    } else if (tag.compare("map_Kd") == 0) {

      std::string textureFile;

      iss >> textureFile;

      object->material.diffuseTexture = "Objects/" + textureFile;
      object->material.hasDiffuseTexture = true;

    } else if (tag.compare("map_Ks") == 0) {

      std::string textureFile;

      iss >> textureFile;

      object->material.specularTexture = "Objects/" + textureFile;
      object->material.hasSpecularTexture = true;

    } else if (tag.compare("map_Ke") == 0) {

      std::string textureFile;

      iss >> textureFile;

      object->material.emissionTexture = "Objects/" + textureFile;
      object->material.hasEmissionTexture = true;

    } else if (tag.compare("map_Bump") == 0) {

      std::string textureFile;

      iss >> textureFile;

      object->material.bumpTexture = "Objects/" + textureFile;
      object->material.hasBumpTexture = true;

    } else if (tag.compare("map_Depth") == 0) {

      std::string textureFile;

      iss >> textureFile;

      object->material.depthTexture = "Objects/" + textureFile;
      object->material.hasDepthTexture = true;

    } else if (tag.compare("map_Disp") == 0) {

      std::string textureFile;

      iss >> textureFile;

      object->material.displacementTexture = "Objects/" + textureFile;
      object->material.hasDisplacementTexture = true;

    } else {}

  }

  ifs.close();
}

GLuint loadTexture(const char *texImagePath) {
  GLuint textureID;

  textureID = SOIL_load_OGL_texture(texImagePath,
    SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

  if (textureID == 0) {
    std::cout << "could not find texture file" << texImagePath << std::endl;
  }

  glBindTexture(GL_TEXTURE_2D, textureID);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#if defined(OSX)
  if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
    GLfloat anisoSetting = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoSetting);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoSetting);
  }
#endif

  return textureID;
}

GLuint loadCubeMap(const char *mapDir) {
  GLuint textureRef;

  std::string xp = mapDir; xp = xp + "right.jpg";
  std::string xn = mapDir; xn = xn + "left.jpg";
  std::string yp = mapDir; yp = yp + "top.jpg";
  std::string yn = mapDir; yn = yn + "bottom.jpg";
  std::string zp = mapDir; zp = zp + "front.jpg";
  std::string zn = mapDir; zn = zn + "back.jpg";

  textureRef = SOIL_load_OGL_cubemap(xp.c_str(), xn.c_str(), yp.c_str(), yn.c_str(),
    zp.c_str(), zn.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

  if (textureRef == 0) {
    std::cout << SOIL_last_result() << std::endl;
  }

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureRef);
    // reduce seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureRef;
}

void setupVertices(mesh& mesh, std::shared_ptr<Object> object) {

  object->verticesCount = mesh.m_vertices.size();

  if (object->isSkyBox) {

    for (int i = 0; i < mesh.m_vertices.size(); i++) {

      sky.vertexPositions.emplace_back(mesh.m_vertices[i].m_p.x);
      sky.vertexPositions.emplace_back(mesh.m_vertices[i].m_p.y);
      sky.vertexPositions.emplace_back(mesh.m_vertices[i].m_p.z);

      sky.normalPositions.emplace_back(mesh.m_vertices[i].m_n.x);
      sky.normalPositions.emplace_back(mesh.m_vertices[i].m_n.y);
      sky.normalPositions.emplace_back(mesh.m_vertices[i].m_n.z);

      sky.texturePositions.emplace_back(mesh.m_vertices[i].m_t.x);
      sky.texturePositions.emplace_back(mesh.m_vertices[i].m_t.y);
    }

  } else {

    for (int i = 0; i < mesh.m_vertices.size(); i++) {

      scene.vertexPositions.emplace_back(mesh.m_vertices[i].m_p.x);
      scene.vertexPositions.emplace_back(mesh.m_vertices[i].m_p.y);
      scene.vertexPositions.emplace_back(mesh.m_vertices[i].m_p.z);

      scene.normalPositions.emplace_back(mesh.m_vertices[i].m_n.x);
      scene.normalPositions.emplace_back(mesh.m_vertices[i].m_n.y);
      scene.normalPositions.emplace_back(mesh.m_vertices[i].m_n.z);

      scene.texturePositions.emplace_back(mesh.m_vertices[i].m_t.x);
      scene.texturePositions.emplace_back(mesh.m_vertices[i].m_t.y);
    }
  }

}

void setupSphereVertices(std::shared_ptr<Sphere> sphere) {

  std::vector<int> ind = sphere->indices;
  std::vector<glm::vec3> vert = sphere->vertices;
  std::vector<glm::vec2> tex = sphere->texCoords;
  std::vector<glm::vec3> norm = sphere->normals;

  std::cout << "vertex: " << glm::to_string(sphere->vertices[100]) << std::endl;
  std::cout << "index: " << sphere->indices[100] << std::endl;
  std::cout << "Vertex Position: " << glm::to_string(vert[ind[100]]) << std::endl;

  // vertex positions
  // texture coordinates // normal vectors
  int numIndices = sphere->numIndices;


  for (int i = 0; i < numIndices; i++) {

    scene.vertexPositions.emplace_back(vert[ind[i]].x);
    scene.vertexPositions.emplace_back(vert[ind[i]].y);
    scene.vertexPositions.emplace_back(vert[ind[i]].z);

    scene.normalPositions.emplace_back(norm[ind[i]].x);
    scene.normalPositions.emplace_back(norm[ind[i]].y);
    scene.normalPositions.emplace_back(norm[ind[i]].z);

    scene.texturePositions.emplace_back(tex[ind[i]].x);
    scene.texturePositions.emplace_back(tex[ind[i]].y);
  }
}

void installMaterials(std::shared_ptr<Object> object) {

  glProgramUniform4fv(g_program, glGetUniformLocation(g_program, "material.ambient"), 1,
    glm::value_ptr(object->material.ambient_coefficient));

  glUniform1i(glGetUniformLocation(g_program, "material.diffuse"), 0);
  glUniform1i(glGetUniformLocation(g_program, "material.specular"), 1);
  glUniform1i(glGetUniformLocation(g_program, "material.emission"), 2);
  glUniform1i(glGetUniformLocation(g_program, "material.bump"), 3);
  glUniform1i(glGetUniformLocation(g_program, "material.depth"), 4);
  glUniform1i(glGetUniformLocation(g_program, "material.displacement"), 5);

  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "material.shininess"),
  object->material.shininess);

  glUniform1i(glGetUniformLocation(g_program, "material.hasDiffuse"), object->material.hasDiffuseTexture);
  glUniform1i(glGetUniformLocation(g_program, "material.hasSpecular"), object->material.hasSpecularTexture);
  glUniform1i(glGetUniformLocation(g_program, "material.hasEmission"), object->material.hasEmissionTexture);
  glUniform1i(glGetUniformLocation(g_program, "material.hasBump"), object->material.hasBumpTexture);
  glUniform1i(glGetUniformLocation(g_program, "material.hasDepth"), object->material.hasDepthTexture);
  glUniform1i(glGetUniformLocation(g_program, "material.hasDisplacement"), object->material.hasDisplacementTexture);
  glUniform1i(glGetUniformLocation(g_program, "object.isLocalLightSource"), object->isLocalLightSource);
  glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
    "lightSource_color"), 1, glm::value_ptr(object->color));
}

void installDirectionalLights(glm::mat4 vMatrix,
  std::shared_ptr<DirectionalLight> light, int i) {

    glProgramUniform3fv(g_program, glGetUniformLocation(g_program,
      ("dirLights[" + std::to_string(i) + "].position").c_str()), 1,
      glm::value_ptr(glm::vec3(vMatrix * glm::vec4(light->position, 1.0))));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("dirLights[" + std::to_string(i) + "].diffuseIntensity").c_str()), 1,
      glm::value_ptr(light->diffuseIntensity));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("dirLights[" + std::to_string(i) + "].specularIntensity").c_str()), 1,
      glm::value_ptr(light->specularIntensity));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("dirLights[" + std::to_string(i) + "].color").c_str()), 1,
      glm::value_ptr(light->color));

}

void installPointLights(glm::mat4 vMatrix,
  std::shared_ptr<PointLight> light, int i) {

  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "pointAc"), light->ac);
  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "pointAl"), light->al);
  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "pointAq"), light->aq);

    glProgramUniform3fv(g_program, glGetUniformLocation(g_program,
      ("pointLights[" + std::to_string(i) + "].position").c_str()), 1,
      glm::value_ptr(glm::vec3(vMatrix * glm::vec4(light->position, 1.0))));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("pointLights[" + std::to_string(i) + "].diffuseIntensity").c_str()), 1,
      glm::value_ptr(light->diffuseIntensity));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("pointLights[" + std::to_string(i) + "].specularIntensity").c_str()), 1,
      glm::value_ptr(light->specularIntensity));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("pointLights[" + std::to_string(i) + "].color").c_str()), 1,
      glm::value_ptr(light->color));

}

void installSpotLights(glm::mat4 vMatrix,
  std::shared_ptr<SpotLight> light, int i) {

  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "spotAc"), light->ac);
  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "spotAl"), light->al);
  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "spotAq"), light->aq);

    glProgramUniform3fv(g_program, glGetUniformLocation(g_program,
      ("spotLights[" + std::to_string(i) + "].position").c_str()), 1,
      glm::value_ptr(glm::vec3(vMatrix * glm::vec4(light->position, 1.0))));

    glProgramUniform3fv(g_program, glGetUniformLocation(g_program,
      ("spotLights[" + std::to_string(i) + "].direction").c_str()), 1,
      glm::value_ptr(glm::vec3(vMatrix * glm::vec4(glm::normalize(light->direction), 0.0))));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("spotLights[" + std::to_string(i) + "].diffuseIntensity").c_str()), 1,
      glm::value_ptr(light->diffuseIntensity));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("spotLights[" + std::to_string(i) + "].specularIntensity").c_str()), 1,
      glm::value_ptr(light->specularIntensity));

    glProgramUniform4fv(g_program, glGetUniformLocation(g_program,
      ("spotLights[" + std::to_string(i) + "].color").c_str()), 1,
      glm::value_ptr(light->color));

    glProgramUniform1f(g_program, glGetUniformLocation(g_program,
      ("spotLights[" + std::to_string(i) + "].cutOffAngle").c_str()),
      glm::cos(light->cutOffAngle));

    glProgramUniform1f(g_program, glGetUniformLocation(g_program,
      ("spotLights[" + std::to_string(i) + "].outerCutOffAngle").c_str()),
      glm::cos(light->outerCutOffAngle));

}

////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize GL settings
void initializeGLFW(GLFWwindow *window, const std::string& _filename)
{
  glClearColor(0.1f, 0.0f, 0.2f, 0.f);
  //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);

  std::string line;
  std::ifstream ifs;
  ifs.open(_filename);

  while(ifs) {
    getline(ifs, line);
    std::istringstream iss(line);

    std::string tag;
    iss >> tag;

    if (tag.compare("Camera:") == 0) {

      std::string cameraView;
      float focal_length;
      glm::vec3 position;

      iss >> cameraView >> position.x >> position.y
      >> position.z >> focal_length;

      Camera camera = Camera(cameraView, position, focal_length, M_PI/3, g_width, g_height);

      if (cameraView.compare("perspective") == 0) {
        camera.projectionMatrix = glm::perspective(1.0472f, camera.aspectRatio, 0.1f, 1000.0f);
      } else {
        camera.projectionMatrix = glm::ortho(camera.l, camera.r, camera.b, camera.t,
        0.1f, 1000.0f);
      }

      scene.addCamera(camera);

    } else if (tag.compare("Global_Ambient:") == 0) {

      glm::vec4 ambient;

      iss >> ambient[0] >> ambient[1] >> ambient[2] >>
      ambient[3];

      GlobalAmbient globAmb = GlobalAmbient(ambient);

      scene.addGlobalAmbient(globAmb);

    } else if(tag.compare("Directional_Light:") == 0) {

      glm::vec3 pos;
      glm::vec4 col;

      iss >> pos.x >> pos.y >> pos.z >>
      col[0] >> col[1] >> col[2] >> col[3];

      std::shared_ptr<DirectionalLight> ptr_light(new DirectionalLight(pos, col));
      scene.addDirectionalLight(ptr_light);

    } else if(tag.compare("Point_Light:") == 0) {

      std::string transformationType;
      std::string fileName;
      glm::vec3 pos;
      glm::vec4 col;
      float ac, al, aq;

      iss >> fileName >> pos.x >> pos.y >> pos.z >>
      col[0] >> col[1] >> col[2] >> col[3] >>
      ac >> al >> aq >> transformationType;

      std::shared_ptr<Object> ptr_object(new Object(pos));
      ptr_object->color = col;
      ptr_object->isLocalLightSource = true;
      std::shared_ptr<PointLight> ptr_light(new PointLight(pos, col));
      ptr_light->ac = ac;
      ptr_light->al = al;
      ptr_light->aq = aq;

      if (transformationType.compare("sc:") == 0) {
        iss >> ptr_object->scale[0] >> ptr_object->scale[1] >> ptr_object->scale[2];
      }

      ptr_object->meshes = objParser("Objects/" + fileName);
      g_program = compileProgram("Shaders/experimental.vert",
                            "Shaders/experimental.frag",
                            "Shaders/experimental.geom");
      setupVertices(ptr_object->meshes, ptr_object);

      scene.addObject(ptr_object);
      scene.addPointLight(ptr_light);

    } else if(tag.compare("Spot_Light:") == 0) {

      std::string transformationType;
      std::string fileName;
      glm::vec3 pos;
      glm::vec3 dir;
      glm::vec4 col;
      float innerAngle;
      float outerAngle;
      float ac, al, aq;

      iss >> fileName >> pos.x >> pos.y >> pos.z >>
      dir.x >> dir.y >> dir.z >>
      col[0] >> col[1] >> col[2] >> col[3] >>
      ac >> al >> aq >>
      innerAngle >> outerAngle >> transformationType;

      std::shared_ptr<Object> ptr_object(new Object(pos));
      ptr_object->color = col;
      ptr_object->isLocalLightSource = true;
      std::shared_ptr<SpotLight> ptr_light(new SpotLight(pos, dir, col, innerAngle, outerAngle));
      ptr_light->ac = ac;
      ptr_light->al = al;
      ptr_light->aq = aq;

      if (transformationType.compare("sc:") == 0) {
        iss >> ptr_object->scale[0] >> ptr_object->scale[1] >> ptr_object->scale[2];
      }

      ptr_object->meshes = objParser("Objects/" + fileName);
      g_program = compileProgram("Shaders/experimental.vert",
                            "Shaders/experimental.frag",
                            "Shaders/experimental.geom");
      setupVertices(ptr_object->meshes, ptr_object);

      scene.addObject(ptr_object);
      scene.addSpotLight(ptr_light);

    } else if (tag.compare("Object:") == 0) {

      std::shared_ptr<Object> ptr_object(new Object());
      std::string transformationType;
      std::string fileName;

      iss >> fileName >> ptr_object->position.x >> ptr_object->position.y
       >> ptr_object->position.z >> ptr_object->color[0]
       >> ptr_object->color[1] >> ptr_object->color[2]
       >> ptr_object->color[3] >> transformationType;

      if (transformationType.compare("trs:") == 0) {
        iss >> ptr_object->translate[0] >> ptr_object->translate[1] >> ptr_object->translate[2];
      }

      iss >> transformationType;

      if (transformationType.compare("sc:") == 0) {
        iss >> ptr_object->scale[0] >> ptr_object->scale[1] >> ptr_object->scale[2];
      }

      iss >> transformationType;

      if (transformationType.compare("rot:") == 0) {
        iss >> ptr_object->rotationAroundX >> ptr_object->rotationAroundY >>
        ptr_object->rotationAroundZ;
      }

      ptr_object->meshes = objParser("Objects/" + fileName);

      setupVertices(ptr_object->meshes, ptr_object);
      setupMaterialProperties("Objects/" + ptr_object->meshes.mtlFile, ptr_object);

      if (ptr_object->material.hasDisplacementTexture) {
          g_program = compileProgram("Shaders/experimental.vert",
                                "Shaders/experimental.frag",
                                "Shaders/experimental.geom");
      } else {
        g_program = compileProgram("Shaders/experimental.vert",
                              "Shaders/experimental.frag",
                              "Shaders/experimental.geom");
      }

      scene.addObject(ptr_object);

    } else if (tag.compare("Sky:") == 0) {

      sky.hasSky = true;

      std::shared_ptr<Object> ptr_object(new Object(glm::vec3(0.0f)));
      std::string fileName;
      std::string textureFile;

      iss >> fileName >> textureFile;

      ptr_object->material.skyboxTexture = textureFile;
      ptr_object->isSkyBox = true;
      ptr_object->meshes = objParser("Objects/" + fileName);

      setupVertices(ptr_object->meshes, ptr_object);

      const char *temp = &ptr_object->material.skyboxTexture[0];
      ptr_object->skyboxTextureID = loadTexture(temp);

      skybox_program = compileProgram("Shaders/skybox.vert",
                            "Shaders/skybox.frag");

      sky.addObject(ptr_object);


    } else if (tag.compare("Fog:") == 0) {

      std::string fog;

      iss >> fog;

      if (fog.compare("enable") == 0) {
        scene.fog = true;
        sky.hasSky = false;
      }

    } else if (tag.compare("Dissection:") == 0) {

      std::string dissect;

      iss >> dissect;

      if (dissect.compare("enable") == 0) {
        scene.dissection = true;
      }

    } else if (tag.compare("ANIMATION:") == 0) {

      animation = true;

    	glEnable(GL_BLEND);
    	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    } else if (tag.compare("ParticleProperties:") == 0) {

      iss >> particle.ColorBegin[0] >> particle.ColorBegin[1] >> particle.ColorBegin[2]
      >> particle.ColorBegin[3] >> particle.ColorEnd[0] >> particle.ColorEnd[1]
      >> particle.ColorEnd[2] >> particle.ColorEnd[3] >> particle.SizeBegin
      >> particle.SizeVariation >> particle.SizeEnd >> particle.LifeTime
      >> particle.Velocity[0] >> particle.Velocity[1] >> particle.Velocity[2];

    } else if (tag.compare("PointGenerator:") == 0) {

      std::shared_ptr<ParticleSystem> particleSystem(new ParticleSystem());
      pSystems.push_back(particleSystem);
      iterator++;

      particleSystem->hasPointGenerator = true;

      iss >> particleSystem->pGen.position[0] >> particleSystem->pGen.position[1]
      >> particleSystem->pGen.position[2];

    } else if (tag.compare("DirectedGenerator:") == 0) {

      std::shared_ptr<ParticleSystem> particleSystem(new ParticleSystem());
      pSystems.push_back(particleSystem);
      iterator++;

      particleSystem->hasDirectedGenerator = true;

      iss >> particleSystem->dirGen.position[0] >> particleSystem->dirGen.position[1]
      >> particleSystem->dirGen.position[2] >> particleSystem->dirGen.direction[0]
      >> particleSystem->dirGen.direction[1] >> particleSystem->dirGen.direction[2];

    } else if (tag.compare("DiscGenerator:") == 0) {

      std::shared_ptr<ParticleSystem> particleSystem(new ParticleSystem());
      pSystems.push_back(particleSystem);
      iterator++;

      particleSystem->hasDiscGenerator = true;

      iss >> particleSystem->discGen.center[0] >> particleSystem->discGen.center[1]
      >> particleSystem->discGen.center[2] >> particleSystem->discGen.radius
      >> particleSystem->discGen.normal[0] >> particleSystem->discGen.normal[1]
      >> particleSystem->discGen.normal[2];

    } else if (tag.compare("Rotator:") == 0) {

      struct ParticleSystem::Rotator r;

      iss >> r.position[0] >> r.position[1]
      >> r.position[2];

      pSystems[iterator]->rotatorSet.push_back(r);

    } else if (tag.compare("Attractor:") == 0) {

      struct ParticleSystem::Attractor a;

      iss >> a.position[0] >> a.position[1]
      >> a.position[2] >> a.mass;

      pSystems[iterator]->attractorSet.push_back(a);

    } else if (tag.compare("Repulsor:") == 0) {

      struct ParticleSystem::Repulsor r;

      iss >> r.position[0] >> r.position[1]
      >> r.position[2] >> r.mass;

      pSystems[iterator]->repulsorSet.push_back(r);

    } else if (tag.compare("Wind:") == 0) {

      struct ParticleSystem::Wind w;

      iss >> w.magnitude[0] >> w.magnitude[1]
      >> w.magnitude[2];

      pSystems[iterator]->windSet.push_back(w);

    } else if (tag.compare("Gravity:") == 0) {

      std::string g;

      iss >> g;

      if (g.compare("On") == 0)
      {
          pSystems[iterator]->hasGravity = true;
      }

    } else {}

  }

  // std::shared_ptr<Sphere> ptr_sphere(new Sphere(48));
  // ptr_sphere->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  // ptr_sphere->position = glm::vec3(0.0f, 10.0f, -20.0f);
  // ptr_sphere->translate = glm::vec3(0.0f, 0.0f, 0.0f);
  // ptr_sphere->scale = glm::vec3(10.0f, 10.0f, 10.0f);
  // ptr_sphere->rotationAroundX = 0.0f;
  // ptr_sphere->rotationAroundY = 0.0f;
  // ptr_sphere->rotationAroundZ = 0.0f;
  // g_program = compileProgram("Shaders/passthrough.vert",
  //                          "Shaders/passthrough.frag");
  // setupSphereVertices(ptr_sphere);
  // scene.addObject(ptr_sphere);

  glUniform1i(glGetUniformLocation(g_program, "scene.fog"), scene.fog);
  glUniform1i(glGetUniformLocation(g_program, "scene.dissection"), scene.dissection);

  for(std::shared_ptr<Object> object : scene.objects) {

    glm::mat4 t = glm::translate(glm::mat4(1.0f), object->position);

    t = glm::translate(t, object->translate);

    glm::mat4 s = glm::scale(glm::mat4(1.0f), object->scale);

    glm::mat4 rx = glm::rotate(glm::radians(object->rotationAroundX), glm::vec3(1,0,0));
    glm::mat4 ry = glm::rotate(glm::radians(object->rotationAroundY), glm::vec3(0,1,0));
    glm::mat4 rz = glm::rotate(glm::radians(object->rotationAroundZ), glm::vec3(0,0,1));

    object->modelMatrix = t * s * rx * ry * rz;

    if (object->material.hasDiffuseTexture) {
      const char *temp = &object->material.diffuseTexture[0];
      object->diffuseTextureID = loadTexture(temp);
    }

    if (object->material.hasSpecularTexture) {
      const char *temp2 = &object->material.specularTexture[0];
      object->specularTextureID = loadTexture(temp2);
    }

    if (object->material.hasEmissionTexture) {
      const char *temp3 = &object->material.emissionTexture[0];
      object->emissionTextureID = loadTexture(temp3);
    }

    if (object->material.hasBumpTexture) {
      const char *temp4 = &object->material.bumpTexture[0];
      object->bumpTextureID = loadTexture(temp4);
    }

    if (object->material.hasDepthTexture) {
      const char *temp5 = &object->material.depthTexture[0];
      object->depthTextureID = loadTexture(temp5);
    }

    if (object->material.hasDisplacementTexture) {
      const char *temp6 = &object->material.displacementTexture[0];
      object->displacementTextureID = loadTexture(temp6);
    }

  }


  //skyboxTexture = loadTexture("Objects/left.jpg");


  // Generate vertex array
  glGenVertexArrays(1, &g_vao);
  glBindVertexArray(g_vao);

  // Generate/specify vertex buffer
  glGenBuffers(3, g_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, scene.vertexPositions.size() * sizeof(float),
  &scene.vertexPositions[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, g_vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, scene.normalPositions.size() * sizeof(float),
  &scene.normalPositions[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, g_vbo[2]);
  glBufferData(GL_ARRAY_BUFFER, scene.texturePositions.size() * sizeof(float),
  &scene.texturePositions[0], GL_STATIC_DRAW);

  if (sky.hasSky) {
    // Generate vertex array
    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);

    // Generate/specify vertex buffer
    glGenBuffers(3, s_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sky.vertexPositions.size() * sizeof(float),
    &sky.vertexPositions[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, s_vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sky.normalPositions.size() * sizeof(float),
    &sky.normalPositions[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, s_vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sky.texturePositions.size() * sizeof(float),
    &sky.texturePositions[0], GL_STATIC_DRAW);
  }



}

////////////////////////////////////////////////////////////////////////////////
/// @brief Callback for resize of window
///
/// Responsible for setting window size (viewport) and projection matrix.
void resizeGLFW(GLFWwindow *window, int _w, int _h)
{
  g_width = _w;
  g_height = _h;

  // Viewport
  glfwGetFramebufferSize(window, &g_width, &g_height);
  glViewport(0, 0, g_width, g_height);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Timer function to fix framerate in a GLUT application
/// @param _v Value (not used here)
///
/// Note, this is rudametary and fragile.
void timerGLFW(int _v)
{
  if (g_window != 0)
  {
    // glutPostRedisplay();

    g_delay = std::max(0.f, 1.f / FPS - g_frameRate);
    // glutTimerFunc((unsigned int)(1000.f * g_delay), timer, 0);
  }
  else
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Draw function for single frame
void drawGLFW(GLFWwindow *window, double currentFrame)
{
  using namespace std::chrono;
  //////////////////////////////////////////////////////////////////////////////
  // Clear
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(g_program);

  float deltaTime = (currentFrame - startFrame);
  startFrame = currentFrame;

  if (glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS){
    scene.camera.verticalAngle += 0.01;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
    scene.camera.verticalAngle -= 0.01;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS){
    scene.camera.horizontalAngle += 0.01;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
    scene.camera.horizontalAngle -= 0.01;
  }

  // Move forward
  if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS){
      scene.camera.position += scene.camera.direction() * speed;
  }
  // Move backward
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
      scene.camera.position -= scene.camera.direction() * speed;
  }
  // Strafe right
  if (glfwGetKey(window, GLFW_KEY_D ) == GLFW_PRESS){
      scene.camera.position += scene.camera.right() * speed;
  }
  // Strafe left
  if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS){
      scene.camera.position -= scene.camera.right() * speed;
  }

  // build view matrix, model matrix, and model-view matrix
  scene.viewMatrix = glm::lookAt(
    scene.camera.position,           // Camera is here
    scene.camera.position + scene.camera.direction(), // and looks here : at the same position, plus "direction"
    scene.camera.up()                  // Head is up (set to 0,-1,0 to look upside-down)
  );

  glProgramUniform3fv(g_program, glGetUniformLocation(g_program, "cameraPosition"), 1,
    glm::value_ptr(glm::vec3(scene.viewMatrix * glm::vec4(scene.camera.position, 1.0f))));

  glUniform4fv(glGetUniformLocation(g_program, "ambientIntensity"), 1,
    glm::value_ptr(scene.globalAmbient.ambientIntensity));

  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "dirCount"),
  scene.directionalLights.size());

  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "pointCount"),
  scene.pointLights.size());

  glProgramUniform1f(g_program, glGetUniformLocation(g_program, "spotCount"),
  scene.spotLights.size());

  int index = 0.0f;
  for (std::shared_ptr<DirectionalLight> light : scene.directionalLights) {
    installDirectionalLights(scene.viewMatrix, light, index);
    index++;
  }

  index = 0.0f;
  for (std::shared_ptr<PointLight> light : scene.pointLights) {
    installPointLights(scene.viewMatrix, light, index);
    index++;
  }

  index = 0.0f;
  for (std::shared_ptr<SpotLight> light : scene.spotLights) {
    installSpotLights(scene.viewMatrix, light, index);
    index++;
  }

  if (scene.dissection) {
    glProgramUniform1f(g_program, glGetUniformLocation(g_program, "time"),
    glfwGetTime());
  }

  float offset = 0.0f;

  for(std::shared_ptr<Object> object : scene.objects) {

    glUniform4fv(glGetUniformLocation(g_program, "obj_color"), 1, glm::value_ptr(object->color));

    glm::mat4 mvMat = scene.viewMatrix * object->modelMatrix;

    // build the inverse-transpose of the MV matrix, for transforming normal vectors
    glm::mat4 invTrMat = glm::transpose(glm::inverse(mvMat));

    installMaterials(object);

    // copy perspective and MV matrices to corresponding uniform variables
    // put the MV, PROJ, and Inverse-transpose(normal) matrices into the corresponding uniforms
    glUniformMatrix4fv(glGetUniformLocation(g_program, "mv_matrix"),
    1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "proj_matrix"),
    1, GL_FALSE, glm::value_ptr(scene.camera.projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "norm_matrix"),
    1, GL_FALSE, glm::value_ptr(invTrMat));

    // associate VBO with the corresponding vertex attribute in the vertex shader
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, g_vbo[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, g_vbo[2]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, object->diffuseTextureID);

    // bind specular map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, object->specularTextureID);

    // bind emission map
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, object->emissionTextureID);

    // bind bump map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, object->bumpTextureID);

    // bind depth map
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, object->depthTextureID);

    // bind depth map
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, object->displacementTextureID);

    // Draw
    //glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, offset, object->verticesCount);

    offset += object->verticesCount;

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo[0]);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo[1]);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo[2]);
  }

  if (sky.hasSky) {

    glUseProgram(skybox_program);

    glUniform1i(glGetUniformLocation(skybox_program, "skybox"), 0);
    glm::mat4 view = glm::mat4(glm::mat3(scene.viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(skybox_program, "view"),
    1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skybox_program, "projection"),
    1, GL_FALSE, glm::value_ptr(scene.camera.projectionMatrix));

    for(std::shared_ptr<Object> object : sky.objects) {

      glBindBuffer(GL_ARRAY_BUFFER, s_vbo[0]);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER, s_vbo[1]);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER, s_vbo[2]);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, object->skyboxTextureID);

      glDrawArrays(GL_TRIANGLES, 0, object->verticesCount);
      glDepthFunc(GL_LESS);

      glDisableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, s_vbo[0]);
      glDisableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, s_vbo[1]);
      glDisableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, s_vbo[2]);
    }
  }

  if (animation) {

  for (std::shared_ptr<ParticleSystem> particleSystem : pSystems) {
    if (particleSystem->hasDiscGenerator)
    {
        particleSystem->DiscGenerator(particle, particleSystem->discGen.center,
          particleSystem->discGen.radius, particleSystem->discGen.normal);
    }

    if (particleSystem->hasDirectedGenerator)
    {
      particleSystem->DirectedGenerator(particle, particleSystem->dirGen.position,
        particleSystem->dirGen.direction);
    }

    if (particleSystem->hasPointGenerator) {
      particleSystem->PointGenerator(particle, particleSystem->pGen.position);
    }


    for (struct ParticleSystem::Rotator r : particleSystem->rotatorSet) {
      particleSystem->Rotator(r.position, deltaTime);
    }

    for (struct ParticleSystem::Attractor a : particleSystem->attractorSet) {
      particleSystem->Attractor(a.position, a.mass, deltaTime);
    }

    for (struct ParticleSystem::Repulsor r : particleSystem->repulsorSet) {
      particleSystem->Repulsor(r.position, r.mass, deltaTime);
    }

    for (struct ParticleSystem::Wind w : particleSystem->windSet) {
      particleSystem->Wind(w.magnitude, deltaTime);
    }

    if (particleSystem->hasGravity) {
      particleSystem->Gravity(deltaTime);
    }
  }

    for (std::shared_ptr<ParticleSystem> particleSystem : pSystems) {
       particleSystem->OnUpdate(deltaTime);
	     particleSystem->OnRender(scene.camera, scene);
    }

  }

  //////////////////////////////////////////////////////////////////////////////
  // Record frame time
  high_resolution_clock::time_point time = high_resolution_clock::now();
  g_frameRate = duration_cast<duration<float>>(time - g_frameTime).count();
  g_frameTime = time;
  g_framesPerSecond = 1.f / (g_delay + g_frameRate);
  printf("FPS: %6.2f\n", g_framesPerSecond);
}

void CollisionDetection()
{
  if (pSystems.size() > 1) {

    for (int i = 0; i < pSystems.size()-1; i++) {

      for (auto& p1 : pSystems[i]->m_ParticlePool) {
        for (auto& p2 : pSystems[i+1]->m_ParticlePool) {
          if (pSystems[i]->CheckCollision(p1, p2)) {

            glm::vec3 v1(0.0f);
            glm::vec3 v2(0.0f);

            v1 = ((p1.SizeBegin - p2.SizeBegin) * p1.Velocity
            / (p1.SizeBegin + p2.SizeBegin)) +
            ((2 * p2.SizeBegin) * p2.Velocity /
            (p1.SizeBegin + p2.SizeBegin));

            v2 = (-(p1.SizeBegin - p2.SizeBegin) * p2.Velocity
            / (p1.SizeBegin + p2.SizeBegin)) +
            ((2 * p2.SizeBegin) * p1.Velocity /
            (p1.SizeBegin + p2.SizeBegin));

            p1.Velocity = v1;
            p2.Velocity = v2;
          }
        }
      }
     }

     for (auto& p1 : pSystems[0]->m_ParticlePool) {
       for (auto& p2 : pSystems[pSystems.size()-1]->m_ParticlePool) {
         if (pSystems[0]->CheckCollision(p1, p2)) {

           glm::vec3 v1(0.0f);
           glm::vec3 v2(0.0f);

           v1 = ((p1.SizeBegin - p2.SizeBegin) * p1.Velocity
           / (p1.SizeBegin + p2.SizeBegin)) +
           ((2 * p2.SizeBegin) * p2.Velocity /
           (p1.SizeBegin + p2.SizeBegin));

           v2 = (-(p1.SizeBegin - p2.SizeBegin) * p2.Velocity
           / (p1.SizeBegin + p2.SizeBegin)) +
           ((2 * p2.SizeBegin) * p1.Velocity /
           (p1.SizeBegin + p2.SizeBegin));

           p1.Velocity = v1;
           p2.Velocity = v2;
         }
       }
     }
    }
}

void appLoop(GLFWwindow *window)
{
  const double maxFPS = 35.0;
  const double maxPeriod = 1.0 / maxFPS;
  double lastTime = 0.0;

  while (!glfwWindowShouldClose(window))
  {
    double time = glfwGetTime();
    double deltaTime = time - lastTime;

    if( deltaTime >= maxPeriod ) {
        lastTime = time;
        drawGLFW(window, glfwGetTime());
        CollisionDetection();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
  }

}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void error_callback(int error, const char *description)
{
  std::cout << error << ":" << description << std::endl;
}
/////////////////////////////////////////////////////////////////////////////
// THESE FUNCTIONS ARE FOR GLUT AND RAYTRACER

Scene
rayTracerParser(const std::string& _filename) {

  float theta = (float) M_PI / 4.0f;
  Material mat = Material("mat");
  Material shiny = Material("shiny");

  Scene scene = Scene();

  std::ifstream ifs;
  ifs.open(_filename);

  if (!ifs) {
        std::cout << "Unable to open file";
        exit(1);
  }

  std::string line;

  while(ifs) {

    getline(ifs, line);
    std::istringstream iss(line);

    std::string tag;
    iss >> tag;

    if (tag.compare("Camera:") == 0) {

      std::string view;
      glm::vec3 pos;
      float focal_length;

      iss >> view >> pos.x >> pos.y >> pos.z >> focal_length;

      Camera camera = Camera(view, pos, focal_length, theta, g_width, g_height);
      scene.addCamera(camera);

    } else if (tag.compare("Light:") == 0) {

      glm::vec3 pos;

      iss >> pos.x >> pos.y >> pos.z;

      std::shared_ptr<Light> ptr_light(new Light(pos));
      scene.addLight(ptr_light);

    } else if (tag.compare("Sphere:") == 0) {

      glm::vec3 pos;
      float r;
      glm::vec4 c;
      Material m = Material();

      iss >> pos.x >> pos.y >> pos.z >> r >> c[0] >> c[1] >> c[2] >> c[3];

      std::string materialType;
      iss >> materialType;

      if (materialType.compare("shiny") == 0) {
        m = shiny;
      } else {
        m = mat;
      }

      std::shared_ptr<Sphere> ptr_sphere(new Sphere(pos, r, c, m));
      scene.addObject(ptr_sphere);

    } else if (tag.compare("Plane:") == 0) {

      glm::vec3 pos;
      glm::vec3 n;
      glm::vec4 c;
      Material m = Material();

      iss >> pos.x >> pos.y >> pos.z >> n.x >> n.y >> n.z
      >> c[0] >> c[1] >> c[2] >> c[3];

      std::string materialType;
      iss >> materialType;

      if (materialType.compare("shiny") == 0) {
        m = shiny;
      } else {
        m = mat;
      }

      std::shared_ptr<Plane> ptr_plane(new Plane(pos, n, c, m));
      scene.addObject(ptr_plane);

    } else {}

  }

  ifs.close();

  return scene;
}

bool fileForRaytrace(const std::string& _filename) {

  std::string line;
  std::ifstream ifs;
  ifs.open(_filename);

  if(ifs) {
    getline(ifs, line);
    std::istringstream iss(line);

    std::string title;
    iss >> title;

    if (title.compare("RAYTRACER") == 0) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Draw function for single frame
void
draw() {
  using namespace std::chrono;

  //////////////////////////////////////////////////////////////////////////////
  // Clear
  for(int i = 0; i < g_width*g_height; ++i) {
    g_frame[i] = glm::vec4(0.f, 0.f, 0.f, 0.f);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Draw
  scene.rayTracer(g_frame, g_width, g_height);
  glDrawPixels(g_width, g_height, GL_RGBA, GL_FLOAT, g_frame.get());

  //////////////////////////////////////////////////////////////////////////////
  // Show
  glutSwapBuffers();

  //////////////////////////////////////////////////////////////////////////////
  // Record frame time
  high_resolution_clock::time_point time = high_resolution_clock::now();
  g_frameRate = duration_cast<duration<float>>(time - g_frameTime).count();
  g_frameTime = time;
  g_framesPerSecond = 1.f/(g_delay + g_frameRate);
  printf("FPS: %6.2f\n", g_framesPerSecond);
}

void
initializeGLUT(const std::string& _filename) {
  glClearColor(0.f, 0.f, 0.0f, 0.f);

  g_frame = std::make_unique<glm::vec4[]>(g_width*g_height);

  std::string line;
  std::ifstream ifs;
  ifs.open(_filename);

  if(ifs) {
    getline(ifs, line);
    std::istringstream iss(line);

    std::string title;
    iss >> title;

    if (title.compare("RAYTRACER") != 0) {
      std::cout << "This file is not for raytracing";
      exit(1);
    } else {
      scene = rayTracerParser(_filename);
    }
  }
}

void
keyPressed(GLubyte _key, GLint _x, GLint _y) {
  switch(_key) {
    // Escape key : quit application
    case 27:
      std::cout << "Destroying window: " << g_window << std::endl;
      glutDestroyWindow(g_window);
      g_window = 0;
      break;
    // Unhandled
    default:
      std::cout << "Unhandled key: " << (int)(_key) << std::endl;
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Callback function for keyboard presses of special keys
/// @param _key Key
/// @param _x X position of mouse
/// @param _y Y position of mouse
void
specialKeyPressed(GLint _key, GLint _x, GLint _y) {
  switch(_key) {
    // Arrow keys
    case GLUT_KEY_UP:
      scene.camera.position += scene.camera.direction() * 5;
      break;
    case GLUT_KEY_DOWN:
      scene.camera.position -= scene.camera.direction() * 5;
      break;
    // Unhandled
    default:
      std::cout << "Unhandled special key: " << _key << std::endl;
      break;
  }
}

void
normalKeyPressed(unsigned char key, int x, int y)
{
  switch(key) {
    case 27:
      std::cout << "Destroying window: " << g_window << std::endl;
      glutDestroyWindow(g_window);
      g_window = 0;
      break;
    case 'w':
    scene.camera.position += scene.camera.up() * 5;
    break;
    case 'a':
    scene.camera.position -= scene.camera.right() * 5;
    break;
    case 's':
    scene.camera.position -= scene.camera.up() * 5;
    break;
    case 'd':
    scene.camera.position += scene.camera.right() * 5;
    break;
    default:
      break;
  }
}

void
resize(GLint _w, GLint _h) {
  g_width = _w;
  g_height = _h;

  // Viewport
  glViewport(0, 0, g_width, g_height);
}

void
timer(int _v) {
  if(g_window != 0) {
    glutPostRedisplay();

    g_delay = std::max(0.f, 1.f/FPS - g_frameRate);
    glutTimerFunc((unsigned int)(1000.f*g_delay), timer, 0);
  }
  else
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////
// Main

////////////////////////////////////////////////////////////////////////////////
/// @brief main
/// @param _argc Count of command line arguments
/// @param _argv Command line arguments
/// @return Application success status
int main(int _argc, char **_argv)
{
  bool rayTraceFile = fileForRaytrace(_argv[1]);

  if (rayTraceFile) {
    //////////////////////////////////////////////////////////////////////////////
    // Initialize GLUT Window
    std::cout << "Initializing GLUT Window\n" << std::endl;
    // GLUT
    glutInit(&_argc, _argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(g_width, g_height); // HD size
    g_window = glutCreateWindow("Spiderling: A Rudamentary Game Engine");

    // GL
    initializeGLUT(_argv[1]);

    //////////////////////////////////////////////////////////////////////////////
    // Assign callback functions
    std::cout << "Assigning Callback functions" << std::endl;
    glutReshapeFunc(resize);
    glutDisplayFunc(draw);
    glutKeyboardFunc(keyPressed);
    glutKeyboardFunc(normalKeyPressed);
    glutSpecialFunc(specialKeyPressed);
    glutTimerFunc(1000/FPS, timer, 0);

    // Start application
    std::cout << "Starting Application" << std::endl;
    glutMainLoop();

    return 0;

  }

  else {

    glfwSetErrorCallback(error_callback);
    std::cout << "Initializing GLFW\n" << std::endl;
    if (!glfwInit())
    {
      std::cout << "GLFW Initialization Failed" << std::endl;
      exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(g_width, g_height, "Spiderling", NULL, NULL);
    if (!window)
    {
      std::cout << "GLFWWindow Initialization Failed" << std::endl;
      glfwTerminate();
      return -1;
    }

    int actualScreenWidth, actualScreenHeight;
    glfwGetFramebufferSize(window, &actualScreenWidth, &actualScreenHeight);
    glfwMakeContextCurrent(window);

    glViewport(0, 0, actualScreenWidth, actualScreenHeight);

#if defined(OSX)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
      exit(EXIT_FAILURE);
    }
#endif
    glfwSwapInterval(1);

    initializeGLFW(window, _argv[1]);

    //////////////////////////////////////////////////////////////////////////////
    // Assign callback functions
    std::cout << "Assigning Callback functions" << std::endl;
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, resizeGLFW);

    // application loop, should be own method
    // need to add frame limiting stuff like in slides
    appLoop(window);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);

    return 0;
  }
}

#if defined(OSX)
#pragma clang diagnostic pop
#endif
