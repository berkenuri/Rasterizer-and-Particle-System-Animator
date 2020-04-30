#include "CompileShaders.h"

// STL
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

// GL
#define GL_GLEXT_PROTOTYPES
#if   defined(OSX)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <GLUT/glut.h>
#elif defined(LINUX)
#include <GL/glut.h>
#endif

string
parseShader(const string& _shader) {
  ifstream ifs(_shader);
  ostringstream oss;
  oss << ifs.rdbuf();
  return oss.str();
}

GLuint
compileProgram(const string& _vertexShader,
               const string& _fragmentShader,
               const string& _geometryShader) {
  int success;
  char infoLog[512];

  // Compile the vertex shader
  string vertexShaderFromFile = parseShader(_vertexShader);
  const char* prog = vertexShaderFromFile.c_str();

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &prog, NULL);
  glCompileShader(vertexShader);

  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cerr << "Error compiling vertex shader '" << _vertexShader
      << "'\n" << infoLog << endl;
    exit(1);
  }

  // Compile the geometry shader
  string geometryShaderFromFile = parseShader(_geometryShader);
  prog = geometryShaderFromFile.c_str();

  GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
  glShaderSource(geometryShader, 1, &prog, NULL);
  glCompileShader(geometryShader);

  glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
    cerr << "Error compiling geometry shader '" << _geometryShader
      << "'\n" << infoLog << endl;
    exit(1);
  }

  // Compile the fragment shader
  string fragmentShaderFromFile = parseShader(_fragmentShader);
  prog = fragmentShaderFromFile.c_str();

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &prog, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cerr << "Error compiling fragment shader '" << _fragmentShader
      << "'\n" << infoLog << endl;
    exit(1);
  }


  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, geometryShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    cerr << "Error linking programs '" << _vertexShader
      << "' and '" << _fragmentShader << "' and '" << _geometryShader
      << "'\n" << infoLog << endl;
    exit(1);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glDeleteShader(geometryShader);

  // Normally return, and specify this when needed
  glUseProgram(shaderProgram);

  return shaderProgram;
}

GLuint
compileProgram(const string& _vertexShader,
               const string& _fragmentShader) {
  int success;
  char infoLog[512];

  // Compile the vertex shader
  string vertexShaderFromFile = parseShader(_vertexShader);
  const char* prog = vertexShaderFromFile.c_str();

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &prog, NULL);
  glCompileShader(vertexShader);

  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cerr << "Error compiling vertex shader '" << _vertexShader
      << "'\n" << infoLog << endl;
    exit(1);
  }

  // Compile the fragment shader
  string fragmentShaderFromFile = parseShader(_fragmentShader);
  prog = fragmentShaderFromFile.c_str();

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &prog, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cerr << "Error compiling fragment shader '" << _fragmentShader
      << "'\n" << infoLog << endl;
    exit(1);
  }

  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    cerr << "Error linking programs '" << _vertexShader
      << "' and '" << _fragmentShader << "'\n" << infoLog << endl;
    exit(1);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // Normally return, and specify this when needed
  glUseProgram(shaderProgram);

  return shaderProgram;
}


#if   defined(OSX)
#pragma clang diagnostic pop
#endif
