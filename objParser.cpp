#include "objParser.h"

// STL
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// GLM
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

////////////////////////////////////////////////////////////////////////////////
/// @brief Parse an obj file into a mesh
/// @param _filename Filename
/// @return Loaded mesh
mesh objParser(const std::string& _filename) {

  std::ifstream ifs(_filename);
  if(!ifs) {
    return mesh();
  }
  std::cout << "Parsing: " << _filename << std::endl;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> textures;
  std::vector<glm::vec3> normals;
  std::vector<vertex> vertices;

  std::string mtlFilename;

  std::string line;
  while(ifs) {
    getline(ifs, line);

    std::istringstream iss(line);
    std::string tag;
    iss >> tag;

    if(tag.compare("v") == 0) {
      glm::vec3 p;
      iss >> p.x >> p.y >> p.z;
      positions.emplace_back(p);
    }
    else if(tag.compare("vt") == 0) {
      glm::vec2 t;
      iss >> t.x >> t.y;
      textures.emplace_back(t);
    }
    else if(tag.compare("vn") == 0) {
      glm::vec3 n;
      iss >> n.x >> n.y >> n.z;
      normals.emplace_back(n);
    }
    else if(tag.compare("f") == 0) {
      for(size_t i = 0; i < 3; ++i) {
        std::string vert;
        iss >> vert;
        size_t p, t, n;
        sscanf(vert.c_str(), "%zu/%zu/%zu", &p, &t, &n);
        vertices.emplace_back(positions[p-1], normals[n-1], textures[t-1]);
      }
    }
    else if(tag.compare("mtllib") == 0) {
      iss >> mtlFilename;
    }
  }

  return mesh(vertices, mtlFilename);
}
