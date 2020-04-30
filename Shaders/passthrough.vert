////////////////////////////////////////////////////////////////////////////////
// Simple pass through vertex shader, i.e., do not alter anything about the
// position, simply set the required gl data. Additionally, pass along the color
// data about the vertex.
////////////////////////////////////////////////////////////////////////////////

#version 330

layout (location = 0)   in vec3   vpos; // Input vertex position from data
layout (location = 1)   in vec3   vnor; // Input vertex normal from data
layout (location = 2)   in vec2   vtext;

//                      in vec4   vcolor; // Input vertex color from data

out vec2 tc; // texture coordinate output to rasterizer for interpolation
out vec3 P;
out vec4 fog_Position;

out vec3 N;
//out vec3 L;
//out vec3 R;
out vec3 V;

struct GlobalAmbient {
  vec4 ambientIntensity;
};

struct DirectionalLight {
  vec3 position;
  vec4 diffuseIntensity;
  vec4 specularIntensity;
  vec4 color;
};

struct PointLight {
  vec3 position;
  vec4 diffuseIntensity;
  vec4 specularIntensity;
  vec4 color;
};

struct SpotLight {
  vec3 position;
  vec3 direction;
  float cutOffAngle;
  vec4 diffuseIntensity;
  vec4 specularIntensity;
  vec4 color;
};

struct Material {
  vec4 ambient;
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  sampler2D bump;
  sampler2D depth;
  sampler2D displacement;
  float shininess;
  bool hasDiffuse;
  bool hasSpecular;
  bool hasEmission;
  bool hasBump;
  bool hasDepth;
  bool hasDisplacement;
};

uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform vec4 obj_color;

void main() {

  P = vec3(mv_matrix * vec4(vpos, 1.0));
  // direction to camera is equivalent to the negative of view space vertex position
  V = -normalize(P.xyz);
  N = normalize(mat3(norm_matrix) * vnor);
  tc = vtext;

  fog_Position = mv_matrix * vec4(vpos, 1.0);
  gl_Position = proj_matrix * mv_matrix * vec4(vpos, 1.0);
/*
  if (material.hasDisplacement) {
    vec3 newVertexPos;
    vec3 dv;
    float df;

    dv = texture(material.displacement, tc).xyz;
    df = 0.3f*dv.x + 0.59f*dv.y + 0.11f*dv.z;

    newVertexPos = vec3(vnor * df * 1.0) + vpos;
    gl_Position = proj_matrix * mv_matrix * vec4(newVertexPos, 1.0);

  } else {

    fog_Position = mv_matrix * vec4(vpos, 1.0);
    gl_Position = proj_matrix * mv_matrix * vec4(vpos, 1.0);
  }
*/

}
