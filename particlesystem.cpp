#ifndef __PARTICLESYSTEM_CPP__
#define __PARTICLESYSTEM_CPP__

#include "particlesystem.h"
#include "random.h"
#include "GLInclude.h"
#include "CompileShaders.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

ParticleSystem::ParticleSystem()
{
	m_ParticlePool.resize(1000);
}

bool ParticleSystem::CheckCollision(Particle& one, Particle& two) // AABB - AABB collision
{
    // Collision x-axis?
    bool collisionX = one.Position.x + one.SizeBegin >= two.Position.x &&
        two.Position.x + two.SizeBegin >= one.Position.x;
    // Collision y-axis?
    bool collisionY = one.Position.y + one.SizeBegin >= two.Position.y &&
        two.Position.y + two.SizeBegin >= one.Position.y;
		// Collision z-axis?
		bool collisionZ = one.Position.z + one.SizeBegin >= two.Position.z &&
		    two.Position.z + two.SizeBegin >= one.Position.z;

    return collisionX && collisionY && collisionZ;
}

void ParticleSystem::Rotator(glm::vec3 position, float deltaTime) {

	for (auto& particle : m_ParticlePool)
	{
		glm::vec3 r = (particle.Position - position);
		glm::vec3 direction = cross(r, glm::vec3(0.0,0.0,1.0));
		glm::vec3 force = (-9.81) * particle.SizeBegin * direction
		/ glm::length(r);
		glm::vec3 acceleration = force / particle.SizeBegin;

		particle.Velocity += acceleration * deltaTime;
		particle.Rotation += 5.0f * deltaTime;
	}

}

void ParticleSystem::Gravity(float deltaTime) {

	for (auto& particle : m_ParticlePool)
	{
		glm::vec3 force = glm::vec3(0.0f, -1.0f, 0.0f) * (9.81) * particle.SizeBegin;

		glm::vec3 acceleration = force / particle.SizeBegin;

		particle.Velocity += acceleration * deltaTime;
	}

}

void ParticleSystem::Wind(glm::vec3 magnitude, float deltaTime) {

	for (auto& particle : m_ParticlePool)
	{
		glm::vec3 force = magnitude;

		glm::vec3 acceleration = force / particle.SizeBegin;

		particle.Velocity += acceleration * deltaTime;
	}

}

void ParticleSystem::Repulsor(glm::vec3 position, float mass, float deltaTime) {

		for (auto& particle : m_ParticlePool)
		{
			glm::vec3 force = (9.81) * mass * particle.SizeBegin * (particle.Position - position)
			/ pow(glm::length(position - particle.Position), 2);

			glm::vec3 acceleration = force / particle.SizeBegin;

			particle.Velocity += acceleration * deltaTime;

			if(glm::length(position - particle.Position) < 3)
				particle.Velocity *= 2;
		}
}

void ParticleSystem::Attractor(glm::vec3 position, float mass, float deltaTime) {

		for (auto& particle : m_ParticlePool)
		{
			glm::vec3 force = (-9.81) * mass * particle.SizeBegin * (particle.Position - position)
			/ pow(glm::length(position - particle.Position), 2);

			glm::vec3 acceleration = force / particle.SizeBegin;

			particle.Velocity += acceleration * deltaTime;

			if(glm::length(position - particle.Position) < 3)
				particle.Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		}
}

void ParticleSystem::OnUpdate(float deltaTime)
{

	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
			continue;

		if (particle.LifeRemaining <= 0.0f)
		{
			particle.Active = false;
			continue;
		}

		particle.LifeRemaining -= deltaTime;
		particle.Position += particle.Velocity * deltaTime;
		particle.Rotation += 0.01f * deltaTime;
	}
}

void ParticleSystem::OnRender(Camera& camera, Scene& scene)
{
	if (!m_QuadVA)
	{
		float vertices[] = {
			 // -0.5f, -0.5f, -0.5f,
			 //  0.5f, -0.5f, 0.5f,
			 //  0.5f,  0.5f, 0.5f,
			 // -0.5f,  0.5f, -0.5f

			 -0.5f, -0.5f, 0.5f,
			 0.5f, -0.5f, 0.5f,
			 -0.5f, 0.5f, 0.5f,
			 0.5f, 0.5f, 0.5f,
			 -0.5f, 0.5f, -0.5f,
			 0.5f, 0.5f, -0.5f,
			 -0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f
		};

		glGenVertexArrays(1, &m_QuadVA);
		glBindVertexArray(m_QuadVA);

		GLuint quadVB[0];
		GLuint quadIB[0];

		glGenBuffers(1, quadVB);
		glBindBuffer(GL_ARRAY_BUFFER, quadVB[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

		uint32_t indices[] = {
			// 0, 1, 2, 2, 3, 0

			0, 1, 2,
			2, 1, 3,
			2, 3, 4,
			4, 3, 5,
			4, 5, 6,
			6, 5, 7,
			6, 7, 0,
			0, 7, 1,
			1, 7, 3,
			3, 7, 5,
			6, 0, 4,
			4, 0, 2
		};

		glGenBuffers(1, quadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB[0]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		animation_program = compileProgram("Shaders/animation.vert",
														 "Shaders/animation.frag");
	}

	//glUseProgram(m_ParticleShader->GetRendererID());
	glUseProgram(animation_program);
	//glUniformMatrix4fv(m_ParticleShaderViewProj, 1, GL_FALSE, glm::value_ptr(camera.GetViewProjectionMatrix()));

	glm::mat4 proj(camera.projectionMatrix*scene.viewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(animation_program, "u_ViewProj"),
	1, GL_FALSE, glm::value_ptr(proj));

	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
			continue;

		// Fade away particles
		float life = particle.LifeRemaining / particle.LifeTime;
		glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
		color.a = color.a * life;

		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), particle.Position )
			* glm::rotate(glm::mat4(1.0f), particle.Rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, size });

		glUniformMatrix4fv(glGetUniformLocation(animation_program, "u_Transform"),
		1, GL_FALSE, glm::value_ptr(transform));
		glUniform4fv(glGetUniformLocation(animation_program, "u_Color"),
		1, glm::value_ptr(color));
		glBindVertexArray(m_QuadVA);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	}
}

void ParticleSystem::PointGenerator(const State& particleProps, glm::vec3 position)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.Active = true;
	particle.Position = position;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	glm::vec3 randomVector(Random::Float() - 0.5f, Random::Float() - 0.5f, Random::Float() - 0.5f);
	particle.Velocity = (particleProps.Velocity * randomVector);

	// Color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}

void ParticleSystem::DirectedGenerator(const State& particleProps, glm::vec3 position, glm::vec3 direction)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.Active = true;
	particle.Position = position;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	glm::vec3 randomVector(Random::Float() - 0.5f, Random::Float() - 0.5f, Random::Float() - 0.5f);
	particle.Velocity = (particleProps.Velocity * (randomVector + direction));

	// Color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}

void ParticleSystem::DiscGenerator(const State& particleProps, glm::vec3 center, float radius, glm::vec3 normal)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.Active = true;

	glm::vec3 randomVector((Random::Float() - 0.5f) * radius,
	(Random::Float() - 0.5f) * radius, (Random::Float() - 0.5f) * radius);

	particle.Position = center + randomVector;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.Velocity = normal;

	// Color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}

#endif
