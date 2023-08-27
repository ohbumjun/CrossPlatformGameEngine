#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <Hazel.h>

struct ParticleProps
{
	glm::vec2 Position;
	glm::vec2 Velocity, VelocityVariation;
	glm::vec4 ColorBegin, ColorEnd;
	float SizeBegin, SizeEnd, SizeVariation;
	float LifeTime = 1.0f;
};

class ParticleSystem
{
public:
	ParticleSystem(uint32_t maxParticles = 1000);

	void OnUpdate(Hazel::Timestep ts);
	void OnRender(Hazel::OrthographicCamera& camera);

	void Emit(const ParticleProps& particleProps);
private:
	struct Particle
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec4 ColorBegin, ColorEnd;
		float Rotation			= 0.0f;
		float SizeBegin, SizeEnd;
		float PrevFrameSize;
		float LifeTime			= 1.0f;
		float LifeRemaining = 0.0f;
		bool Active = false;
	};
	std::vector<Particle> m_ParticlePool;
	uint32_t m_PoolIndex = 0;
};