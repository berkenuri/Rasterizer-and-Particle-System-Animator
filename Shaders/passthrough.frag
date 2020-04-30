////////////////////////////////////////////////////////////////////////////////
// Simple pass through vertex shader, i.e., do not alter anything about the
// position, simply set the required gl data.
////////////////////////////////////////////////////////////////////////////////

#version 330

in vec4 fog_Position;
in vec3 N;
in vec3 V;
in vec3 P;
in vec2 tc; // interpolated incoming texture coordinate

out vec4 fcolor; // Output fragment color

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
  float outerCutOffAngle;
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

struct Object {
  bool isLocalLightSource;
};

struct Scene {
  bool fog;
};

uniform Scene scene;
uniform Object object;
uniform Material material;

uniform float pointAc;
uniform float pointAl;
uniform float pointAq;

uniform float spotAc;
uniform float spotAl;
uniform float spotAq;

uniform float dirCount;
uniform float pointCount;
uniform float spotCount;

const int size = 10;

uniform DirectionalLight dirLights[size];
uniform PointLight pointLights[size];
uniform SpotLight spotLights[size];

uniform vec4 obj_color;
uniform vec4 lightSource_color;
uniform vec4 ambientIntensity;
uniform vec3 cameraPosition;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 tc);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 tc);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 tc);

