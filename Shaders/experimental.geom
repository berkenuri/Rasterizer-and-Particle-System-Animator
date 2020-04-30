#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 texture[];
in vec3 p[];
in vec4 fog_position[];
in vec3 n[];
in vec3 v[];

out vec2 tc;
out vec3 P;
out vec4 fog_Position;
out vec3 N;
out vec3 V;

struct Scene {
  bool fog;
  bool dissection;
};

uniform float time;
uniform Scene scene;


vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position.xyz) - vec3(gl_in[1].gl_Position.xyz);
    vec3 b = vec3(gl_in[2].gl_Position.xyz) - vec3(gl_in[1].gl_Position.xyz);

    return normalize(cross(a, b));
}

void main() {
    vec3 normal = GetNormal();

    P = p[0];
    N = n[0];
    V = v[0];
    fog_Position = fog_position[0];

    if (scene.dissection) {
      gl_Position = explode(gl_in[0].gl_Position, normal);
    } else {
      gl_Position = gl_in[0].gl_Position;
    }

    tc = texture[0];
    EmitVertex();

    P = p[1];
    N = n[1];
    V = v[1];
    fog_Position = fog_position[1];

    if (scene.dissection) {
      gl_Position = explode(gl_in[1].gl_Position, normal);
    } else {
      gl_Position = gl_in[1].gl_Position;
    }

    tc = texture[1];
    EmitVertex();

    P = p[2];
    N = n[2];
    V = v[2];
    fog_Position = fog_position[2];

    if (scene.dissection) {
      gl_Position = explode(gl_in[2].gl_Position, normal);
    } else {
      gl_Position = gl_in[2].gl_Position;
    }

    tc = texture[2];
    EmitVertex();

    EndPrimitive();
}
