#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#include "GLInclude.h"
#include "camera.h"
#include <vector>
#include "scene.h"

struct State
{
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec4 ColorBegin, ColorEnd;
	float SizeBegin, SizeEnd, SizeVariation;
	float LifeTime = 1.0f;
};

class ParticleSystem
{
public:
	ParticleSystem();

	struct Particle
	{
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec4 ColorBegin, ColorEnd;
		float Rotation = 0.0f;
		float SizeBegin, SizeEnd;

		float LifeTime = 1.0f;
		float LifeRemaining = 0.0f;

		bool Active = false;
	};

	struct Particle particle;

	bool CheckCollision(Particle& objectOne, Particle& objectTwo);

	bool hasPointGenerator = false;
	bool hasDirectedGenerator = false;
	bool hasDiscGenerator = false;
	bool hasGravity = false;

	void OnUpdate(float ts);
	void OnRender(Camera& camera, Scene& scene);

	void PointGenerator(const State& particleProperties, glm::vec3 position);
	void DirectedGenerator(const State& particleProperties, glm::vec3 position, glm::vec3 direction);
	void DiscGenerator(const State& particleProperties, glm::vec3 center, float radius, glm::vec3 normal);

	void Attractor(glm::vec3 position, float mass, float deltaTime);
	void Repulsor(glm::vec3 position, float mass, float deltaTime);
	void Wind(glm::vec3 magnitude, float deltaTime);
	void Rotator(glm::vec3 position, float deltaTime);
	void Gravity(float deltaTime);

	struct PointGenerator
	{
		State particleProperties;
		glm::vec3 position;
	};

	struct DirectedGenerator
	{
		State particleProperties;
		glm::vec3 position;
		glm::vec3 direction;
	};

	struct DiscGenerator
	{
		State particleProperties;
		glm::vec3 center;
		float radius;
		glm::vec3 normal;
	};

	struct Attractor
	{
		glm::vec3 position;
		float mass;
	};

	struct Repulsor
	{
		glm::vec3 position;
		float mass;
	};

	struct Wind
	{
		glm::vec3 magnitude;
	};

	struct Rotator
	{
		glm::vec3 position;
	};

	struct DiscGenerator discGen;
	struct DirectedGenerator dirGen;
	struct PointGenerator pGen;

	std::vector<Particle> m_ParticlePool;
	std::vector<struct Attractor> attractorSet;
	std::vector<struct Repulsor> repulsorSet;
	std::vector<struct Wind> windSet;
	std::vector<struct Rotator> rotatorSet;
	uint32_t m_PoolIndex = 999;

	GLuint m_QuadVA{0};
  GLuint animation_program{0};
	GLint m_ParticleShaderViewProj, m_ParticleShaderTransform, m_ParticleShaderColor;
};

#endif