void main() {

  if (object.isLocalLightSource) {

    fcolor = lightSource_color;

  } else {

  // compute derivations of the world position
  vec3 p_dx = dFdx(P);
  vec3 p_dy = dFdy(P);
  // compute derivations of the texture coordinate
  vec2 tc_dx = dFdx(tc);
  vec2 tc_dy = dFdy(tc);
  // compute initial tangent and bi-tangent
  vec3 t = normalize( tc_dy.y * p_dx - tc_dx.y * p_dy );
  vec3 b = normalize( tc_dy.x * p_dx - tc_dx.x * p_dy ); // sign inversion
  // get new tangent from a given mesh normal
  vec3 n = N;
  vec3 x = cross(n, t);
  t = cross(x, n);
  t = normalize(t);
  // get updated bi-tangent
  x = cross(b, n);
  b = cross(n, x);
  b = normalize(b);
  mat3 TBN = mat3(t, b, n);


  vec2 textCoord;
  if (material.hasDepth) {

    vec3 viewDir = normalize(V * transpose(TBN));
    textCoord = ParallaxMapping(tc, viewDir);

    //if(textCoord.x > 1.0 || textCoord.y > 1.0 || textCoord.x < 0.0 || textCoord.y < 0.0)
      //discard;

  } else {
    textCoord = tc;
  }

  // compute derivations of the texture coordinate
  tc_dx = dFdx(textCoord);
  tc_dy = dFdy(textCoord);
  // compute initial tangent and bi-tangent
  t = normalize( tc_dy.y * p_dx - tc_dx.y * p_dy );
  b = normalize( tc_dy.x * p_dx - tc_dx.x * p_dy ); // sign inversion
  // get new tangent from a given mesh normal
  n = N;
  x = cross(n, t);
  t = cross(x, n);
  t = normalize(t);
  // get updated bi-tangent
  x = cross(b, n);
  b = cross(n, x);
  b = normalize(b);
  //TBN = mat3(t, b, n);

    vec3 norm;
    if (material.hasBump) {

    norm = vec3(texture(material.bump, textCoord).rgb);
    norm = normalize(norm * 2.0 - 1.0);
    norm = normalize(TBN * norm);

    } else {
      norm = normalize(N);
    }

    vec3 result = vec3(0.0f, 0.0f, 0.0f);

    // phase 1: directional lighting
    for (int i = 0; i < dirCount; i++) {
      result += CalcDirLight(dirLights[i], norm, V, textCoord);
    }

    // phase 2: point lights
    for(int i = 0; i < pointCount; i++) {
        result += CalcPointLight(pointLights[i], norm, P, V, textCoord);
    }

    // phase 3: spot light
    for (int i = 0; i < spotCount; i++) {
      result += CalcSpotLight(spotLights[i], norm, P, V, textCoord);
    }

    vec3 emission = vec3(0.0f);
    if (material.hasEmission) {
      emission = vec3(texture(material.emission, textCoord));
    }

    if (scene.fog) {
      float d = length(cameraPosition - P);
      float fs = 0.0f;
      float fe = 100.0f;

      float f = (fe - d) / (fe - fs);
      vec4 cf = vec4(.1f, 0.0f, .2f, 1.0f);
      f = 1.0 - clamp(f, 0.0, 1.0);

      fcolor = mix(vec4(result + emission, 1.0), cf, f);

    } else {
      fcolor = vec4(result + emission, 1.0);
    }

  }
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    /*
    float height =  (texture(material.depth, texCoords).r * .1f) - .00f;
    return texCoords - (viewDir.xy * height);
    */

    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 p = (viewDir.xy / viewDir.z) * .1f;
    vec2 deltaTexCoords = p / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(material.depth, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(material.depth, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(material.depth, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec2 tc)
{
    vec3 lightDir = normalize(light.position - P);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // combine results
    vec3 ambient;
    vec3 diffuse;

    if (material.hasDiffuse) {
      ambient = vec3(ambientIntensity) * vec3(texture(material.diffuse, tc))
      * vec3(light.color);
      diffuse = vec3(light.diffuseIntensity) * diff * vec3(texture(material.diffuse, tc))
      * vec3(light.color);
    } else {
      ambient = vec3(ambientIntensity) * vec3(light.color);
      diffuse = vec3(light.diffuseIntensity) * diff * vec3(light.color);
    }

    vec3 specular;
    if (material.hasSpecular) {
      specular = vec3(light.specularIntensity) * spec * vec3(texture(material.specular, tc))
      * vec3(light.color);
    } else {
      specular = vec3(light.specularIntensity) * spec
      * vec3(light.color);
    }

    vec3 emission = vec3(0.0f);
    if (material.hasEmission) {
      emission = vec3(texture(material.emission, tc));
    }

    vec3 result = (ambient + diffuse + specular);

    return pow(result, vec3(2.2));
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 tc)
{
    vec3 lightDir = normalize(light.position - P);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(V, reflectDir), 0.0), material.shininess);

    // attenuation
    float distance = length(light.position - P);
    float attenuation = 1.0 / (pointAc + (pointAl * distance) + (pointAq * distance * distance));

    // combine results
    vec3 ambient;
    vec3 diffuse;

    if (material.hasDiffuse) {
      ambient = vec3(ambientIntensity) * vec3(texture(material.diffuse, tc))
      * vec3(light.color);
      diffuse = vec3(light.diffuseIntensity) * diff * vec3(texture(material.diffuse, tc))
      * vec3(light.color);
    } else {
      ambient = vec3(ambientIntensity) * vec3(light.color);
      diffuse = vec3(light.diffuseIntensity) * diff * vec3(light.color);
    }

    vec3 specular;
    if (material.hasSpecular) {
      specular = vec3(light.specularIntensity) * spec * vec3(texture(material.specular, tc))
      * vec3(light.color);
    } else {
      specular = vec3(light.specularIntensity) * spec
      * vec3(light.color);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = (ambient + diffuse + specular);

    return pow(result, vec3(1.2));
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 tc)
{
    vec3 lightDir = normalize(light.position - P);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(V, reflectDir), 0.0), material.shininess);

    // attenuation
    float distance = length(light.position - P);
    float attenuation = 1.0 / (spotAc + (spotAl * distance) + (spotAq * distance * distance));

    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOffAngle - light.outerCutOffAngle;
    float intensity = clamp((theta - light.outerCutOffAngle) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient;
    vec3 diffuse;

    if (material.hasDiffuse) {
      ambient = vec3(ambientIntensity) * vec3(texture(material.diffuse, tc))
      * vec3(light.color);
      diffuse = vec3(light.diffuseIntensity) * diff * vec3(texture(material.diffuse, tc))
      * vec3(light.color);
    } else {
      ambient = vec3(ambientIntensity) * vec3(light.color);
      diffuse = vec3(light.diffuseIntensity) * diff * vec3(light.color);
    }

    vec3 specular;
    if (material.hasSpecular) {
      specular = vec3(light.specularIntensity) * spec * vec3(texture(material.specular, tc))
      * vec3(light.color);
    } else {
      specular = vec3(light.specularIntensity) * spec
      * vec3(light.color);
    }

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    vec3 result = (ambient + diffuse + specular);

    return pow(result, vec3(0.8));
}
