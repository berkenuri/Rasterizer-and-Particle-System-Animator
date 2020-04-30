#ifndef __OBJPARSER_H__
#define __OBJPARSER_H__

// STL
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// GL
#include "GLInclude.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief One possible storage of vertex information.
////////////////////////////////////////////////////////////////////////////////
struct vertex {

  glm::vec3 m_p; ///< Position
  glm::vec3 m_n; ///< Normal
  glm::vec2 m_t; ///< Texture

  vertex(const glm::vec3& _p, const glm::vec3& _n, const glm::vec2& _t) :
    m_p(_p), m_n(_n), m_t(_t) {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief One possible mesh data structure
///
/// Vertices will be ordered such that every three form a triangle, e.g.,
/// vertices at indices 0, 1, 2 form a triangle, and then vertices at indices
/// 3, 4, 5 form a triangle, etc.
////////////////////////////////////////////////////////////////////////////////
struct mesh {
  std::vector<vertex> m_vertices;
  std::string mtlFile;

  mesh(const std::vector<vertex>& _vertices = std::vector<vertex>(),
  const std::string& _filename = "") :
    m_vertices(_vertices), mtlFile(_filename) {}
};

mesh objParser(const std::string& _filename);


#if   defined(OSX)
#pragma clang diagnostic pop
#endif

#endif
